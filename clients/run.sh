#!/bin/bash


# TEST 1
# Make sure two clients can send each other a complete message
# The unique string in natalia_output.txt should be "peach goose pie"
# The unique string in liam_output.txt should be "hands down boys"
./simple_client.out Liam > liam_output.txt <<EOF &
HELLO
PAUSE
PAUSE
PAUSE
MSG
Natalia
You are a peach goose pie
EXIT
EOF


sleep 0.5
./simple_client.out Natalia > natalia_output.txt <<EOF &
HELLO
PAUSE
PAUSE
MSG
Liam
Corey and trevor, hands down boys, you messed up
PAUSE
EXIT
EOF

wait

echo -e "______________________________________"
echo "Test 1: Sending a simple message back and forth"
if grep "peach goose pie" natalia_output.txt > /dev/null 2>&1 && grep "hands down boys" liam_output.txt > /dev/null 2>&1; then
    echo -e "Test 1 Passed"
else
    echo -e "Test 1 Failed"
fi
rm liam_output.txt natalia_output.txt
echo -e "______________________________________\n"



# TEST 2
# Make sure that a client can send a partial message to a friend
# The unique string in natalia_output.txt should be "little buddy"
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
Hi there little buddy
5
60
1
EOF

wait

echo -e "______________________________________"
echo "Test 2: Sending a partial message to another client"
if grep "little buddy" natalia_output.txt > /dev/null 2>&1; then
    echo -e "Test 2 Passed"
else
    echo -e "Test 2 Failed"
fi
rm liam_output.txt natalia_output.txt
echo -e "______________________________________\n"



# TEST 3: Was removed due to redundancy
echo -e "______________________________________"
echo "Test 3: Doing nothing"
echo -e "Test 3 Passed"
echo -e "______________________________________\n"


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
I took her to my penthouse and I -------- it
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



# TEST 5: Make sure the client can send a partial header + full message to
# their friend

./simple_client.out Natalia > natalia_output.txt <<EOF &
HELLO
PAUSE
PAUSE
PAUSE
EOF

./simple_client.out Liam > liam_output.txt <<EOF &
HELLO
PAUSE
PAUSE
PARTIAL
Natalia
I took her to my penthouse and I -------- it
30
-1
1
EXIT
EOF

wait

echo -e "______________________________________"
echo "Test 5: Sending a partial header + full message to another client"
if grep "Message content is: I took her to my penthouse" natalia_output.txt > /dev/null 2>&1; then
    echo -e "Test 5 Passed"
else
    echo -e "Test 5 Failed"
fi
rm liam_output.txt natalia_output.txt
echo -e "______________________________________\n"



# TEST 6: New client sends chat message instead of hello
./simple_client.out Liam > liam_output.txt <<EOF &
MSG
Liam
I took her to my penthouse and I -------- it
EOF

wait

echo -e "______________________________________"
echo "Test 6: Sending a partial header + full message to another client"
if grep "Shutting down the client" liam_output.txt > /dev/null 2>&1; then
    echo -e "Test 6 Passed"
else
    echo -e "Test 6 Failed"
fi
rm liam_output.txt
echo -e "______________________________________\n"



# TEST 7: Client arbitarily closes the socket after hello
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
MSG
Natalia
Hi there honey pie
EOF

wait

echo -e "______________________________________"
echo "Test 7: Arbitrarily closing socket after hello"
if grep "Message content is: Natalia.Liam" liam_output.txt > /dev/null 2>&1; then
    echo -e "Test 7 Passed"
else
    echo -e "Test 7 Failed"
fi
rm liam_output.txt natalia_output.txt
echo -e "______________________________________\n"



# TEST 8: Client exits before hello
./simple_client.out Liam > liam_output.txt <<EOF &
EXIT
HELLO
EOF

wait

echo -e "______________________________________"
echo "Test 8: Client exits before hello"
if grep "Shutting down the client" liam_output.txt > /dev/null 2>&1; then
    echo -e "Test 8 Passed"
else
    echo -e "Test 8 Failed"
fi
rm liam_output.txt
echo -e "______________________________________\n"



# TEST 9: Client arbitarily closes the socket before hello
./simple_client.out Liam > liam_output.txt <<EOF &
EOF

wait

echo -e "______________________________________"
echo "Test 9: Client disconnects before hello"
if grep "Shutting down the client" liam_output.txt > /dev/null 2>&1; then
    echo -e "Test 9 Passed"
else
    echo -e "Test 9 Failed"
fi
rm liam_output.txt
echo -e "______________________________________\n"



# TEST 10: Data part of message is greater than specified

./simple_client.out Natalia > natalia_output.txt <<EOF &
HELLO
PAUSE
PAUSE
PAUSE
EXIT
EOF


./simple_client.out Liam > liam_output.txt <<EOF &
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
    echo -e "Test 10 Passed"
else
    echo -e "Test 10 Failed"
