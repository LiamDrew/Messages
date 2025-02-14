#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <assert.h>
#include <stdbool.h>
#include <ctype.h>

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

/* UTILITIES*/
#define BUFSIZE 1024
// #define IP "10.4.2.20"
// #define PORT 9052
#define MSG_SIZE sizeof(message)
#define HDR_SIZE sizeof(header)
#define SERVER "Server"

/* METHOD DEFINITIONS */

void strip_newline(char *str, int max_size);

void strip_newline(char *str, int max_size)
{
    if (str == NULL)
        return;

    for (int i = 0; i < max_size; i++)
    {
        if (str[i] == '\n')
        {
            str[i] = '\0';
            return;
        }
    }

    // should have found a newline by now
    assert(false);
}

char **get_client_list(message *l)
{
    int l_size = 10;
    char **clients = calloc(l_size, sizeof(char *));
    assert(clients != NULL);

    int j = 0;

    // read through the data that was send by server
    char *buf = calloc(20, sizeof(char));
    assert(buf != NULL);
    int k = 0;

    for (int i = 0; i < l->h.length + 1; i++) {

        if (l->data[i] == '\0') {
            buf[k] = '\0';
            k = 0;
            clients[j] = buf;
            j++;
            buf = calloc(20, sizeof(char));
        }
        else {
            buf[k] = l->data[i];
            k++;
        }
    }

    free(buf);
    return clients;
}

void free_client_list(char **clients)
{
    assert(clients != NULL);

    int i = 0;
    while (clients[i] != NULL)
    {
        free(clients[i]);
        clients[i] = NULL;
        i++;
    }

    free(clients);
}

