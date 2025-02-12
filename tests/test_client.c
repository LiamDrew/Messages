// This is Ismail's test client
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <ctype.h>

#define MAX_DATA_SIZE 400

// Message header structure
struct __attribute__((__packed__)) message_header {
    unsigned short type;        // 2 bytes
    char source[20];            // 20 bytes (ClientID)
    char destination[20];       // 20 bytes (should be "Server" or other client ID)
    unsigned int length;        // 4 bytes (length of the data, can be > 0)
    unsigned int message_id;    // 4 bytes (message ID, should be >= 1 for CHAT)
};

// Complete message structure (header + data if any)
struct __attribute__((__packed__)) message {
    struct message_header header;
    char data[MAX_DATA_SIZE];  // Maximum data size for messages
};

// Function to create and send a CHAT message that can handle binary data
int send_chat_message(int sockfd, const char *client_id, const char *dest_client_id, const void *chat_data, size_t data_len, unsigned int msg_id) {
    struct message msg;

    // Prepare the CHAT message header
    msg.header.type = htons(5);  // CHAT message type
    strncpy(msg.header.source, client_id, sizeof(msg.header.source) - 1);
    msg.header.source[sizeof(msg.header.source) - 1] = '\0';  // Ensure null-termination
    strncpy(msg.header.destination, dest_client_id, sizeof(msg.header.destination) - 1);
    msg.header.destination[sizeof(msg.header.destination) - 1] = '\0';  // Ensure null-termination
    msg.header.message_id = htonl(msg_id);  // Set the message ID

    // Validate the length of the binary data
    if (data_len > MAX_DATA_SIZE) {
        fprintf(stderr, "Chat message data too long (max %d bytes)\n", MAX_DATA_SIZE);
        return -1;
    }

    // Copy the binary data into the message
    memcpy(msg.data, chat_data, data_len);
    msg.header.length = htonl(data_len);  // Set the data length in the header

    // Send the CHAT message (header + binary data)
    ssize_t bytes_sent = write(sockfd, &msg, sizeof(msg.header) + data_len);
    if (bytes_sent == -1) {
        perror("send_chat_message: Error sending CHAT message");
        return -1;
    }

    printf("Sent CHAT message to %s (%zu bytes of data)\n", dest_client_id, sizeof(msg.header) + data_len);
    return 0;
}

// Function to create and send a CHAT message header first, then the data after a delay
// Function to send a message in multiple packets with a 45-second delay between each
int send_chat_message_delayed_in_packets(int sockfd, const char *client_id, const char *dest_client_id, const void *chat_data, size_t data_len, unsigned int msg_id, size_t packet_size) {
    struct message msg;

    // Prepare the CHAT message header
    msg.header.type = htons(5);  // CHAT message type
    strncpy(msg.header.source, client_id, sizeof(msg.header.source) - 1);
    msg.header.source[sizeof(msg.header.source) - 1] = '\0';  // Ensure null-termination
    strncpy(msg.header.destination, dest_client_id, sizeof(msg.header.destination) - 1);
    msg.header.destination[sizeof(msg.header.destination) - 1] = '\0';  // Ensure null-termination
    msg.header.message_id = htonl(msg_id);  // Set the message ID
    msg.header.length = htonl(data_len);  // Set the data length in the header

    // First, send the header
    ssize_t header_bytes_sent = write(sockfd, &msg.header, sizeof(msg.header));
    if (header_bytes_sent == -1) {
        perror("send_chat_message_delayed_in_packets: Error sending CHAT message header");
        return -1;
    }

    printf("Sent CHAT message header to %s, waiting 45 seconds before sending data...\n", dest_client_id);

    // Now, send the data in multiple packets, with a delay between each packet
    size_t bytes_sent = 0;
    const char *data_ptr = chat_data;

    while (bytes_sent < data_len) {
        // Calculate the size of the current packet
        size_t current_packet_size = (data_len - bytes_sent > packet_size) ? packet_size : data_len - bytes_sent;

        // Send the current packet
        ssize_t current_bytes_sent = write(sockfd, data_ptr + bytes_sent, current_packet_size);
        if (current_bytes_sent == -1) {
            perror("send_chat_message_delayed_in_packets: Error sending CHAT message data");
            return -1;
        }

        bytes_sent += current_bytes_sent;
        printf("Sent %zu bytes of CHAT message data to %s, waiting 45 seconds before next packet...\n", current_bytes_sent, dest_client_id);

        // Wait for 45 seconds before sending the next packet
        sleep(45);
    }

    printf("Sent entire CHAT message data to %s (%zu bytes of data)\n", dest_client_id, data_len);
    return 0;
}



