#include "dispatch.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>

#define MSG_SIZE sizeof(message)
#define SERVER "Server"
#define HDR_SIZE sizeof(header)
#define MSG_ONLY_SIZE (MSG_SIZE - HDR_SIZE)

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
    message *m = calloc(MSG_SIZE, sizeof(char));
    assert(m != NULL);

    m->h.type = type;
    strcpy(m->h.source, src);
    strcpy(m->h.dest, dest);
    m->h.length = length;
    m->h.message_id = message_id;

    if (data != NULL) {
        memcpy(m->data, data, m->h.length);
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
    printf("\nSending response to client\n");
    print_message(m);

    // may need to reevaluate the write size;
    int write_size = sizeof(m->h) + m->h.length;

    // Putting bytes for int fields in Network order
    header_host_to_net(&m->h);

    char buf[write_size];
    memset(buf, 0, write_size);
    memcpy(buf, &m->h, write_size);

    // NOTE: This cannot be blocking. If this blocks we have a big problem
    // printf("Trying to write %d bytes\n", write_size);
    int n = write(fd, buf, write_size);

    if (n < 0)
        error("Error writing to socket");
    
    return n;
}

/* Class specific functions below */
Dispatch_T *new_dispatch()
{
    Dispatch_T *dispatch = malloc(sizeof(Dispatch_T));
    assert(dispatch != NULL);
    dispatch->clients = new_list();
    dispatch->table = new_table();
    return dispatch;
}

void free_dispatch(Dispatch_T **dispatch)
{
    assert(dispatch != NULL && *dispatch != NULL);
    free_list(&(*dispatch)->clients);
    free_table(&(*dispatch)->table);
    free(*dispatch);
    *dispatch = NULL;
}

void remove_client_and_buffer(Dispatch_T *dispatch, int fd)
{
    free_buffered_message(dispatch->table, fd);
    remove_client_by_fd(dispatch->clients, fd);
}

// Validation helper functions to make sure messages are used correctly
bool check_client_not_named_server(message *m, int fd)
{
    if (strlen(m->h.source) == 6) {
        if (strcmp(m->h.source, "Server") == 0) {
            printf("Client is not allowed to be named 'Server'\n");
            return false;      // No need to write an error message here
        }
    }

    // If the length of the client name isn't 6, it can't be named Server
    return true;
}

bool validate_client_name(message *m, Dispatch_T *dispatch, int fd)
{
    // Verify source name is consistent with our records
    printf("Validating client name\n\n\n");
    char *name = get_clientID_from_fd(dispatch->clients, fd);
    if (name == NULL) {
        return false;   // client should exist by now
    }

    if (strlen(m->h.source) == strlen(name)) {
        if (strcmp(m->h.source, name) == 0) {
            return true;
        }
    }

    printf("Wrong name on file for the client\n");
    return false;
}

bool validate_client_name_for_hello(message *m, Dispatch_T *dispatch, int fd)
{
    // Verify source name is consistent with our records
    printf("Validating client name\n\n\n");
    char *name = get_clientID_from_fd(dispatch->clients, fd);
    if (name == NULL) {
        return true;   // client doesn't exist yet, assume name is valid
    }

    if (strlen(m->h.source) == strlen(name)) {
        if (strcmp(m->h.source, name) == 0) {
            return true;
        }
    }

    printf("Wrong name on file for the client\n");
    return false;
}

bool validate_destination_is_server(message *m, int fd)
{
    // returns true if the message was addressed to the server
    if (strlen(m->h.dest) == 6) {
        if (strcmp(m->h.dest, "Server") == 0) {
            return true; 
        }
    }

    return false;
}

bool validate_length_zero(message *m, int fd)
{
    // length field
    if (m->h.length != 0) {
        printf("List req should not have any length component\n");
        return false;
    }

    return true;
}

bool validate_message_id_zero(message *m, int fd)
{
    // message ID field
    // Verify client is not trying to use message ID
    if (m->h.message_id != 0) {
        return false;
    }

    return true;
}

bool validate_dest_not_src(message *m, int fd)
{
    // returns true if the message was addressed to the server
    if (strlen(m->h.source) == strlen(m->h.dest)) {
        if (strcmp(m->h.source, m->h.dest) == 0) {
            return false;
        }
    }

    return true;
}

bool validate_chat_size_less_than_400(message *m, int fd)
{
    if (m->h.length > 400) {
        return false;
    }

    return true;
}

// Handler helper functions
int handle_list_request(message *m, Dispatch_T *dispatch, int fd)
{
    // Verify client is connected
    if (!existing_client(dispatch->clients, NULL, fd)) {
        return -1;
    }

    // Verify source name is consistent with our records
    if (!validate_client_name(m, dispatch, fd))
        return -1;

    // dest field
    // Verify LISTREQ is addressed to the server
    if (strcmp(m->h.dest, "Server") != 0) {
        return -1;  // LISTREQ must be addressed to the server
    }

    // length field
    if (m->h.length > 0) {
        return -1;
    }

    // message ID field
    // Verify client is not trying to use message ID
    if (m->h.message_id != 0) {
        return -1;
    }

    int size = 0;
    char *data = return_contents(dispatch->clients, &size);


    message *list_msg = new_message(4, "Server", m->h.source, size, 0, data);
    write_message_to_client(list_msg, fd);

    free_message(&list_msg);
    free(data);

    return 0;
}

