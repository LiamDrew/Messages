# Define color codes
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Define IP address as localhost
IP=127.0.0.1
PORT=1026



# TEST 1
# Make sure two clients can send each other a complete message
# The unique string in natalia_output.txt should be "peach goose pie"
# The unique string in liam_output.txt should be "hands down boys"
./simple_client.out Liam $IP $PORT > liam_output.txt <<EOF &
HELLO
PAUSE
PAUSE
PAUSE
MSG
Natalia
Hello everybody, so glad to see you
EXIT
EOF

sleep 0.5
./simple_client.out Natalia $IP $PORT > natalia_output.txt <<EOF &
HELLO
PAUSE
PAUSE
MSG
Liam
Corey and trevor, hands down boys,
PAUSE
EXIT
EOF

wait

echo "______________________________________"
echo "Test 1: Sending a simple message back and forth"
if grep "so glad to see" natalia_output.txt > /dev/null 2>&1 && grep "hands down boys" liam_output.txt > /dev/null 2>&1; then
    echo "${GREEN}Test 1 Passed${NC}"
else
    echo "${RED}Test 1 Failed${NC}"
fi
rm liam_output.txt natalia_output.txt
echo "______________________________________\n"



# TEST 2
# Make sure that a client can send a partial message to a friend
# The unique string in natalia_output.txt should be "little buddy"
./simple_client.out Natalia $IP $PORT > natalia_output.txt <<EOF &
HELLO
PAUSE
PAUSE
PAUSE
EOF

sleep 0.5

./simple_client.out Liam $IP $PORT > liam_output.txt <<EOF &
HELLO
PAUSE
PAUSE
PARTIAL
Natalia
Hi there little buddy
5
60
1
EOF

wait

echo "______________________________________"
echo "Test 2: Sending a partial message to another client"
if grep "little buddy" natalia_output.txt > /dev/null 2>&1; then
    echo "${GREEN}Test 2 Passed${NC}"
else
    echo "${RED}Test 2 Failed${NC}"
fi
rm liam_output.txt natalia_output.txt
echo "______________________________________\n"



# TEST 3: 
# Client already present
# Make sure that a new client cannot join with the name of an existing client
./simple_client.out Liam $IP $PORT > liam_output1.txt <<EOF &
HELLO
PAUSE
PAUSE
PAUSE
EOF

sleep 0.5

./simple_client.out Liam $IP $PORT > liam_output.txt <<EOF &
HELLO
SLEEP
EXIT
EOF

sleep 0.5

./simple_client.out Natalia $IP $PORT > natalia_output.txt <<EOF &
HELLO
PAUSE
PAUSE
MSG
Liam
Hi there sweet
SLEEP
EOF

wait

echo "______________________________________"
echo "Test 3: Client already present"
if grep "Shutting down the client" liam_output.txt > /dev/null 2>&1 && \
   ! grep "Type: 2, Source: Server" liam_output.txt > /dev/null 2>&1 && \
   grep "Hi there sweet" liam_output1.txt > /dev/null 2>&1; then
    echo "${GREEN}Test 3 Passed${NC}"
else
    echo "${RED}Test 3 Failed${NC}"
fi
rm liam_output1.txt liam_output.txt natalia_output.txt
echo "______________________________________\n"



# TEST 4: 
# Make sure the client can send a full header + partial message to their friend
./simple_client.out Natalia $IP $PORT > natalia_output.txt <<EOF &
HELLO
PAUSE
PAUSE
PAUSE
EOF

sleep 0.5

./simple_client.out Liam $IP $PORT > liam_output.txt <<EOF &
HELLO
PAUSE
PAUSE
PARTIAL
Natalia
I took her to my favorite ice cream shop
-1
60
1
EOF

wait

echo "______________________________________"
echo "Test 4: Sending a full header + message to another client"
if grep "Message content is: I took her to my" natalia_output.txt > /dev/null 2>&1; then
    echo "${GREEN}Test 4 Passed${NC}"
else
    echo "${RED}Test 4 Failed${NC}"
fi
rm liam_output.txt natalia_output.txt
echo "______________________________________\n"



# TEST 5: 
# Make sure the client can send a partial header + full message to their friend
./simple_client.out Natalia $IP $PORT > natalia_output.txt <<EOF &
HELLO
PAUSE
PAUSE
PAUSE
EOF

./simple_client.out Liam $IP $PORT > liam_output.txt <<EOF &
HELLO
PAUSE
PAUSE
PARTIAL
Natalia
I took her to my favorite ice cream shop
30
-1
1
EXIT
EOF

wait

