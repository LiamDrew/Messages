// using structs to represent the list under the hood
// these provide flexibility for changing the implementation without changing
// the interface
#include <stdbool.h>
#include <stddef.h>
#include <time.h>
#include <sys/select.h>

#define BUF_TTL 60      //setting the buffer TTL to only 10 seconds FOR NOW
#define IDSIZE 20

// this shouldn't have to go in the interface, but whatever
typedef struct {
    char *clientID;
    int fd;
    char *buffer;
    size_t len;
    time_t last_update;
} Entry_T;

typedef struct {
    Entry_T **e;
    size_t size;
    size_t capacity;
} List_T;

List_T* new_list();

void free_list(List_T **list);

void add_client(List_T *list, char *clientID, int fd);

void remove_client(List_T *list, char *clientID);

void remove_client_by_fd(List_T *list, int fd);

// return if the client exists in the list
bool existing_client(List_T *list, char *clientID, int fd);

int get_fd_from_clientID(List_T *list, char *clientID);

char* get_clientID_from_fd(List_T *list, int fd);

// return all the connect clients as a comma separated string
char* return_contents(List_T *list, int *size);