int handle_hello(message *m, Dispatch_T *dispatch, int fd)
{
    // Message Type checked already

    // Source
    if (!check_client_not_named_server(m, fd))
        return -1;

    if (!validate_client_name_for_hello(m, dispatch, fd))
        return -1;

    // Dest
    if (!validate_destination_is_server(m, fd))
        return -1;

    // Length
    if (!validate_length_zero(m, fd))
        return -1;

    // Message ID
    if (!validate_message_id_zero(m, fd))
        return -1;

    // 1. Check if client ID already exists
    if (existing_client(dispatch->clients, m->h.source, -1)) {
        printf("\nCLIENT EXISTS ALREADY\n");
        // If it exists, send the client an error message
        message *err_resp = new_message(7, SERVER, m->h.source, 0, 0, NULL);
        
        write_message_to_client(err_resp, fd);

        free_message(&err_resp);
        return -1;
    }

    // 2. If new client ID, add it to the list
    add_client(dispatch->clients, m->h.source, fd);

    // 3. Send back a hello ACK
    message *resp = new_message(2, SERVER, m->h.source, 0, 0, NULL);
    write_message_to_client(resp, fd);

    free_message(&resp);

    // 4. Send a client list message (including the client that just joined)
    handle_list_request(m, dispatch, fd);

    return 0;
}

int handle_exit(message *m, Dispatch_T *dispatch, int fd)
{
    // No matter how messed up the exit message is, we're removing the client
    // anyway, so don't bother checking anything
    // line below isn't necessary, happens elsewhere
    // remove_client_and_buffer(dispatch, fd);
    return -1;
}

int handle_chat(message *m, Dispatch_T *dispatch, int source_fd)
{
    // Source
    if (!validate_client_name(m, dispatch, source_fd))
        return -1;
    

    // Dest
    if (validate_destination_is_server(m, source_fd)) {
        printf("Client is not allowed to chat with the server'\n");
        return -1;
    }

    if (!validate_dest_not_src(m, source_fd)) {
        printf("Client is not allowed to chat with themselves\n");
        return -1;
    }

    if (!validate_chat_size_less_than_400(m, source_fd)) {
        printf("\n BADTrying to send too big a message\n");
        return -1;
    }


    // get the active socket associated with the destination clientID
    int dest_fd = get_fd_from_clientID(dispatch->clients, m->h.dest);

    int n = 0;
    if (dest_fd >= 0) {
        // write the message to the destination socket
        n = write_message_to_client(m, dest_fd);
        if (n > 0) {    // if message was successfully sent
            return 0;
        }
    }

    /* If the destination fd doesn't exist, or if chat cannot be delivered:
     * Send error message to the source client, with the message ID that wasn't
     * delivered */

    printf("Message id is %d\n", m->h.message_id);
    printf("Modified Message id is %d\n", ntohl(m->h.message_id));


    message *err_resp = new_message(8, SERVER, m->h.source, 0, 
        m->h.message_id, NULL);
    
    write_message_to_client(err_resp, source_fd);

    free(err_resp);

    return -1; //TODO
}

int parse_message(message *m, Dispatch_T *dispatch, int fd)
{
    switch (m->h.type) {
        case 1:
            // HELLO from client
            return handle_hello(m, dispatch, fd);
        case 2:
            // SERVER SHOULD NOT RECEIVE THIS
            printf("Hello ack for server use only");
            return -1;
        case 3:
            // LIST REQUEST from client
            return handle_list_request(m, dispatch, fd);
        case 4:
            // SERVER SHOULD NOT RECEIVE THIS
            printf("Client list for server use only");
            return -1;
        case 5:
            // CHAT from client
            return handle_chat(m, dispatch, fd);
        case 6:
            // EXIT from client
            return handle_exit(m, dispatch, fd);    // always returns -1
        case 7:
            // SERVER SHOULD NOT RECEIVE THIS
            printf("Client present error for server use only");
            return -1;
        case 8:
            // SERVER SHOULD NOT RECEIVE THIS
            printf("Cannot deliver for server use only");
            return -1;
        default:
            // SERVER SHOULD NOT RECEIVE THIS
            printf("Message type not recognized %d\n", m->h.type);
            return -1;
    }

    return 0;

}

