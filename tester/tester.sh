#! /bin/bash

SERVER_IP="127.0.0.1"
SERVER_PORT="8080"

TEST_MESSAGE_DIR="sample_messages"

LONG_TEXT=$(cat $TEST_MESSAGE_DIR/hhgttg.txt)

echo "---Test simple GET---"
curl http://$SERVER_IP:$SERVER_PORT/success.html
echo ""

echo "---Test GET that does not exist---"
curl http://$SERVER_IP:$SERVER_PORT/doesnotexist
echo ""

echo "---Test GET from port with no path to resource---"
curl http://127.0.0.1:8000/success.html
echo ""

echo "---Test simple POST---"
curl -d "message=Hello, World!&name=mom" http://$SERVER_IP:$SERVER_PORT/success.html
echo ""

echo "---Test POST to server with no permissions---"
curl -d "message=Hello, World!&name=mom" http://127.42.42.42:8080/success.html
echo ""

echo "---Test simple POST with false line length---"
curl -H "Content-Length: 100" -d "message=Hello, World!&name=mom" http://$SERVER_IP:$SERVER_PORT/success.html
echo ""

echo "---Test chunked transfer---"
curl -H "Transfer-Encoding: chunked" -d @$TEST_MESSAGE_DIR/hhgttg_short.txt http://$SERVER_IP:$SERVER_PORT/success.html
echo ""

echo "---Test chunked transfer empty---"
curl -H "Transfer-Encoding: chunked" -d "" http://$SERVER_IP:$SERVER_PORT/success.html
echo ""

echo "---Test POST 1 upload---"
curl -F "files=@$TEST_MESSAGE_DIR/hhgttg_short.txt" http://$SERVER_IP:$SERVER_PORT/uploads
echo ""

echo "---Test POST 2 upload---"
curl -F "files=@$TEST_MESSAGE_DIR/hhgttg_short.txt,$TEST_MESSAGE_DIR/todo.txt" http://$SERVER_IP:$SERVER_PORT/uploads
echo ""

echo "---Test PUT upload---"
curl -T "tester.sh" http://$SERVER_IP:$SERVER_PORT/uploads
echo ""

echo "---Test DELETE---"
curl -X "DELETE" http://$SERVER_IP:$SERVER_PORT/uploads/todo.txt
echo ""

echo "---Test DELETE nonexisting---"
curl -X "DELETE" http://$SERVER_IP:$SERVER_PORT/uploads/doesnotexist
echo ""

echo "---Duplicate keys---"
curl -H "Key:value" -H "Key:value2" http://$SERVER_IP:$SERVER_PORT/success.html
echo ""

echo "---Key Value Together---" # NGINX allows
curl -H "Key:value" http://$SERVER_IP:$SERVER_PORT/success.html
echo ""

echo "---Key Value Spaces---" # NGINX allows
curl -H "Key:        value" http://$SERVER_IP:$SERVER_PORT/success.html
echo ""

echo "---Test Space In Value---" # NGINX allows
curl -H "Key: value space" http://$SERVER_IP:$SERVER_PORT/success.html
echo ""

echo "---Test Space In Key1---" # NGINX does not allow
curl -H "Key space: value" http://$SERVER_IP:$SERVER_PORT/success.html
echo ""

echo "---Test Space In Key2---" # NGINX does not allow
curl -H " Key: value" http://$SERVER_IP:$SERVER_PORT/success.html
echo ""

echo "---Test Space In Key3---" # NGINX does not allow
curl -H "Key : value" http://$SERVER_IP:$SERVER_PORT/success.html
echo ""

echo "---Test Empty Header Value1---" # NGINX allows
curl -H "Key:" http://$SERVER_IP:$SERVER_PORT/success.html
echo ""

echo "---Test Empty Header Value2---" # NGINX allows
curl -H "Key: " http://$SERVER_IP:$SERVER_PORT/success.html
echo ""

echo "---Test Empty Header Value3---" # NGINX allows
curl -H "Key:  " http://$SERVER_IP:$SERVER_PORT/success.html
echo ""

echo "---Test Empty Header Key1---" # NGINX does not allow?
curl -H ": asdf" http://$SERVER_IP:$SERVER_PORT/success.html
echo ""

echo "---Test Empty Header Key2---" # NGINX does not allow?
curl -H " : asdf" http://$SERVER_IP:$SERVER_PORT/success.html
echo ""

