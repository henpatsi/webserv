# webserv
## HTTP server written in C++

### Installing

```
git clone https://github.com/henpatsi/webserv.git
cd webserv
make
```

### Running

```
./webserv [config_name]
```

### Config setup

```
server {
 [server setting]: [value]

 location [route] {
   [route setting]: [value]
 }
}
```

#### Server settings

 - `host` - ip address
 - `port`
 - `name` - name of the server
 - `connection_timeout` - time (s) until a client times out, after this a 408 response is attempted and the client is disconnected
 - `session_timeout` - time (s) until a session times out, including this enables sessions through cookies
 - `size_limit` - maximum request body size
 - `error_page` - custom error page for all errors (400 and 500 codes), can contain <%ERROR%> which is replaced by the error code

#### Route settings

 - `root` - the root path for the route, relative to webserv
 - `allowedMethods` - request methods allowed by the route
 - `dirListing` - if true, GET requests to directories lists their content
 - `index` - if dirListing is false, GET requests to directories get this file in directory
 - `acceptUpload` - whether uploads to this route through POST requests are accepted or not
 - `uploadDir` - the directory to which uploads through POST to this route are stored
 - `redirect` - the location to which requests to this route should be redirected to (temporary redirect 307)
 - `CGI` - a space separated list of allowed cgi types, defined by extension
