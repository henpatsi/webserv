#include "ServerManager.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

static void
setFdNonBlocking (int fd)
{
  int flags = fcntl (fd, F_GETFL);
  if (flags == -1)
    throw std::system_error ();
  flags |= O_NONBLOCK;
  flags |= O_CLOEXEC;
  int res = fcntl (fd, F_SETFL, flags);
  if (res == -1)
    throw std::system_error ();
}

ServerManager::ServerManager (const std::string path) : _path (path)
{
  // keeps track of the stringstreams for each server
  std::vector<std::stringstream> configs;
  // utils for locations to have { } and for error handling
  bool is_in_server = false;
  int depth = 0;
  // checks if file is openable
  std::ifstream config{ _path };
  if (!config.is_open ())
    throw FileIssueException ();

  // reads the file into a stringstream
  for (std::string line; std::getline (config, line);)
    {
      // ignores lines starting with #
      size_t firstIndex = line.find_first_not_of (" \f\n\r\t\v");
      if (firstIndex < line.length () && line[firstIndex] != '#')
        {
          // if we find {
          size_t open_index = line.find ('{');
          if (open_index != std::string::npos)
            {
              // check if we are creating a new server
              // should check if anything between server and {
              if (line.compare (0, 6, "server") == 0)
                {
                  // if (is_in_server)
                  //   throw ServerInServerException();
                  // start this servers config stringstream
                  configs.push_back (std::stringstream{ "" });
                  is_in_server = true;
                }
              depth += 1;
            }
          size_t close_index = line.find ('}');
          if (close_index != std::string::npos)
            {
              // if we are not in parenthesies, or we just got into them but
              // close before
              if (depth == 0)
                throw InvalidBraceException ();
              else
                depth--;
            }
          if (depth == 0)
            is_in_server = false;
          if (is_in_server) // remove tailing }
            configs.back () << line << "\n";
        }
    }
  config.close ();
  if (depth != 0)
    throw UnclosedBraceException ();

  // Initialises the Serverconfigs with the correct string
  bool success = true;
  for (std::stringstream &configuration : configs)
    {
      try
        {
          ServerConfig current (configuration);
          servers.push_back (new Server (current));
        }
      catch (std::exception const &e)
        {
          success = false;
          std::cerr << "ServerManager: ServerInitError: " << e.what () << "\n";
        }
      catch (...)
        {
          success = false;
          std::cerr << "really stupid" << "\n";
        }
    }
  if (!success)
    {
      std::cout << "failed to create\n";
      throw ServerCreationException ();
    }
  // setup epoll
  polled = epoll_create (EPOLL_SIZE);
  if (polled == -1)
    {
      std::cerr << "bullshit\n";
    }
  events = new epoll_event[EPOLL_SIZE * 2];
  trackedFds = 0;
  try
    {
      registerServerSockets ();
    }
  catch (std::exception &e)
    {
      std::cerr << "ServerManager: EpollInitError: " << e.what () << "\n";
    }
}

ServerManager::~ServerManager ()
{
  // deletes all the servers managed by it
  delete[] events;
}

std::string
ServerManager::GetPath () const
{
  return (_path);
}

const char *
ServerManager::FileIssueException::what () const noexcept
{
  return "Manager: FileError: Failed to open file";
}

const char *
ServerManager::ServerInServerException::what () const noexcept
{
  return "Manager: LogicError: Server inside Server";
}

const char *
ServerManager::InvalidBraceException::what () const noexcept
{
  return "Manager: ParsingError: invalid '}'";
}

const char *
ServerManager::CharOutsideServerBlockException::what () const noexcept
{
  return "Manager: ParsingError: Character outside server block";
}

const char *
ServerManager::ServerCreationException::what () const noexcept
{
  return "Manager: ParsingError: Failed to create Servers";
}

const char *
ServerManager::UnclosedBraceException::what () const noexcept
{
  return "Manager: ParsingError: Unclosed brace";
}

ServerManager::ManagerRuntimeException::ManagerRuntimeException (
    std::string error)
{
  this->error = error;
}

const char *
ServerManager::ManagerRuntimeException::what () const noexcept
{
  return error.c_str ();
}

