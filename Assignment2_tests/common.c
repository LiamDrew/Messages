#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include "common.h"

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void clear_input_buffer() {
    int c;
    // Read characters until newline or EOF
    while ((c = getchar()) != '\n' && c != EOF);
}

void print_client_list(const char *client_list_buffer, size_t length) {
    size_t i = 0;

    while (i < length) {
        // Print the current string
        if (strlen(&client_list_buffer[i]) > 0) {
            printf("Client ID: %s\n", &client_list_buffer[i]);
        }

        // Move to the next string (after the current null terminator)
        i += strlen(&client_list_buffer[i]) + 1;
    }
}

message create_hello_message(char *clientID) {
    message hello_msg; 
    strncpy(hello_msg.header.source, clientID, SOURCE_LENGTH);
    strncpy(hello_msg.header.destination, "Server", SOURCE_LENGTH);
    hello_msg.header.Type = (short int) htons(HELLO);
    hello_msg.header.Message_ID = (uint32_t) htonl(0); 
    hello_msg.header.Length = (uint32_t) htonl(0); 

    memset(hello_msg.content, 0, MAX_PACKET_SIZE);
    return hello_msg;
}

message create_client_list_message(char *clientID) {
    message list_request_msg; 
    strncpy(list_request_msg.header.source, clientID, SOURCE_LENGTH);
    strncpy(list_request_msg.header.destination, "Server", SOURCE_LENGTH);
    list_request_msg.header.Type = (short int) htons(LIST_REQUEST);
    list_request_msg.header.Message_ID = (uint32_t) htonl(0); 
    list_request_msg.header.Length = (uint32_t) htonl(0); 

    memset(list_request_msg.content, 0, MAX_PACKET_SIZE);
    return list_request_msg;
}

message create_exit_message(char *clientID) {
    message exit_msg;
    strncpy(exit_msg.header.source, clientID, SOURCE_LENGTH);
    strncpy(exit_msg.header.destination, "Server", SOURCE_LENGTH);
    exit_msg.header.Type = (short int) htons(EXIT);
    exit_msg.header.Message_ID = (uint32_t) htonl(0); 
    exit_msg.header.Length = (uint32_t) htonl(0); 

    memset(exit_msg.content, 0, MAX_PACKET_SIZE);
    return exit_msg;
}

message create_chat_message(char *clientID) {
    message chat_msg;
    char destinationID[SOURCE_LENGTH];
    uint32_t messageID;

    strncpy(chat_msg.header.source, clientID, strlen(clientID));
    chat_msg.header.Type = (short int) htons(CHAT);
    // clear_input_buffer();
    // printf("Enter a destinationID: ");
    // if (fgets(destinationID, sizeof(destinationID), stdin) != NULL) {
    //     // Remove the trailing newline character if present
    //     size_t len = strlen(destinationID);
    //     if (len > 0 && destinationID[len - 1] == '\n') {
    //         destinationID[len - 1] = '\0';  // Replace newline with null terminator
    //     }
    // }
    // strncpy(chat_msg.header.destination, destinationID, strlen(destinationID));
    strncpy(chat_msg.header.destination, "Facu0210", strlen("Facu0210"));
    chat_msg.header.Message_ID = (uint32_t) htonl(1);



    char *test_message = "Hello there!";
    chat_msg.header.Length = (uint32_t) htonl(strlen(test_message) + 1);
    memcpy(chat_msg.content, test_message, strlen(test_message));
    printf("Created CHAT message!");
    return chat_msg;
}

