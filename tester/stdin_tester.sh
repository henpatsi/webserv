#! /bin/bash

make -C ../

TEST_REQUEST_DIR="requests"

# REQUEST=$(cat $TEST_REQUEST_DIR/simple_get)

# ../webserv "$REQUEST"

echo "------------------------------------------------------------------"
for file in $TEST_REQUEST_DIR/*
do
	echo "Testing $file"
	echo "------------------------------------------------------------------"
	REQUEST=$(cat $file)
	../webserv "$REQUEST"
	echo
	echo "------------------------------------------------------------------"
done