// Function to send the four CHAT messages (modified to handle binary data)
void send_chat_messages(int sockfd, const char *client_id) {
    // Message 1: Send a string message as binary data
    const char *msg1 = "Hello my friend";
    const char *my_client = "Liam";
    send_chat_message(sockfd, client_id, my_client, msg1, strlen(msg1), 1);
    sleep(1);  // Add a slight delay between messages

    // Send the message header, wait 45 seconds, then send the data
    send_chat_message_delayed_in_packets(sockfd, client_id, my_client, msg1, 60, 1, 5); // 5 bytes per packet

    // Message 2: Another string message
    const char *msg2 = "Bonjour mon ami";
    send_chat_message(sockfd, client_id, my_client, msg2, strlen(msg2), 2);
    sleep(1);  // Add a slight delay between messages

    // Message 3: Send a binary message with null byte in the middle
    unsigned char msg3[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x00, 0x48, 0x61, 0x62, 0x69, 0x62, 0x69};  // "Hello\0Habibi"
    send_chat_message(sockfd, client_id, my_client, msg3, sizeof(msg3), 3);
    sleep(1);  // Add a slight delay between messages

    // Message 4: Send another string message with a zero message ID (optional case)
    const char *msg4 = "Ciao amico";
    send_chat_message(sockfd, client_id, my_client, msg4, strlen(msg4), 4); // This is allowed if msg_id logic is correctly set to 1+ as needed.
    sleep(1);  // Add a slight delay between messages

    // Message 5: Another message
    const char *msg5 = "Hola amigo";
    send_chat_message(sockfd, client_id, my_client, msg5, strlen(msg5), 5);
}



// Function to create and send a HELLO message
int send_hello_message(int sockfd, const char *client_id) {
    struct message msg;

    // Prepare the HELLO message header
    msg.header.type = htons(1);  // HELLO message type
    strncpy(msg.header.source, client_id, sizeof(msg.header.source) - 1);
    msg.header.source[sizeof(msg.header.source) - 1] = '\0';  // Ensure null-termination
    strncpy(msg.header.destination, "Server", sizeof(msg.header.destination) - 1);
    msg.header.destination[sizeof(msg.header.destination) - 1] = '\0';  // Ensure null-termination
    msg.header.length = htonl(0);  // No data part
    msg.header.message_id = htonl(0);  // HELLO message should have message_id == 0

    // Send the HELLO message
    ssize_t bytes_sent = write(sockfd, &msg, sizeof(msg.header));
    if (bytes_sent == -1) {
        perror("send_hello_message: Error sending HELLO message");
        return -1;
    }

    printf("Sent HELLO message to server\n");
    return 0;
}

int send_exit_message(int sockfd, const char *client_id) {
    struct message msg;

    // Prepare the EXIT message header
    msg.header.type = htons(6);  // EXIT message type
    strncpy(msg.header.source, client_id, sizeof(msg.header.source) - 1);
    msg.header.source[sizeof(msg.header.source) - 1] = '\0';  // Ensure null-termination
    strncpy(msg.header.destination, "Server", sizeof(msg.header.destination) - 1);
    msg.header.destination[sizeof(msg.header.destination) - 1] = '\0';  // Ensure null-termination
    msg.header.length = htonl(0);  // No data part
    msg.header.message_id = htonl(0);  // EXIT message should have message_id == 0

    // Send the EXIT message
    ssize_t bytes_sent = write(sockfd, &msg, sizeof(msg.header));
    if (bytes_sent == -1) {
        perror("send_exit_message: Error sending EXIT message");
        return -1;
    }

    printf("Sent EXIT message to server\n");
    return 0;
}

// Function to create and send a LIST_REQUEST message
int send_list_request(int sockfd, const char *client_id) {
    struct message msg;

    // Prepare the LIST_REQUEST message header
    msg.header.type = htons(3);  // LIST_REQUEST message type
    strncpy(msg.header.source, client_id, sizeof(msg.header.source) - 1);
    // msg.header.source[sizeof(msg.header.source) - 1] = '\0';  // Ensure null-termination
    strncpy(msg.header.destination, "Server", sizeof(msg.header.destination) - 1);
    msg.header.destination[sizeof(msg.header.destination) - 1] = '\0';  // Ensure null-termination
    msg.header.length = htonl(0);  // No data part
    msg.header.message_id = htonl(0);  // LIST_REQUEST should have message_id == 0

    // Send the LIST_REQUEST message
    ssize_t bytes_sent = write(sockfd, &msg, sizeof(msg.header));
    if (bytes_sent == -1) {
        perror("send_list_request: Error sending LIST_REQUEST message");
        return -1;
    }

    printf("Sent LIST_REQUEST message to server\n");
    return 0;
}