void handle_message_from_server(char *buffer, short int msg_type) {
    printf("This is the type: %d\n", msg_type);
    switch (msg_type) {
    case HELLO:
        message ack_message; 
        // store the first 50 bytes into the header
        memcpy((char *) &ack_message.header, buffer, HEADER_SIZE);
        ack_message.header.Type = ntohs(ack_message.header.Type);
        ack_message.header.Length = ntohl(ack_message.header.Length);
        ack_message.header.Message_ID = ntohl(ack_message.header.Message_ID);
        printf("**************************\n");
        printf("Received Hello ACK\n");
        printf("Info about header: \n");
        printf("Header Type: %hd\n", ack_message.header.Type);
        printf("Header MessageID: %u\n", ack_message.header.Message_ID);
        printf("Header Length: %hd\n", ack_message.header.Length);
        printf("ClientID: %s\n", ack_message.header.source);
        printf("Destination: %s\n", ack_message.header.destination);
        printf("**************************\n");

        // Read the header of the client_list 
        message client_list;
        memcpy((char *)&client_list.header, buffer + HEADER_SIZE, HEADER_SIZE);
        client_list.header.Type = ntohs(client_list.header.Type);
        client_list.header.Length = ntohl(client_list.header.Length);
        client_list.header.Message_ID = ntohl(client_list.header.Message_ID);
        printf("**************************\n");
        printf("Received client_list header\n");
        printf("Info about header: \n");
        printf("Header Type: %hd\n", client_list.header.Type);
        printf("Header MessageID: %u\n", client_list.header.Message_ID);
        printf("Header Length: %hd\n", client_list.header.Length);
        printf("ClientID: %s\n", client_list.header.source);
        printf("Destination: %s\n", client_list.header.destination);
        printf("**************************\n");
        // Copy the client_list itself
        memcpy((char *)client_list.content, buffer + 2*HEADER_SIZE, client_list.header.Length);
        print_client_list(client_list.content, client_list.header.Length);
        break;
    case HELLO_ALR_PRESENT:
        message error_msg;
        // store the first 50 bytes into the header
        memcpy((char *) &error_msg.header, buffer, HEADER_SIZE);
        error_msg.header.Type = ntohs(error_msg.header.Type);
        error_msg.header.Length = ntohl(error_msg.header.Length);
        error_msg.header.Message_ID = ntohl(error_msg.header.Message_ID);
        printf("**************************\n");
        printf("Received Hello ACK\n");
        printf("Info about header: \n");
        printf("Header Type: %hd\n", error_msg.header.Type);
        printf("Header MessageID: %u\n", error_msg.header.Message_ID);
        printf("Header Length: %hd\n", error_msg.header.Length);
        printf("ClientID: %s\n", error_msg.header.source);
        printf("Destination: %s\n", error_msg.header.destination);
        printf("**************************\n");
        break;
    case LIST_REQUEST:
        // Read the header of the client_list 
        message client_list_req;
        memcpy((char *)&client_list_req.header, buffer, HEADER_SIZE);
        client_list_req.header.Type = ntohs(client_list_req.header.Type);
        client_list_req.header.Length = ntohl(client_list_req.header.Length);
        client_list_req.header.Message_ID = ntohl(client_list_req.header.Message_ID);
        printf("**************************\n");
        printf("Received client_list header\n");
        printf("Info about header: \n");
        printf("Header Type: %hd\n", client_list_req.header.Type);
        printf("Header MessageID: %u\n", client_list_req.header.Message_ID);
        printf("Header Length: %hd\n", client_list_req.header.Length);
        printf("ClientID: %s\n", client_list_req.header.source);
        printf("Destination: %s\n", client_list_req.header.destination);
        printf("**************************\n");
        // Copy the client_list itself
        memcpy((char *)client_list_req.content, buffer + HEADER_SIZE, client_list_req.header.Length);
        print_client_list(client_list_req.content, client_list_req.header.Length);
        break;
    case CHAT:
        printf("No response from server as we did not implement error_handlind yet!\n");
        break;
    case EXIT:
        printf("You just exited the program the server will not send a response!\n");
        break;
    default:
        break;
    }
}

size_t handle_type_choice(int fd, short int msg_type, char *clientID) {
    message msg;
    size_t nbytes = 0;
    switch (msg_type)
    {
    case HELLO:
        printf("Creating a HELLO message\n");
        msg = create_hello_message(clientID);
        nbytes = write(fd, (char *)&msg, HEADER_SIZE);
        break;
    case LIST_REQUEST:
        printf("Creating a LIST_REQUEST message\n");
        msg = create_client_list_message(clientID);
        nbytes = write(fd, (char *)&msg, HEADER_SIZE);
        break;
    case CHAT:
        printf("Creating a CHAT message\n");
        msg = create_chat_message(clientID);
        printf("writing %d bytes\n", ntohl(msg.header.Length));
        nbytes = write(fd, (char *)&msg, HEADER_SIZE + ntohl(msg.header.Length));
        break;
    case EXIT:
        printf("Creating a EXIT message\n");
        msg = create_exit_message(clientID);
        nbytes = write(fd, (char *)&msg, HEADER_SIZE);
        break;
    default:
        printf("INVALID TYPE!\n");
        exit(EXIT_FAILURE);
        break;
    }
    return nbytes;
}