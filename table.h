#include <time.h>
#include <stdbool.h>
#include <sys/select.h>
#include "list.h"


typedef struct {
    int fd;
    char *buffer;
    size_t len;
    time_t last_update;
} Buf_T;

typedef struct {
    size_t actual_size;     // It's possible earlier fds will get disconnected
    size_t max_idx;         // Keep track of how high the count gets in table
    size_t capacity;
    Buf_T **bufs;
} Table_T;

Table_T* new_table();

void free_table(Table_T **table);

/*
 * Will overwrite anything in existing buffers
 * 
 * */
bool buffer_message(Table_T *table, int fd, size_t len, char *buf);

/*
 * will return NULL if there is no buffered data
 * Effects: sets len to whatever the size of the buffer is, 0 if buf is NULL
 * Will free the buffers stored in the table when getting message
 * (they will be put back if necessary)*/
char *get_buffered_message(Table_T *table, int fd, size_t *len);

/*
 * Get the number of seconds until the next buffered message expires
 * Returns a default of 60 secs
 */
struct timeval get_next_timeout(Table_T *table);

/*
 * Will free all old buffers from the table and remove those clients from the
 * fd set 
 * We also will want this to remove disconnected clients from the client list*/
void free_old_buffers(Table_T *table, fd_set *set, List_T *list);

/*
*/
void free_buffered_message(Table_T *table, int fd);


