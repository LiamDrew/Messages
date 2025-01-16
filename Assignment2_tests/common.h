#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define HELLO 1
#define HELLO_ACK 2
#define LIST_REQUEST 3
#define CLIENT_LIST 4
#define CHAT 5
#define EXIT 6
#define ERROR_ALREADY_PRESENT 7
#define ERROR_CANNOT_DELIVER 8
#define HELLO_ALR_PRESENT 10

#define MAX_PACKET_SIZE 400
#define SOURCE_LENGTH 20
#define HEADER_SIZE sizeof(header)

typedef struct __attribute__((__packed__)) header {
    short int Type; 
    char source[SOURCE_LENGTH];
    char destination[SOURCE_LENGTH];
    uint32_t Length; 
    uint32_t Message_ID;
} header; 

typedef struct __attribute__((__packed__)) message {
    header header;
    char content[MAX_PACKET_SIZE];
} message;

void error(const char *msg);

void clear_input_buffer();

void print_client_list(const char *client_list_buffer, size_t length);

message create_hello_message(char *clienID);

message create_client_list_message(char *clientID);

message create_exit_message(char *clientID);

message create_chat_message(char *clientID);

void handle_message_from_server(char *buffer, short int msg_type);

size_t handle_type_choice(int fd, short int msg_type, char *clientID);
