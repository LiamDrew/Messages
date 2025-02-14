# Messages
 
 A reliable server-based command line chat messaging service over TCP.

 ### Overview
 This repository contains the implementation of a server-based chat messaging service over TCP Once started, the server will run indefinitely, listening for connection requests on its network at the port number specified in the `.env` file. Clients will connect to the server via a "hello" message, at which point the server will acknowledge their connection request and respond with a list of all currently connected clients. After clients have connected to the server, they can send chat messages to each other through the server.


 ### Running the Program
 To run the server:  
 1. Navigate to `Messages/`  
 2. Run `./run.sh` 

 At this point, clients will be able to connect to the server and chat with each other.

 #### Testing
  This program features a comprehensive unit test suite implemented in a bash shell script. This script will run 23 tests using test clients to verify the functionality of the server. Here is the format of this test suite:
 ______________________________________
Test 1: Sending a simple message back and forth  
<span style="color: lightgreen">Test 1 Passed</span>
______________________________________
______________________________________
Test 2: Sending a partial message to another client  
<span style="color: lightgreen">Test 2 Passed</span>
______________________________________
______________________________________
Test 3: Client already present  
<span style="color: lightgreen">Test 3 Passed</span>
______________________________________
etc.

 To run the test suite:
 1. Run the server
 2. Open a new terminal window
 3. Navigate to `Messages/`
 4. Run `make test`

 The source code for the test suite is located in `Messages/tests/test.sh`.
 



 ### Acknowledgements
 Professor Fahad Dogar for designing this project.