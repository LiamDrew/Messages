#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include "common.h"

#define TYPE_SIZE 2 
#define SOURCE_SIZE 20
#define DESTINATION_SIZE 20

// THIS TESTS MULTIPLE MESSAGES BEING SENT AT ONE!
int main(int argc, char *argv[]) {
    int bytes_send;
    int sockfd, portno, nbytes;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[MAX_PACKET_SIZE];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    char clientID[SOURCE_LENGTH];
    printf("Please enter your desired ID: ");
    scanf("%s", clientID);

    // Create buffer that will store two messages 
    char multiple_msgs_buffer[MAX_PACKET_SIZE];

    // Create hello_msg
    message hello_msg = create_hello_message(clientID);

    // Send HELLO message 
    write(sockfd, (char *)&hello_msg, HEADER_SIZE);

    read(sockfd, buffer, MAX_PACKET_SIZE);
    handle_message_from_server(buffer, HELLO);

    // Try sending List request + Exit in one go!
    // create list_request
    printf("Sending list_request_msg\n");
    message list_request_msg = create_client_list_message(clientID);

    memcpy(multiple_msgs_buffer, (char *)&list_request_msg, HEADER_SIZE);

    // create exit_msg 
    message exit_msg = create_exit_message(clientID);

    memcpy(multiple_msgs_buffer + HEADER_SIZE, (char *)&exit_msg, HEADER_SIZE);

    // Write! 
    write(sockfd, multiple_msgs_buffer, 2*HEADER_SIZE);

    memset(buffer, 0, MAX_PACKET_SIZE);
    read(sockfd, buffer, MAX_PACKET_SIZE);
    handle_message_from_server(buffer, LIST_REQUEST);

    close(sockfd);
}