void error(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void print_message(message *m)
{
    printf("Type: %d, Source: %s, Dest: %s, Length: %d, ID: %d\n",
           m->h.type, m->h.source, m->h.dest, m->h.length, m->h.message_id);
    
    if (m->h.length > 0) {
        printf("Message content is: ");
        for (int offset = 0; offset < m->h.length; offset++) {
            if (m->data[offset] == '\0') {
                putchar('.');
            }

            else {
                putchar(m->data[offset]);
            }
        }

        printf("\n");
    }
}

message *new_message(short int type, char *src, char *dest, int length,
                     int message_id, char *data)
{
    message *m = malloc(MSG_SIZE);
    assert(m != NULL);
    memset(m, 0, MSG_SIZE);

    m->h.type = type;
    strcpy(m->h.source, src);
    strcpy(m->h.dest, dest);
    m->h.length = length;
    m->h.message_id = message_id;

    if (data != NULL)
    {
        strcpy(m->data, data);
    }

    return m;
}

void free_message(message **m)
{
    assert(m != NULL && *m != NULL);
    free(*m);
    *m = NULL;
}

void header_host_to_net(header *h)
{
    h->type = htons(h->type);
    h->length = htonl(h->length);
    h->message_id = htonl(h->message_id);
}

void header_net_to_host(header *h)
{
    h->type = ntohs(h->type);
    h->length = ntohl(h->length);
    h->message_id = ntohl(h->message_id);
}

int write_message_to_client(message *m, int fd)
{
    int len = m->h.length;
    int write_size = HDR_SIZE + len;

    // Putting bytes for int fields in Network order
    header_host_to_net(&m->h);

    char buf[write_size];
    memset(buf, 0, write_size);
    memcpy(buf, m, write_size);

    // NOTE: This cannot be blocking. If this blocks we have a big problem
    int n = write(fd, buf, write_size);

    if (n < 0)
        error("Error writing to socket");

    return n;
}


message *read_message_from_server(int fd)
{
    // Read the header first
    char head_buf[HDR_SIZE];
    memset(head_buf, 0, HDR_SIZE);

    int nbytes = read(fd, head_buf, HDR_SIZE);
    if (nbytes < 0) {
        error("Read error");
    } else if (nbytes == 0) {
        error("Reached EOF while reading header\n");
    }

    header *h = calloc(HDR_SIZE, sizeof(char));
    assert(h != NULL);
    memcpy(h, head_buf, HDR_SIZE);

    // Convert the header from network byte order to host byte order
    header_net_to_host(h);

    // Allocate memory for message
    message *m = calloc(MSG_SIZE, sizeof(char));
    assert(m != NULL);

    // Copy the header into the message
    memcpy(&m->h, h, HDR_SIZE);
    free(h);

    // If the message has data, read it
    if (m->h.length > 0) {
        nbytes = read(fd, m->data, m->h.length);
        if (nbytes < 0) {
            error("Read error");
            free(m);
            return NULL;
        } else if (nbytes == 0) {
            printf("Reached EOF\n");
            free(m);
            error("Wrong number of bytes in the message");
        }
    }

    return m;
}

int write_partial_message_to_client(message *m, int fd, time_t delay_s, int first_break, int second_break)
{
    int len = m->h.length;
    int write_size = HDR_SIZE + len;

    // Putting bytes for int fields in Network order
    header_host_to_net(&m->h);

    char buf[write_size];
    memset(buf, 0, write_size);
    memcpy(buf, m, write_size);

    // Here, we have the message in byte form. Now we can start screwing around
    // with writes
    printf("Delay: %ld, First breakpoint: %d, Second breakpoint: %d\n", delay_s, first_break, second_break);

    // printf("Write size is: %d\n", write_size);
    int remaining_size = write_size;
    int bytes_written = 0;
    int n = 0;


    if (first_break != -1) {
        n = write(fd, buf, first_break);
        if (n < 0)
            error("Error writing to socket");
        
        remaining_size -= first_break;
        bytes_written += first_break;

        // printf("\nWritten %d bytes so far\n", bytes_written);
        // printf("Sleeping for %ld seconds\n", delay_s);
        sleep(delay_s);

        if (second_break != -1) {
            n = write(fd, buf + bytes_written, second_break - first_break);
            if (n < 0)
                error("Error writing to socket");
            
            remaining_size -= (second_break - first_break);
            bytes_written += (second_break - first_break);

            // printf("\nWritten %d bytes so far\n", bytes_written);
            // printf("Sleeping for %ld seconds\n", delay_s);
            sleep(delay_s);

            n = write(fd, buf + bytes_written, remaining_size);
            if (n < 0)
                error("Error writing to socket");
            bytes_written += remaining_size;
            // printf("\nSent a total of %d bytes\n", bytes_written);
        }

        else {
            n = write(fd, buf + bytes_written, remaining_size);
            if (n < 0)
                error("Error writing to socket");
            bytes_written += remaining_size;
        }
    }

    else {
        if (second_break != -1) {
            // need to write everything to the second break;
            n = write(fd, buf + bytes_written, second_break);
            if (n < 0)
                error("Error writing to socket");
            
            remaining_size -= (second_break);
            bytes_written += (second_break);

            printf("Second breakpoint only\n");
            printf("\nWritten %d bytes so far\n", bytes_written);
            printf("Sleeping for %ld seconds\n", delay_s);
            sleep(delay_s);

            n = write(fd, buf + bytes_written, remaining_size);
            if (n < 0)
                error("Error writing to socket");
            bytes_written += remaining_size;
            // printf("\nSent a total of %d bytes\n", bytes_written);
        }

        else {
            // just write the entire message
            n = write(fd, buf + bytes_written, remaining_size);
            if (n < 0)
                error("Error writing to socket");
            bytes_written += remaining_size;
        }

    }




    return 0;
}

int send_message(char *name, int fd)
{
    // first, confirm that the user actually wants to send a message
    char buf[20];
    memset(buf, 0, 20);

    if (fgets(buf, sizeof(buf), stdin) != NULL) {
        strip_newline(buf, sizeof(buf));
    }
    printf("Command: %s\n\n", buf);

    if (strcmp("PAUSE", buf) == 0) {
        // pause stdin
        printf("Trying to pause\n");
        return -1;
    }

    // Client wants to send message
    else if (strcmp("MSG", buf) == 0) {
        printf("Who are you sending a message to?\n");
        memset(buf, 0, 20);
        
        if (fgets(buf, sizeof(buf), stdin) != NULL) {
            strip_newline(buf, sizeof(buf));
        }
        printf("Recipient: %s\n\n", buf);

        printf("What is the message?\n");
        char bigbuf[100];
        memset(bigbuf, 0, 100);

        if (fgets(bigbuf, sizeof(bigbuf), stdin) != NULL) {
            strip_newline(bigbuf, sizeof(bigbuf));
        }

        // printf("Message: %s\n\n", bigbuf);

        message *m = new_message(5, name, buf, 100, 1, bigbuf);
        write_message_to_client(m, fd);
        free_message(&m);
    }

    // Client wants the list of active users
    else if (strcmp("LISTREQ", buf) == 0) {
        message *m = new_message(3, name, SERVER, 0, 0, NULL);
        write_message_to_client(m, fd);
        free_message(&m);
    }

    // Client wants to exist
    else if (strcmp("EXIT", buf) == 0) {
        message *m = new_message(6, name, SERVER, 0, 0, NULL);
        write_message_to_client(m, fd);
        free_message(&m);
    }

    // Client wants to say hello
    // For testing purposes only
    else if (strcmp("HELLO", buf) == 0) {
        message *m = new_message(1, name, SERVER, 0, 0, NULL);
        write_message_to_client(m, fd);
        free_message(&m);
    }

    else if (strcmp("PARTIAL", buf) == 0) {
        // message *m = new_message(5, name, name, 13, 1, "Hello there");
        // write_partial_message_to_client(m, fd, 3, 7);
        // free_message(&m);

        printf("Who are you sending a message to?\n");
        memset(buf, 0, 20);
        
        if (fgets(buf, sizeof(buf), stdin) != NULL) {
            strip_newline(buf, sizeof(buf));
        }
        printf("Recipient: %s\n\n", buf);


        printf("What is the message?\n");
        char bigbuf[400];
        memset(bigbuf, 0, 400);

        if (fgets(bigbuf, sizeof(bigbuf), stdin) != NULL) {
            strip_newline(bigbuf, sizeof(bigbuf));
        }
        // printf("Message: %s\n\n", bigbuf);


        printf("Where do you want the first breakpoint?\n");
        char b1[20];
        memset(b1, 0, 20);
        if (fgets(b1, sizeof(b1), stdin) != NULL) {
            strip_newline(b1, sizeof(b1));
        }

        printf("Where do you want the second breakpoint?\n");
        char b2[20];
        memset(b2, 0, 20);
        if (fgets(b2, sizeof(b2), stdin) != NULL) {
            strip_newline(b2, sizeof(b2));
        }

        char b3[20];
        printf("How long a delay between messages?\n");
        memset(b3, 0, 20);
        if (fgets(b3, sizeof(b3), stdin) != NULL) {
            strip_newline(b3, sizeof(b3));
        }

        int break1 = atoi(b1);
        int break2 = atoi(b2);
        int delay = atoi(b3);

        message *m = new_message(5, name, buf, strlen(bigbuf) + 1, 1, bigbuf);

        write_partial_message_to_client(m, fd, delay, break1, break2);
        free_message(&m);
    }

    return 0;
 
}


/* MAIN */
int main(int argc, char **argv)
{
    int sockfd, portno;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;

    /* check command line arguments */
    if (argc != 4) {
        fprintf(stderr, "usage: %s <name> <hostname> <port>\n", argv[0]);
        return 0;
    }

    char *name = argv[1];
    hostname = argv[2];
    portno = atoi(argv[3]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
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



    // Manual communication setup if necessary
    // 1. Send Hello Message
    // write hello as a partial message to the client
    message *m = new_message(1, name, SERVER, 0, 0, NULL);
    write_partial_message_to_client(m, sockfd, 3, 25, -1);
    // write_message_to_client(m, sockfd);
    free_message(&m);

    // 2. Get back Hello Ack
    message *r = read_message_from_server(sockfd);
    assert(r->h.type == 2);     // assert that the message is a hello ack
    free_message(&r);

    // 3. Get back client list
    message *l = read_message_from_server(sockfd);
    assert(l->h.type == 4);      // assert that the message is a client list

    char **clients = get_client_list(l);
    free_message(&l);

    printf("Client List: ");
    int i = 0;
    while(clients[i] != NULL) {
        printf("%s ", clients[i]);
        i++;
    }
    printf("\n");

    // 4. Now that communication has been established, use select to wait for 
    // input and do stuff

    fd_set active_fd_set, read_fd_set;

    FD_ZERO (&active_fd_set);
    FD_SET (sockfd, &active_fd_set);
    FD_SET (STDIN_FILENO, &active_fd_set);

    bool stdin_paused = false;

    while (true) {

        if (stdin_paused) {
            printf("\nWaiting for server message:\n");
        } else {
            printf("\nEnter Command OR read server response:\n");
        }

        read_fd_set = active_fd_set;
        if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
            error("error with select");
        }

        /* Read from the server, or STDIN if it is active */
        for (int i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &read_fd_set)) {
                if (i == sockfd) {
                    /* connection coming in from server */
                    message *m = read_message_from_server(i);
                    printf("\nReceived message from the server:\n");
                    print_message(m);
                    free_message(&m);

                    // unpause STDIN so we can respond
                    if (stdin_paused) {
                        printf("Input Reactivated\n");
                        stdin_paused = false;
                        FD_SET(STDIN_FILENO, &active_fd_set);   //reactivate
                    }


                } else if (i == STDIN_FILENO) {
                    if (stdin_paused) {
                        //trying to read from stdin while paused
                        continue;
                        // assert(false);
                    }
                    /* connection coming in from stdin */
                    if (feof(stdin)) {
                        FD_CLR(STDIN_FILENO, &active_fd_set);

                        // shut down the program;
                        printf("\nShutting down the client.\n\n");
                        // free_client_list(clients);
                        close(sockfd);
                        return 0;
                    } 

                    int resp = send_message(name, sockfd);
                    if (resp < 0) {
                        printf("Input Deactivated\n");
                        stdin_paused = true;
                        FD_CLR(STDIN_FILENO, &active_fd_set);   // close
                    }        

                } else {
                    error("Unexpected file descriptor\n");
                }
            }
        }
    }


    printf("message sending done\n");

    // 5. Shut down
    // free_client_list(clients);
    close(sockfd);
    return 0;
}