fi
rm liam_output.txt natalia_output.txt
echo -e "______________________________________\n"



# TEST 11: Data part of message is less than specified
./simple_client.out Liam > liam_output.txt <<EOF &
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
    echo -e "Test 11 Passed"
else
    echo -e "Test 11 Failed"
fi
rm liam_output.txt
echo -e "______________________________________\n"



# TEST 12: Already connected client says hello with the same name
./simple_client.out Liam > liam_output.txt <<EOF &
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
    echo -e "Test 12 Passed"
else
    echo -e "Test 12 Failed"
fi
rm liam_output.txt
echo -e "______________________________________\n"



# TEST 13: Already connected client says hello again with a different name
./simple_client.out Liam > liam_output.txt <<EOF &
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
    echo -e "Test 13 Failed"
else
    echo -e "Test 13 Passed"
fi
rm liam_output.txt
echo -e "______________________________________\n"



# TEST 14: New client ID is the same as existing
./simple_client.out Liam > liam_output1.txt <<EOF &
HELLO
PAUSE
PAUSE
SLEEP
EOF

sleep 0.1

./simple_client.out Liam > liam_output.txt <<EOF &
HELLO
PAUSE
EOF

wait

echo -e "______________________________________"
echo "Test 14: New client tries to connect with an ID that exists already"
if grep "Type: 7, Source: Server, Dest: Liam" liam_output.txt > /dev/null 2>&1; then
    echo -e "Test 14 Passed"
else
    echo -e "Test 14 Failed"
fi
rm liam_output.txt liam_output1.txt
echo -e "______________________________________\n"

# TEST 15: Sending a partial message to another client, but timing out before
# it can be sent
# NOTE: This test assumes a 5 second timeout value for the sake of speed
# MAKE SURE TO RESET IT TO 60 BEFORE SUBMITTING

./simple_client.out Natalia > natalia_output.txt <<EOF &
HELLO
PAUSE
PAUSE
SLEEP
EOF

sleep 0.5

./simple_client.out Liam > liam_output.txt <<EOF &
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
#6 second timeout should prevent message delivery

wait

echo -e "______________________________________"
echo "Test 15: Sending a partial message to another client, but timing out"
if grep "Shutting down the client" liam_output.txt > /dev/null 2>&1 && ! grep "Hi there little buddy" natalia_output.txt > /dev/null 2>&1; then
    echo -e "Test 15 Passed"
else
    echo -e "Test 15 Failed"
fi
rm liam_output.txt natalia_output.txt
echo -e "______________________________________\n"



# TEST 16: Sending a partial Hello message
# This test also test the correct ordering for hello. The first complete hello
# recieved by the server should be put into the client list first

./partial_client.out Natalia > natalia_output.txt <<EOF &
LISTREQ
MSG
Liam
Hi peach goose
EOF

sleep 0.5

./simple_client.out Liam > liam_output.txt <<EOF &
HELLO
PAUSE
PAUSE
PAUSE
EOF

wait

echo -e "______________________________________"
echo "Test 16: Testing partial hello AND correct hello ordering"
if grep "Liam Natalia" natalia_output.txt > /dev/null 2>&1 && grep "Hi peach goose" liam_output.txt > /dev/null 2>&1; then
    echo -e "Test 16 Passed"
else
    echo -e "Test 16 Failed"
fi
rm liam_output.txt natalia_output.txt
echo -e "______________________________________\n"




# TEST 17: Spamming server with invalid message types
./simple_client.out Liam > liam_output1.txt <<EOF &
HELLO
PAUSE
PAUSE
LISTREQ
HELLOACK
SLEEP
EXIT
EOF

sleep 0.1

./simple_client.out Liam2 > liam_output2.txt <<EOF &
HELLO
PAUSE
PAUSE
CLIENTLIST
SLEEP
EXIT
EOF

sleep 0.1

./simple_client.out Liam3 > liam_output3.txt <<EOF &
HELLO
PAUSE
PAUSE
ERRORPRESENT
SLEEP
EXIT
EOF

sleep 0.1

./simple_client.out Liam4 > liam_output4.txt <<EOF &
HELLO
PAUSE
PAUSE
ERRORDELIVER
SLEEP
EXIT
EOF

sleep 0.1

./simple_client.out Liam5 > liam_output5.txt <<EOF &
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
    echo -e "Test 17 Passed"
else
    echo -e "Test 17 Failed"
fi
rm liam_output1.txt liam_output2.txt liam_output3.txt liam_output4.txt liam_output5.txt
echo -e "______________________________________\n"



# TEST 18: Client cannot name itself server

./simple_client.out Server > liam_output1.txt <<EOF &
HELLO
SLEEP
EOF

wait

echo -e "______________________________________"
echo "Test 18: Client tries naming itself server"
if grep "Shutting down the client" liam_output1.txt > /dev/null 2>&1 && ! grep "Type: 2, Source: Server" liam_output1.txt > /dev/null 2>&1; then
    echo -e "Test 18 Passed"
