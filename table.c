#include "table.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#define INIT_CAPACITY 20

// should be given heap allocated buffer data
Buf_T *new_buf(int fd, char *buffer, size_t len, time_t last_update)
{
    Buf_T *b = malloc(sizeof(Buf_T));
    assert(b != NULL);

    b->fd = fd;
    b->len = len;
    b->last_update = last_update;
    b->buffer = buffer;     // this works because buffer should be heap alloc'd

    return b;
}

void free_buf(Buf_T **b)
{
    assert(b != NULL);
    assert(*b != NULL);

    if ((*b)->buffer != NULL) {
        free((*b)->buffer);  // Freeing the buffer if it exists
    }

    free(*b);
    *b = NULL;
}

Table_T *new_table()
{
    Table_T *table = malloc(sizeof(Table_T));
    assert(table != NULL);

    table->actual_size = 0;
    table->max_idx = 0;
    table->capacity = INIT_CAPACITY;

    table->bufs = calloc(table->capacity, sizeof(Buf_T*));
    assert(table->bufs != NULL);

    return table;
}

void free_table(Table_T **table)
{
    assert(table != NULL);
    assert(*table != NULL);

    for (size_t i = 0; i < (*table)->max_idx; i++) {
        if ((*table)->bufs[i] != NULL) {
            free_buf(&(*table)->bufs[i]);
        }
    }

    free((*table)->bufs);
    free(*table);
    *table = NULL;
}

static void expand_table(Table_T *table)
{
    table->capacity *= 2;
    table->bufs = realloc(table->bufs, table->capacity * sizeof(Buf_T *));
    assert(table->bufs != NULL);
}

Buf_T *get_buf_from_fd(Table_T *table, int fd)
{
    if (fd >= table->max_idx) {
        return NULL;
    }

    return table->bufs[fd];
}

Buf_T **get_buf_addr_from_fd(Table_T *table, int fd)
{
    if (fd >= table->max_idx) {
        return NULL;
    }

    return &table->bufs[fd];
}

bool buffer_message(Table_T *table, int fd, size_t len, char *buf)
{
    if (fd >= table->capacity) {
        expand_table(table); // Expand the table if necessary
    }

    time_t now = time(NULL);
    assert(now != -1);

    if (fd >= table->max_idx) {
        table->max_idx = fd + 1;
    }

    // If there's an existing buffer for this fd, free it before overwriting
    if (table->bufs[fd] != NULL) {
        free_buf(&table->bufs[fd]);
    }

    table->bufs[fd] = new_buf(fd, buf, len, now);
    table->actual_size++;

    return true;
}

char *get_buffered_message(Table_T *table, int fd, size_t *len)
{
    Buf_T *b = get_buf_from_fd(table, fd);

    if (b == NULL) {
        return NULL;
    }

    if (b->len == 0) {
        *len = 0;
        return NULL;
    }

    *len = b->len;
    char *copy = calloc(b->len, sizeof(char));
    assert(copy != NULL);
    memcpy(copy, b->buffer, b->len);

    free(b->buffer);  // Wipe the stored buffer before returning contents
    b->buffer = NULL;
    b->len = 0;

    return copy;
}

struct timeval get_next_timeout(Table_T *table)
{
    struct timeval timeout;
    timeout.tv_sec = BUF_TTL;
    timeout.tv_usec = 0;

    time_t now = time(NULL);
    assert(now != -1);

    for (size_t i = 0; i < table->max_idx; i++) {
        if (table->bufs[i] != NULL && table->bufs[i]->len != 0) {
            time_t diff = BUF_TTL - (now - table->bufs[i]->last_update);
            printf("Diff is %ld\n", diff);
            if (diff < timeout.tv_sec) {
                timeout.tv_sec = diff;
            }
        }
    }

    return timeout;
}

void free_old_buffers(Table_T *table, fd_set *set, List_T *list)
{
    time_t now = time(NULL);
    assert(now != -1);

    for (size_t i = 0; i < table->max_idx; i++) {
        if (table->bufs[i] != NULL && table->bufs[i]->len != 0) {
            time_t diff = now - table->bufs[i]->last_update;
            if (diff >= BUF_TTL) {
                FD_CLR(table->bufs[i]->fd, set);  // remove fd from active set
                printf("\nFREED A BUFFERED MESSAGE AND CLOSED CLIENT\n");

                char *client = get_clientID_from_fd(list, table->bufs[i]->fd);
                remove_client(list, client);

                free_buf(&table->bufs[i]);
                table->actual_size--;
            }
        }
    }
}

void free_buffered_message(Table_T *table, int fd)
{
    Buf_T **b = get_buf_addr_from_fd(table, fd);

    if (b != NULL && *b != NULL) {
        free_buf(b);
    }
}
