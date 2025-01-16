// #include <stdbool.h>

// typedef struct {
//     int key;
//     char *buffer;
//     int len;
// } Entry_H;

// Entry_H* create_entry(int key, char *msg, int len);

// void free_table_entry(Entry_H **entry);

// typedef struct
// {
//     unsigned long buckets;
//     unsigned long size;
//     Entry_H **entries;
// } HashTable_T;

// HashTable_T *create_table(unsigned capacity);

// void free_table(HashTable_T **table);

// bool add_entry_to_table(HashTable_T *table, Entry_H *new_entry);

// Entry_H *get_entry_from_table(HashTable_T *table, int key);

// bool delete_entry_from_table(HashTable_T *table, Entry_H *to_delete);