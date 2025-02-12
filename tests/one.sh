#!/bin/bash

# TEST 22: HELLO message cannot have a length
./simple_client.out Liam > liam_output1.txt <<EOF &

BUILDMSG
1
Liam
Server
50
0
NULL
EOF

wait

echo -e "______________________________________"
echo "Test 22: Hello message must have length 0"
if grep "Shutting down the client" liam_output1.txt > /dev/null 2>&1 && ! grep "Type: 2, Source: Server" liam_output1.txt > /dev/null 2>&1; then
    echo -e "Test 22 Passed"
else
    echo -e "Test 22 Failed"
fi
rm liam_output1.txt
echo -e "______________________________________\n"