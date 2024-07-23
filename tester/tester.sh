#! /bin/bash

SERVER_IP="127.0.0.1"
SERVER_PORT="8080"

TEST_MESSAGE_DIR="sample_messages"

LONG_TEXT=$(cat $TEST_MESSAGE_DIR/hhgttg.txt)

echo "---Test simple GET---"
curl http://$SERVER_IP:$SERVER_PORT

echo "---Test simple POST---"
curl -d "message=Hello, World!&name=mom" http://$SERVER_IP:$SERVER_PORT

echo "---Test simple POST with false line length---"
curl -H "Content-Length: 100" -d "message=Hello, World!&name=mom" http://$SERVER_IP:$SERVER_PORT

echo "---Test chunked transfer---"
curl -H "Transfer-Encoding: chunked" -d @$TEST_MESSAGE_DIR/hhgttg_short.txt http://$SERVER_IP:$SERVER_PORT

echo "---Test POST 1 upload---"
curl -F "files=@$TEST_MESSAGE_DIR/hhgttg_short.txt" http://$SERVER_IP:$SERVER_PORT/uploads

echo "---Test POST 2 upload---"
curl -F "files=@$TEST_MESSAGE_DIR/hhgttg_short.txt,$TEST_MESSAGE_DIR/todo.txt" http://$SERVER_IP:$SERVER_PORT/uploads

echo "Test PUT upload"
curl -T "tester.sh" http://$SERVER_IP:$SERVER_PORT/uploads