void
ServerManager::runServers ()
{
  while (1)
    {
    //  std::cout << "waiting...\n";
      WaitForEvents ();
      for (int i = 0; i < eventAmount; i++)
        {
     //     std::cout << "\nEvent on fd " << events[i].data.fd << "\n";
          for (Server *server : servers)
            {
              // if someone initiates a connection to the registered sockets
              if (server->IsServerSocketFD (events[i].data.fd))
                {
                  try
                    {
                      int incommingFD = acceptConnection (events[i]);
                      setFdNonBlocking (incommingFD);
                      AddToEpoll (incommingFD);
                      server->connect (incommingFD, events[i].data.fd,
                                       client_addr);
                    }
                  catch (std::exception &e)
                    {
                      std::cerr
                          << "ServerManager: RunServerError: " << e.what ()
                          << "\n";
                    }
                }
              // if we can send them data and resolve the request
              else if (server->IsListeningFD (events[i].data.fd))
                {
                  if (events[i].events & EPOLLIN)
                    {
                      try
                        {
                          server->getRequest (events[i].data.fd);
                        }
                      catch (std::exception &e)
                        {
                          std::cerr
                              << "ServerManager: RunServerError: " << e.what ()
                              << "\n";
                        }
                    }
                  if (events[i].events & EPOLLOUT)
                    {
                      try
                        {
                          std::pair<int, int> response;
                          response = server->respond (events[i].data.fd);
                          if (response.second == 1)
                            {
                              DelFromEpoll (events[i].data.fd);
                              close (events[i].data.fd);
                            }
                          if (response.second > 2)
                            {
                              DelFromEpoll (events[i].data.fd);
                              temp_event.events = EPOLLIN;
                              temp_event.data.fd = response.second;
                              AddToEpoll (response.second);
                              info.push_back ((cgiInfo){
                                  response.second, response.first,
                                  cgiResponse{ response.second },
                                  events[i].data.fd, std::time (nullptr) });
                            }
                        }
                      catch (const std::exception &e)
                        {
                          std::cerr
                              << "ServerManager: RunServerError: " << e.what ()
                              << "\n";
                          // send error response here???
                        }
                    }
                }
            }
          std::vector<cgiInfo>::iterator it = std::find_if (
              info.begin (), info.end (),
              [&] (cgiInfo fdinfo) { return fdinfo.fd == events[i].data.fd; });
          if (it != info.end ())
            {
              if (it->response.isDone ())
                {
                  handleCgiResponse (it);
                  info.erase (it);
                }
              else
                {
                  it->response.readCgiResponse ();
                }
            }
        }
      try
        {
          checkTimeouts ();
        }
      catch (const std::exception &e)
        {
          std::cerr << e.what () << std::endl;
        }
    }
}

void
ServerManager::AddToEpoll (int fd)
{
  if (epoll_ctl (polled, EPOLL_CTL_ADD, fd, &temp_event) < 0)
    std::cout << "Error while polling fds\n";
  else
    std::cout << "Added fd " << fd << " to epoll\n";
  // keeps the tracked fds for epoll_wait
  trackedFds++;
}

void
ServerManager::DelFromEpoll (int fd)
{
  if (epoll_ctl (polled, EPOLL_CTL_DEL, fd, &temp_event) < 0)
    std::cout << "Error while removing fd\n";
  else
    std::cout << "Removed fd " << fd << " from epoll\n";
  // keeps the tracked fds for epoll_wait
  trackedFds--;
}

void
ServerManager::registerServerSockets ()
{
  // incomming connections
  temp_event.events = EPOLLIN | EPOLLET;
  for (Server *server : servers)
    {
      for (auto &socket : server->GetSocketFDs ())
        {
          std::cout << "Adding server socket\n";
          temp_event.data.fd = socket.first;
          AddToEpoll (socket.first);
          setFdNonBlocking (socket.first);
        }
    }
}

void
ServerManager::WaitForEvents ()
{
  eventAmount = epoll_wait (polled, events, 50,
                            1000); // Block for max 1 second to allow timeouts
  if (eventAmount == -1)
    {
      std::cerr << "ServerManager: WaitForEpollEvents: "; // should be changed
                                                          // to exception
      perror ("");
    }
}

void
ServerManager::handleCgiResponse (std::vector<cgiInfo>::iterator it)
{
  Route _;
  HttpResponse response (it->response, _);
  if (send (it->listeningFd, &response.getResponse()[0],
            response.getResponse().size (), 0)
      == -1)
    throw ManagerRuntimeException ("Failed to send response");
  DelFromEpoll (it->fd);
  if (close(it->fd) == -1 || close (it->listeningFd))
    throw ManagerRuntimeException ("Failed to close fd");
}

void
ServerManager::checkTimeouts ()
{
  for (Server *server : servers)
    {
      std::vector<int> timedOutFDs = server->checkTimeouts ();
      for (int fd : timedOutFDs)
        {
          DelFromEpoll (fd);
          close (fd);
        }
    }
  std::time_t currentTime = std::time (nullptr);
  for (auto it = info.begin (); it != info.end ();)
    {
      if (currentTime - it->cgiStarted > TIMEOUT_SEC)
        {
          std::cout << "Cgi timed out\n";
          it->response.setFailResponseCode (500);
          handleCgiResponse (it);
          kill (it->pid, SIGKILL);
          std::cout << "Killed " << it->pid << std::endl;
          it = info.erase (it);
        }
      else
        it++;
    }
}

int
ServerManager::acceptConnection (epoll_event event)
{
  int incommingFd = accept (event.data.fd, (sockaddr *)&client_addr,
                            (socklen_t *)&addressSize);
  if (incommingFd == -1)
    std::cerr << "ServerManager: AcceptException\n"; // should be exception
  setFdNonBlocking (incommingFd);
  temp_event.events = EPOLLIN | EPOLLOUT;
  temp_event.data.fd = incommingFd;
  return incommingFd;
}
