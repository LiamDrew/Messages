#!/bin/bash

source .env
make clean > /dev/null
make > /dev/null
./server $PORT