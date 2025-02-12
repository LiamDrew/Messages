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

    // Create hello_msg
    message hello_msg = create_hello_message(clientID);

    // Send HELLO message 
    write(sockfd, (char *)&hello_msg, HEADER_SIZE);

    read(sockfd, buffer, MAX_PACKET_SIZE);
    handle_message_from_server(buffer, HELLO);

    // read from server 
    char content_buffer[MAX_PACKET_SIZE];
    read(sockfd, content_buffer, MAX_PACKET_SIZE);
    message chat_msg;
    // Get header first
    memcpy((char *)&chat_msg.header, content_buffer, HEADER_SIZE);
    chat_msg.header.Type = ntohs((short int) chat_msg.header.Type);
    chat_msg.header.Length = ntohl((uint32_t) chat_msg.header.Length);
    chat_msg.header.Message_ID = ntohl((uint32_t) chat_msg.header.Message_ID);
    printf("**************************\n");
    printf("Received client_list header\n");
    printf("Info about header: \n");
    printf("Header Type: %hd\n", chat_msg.header.Type);
    printf("Header MessageID: %u\n", chat_msg.header.Message_ID);
    printf("Header Length: %hd\n", chat_msg.header.Length);
    printf("ClientID: %s\n", chat_msg.header.source);
    printf("Destination: %s\n", chat_msg.header.destination);
    printf("**************************\n");
    // Get content
    memcpy((char *)&chat_msg.content, content_buffer + HEADER_SIZE, chat_msg.header.Length);
    printf("PRINTING Message received!\n");
    printf("%s\n", chat_msg.content);

    // Create and send partial exit_msg
    message exit_msg = create_exit_message(clientID);
    printf("Sending exit message with source: %s\n", exit_msg.header.source);

    // Send only the type + source + destination 
    write(sockfd, (char *)&exit_msg, HEADER_SIZE);

    close(sockfd);
}
