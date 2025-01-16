#include "list.h"
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define BUFSIZE 400

Entry_T* new_entry(char *clientID, int fd)
{
    Entry_T *e = malloc(sizeof(Entry_T));
    assert(e != NULL);

    char *buf = calloc(IDSIZE, sizeof(char));
    assert(buf != NULL);

    strcpy(buf, clientID);
    e->clientID = buf;
    e->fd = fd;

    // TODO: Move to hash table
    e->len = 0;
    e->buffer = NULL;
    e->last_update = -1;

    return e;
}

/*
 * NOTE: Not responsible for freeing any buffers
 *       That must be done when the client disconnects.
 */
void free_entry(Entry_T **entry)
{
    assert(entry != NULL);
    assert(*entry != NULL);

    free((*entry)->clientID);
    (*entry)->clientID = NULL;

    free(*entry);
    *entry = NULL;
}

List_T* new_list()
{
    List_T *list = malloc(sizeof(List_T));
    assert(list != NULL);

    list->capacity = 20;
    list->size = 0;
    list->e = calloc(sizeof(Entry_T*), list->capacity);

    return list;
}

void free_list(List_T **list)
{
    // Do not allow free_list to be misused
    assert(list != NULL);
    assert(*list != NULL);

    // Free every entry in the list
    for (size_t i = 0; i < (*list)->size; i++) {
        free_entry(&(*list)->e[i]);
    }

    free((*list)->e);
    free(*list);
    *list = NULL;
}

void expand(List_T *list)
{
    list->capacity *= 2;
    Entry_T **new_e = calloc(sizeof(Entry_T*), list->capacity);

    // copy over elements
    for (size_t i = 0; i < list->size; i++) {
        new_e[i] = list->e[i];
    }

    // free the old list memory
    free(list->e);
    list->e = new_e;

    return;
}

void add_client(List_T *list, char *clientID, int fd)
{
    // If the list is full, expand
    if (list->size == list->capacity) {
        expand(list);
    }

    list->e[list->size] = new_entry(clientID, fd);
    list->size++;
}

void print_client_list(List_T *list)
{
    printf("List size is %ld\n", list->size);
    for (size_t i = 0; i < list->size; i++) {
        printf("%s, ", list->e[i]->clientID);
    }
    printf("\n");
}

void remove_client(List_T *list, char *clientID)
{
    // first, find client
    for (size_t i = 0; i < list->size; i++) {
        if (strcmp(list->e[i]->clientID, clientID) == 0) {            
            // Remove from list
            free_entry(&list->e[i]);
            list->size--;

            // Shift over elements
            for (size_t j = i; j < list->size; j++) {
                list->e[j] = list->e[j + 1];
            }

            // There are no duplicates, can stop once first client is removed
            break;
        }
    }
}

void remove_client_by_fd(List_T *list, int fd)
{    
    // first, find client
    for (size_t i = 0; i < list->size; i++) {
        if (list->e[i]->fd == fd) {            
            // Remove from list
            free_entry(&list->e[i]);
            list->size--;

            // Shift over elements
            for (size_t j = i; j < list->size; j++) {
                list->e[j] = list->e[j + 1];
            }

            // There are no duplicates, can stop once first client is removed
            break;
        }
    }
    
}

bool existing_client(List_T *list, char *clientID, int fd)
{
    if (fd == -1) {
        for (size_t i = 0; i < list->size; i++) {
            if (strcmp(list->e[i]->clientID, clientID) == 0) {
                return true;
            }
        }

        return false;
    }
    
    for (size_t i = 0; i < list->size; i++) {
        if (fd == list->e[i]->fd) {
            return true;
        }
    }

    return false;

}

Entry_T* get_Entry_from_clientID(List_T *list, char *clientID)
{
    for (size_t i = 0; i < list->size; i++) {
        if (strcmp(list->e[i]->clientID, clientID) == 0) {
            return list->e[i];
        }
    }

    return NULL;
}

Entry_T *get_Entry_from_fd(List_T *list, int fd)
{
    for (size_t i = 0; i < list->size; i++) {
        if (list->e[i]->fd == fd) {
            return list->e[i];
        }
    }

    return NULL;
}

int get_fd_from_clientID(List_T *list, char *clientID)
{
    Entry_T *entry = get_Entry_from_clientID(list, clientID);
    if (entry != NULL) {
        return entry->fd;
    }

    // could not find fd associated with clientID
    return -1;
}

char* get_clientID_from_fd(List_T *list, int fd)
{
    Entry_T *entry = get_Entry_from_fd(list, fd);
    if (entry != NULL) {
        return entry->clientID;
    }

    return NULL;
}

/*
 * Returns the contents of the client list
 * (Used for hello messages and listreq)
 */
char *return_contents(List_T *list, int *size)
{
    // Initializing buffer size to 400
    char *buf = calloc(BUFSIZE, sizeof(char));
    assert(buf != NULL);

    int offset = 0;

    for (size_t i = 0; i < list->size; i++) {
        int len = strlen(list->e[i]->clientID);
        if (offset + len + 1 >= BUFSIZE) {
            fprintf(stderr, "Client list contents too long\n");
            assert(false);  // spec says not to worry about this case
        }

        // Copy client ID into buffer
        memcpy(buf + offset, list->e[i]->clientID, len);
        offset += len;

        // Add a null-terminator after each client ID
        buf[offset] = '\0';
        offset++;
    }

    *size = offset;
    return buf;
}