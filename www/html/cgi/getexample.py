import urllib.parse
import os

query_string = os.environ.get("QUERY_STRING")
form = urllib.parse.parse_qs(query_string)
first_name = form["first_name"]
last_name = form["last_name"]

print ("Content-type:text/html")
print("")
print ("<html>")
print ('<head>')
print ("<title>Hello - Second CGI Program</title>")
print ('</head>')
print ('<body>')
print ("<h2>Hello %s %s</h2>" % (first_name, last_name))
print ('</body>')
print ('</html>')
#print (query_string)
