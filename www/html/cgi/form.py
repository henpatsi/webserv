import cgi
import cgitb
cgitb.enable()
#cgi.test()
print("content-type: text/plain\r\n\r\n")
form = cgi.FieldStorage()
if "name" not in form or "addr" not in form:
    print("<H1>Error</H1>")
    print("Please fill in the name and addr fields.")
print("<p>name:", form["name"].value)
print("<p>addr:", form["addr"].value)
