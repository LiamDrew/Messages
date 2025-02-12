// this doesn't build out of the box either

/*
 * tcpclient.c - A simple TCP client
 * usage: tcpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFSIZE 1024

#define IP "10.4.2.20"

typedef struct __attribute__((packed))
{
    short int type;
    char source[20];
    char dest[20];
    int length;
    int message_id;
} header;

typedef struct __attribute__((packed))
{
    header h;
    char data[400];
} message;

/*
 * error - wrapper for perror
 */
void error(char *msg)
{
    perror(msg);
    exit(0);
}

// void prepare_message(header *h) {
//     h->type = htons(h->type);

// }

int main(int argc, char **argv)
{
    int sockfd, portno, n;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];

    /* check command line arguments */
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
    hostname = IP;
    portno = atoi(argv[1]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    /* connect: create a connection with the server */
    if (connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
        error("ERROR connecting");

    // /* get message line from the user */
    // printf("Please enter msg: ");
    // bzero(buf, BUFSIZE);
    // fgets(buf, BUFSIZE, stdin);

    /* Assembling message */
    // printf("Assembling message\n");

    header *h = malloc(sizeof(header));
    h->type = 1;
    strcpy(h->source, "PeachGoose\0");
    strcpy(h->dest, "Server\0");
    h->length = 0;
    h->message_id = 0;


    char buf2[50];

    // more preparation will have to be done
    memcpy(buf2, h, sizeof(header));

    /* send the message line to the server */
    n = write(sockfd, buf2, sizeof(header));
    if (n < 0)
        error("ERROR writing to socket");


    // /* print the server's reply */
    // bzero(buf, BUFSIZE);
    // n = read(sockfd, buf, BUFSIZE);
    // if (n < 0)
    //     error("ERROR reading from socket");
    // printf("Echo from server: %s", buf);
    close(sockfd);
    return 0;
}