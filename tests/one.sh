#!/bin/bash

# Define color codes
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

IP=127.0.0.1
PORT=1026


# TEST 4: Make sure the client can send a full header + partial message to
# their friend

./simple_client.out Natalia > natalia_output.txt <<EOF &
HELLO
PAUSE
PAUSE
PAUSE
EOF

sleep 0.5

./simple_client.out Liam > liam_output.txt <<EOF &
HELLO
PAUSE
PAUSE
PARTIAL
Natalia
I took her to my penthouse and
-1
60
1
EOF

wait

echo -e "______________________________________"
echo "Test 4: Sending a full header + message to another client"
if grep "Message content is: I took her to my penthouse" natalia_output.txt > /dev/null 2>&1; then
    echo -e "Test 4 Passed"
else
    echo -e "Test 4 Failed"
fi
rm liam_output.txt natalia_output.txt
echo -e "______________________________________\n"