echo -e "______________________________________"
echo "Test 5: Sending a partial header + full message to another client"
if grep "Message content is: I took her to my penthouse" natalia_output.txt > /dev/null 2>&1; then
    echo -e "${GREEN}Test 5 Passed${NC}"
else
    echo -e "${RED}Test 5 Failed${NC}"
fi
rm liam_output.txt natalia_output.txt
echo -e "______________________________________\n"

# TEST 6: New client sends chat message instead of hello
./simple_client.out Liam $IP $PORT > liam_output.txt <<EOF &
MSG
Liam
I took her to my penthouse and I -------- it
EOF

wait

echo -e "______________________________________"
echo "Test 6: Sending a partial header + full message to another client"
if grep "Shutting down the client" liam_output.txt > /dev/null 2>&1; then
    echo -e "${GREEN}Test 6 Passed${NC}"
else
    echo -e "${RED}Test 6 Failed${NC}"
fi
rm liam_output.txt
echo -e "______________________________________\n"

# TEST 7: Client arbitarily closes the socket after hello
./simple_client.out Natalia $IP $PORT > natalia_output.txt <<EOF &
HELLO
PAUSE
PAUSE
PAUSE
EOF

sleep 0.5

./simple_client.out Liam $IP $PORT > liam_output.txt <<EOF &
HELLO
PAUSE
PAUSE
MSG
Natalia
Hi there honey pie
EOF

wait

echo -e "______________________________________"
echo "Test 7: Arbitrarily closing socket after hello"
if grep "Message content is: Natalia.Liam" liam_output.txt > /dev/null 2>&1; then
    echo -e "${GREEN}Test 7 Passed${NC}"
else
    echo -e "${RED}Test 7 Failed${NC}"
fi
rm liam_output.txt natalia_output.txt
echo -e "______________________________________\n"

# TEST 8: Client exits before hello
./simple_client.out Liam $IP $PORT > liam_output.txt <<EOF &
EXIT
HELLO
EOF

wait

echo -e "______________________________________"
echo "Test 8: Client exits before hello"
if grep "Shutting down the client" liam_output.txt > /dev/null 2>&1; then
    echo -e "${GREEN}Test 8 Passed${NC}"
else
    echo -e "${RED}Test 8 Failed${NC}"
fi
rm liam_output.txt
echo -e "______________________________________\n"

# TEST 9: Client arbitarily closes the socket before hello
./simple_client.out Liam $IP $PORT > liam_output.txt <<EOF &
EOF

wait

echo -e "______________________________________"
echo "Test 9: Client disconnects before hello"
if grep "Shutting down the client" liam_output.txt > /dev/null 2>&1; then
    echo -e "${GREEN}Test 9 Passed${NC}"
else
    echo -e "${RED}Test 9 Failed${NC}"
fi
rm liam_output.txt
echo -e "______________________________________\n"

# TEST 10: Data part of message is greater than specified
./simple_client.out Natalia $IP $PORT > natalia_output.txt <<EOF &
HELLO
PAUSE
PAUSE
PAUSE
EXIT
EOF

./simple_client.out Liam $IP $PORT > liam_output.txt <<EOF &
HELLO
PAUSE
PAUSE
MSGTOOBIG
Natalia
Far over the misty mountains cold
EXIT
EOF

wait

echo -e "______________________________________"
echo "Test 10: Data part of message is bigger than length"
if grep "Far over" natalia_output.txt > /dev/null 2>&1 && ! grep "Far over the" natalia_output.txt > /dev/null 2>&1; then
    echo -e "${GREEN}Test 10 Passed${NC}"
else
    echo -e "${RED}Test 10 Failed${NC}"
fi
rm liam_output.txt natalia_output.txt
echo -e "______________________________________\n"

# TEST 11: Data part of message is less than specified
./simple_client.out Liam $IP $PORT > liam_output.txt <<EOF &
HELLO
PAUSE
PAUSE
MSGTOOSMALL
Liam
Far over the misty mountains cold
SLEEP
EOF

wait

echo -e "______________________________________"
echo "Test 11: Data part of message is bigger than length"
if grep "Shutting down the client" liam_output.txt > /dev/null 2>&1; then
    echo -e "${GREEN}Test 11 Passed${NC}"
else
    echo -e "${RED}Test 11 Failed${NC}"
fi
rm liam_output.txt
echo -e "______________________________________\n"

# TEST 12: Already connected client says hello with the same name
./simple_client.out Liam $IP $PORT > liam_output.txt <<EOF &
HELLO
PAUSE
PAUSE
HELLO
SLEEP
EOF

