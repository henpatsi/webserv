#! /bin/bash

TEST_REQUEST_DIR="tester/requests"

cd ..
make

echo "------------------------------------------------------------------"
for file in $TEST_REQUEST_DIR/*
do
	echo "Testing $file"
	echo "------------------------------------------------------------------"
	./webserv $file
	echo
	echo "------------------------------------------------------------------"
done