// Function to read a message from the server
int read_message(int sockfd, struct message *msg) {
    // Read the message header in a loop to ensure we get the full header
    ssize_t bytes_received;
    ssize_t bytes_left = sizeof(msg->header);
    while (bytes_left > 0) {
        bytes_received = read(sockfd, ((char*)&msg->header) + (sizeof(msg->header) - bytes_left), bytes_left);
        if (bytes_received <= 0) {
            perror("read_message: Error reading message header from server");
            return -1;
        }
        bytes_left -= bytes_received;
    }

    // Convert fields from network byte order to host byte order
    msg->header.type = ntohs(msg->header.type);
    msg->header.length = ntohl(msg->header.length);
    msg->header.message_id = ntohl(msg->header.message_id);

    printf("Length of the received message: %d.\n", msg->header.length);

    // If there is a data part, read the data
    if (msg->header.length > 0) {
        bytes_left = msg->header.length;
        while (bytes_left > 0) {
            bytes_received = read(sockfd, msg->data + (msg->header.length - bytes_left), bytes_left);
            if (bytes_received <= 0) {
                perror("read_message: Error reading message data from server");
                return -1;
            }
            bytes_left -= bytes_received;
        }
        msg->data[msg->header.length] = '\0';  // Null-terminate the data
    } else {
        printf("No data sent.\n");
        msg->data[0] = '\0';  // No data
    }

    return 0;
}

// Function to print message details and validate data presence (binary-safe)
void handleMessageReceived(struct message msg) {
    printf("Message Details:\n");
    printf("Type: %u\n", msg.header.type);
    printf("Source: %s\n", msg.header.source);
    printf("Destination: %s\n", msg.header.destination);
    printf("Data Length: %u\n", msg.header.length);
    printf("Message ID: %u\n", msg.header.message_id);

    if (msg.header.length == 0) {
        printf("Data: NULL (No data as expected)\n");
    } else {
        printf("Message Data (Hexadecimal): ");
        for (unsigned int i = 0; i < msg.header.length && i < sizeof(msg.data); i++) {
            // Print each byte in hexadecimal format
            printf("%02X ", (unsigned char)msg.data[i]);
        }
        printf("\n");

        // Optionally, you can also print the data in a more human-readable way for printable characters
        printf("Message Data (Printable): ");
        for (unsigned int i = 0; i < msg.header.length && i < sizeof(msg.data); i++) {
            if (isprint((unsigned char)msg.data[i])) {
                putchar(msg.data[i]);  // Print printable characters as-is
            } else {
                putchar('.');  // Replace non-printable characters with a dot
            }
        }
        printf("\n");
    }
}


// Function to handle incoming messages from the server
void handle_server_response(int sockfd, const char *client_id) {
    struct message msg;

    while (1) {  // Infinite loop to keep reading messages from the server
        if (read_message(sockfd, &msg) == -1) {
            fprintf(stderr, "Server closed the connection or an error occurred.\n");
            break;  // Exit loop if there is an error or server closes connection
        }

        // Handle different message types
        handleMessageReceived(msg);  // Print message details

        switch (msg.header.type) {
            case 2:  // HELLO_ACK
                printf("Received HELLO_ACK from server\n");
                sleep(5);  // Wait for 5 seconds before sending the next LIST_REQUEST
                if (send_list_request(sockfd, msg.header.destination) == -1) {
                    fprintf(stderr, "Failed to send LIST_REQUEST after HELLO_ACK\n");
                }
                break;

            case 5:  // CHAT message
                printf("Received CHAT message from %s to %s\n", msg.header.source, msg.header.destination);
                printf("Message content: %.*s\n", msg.header.length, msg.data);
                break;

            case 8:  // ERROR message: CANNOT_DELIVER
                printf("Received CANNOT_DELIVER error for message ID: %u\n", msg.header.message_id);
                printf("Error Details: Could not deliver message from %s to %s\n", msg.header.source, msg.header.destination);
                break;

            case 7:  // CLIENT_ALREADY_PRESENT
                printf("Received CLIENT_ALREADY_PRESENT from server\n");
                break;

            case 4:  // CLIENT_LIST
                printf("Received CLIENT_LIST from server:\n");
                if (msg.header.length > 0) {
                    int i = 0;
                    while (i < msg.header.length) {
                        if (msg.data[i] == '\0') {
                            printf("\\0\n");
                        } else {
                            putchar(msg.data[i]);
                        }
                        i++;
                    }
                } else {
                    printf("Client list is empty.\n");
                }

                // Send EXIT message
                send_exit_message(sockfd, client_id);

                break;

            default:
                printf("Received unexpected message type: %d\n", msg.header.type);
        }
    }
}


int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <client_id> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server_ip = "10.4.2.20"; // IP address of the server
    long server_port = strtol(argv[2], NULL, 10);
    if (server_port <= 0 || server_port > 65535) {
        printf("Invalid port. Please provide a valid port number.\n");
        return 1;
    }
    const char *client_id = argv[1];  // Client ID provided as command-line argument

    // Create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set up the server address struct
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;  // IPv4
    server_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server at %s:%ld\n", server_ip, server_port);

    sleep(3);  // Wait for 3 seconds before sending the HELLO message

    if (send_hello_message(sockfd, client_id) == -1) {
        fprintf(stderr, "Failed to send HELLO message\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Send 5 chat messages (second one is delayed)
    send_chat_messages(sockfd, client_id);

    // Handle responses from the server (stay connected and keep reading in a loop)
    handle_server_response(sockfd, client_id);

    close(sockfd);
    printf("Disconnected from server\n");

    return 0;
}
