#!/bin/bash

# Get the directory where the script is located
ORIGINAL_DIR=$(pwd)
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"  # Change to script directory

# Define colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

# Define IP address and Port number
source ../.env


# TEST 17: Spamming server with invalid message types
./simple_client.out Liam $IP $PORT > liam_output1.txt <<EOF 2> /dev/null &
HELLO
PAUSE
PAUSE
LISTREQ
HELLOACK
SLEEP
EXIT
EOF

sleep 0.1

./simple_client.out Liam2 $IP $PORT > liam_output2.txt <<EOF 2> /dev/null &
HELLO
PAUSE
PAUSE
CLIENTLIST
SLEEP
EXIT
EOF

sleep 0.1

./simple_client.out Liam3 $IP $PORT > liam_output3.txt <<EOF 2> /dev/null &
HELLO
PAUSE
PAUSE
ERRORPRESENT
SLEEP
EXIT
EOF

sleep 0.1

./simple_client.out Liam4 $IP $PORT > liam_output4.txt <<EOF 2> /dev/null &
HELLO
PAUSE
PAUSE
ERRORDELIVER
SLEEP
EXIT
EOF

sleep 0.1

./simple_client.out Liam5 $IP $PORT > liam_output5.txt <<EOF 2> /dev/null &
HELLO
PAUSE
PAUSE
MEGATYPE
SLEEP
EXIT
EOF


wait

echo -e "______________________________________"
echo "Test 17: Spamming the server with bad messages"
if grep "Shutting down the client" liam_output1.txt > /dev/null 2>&1; then
    echo -e "${GREEN}Test 17 Passed${NC}"
else
    echo -e "${RED}Test 17 Failed${NC}"
fi
rm liam_output1.txt liam_output2.txt liam_output3.txt liam_output4.txt liam_output5.txt
echo -e "______________________________________"