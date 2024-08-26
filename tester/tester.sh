#! /bin/bash

SERVER_IP="127.0.0.1"
SERVER_PORT="8080"

TEST_MESSAGE_DIR="sample_messages"

LONG_TEXT=$(cat $TEST_MESSAGE_DIR/hhgttg.txt)

print_name() {
	echo ""
	echo "--------------------------------------------"
	echo $NAME
	echo "--------------------------------------------"
	echo ""
}

NAME="Test simple GET"
print_name
curl http://$SERVER_IP:$SERVER_PORT/success.html

NAME="Test GET that does not exist"
print_name
curl http://$SERVER_IP:$SERVER_PORT/doesnotexist

NAME="Test GET from port with no path to resource"
print_name
curl http://127.0.0.1:8000/success.html

NAME="Test simple POST"
print_name
curl -d "message=Hello, World!&name=mom" http://$SERVER_IP:$SERVER_PORT/success.html

NAME="Test POST to server with no permissions"
print_name
curl -d "message=Hello, World!&name=mom" http://127.42.42.42:8080/success.html

NAME="Test simple POST with false line length"
print_name
curl -H "Content-Length: 100" -d "message=Hello, World!&name=mom" http://$SERVER_IP:$SERVER_PORT/success.html

NAME="Test chunked transfer"
print_name
curl -H "Transfer-Encoding: chunked" -d @$TEST_MESSAGE_DIR/hhgttg_short.txt http://$SERVER_IP:$SERVER_PORT/success.html

NAME="Test chunked transfer empty"
print_name
curl -H "Transfer-Encoding: chunked" -d "" http://$SERVER_IP:$SERVER_PORT/success.html

NAME="Test POST 1 upload"
print_name
curl -F "files=@$TEST_MESSAGE_DIR/hhgttg_short.txt" http://$SERVER_IP:$SERVER_PORT/uploads

NAME="Test POST 2 upload"
print_name
curl -F "files=@$TEST_MESSAGE_DIR/hhgttg_short.txt,$TEST_MESSAGE_DIR/todo.txt" http://$SERVER_IP:$SERVER_PORT/uploads

NAME="Test PUT upload"
print_name
curl -T "tester.sh" http://$SERVER_IP:$SERVER_PORT/uploads

NAME="Test DELETE"
print_name
curl -X "DELETE" http://$SERVER_IP:$SERVER_PORT/uploads/todo.txt

NAME="Test DELETE nonexisting"
print_name
curl -X "DELETE" http://$SERVER_IP:$SERVER_PORT/uploads/doesnotexist

NAME="Duplicate keys"
print_name
curl -H "Key:value" -H "Key:value2" http://$SERVER_IP:$SERVER_PORT/success.html

NAME="Key Value Together" # NGINX allows
print_name
curl -H "Key:value" http://$SERVER_IP:$SERVER_PORT/success.html

NAME="Key Value Spaces" # NGINX allows
print_name
curl -H "Key:        value" http://$SERVER_IP:$SERVER_PORT/success.html

NAME="Test Space In Value" # NGINX allows
print_name
curl -H "Key: value space" http://$SERVER_IP:$SERVER_PORT/success.html

NAME="Test Space In Key1" # NGINX does not allow
print_name
curl -H "Key space: value" http://$SERVER_IP:$SERVER_PORT/success.html

NAME="Test Space In Key2" # NGINX does not allow
print_name
curl -H " Key: value" http://$SERVER_IP:$SERVER_PORT/success.html

NAME="Test Space In Key3" # NGINX does not allow
print_name
curl -H "Key : value" http://$SERVER_IP:$SERVER_PORT/success.html

NAME="Test Empty Header Value1" # NGINX allows
print_name
curl -H "Key:" http://$SERVER_IP:$SERVER_PORT/success.html

NAME="Test Empty Header Value2" # NGINX allows
print_name
curl -H "Key: " http://$SERVER_IP:$SERVER_PORT/success.html

NAME="Test Empty Header Value3" # NGINX allows
print_name
curl -H "Key:  " http://$SERVER_IP:$SERVER_PORT/success.html

NAME="Test Empty Header Key1" # NGINX allows
print_name
curl -H ": asdf" http://$SERVER_IP:$SERVER_PORT/success.html

NAME="Test Empty Header Key2" # NGINX does not allow
print_name
curl -H " : asdf" http://$SERVER_IP:$SERVER_PORT/success.html