else
    echo -e "Test 18 Failed"
fi
rm liam_output1.txt
echo -e "______________________________________\n"



# TEST 19: Client cannot send chat message to server
./simple_client.out Liam > liam_output1.txt <<EOF &
HELLO
SLEEP
SLEEP
MSG
Server
Hi server! Here's a message
SLEEP
EOF

wait

echo -e "______________________________________"
echo "Test 19: Client tries sending chat message to server"
if grep "Shutting down the client" liam_output1.txt > /dev/null 2>&1 && ! grep "Type: 8, Source: Server" liam_output1.txt > /dev/null 2>&1; then
    echo -e "Test 19 Passed"
else
    echo -e "Test 19 Failed"
fi
rm liam_output1.txt
echo -e "______________________________________\n"



# TEST 20: Cannot deliver message to non-connected client

./simple_client.out Liam > liam_output1.txt <<EOF &
HELLO
SLEEP
SLEEP
BUILDMSG
5
Liam
Labib
100
0
Do you like fried chicken?
SLEEP
EOF

wait

echo -e "______________________________________"
echo "Test 20: Cannot deliver message to non-connected client"
if grep "Shutting down the client" liam_output1.txt > /dev/null 2>&1 && grep "Type: 8, Source: Server" liam_output1.txt > /dev/null 2>&1; then
    echo -e "Test 20 Passed"
else
    echo -e "Test 20 Failed"
fi
rm liam_output1.txt
echo -e "______________________________________\n"



# TEST 21: Cannot have a chat message with length of 0 (no data)

./simple_client.out Liam > liam_output1.txt <<EOF &
HELLO
SLEEP
SLEEP
BUILDMSG
5
Liam
Labib
0
0
Do you like fried chicken?
SLEEP
EOF

wait

echo -e "______________________________________"
echo "Test 21: Empty chat message"
if grep "Shutting down the client" liam_output1.txt > /dev/null 2>&1 && ! grep "Type: 8, Source: Server" liam_output1.txt > /dev/null 2>&1; then
    echo -e "Test 21 Passed"
else
    echo -e "Test 21 Failed"
fi
rm liam_output1.txt
echo -e "______________________________________\n"



# TEST 22: HELLO message cannot have a length
./simple_client.out Liam > liam_output1.txt <<EOF &

BUILDMSG
1
Liam
Server
50
0
NULL
SLEEP
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



# TEST 23: Client already present

./simple_client.out Liam > liam_output1.txt <<EOF &
HELLO
PAUSE
PAUSE
PAUSE
EOF

sleep 0.5

./simple_client.out Liam > liam_output.txt <<EOF &
HELLO
SLEEP
EXIT
EOF

sleep 0.5

./simple_client.out Natalia > natalia_output.txt <<EOF &
HELLO
PAUSE
PAUSE
MSG
Liam
Hi there sweet
SLEEP
EOF

wait

echo -e "______________________________________"
echo "Test 23: Client already present"
if grep "Shutting down the client" liam_output.txt > /dev/null 2>&1 && \
   ! grep "Type: 2, Source: Server" liam_output.txt > /dev/null 2>&1 && \
   grep "Hi there sweet" liam_output1.txt > /dev/null 2>&1; then
    echo -e "Test 23 Passed"
else
    echo -e "Test 23 Failed"
fi
rm liam_output1.txt liam_output.txt natalia_output.txt
echo -e "______________________________________\n"



# TEST 24: Client message too big

./simple_client.out Liam > liam_output1.txt <<EOF &
HELLO
PAUSE
PAUSE
PAUSE
EOF

sleep 0.5


./simple_client.out Natalia > natalia_output.txt <<EOF &
HELLO
PAUSE
PAUSE
BUILDMSG
5
Natalia
Liam
401
1
Hi there sweet
SLEEP
EOF

sleep 0.5

./simple_client.out Labib > labib_output.txt <<EOF &
HELLO
PAUSE
PAUSE
MSG
Liam
Bro I love fried chicken
SLEEP
EOF

wait

echo -e "______________________________________"
echo "Test 24: Client message too big"
if grep "Shutting down the client" natalia_output.txt > /dev/null 2>&1 && \
   ! grep "Type: 8, Source: Server" natalia_output.txt > /dev/null 2>&1 && \
   grep "Bro I love fried chicken" liam_output1.txt > /dev/null 2>&1; then
    echo -e "Test 24 Passed"
else
    echo -e "Test 24 Failed"
fi
rm liam_output1.txt labib_output.txt natalia_output.txt
echo -e "______________________________________\n"

# chat message with length > 400

# TODO: Make sure the timeout value is getting set correctly!




# Test this later
# TEST 15: Server recieves message types that aren't compatible with the size
# it recieves