#include "table.h"

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

typedef struct {
    List_T *clients;
    Table_T *table;
} Dispatch_T;

Dispatch_T* new_dispatch();

void free_dispatch(Dispatch_T **dipatch);

int read_message(int fd, Dispatch_T *dispatch);