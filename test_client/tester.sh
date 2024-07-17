#! /bin/bash

ARG=$(cat example_messages/get_root)
./client 127.0.0.1 8080 "$ARG"