wait

echo -e "______________________________________"
echo "Test 12: Already connected client says hello again with the same name"
if grep "Type: 7, Source: Server" liam_output.txt > /dev/null 2>&1; then
    echo -e "${GREEN}Test 12 Passed${NC}"
else
    echo -e "${RED}Test 12 Failed${NC}"
fi
rm liam_output.txt
echo -e "______________________________________\n"

# TEST 13: Already connected client says hello again with a different name
./simple_client.out Liam $IP $PORT > liam_output.txt <<EOF &
HELLO
PAUSE
PAUSE
BADHELLO
Liam2
Server
0
0
LISTREQ
SLEEP
EOF

wait

echo -e "______________________________________"
echo "Test 13: Already connected client says hello again with a different name"
if grep "Type: 2, Source: Server, Dest: Liam1" liam_output.txt > /dev/null 2>&1; then
    echo -e "${RED}Test 13 Failed${NC}"
else
    echo -e "${GREEN}Test 13 Passed${NC}"
fi
rm liam_output.txt
echo -e "______________________________________\n"

# TEST 14: New client ID is the same as existing
./simple_client.out Liam $IP $PORT > liam_output1.txt <<EOF &
HELLO
PAUSE
PAUSE
SLEEP
EOF

sleep 0.1

./simple_client.out Liam $IP $PORT > liam_output.txt <<EOF &
HELLO
PAUSE
EOF

wait

echo -e "______________________________________"
echo "Test 14: New client tries to connect with an ID that exists already"
if grep "Type: 7, Source: Server, Dest: Liam" liam_output.txt > /dev/null 2>&1; then
    echo -e "${GREEN}Test 14 Passed${NC}"
else
    echo -e "${RED}Test 14 Failed${NC}"
fi
rm liam_output.txt liam_output1.txt
echo -e "______________________________________\n"

# TEST 15: Sending a partial message to another client, but timing out
./simple_client.out Natalia $IP $PORT > natalia_output.txt <<EOF &
HELLO
PAUSE
PAUSE
SLEEP
EOF

sleep 0.5

./simple_client.out Liam $IP $PORT > liam_output.txt <<EOF &
HELLO
PAUSE
PAUSE
PARTIAL
Natalia
Hi there little buddy
-1
60
6
EOF

wait

echo -e "______________________________________"
echo "Test 15: Sending a partial message to another client, but timing out"
if grep "Shutting down the client" liam_output.txt > /dev/null 2>&1 && ! grep "Hi there little buddy" natalia_output.txt > /dev/null 2>&1; then
    echo -e "${GREEN}Test 15 Passed${NC}"
else
    echo -e "${RED}Test 15 Failed${NC}"
fi
rm liam_output.txt natalia_output.txt
echo -e "______________________________________\n"

# TEST 16: Sending a partial Hello message
./partial_client.out Natalia $IP $PORT > natalia_output.txt <<EOF &
LISTREQ
MSG
Liam
Hi peach goose
EOF

sleep 0.5

./simple_client.out Liam $IP $PORT > liam_output.txt <<EOF &
HELLO
PAUSE
PAUSE
PAUSE
EOF

wait

echo -e "______________________________________"
echo "Test 16: Testing partial hello AND correct hello ordering"
if grep "Liam Natalia" natalia_output.txt > /dev/null 2>&1 && grep "Hi peach goose" liam_output.txt > /dev/null 2>&1; then
    echo -e "${GREEN}Test 16 Passed${NC}"
else
    echo -e "${RED}Test 16 Failed${NC}"
fi
rm liam_output.txt natalia_output.txt
echo -e "______________________________________\n"

# TEST 17: Spamming server with invalid message types
./simple_client.out Liam $IP $PORT > liam_output1.txt <<EOF &
HELLO
PAUSE
PAUSE
LISTREQ
HELLOACK
SLEEP
EXIT
EOF

sleep 0.1

./simple_client.out Liam2 $IP $PORT > liam_output2.txt <<EOF &
HELLO
PAUSE
PAUSE
CLIENTLIST
SLEEP
EXIT
EOF

sleep 0.1

./simple_client.out Liam3 $IP $PORT > liam_output3.txt <<EOF &
HELLO
PAUSE
PAUSE
ERRORPRESENT
SLEEP
EXIT
EOF

sleep 0.1

./simple_client.out Liam4 $IP $PORT > liam_output4.txt <<EOF &
HELLO
PAUSE
PAUSE
ERRORDELIVER
SLEEP
EXIT
EOF

sleep 0.1

./simple_client.out Liam5 $IP $PORT > liam_output