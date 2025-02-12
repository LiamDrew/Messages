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

#define PORT 9052

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

// typedef struct __attribute__((packed))
// {
//     short int type;
//     char source[20];
//     char dest[20];
//     int length;
//     int message_id;
//     char data[400]; //optional field, and by far the largest one
// } message;

void error(char *msg)
{
    perror(msg);
    exit(0);
}

void print_message(message *m)
{
    printf("\nMessage type %d\n", m->h.type);
    printf("Source: %s\n", m->h.source);
    printf("Dest: %s\n", m->h.dest);
    printf("Message Length is %d\n", m->h.length);
    printf("Message ID is %d\n", m->h.message_id);

    if (m->h.length > 0)
    {
        printf("Data is:\n%s\n", m->data);
    }

    printf("\n");
}

int main(int argc, char **argv)
{
    int sockfd, portno, n;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;

    /* check command line arguments */
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <name>\n", argv[0]);
        exit(0);
    }
    hostname = IP;
    portno = PORT;
    char *name = argv[1];

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


    message *m = malloc(sizeof(message));
    m->h.type = 1;
    strcpy(m->h.source, name);
    strcpy(m->h.dest, "Server");
    m->h.length = 100;
    m->h.message_id = 1;
    strcpy(m->data, "Hi sweet lovie pie\0");

    char buf[450];
    memset(buf, 0, sizeof(message));

    memcpy(buf, m, sizeof(message));

    printf("Sending message to server\n");
    print_message(m);


    // NOTE: even if the message struct is 450 bytes, we don't wanna send the
    // whole struct OTW if it's not necessary. 

    // int m_size = strlen(m->data);

    // sleep(10);

    // while (1) {
    /* send the message line to the server */
    // n = write(sockfd, buf, sizeof(header) + m_size);
    n = write(sockfd, buf, sizeof(message));
    if (n < 0)
        error("ERROR writing to socket");


    /* print the server's reply */
    bzero(buf, 450);
    n = 1;
    //until we read 0 bytes, keep reading
    while (n > 0) {
        n = read(sockfd, buf, 450);
        printf("Read %d bytes\n", n);
        message resp;
        memcpy(&resp, buf, sizeof(message));
        print_message(&resp);
    }

    if (n < 0)
        error("ERROR reading from socket");
    // printf("Echo from server: %s", resp.source);
    close(sockfd);
    return 0;
}