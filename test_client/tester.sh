#! /bin/bash

CLIENT="./client"

SERVER_IP="127.0.0.1"
SERVER_PORT="8080"

EXAMPLES_DIR="./example_messages/"

test_file() {
	echo "Testing $file"
	MESSAGE=$(cat $file)
	#echo "$MESSAGE"
	$CLIENT $SERVER_IP $SERVER_PORT "$MESSAGE"
	echo ""
}

file=$EXAMPLES_DIR"get_root"
test_file

file=$EXAMPLES_DIR"content_test"
test_file

file=$EXAMPLES_DIR"mozilla_query"
test_file