header* get_header(int fd, Dispatch_T *dispatch, int size_pending, 
                   char *pending_bytes, int *exit_code, bool exists)
{
    char buf[HDR_SIZE];
    memset(buf, 0, HDR_SIZE);

    if (size_pending >= HDR_SIZE) {    // The whole header was already buffered
        header *h = calloc(HDR_SIZE, sizeof(char));
        assert(h != NULL);

        memcpy(h, pending_bytes, HDR_SIZE);    // Header already in host format
        return h;
    } 

    // If any part of the header buffered, add it before we start reading
    memcpy(buf, pending_bytes, size_pending);
    int n = read(fd, buf + size_pending, HDR_SIZE - size_pending);
    if (n <= 0) {
        *exit_code = n;     // setting exit code will ensure client gets removed
        printf("Reached EOF, remove client\n");
        return NULL;
    }

    int hdr_bytes_read = n + size_pending;

    if (hdr_bytes_read < HDR_SIZE) {    // We only got part of the header
    
        char *save = calloc(hdr_bytes_read, sizeof(char));
        assert(save != NULL);
        memcpy(save, buf, hdr_bytes_read);

        printf("Only recieved part of the header %d bytes\n", hdr_bytes_read);
        buffer_message(dispatch->table, fd, hdr_bytes_read, save);
        return NULL;
    }

    header *h = calloc(HDR_SIZE, sizeof(char));
    assert(h != NULL);

    memcpy(h, buf, HDR_SIZE);

    header_net_to_host(h); // Now that the header is complete, convert to host
    return h;
}

message* get_message(int fd, Dispatch_T *dispatch, size_t msg_size_pending, 
    char *pending_msg, header *h, int *exit_code, bool exists)
{
    char buf[MSG_SIZE];     // Buffer will contain message and header
    memset(buf, 0, MSG_SIZE);

    int target_size = h->length;
    message *m = NULL;

    if (target_size == 0) {
        m = calloc(MSG_SIZE, sizeof(char));
        assert(m != NULL);

        printf("\nRecieved a header-only message\n");
        
        memcpy(m, h, HDR_SIZE);
        free(h);    // free the heap alloc'd header
        return m;
    }

    assert(msg_size_pending < target_size);    // assert msg buffered correctly
    int bytes_buffered = 0;

    memcpy(buf, h, HDR_SIZE);   // copy the header into the new buffer
    bytes_buffered += HDR_SIZE;
    free(h);    // free the heap alloc'd header

    // copy over any additional pending bytes into the new buffer
    memcpy(buf + bytes_buffered, pending_msg, msg_size_pending);
    bytes_buffered += msg_size_pending;

    int new_target_size = target_size - msg_size_pending;
    int nbytes = read(fd, buf + bytes_buffered, new_target_size);
    bytes_buffered += nbytes;

    if (nbytes <= 0) {
        *exit_code = -1;
        printf("Reached EOF, remove client\n");
        return NULL;
    }

    if (nbytes < new_target_size) {
        printf("Buffering the unfinished message\n");
        // buffering the ENTIRE unfinished message, including the header
        char *save = calloc(bytes_buffered, sizeof(char));
        assert(save != NULL);
        memcpy(save, buf, bytes_buffered);

        buffer_message(dispatch->table, fd, bytes_buffered, save);

        return NULL;
    }

    printf("\nRecieved a content message\n");
    m = calloc(MSG_SIZE, sizeof(char));
    assert(m != NULL);

    memcpy(&m->h, buf, HDR_SIZE);
    memcpy(&m->h + 1, buf + HDR_SIZE, target_size);

    return m;
}

char* get_pending_bytes(int fd, size_t *len, Dispatch_T *dispatch)
{
    char *msg = get_buffered_message(dispatch->table, fd, len);
    printf("Getting %ld bytes that were buffered for this client\n", *len);
    return msg;
}

int read_message(int fd, Dispatch_T *dispatch)
{
    bool client_exists = existing_client(dispatch->clients, NULL, fd);
    if (!client_exists)
        printf("\nClient not recognized\n");

    size_t size_pending = 0;    // Check if there are pending bytes
    char *pending_bytes = get_pending_bytes(fd, &size_pending, dispatch);

    int bytes_read = 1;     // Try to read the header in:
    header *h1 = get_header(fd, dispatch, size_pending, pending_bytes, 
        &bytes_read, client_exists);
    if (h1 == NULL) {
        free(pending_bytes);
        if (bytes_read <= 0) {
            // No bytes read, erase buffers and disconnect client
            remove_client_and_buffer(dispatch, fd);
            return -1;
        }

        return 0;   // Header incomplete, bytes were buffered
    }

    printf("Finished reading the header\n");

    size_t msg_size_pending = 0;
    char *pending_msg = NULL;

    if (size_pending > HDR_SIZE) {    // If more than just header was buffered
        msg_size_pending = size_pending - HDR_SIZE;
        pending_msg = pending_bytes + HDR_SIZE;
    }

    int exit_code = 1;
    message *m1 = get_message(fd, dispatch, msg_size_pending, pending_msg, h1, 
        &exit_code, client_exists);
    if (m1 == NULL) {
        free(pending_bytes);
        if (exit_code < 0) {
            // Message header was invalid in some way, remove client
            remove_client_and_buffer(dispatch, fd);
            return -1;
        }
        return 0;   // Message incomplete, bytes were buffered
    }

    // The message was successfully read in 
    print_message(m1);
    int n = parse_message(m1, dispatch, fd);

    free_message(&m1);
    free(pending_bytes);

    if (n < 0) {
        printf("\nRemoving the client and buffer\n");
        remove_client_and_buffer(dispatch, fd);
    }

    return n;
}