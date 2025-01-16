// #include "hash_table.h"
// #include <stdbool.h>
// #include <stdio.h>
// #include <unistd.h>
// #include <stdlib.h>
// #include <string.h>
// #include <netdb.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <time.h>
// #include <assert.h>

// /* Making the table twice as large as it strictly needs to be to minimize
//  * collisions */
// unsigned BUCKET_MULTIPLIER = 2;

// Entry_H* create_entry(int key, char *msg, int len) {
//     Entry_H *entry = malloc(sizeof(Entry_H));
//     assert(entry != NULL);

//     entry->key = key;
//     entry->buffer = msg;
//     entry->len = len;

//     // entry->response = response;
//     // entry->max_age = max_age;
//     // entry->insert_time = insert_time;
//     // entry->access_time = access_time;

//     return entry;
// }

// void free_table_entry(Entry_H **entry)
// {
//     // printf("\nFREEING ENTRY WITH FILE NAME %s\n\n", (*entry)->filename);
//     if (entry != NULL && *entry != NULL)
//     {
//         // free((*entry)->key);
//         // free_Buffer_T(&(*entry)->response);
//         free(*entry);
//         *entry = NULL;
//     }
// }

// unsigned long compute_index(int key, unsigned int table_size)
// {
//     // 2^32 / (sqrt(5) - 1)
//     const unsigned long prime = 2654435761u;
//     return (key * prime) % table_size;
// }

// HashTable_T *create_table(unsigned capacity)
// {
//     HashTable_T *table = malloc(sizeof(HashTable_T));
//     assert(table != NULL);

//     /* Initializing all memory to NULL to make sure that we can differentiate
//      * between initialized and uninitialized entries */
//     table->entries = calloc(BUCKET_MULTIPLIER * capacity, sizeof(Entry_H*));
//     assert(table->entries != NULL);

//     table->buckets = BUCKET_MULTIPLIER * capacity;
//     table->size = 0;
//     return table;
// }

// void free_table(HashTable_T **table)
// {
//     if (table != NULL && *table != NULL)
//     {
//         for (int i = 0; i < (*table)->buckets; i++)
//         {
//             // If there is an entry at that bucket, free it.
//             if ((*table)->entries[i] != NULL)
//             {
//                 free_table_entry(&(*table)->entries[i]);
//             }
//         }

//         free((*table)->entries);
//         (*table)->entries = NULL;

//         free(*table);
//         *table = NULL;
//     }
// }

// bool add_entry_to_table(HashTable_T *table, Entry_H *new_entry)
// {
//     unsigned long index = compute_index(new_entry->key, table->buckets);

//     /* If there is already an entry in the target insertion slot:
//      * 1. Check to see if it is the same file (and if so, update it)
//      * 2. If it isn't, use linear probing to find the next open bucket */

//     unsigned i = 0;

//     Entry_H **target = &table->entries[index];

//     /* If the target insertion slot has an entry*/
//     while (*target != NULL)
//     {

//         /* Check if the filename in the table is the same as the one being
//          * inserted */
//         // TODO compare ints
//         // if (strcmp((*target)->key, new_entry->key) == 0)
//         if ((*target)->key == new_entry->key) {
//             // If it is the same file, replace the old entry with the new
//             free_table_entry(target);
//             *target = new_entry;
//             return true;
//         }

//         // Otherwise, keep linear probing
//         i++;
//         target = &table->entries[(index + i) % table->buckets];

//         // Major issue
//         if (i >= table->buckets)
//         {
//             assert(false);
//         }
//     }

//     // Debug print
//     // printf("\nPROBING INSERTION: i ended up being %d\n\n", i);

//     /* If the target insertion slot has no entry, go ahead and insert */

//     *target = new_entry;
//     return false;
// }

// // helper function
// // TODO
// Entry_H **find_entry_address_in_table(HashTable_T *table, int key)
// {
//     unsigned long index = compute_index(key, table->buckets);
//     unsigned i = 0;
//     Entry_H **address = &table->entries[index];

//     // If the file we are looking for doesn't exist in the table
//     if (address == NULL)
//     {
//         return NULL;
//     }
//     if (*address == NULL)
//     {
//         return NULL;
//     }

//     // If filenames match immediately, we're already at the right index
//     bool filenames_match = ((*address)->key == key);

//     while (!filenames_match) {
//         i++;
//         address = &table->entries[(index + i) % table->buckets];

//         /* If we run out of existing entries to probe, the file isn't in the
//          * table */
//         if (address == NULL)
//         {
//             return NULL;
//         }
//         if (*address == NULL)
//         {
//             return NULL;
//         }

//         // i should never get this big. We have a major problem
//         if (i >= table->buckets)
//         {
//             assert(false);
//         }

//         filenames_match = ((*address)->key == key);
//     }

//     // Debug print
//     // printf("\nPROBING RETRIEVAL: i ended up being %d\n\n", i);
//     return address;
// }

// // TODO
// Entry_H *get_entry_from_table(HashTable_T *table, int key)
// {
//     Entry_H **address = find_entry_address_in_table(table, key);
//     if (address == NULL)
//     {
//         // printf("The FILE %s DOES NOT already exist in the table.\n", filename);
//         return NULL;
//     }

//     Entry_H *target = *address;
//     return target;
// }


// // bool update_entry_in_table(HashTable_T *table, Entry_T *to_update)
// // {
// //     Entry_T **target = find_entry_address_in_table(table, to_update->key);
// //     assert(target != NULL);

// //     // free(target);
// //     // Intentionally leaking memory in the hash table because I'm dense
// //     *target = to_update;

// //     return true;
// // }

// bool delete_entry_from_table(HashTable_T *table, Entry_H *to_delete)
// {
//     Entry_H **target = find_entry_address_in_table(table, to_delete->key);
//     // If there is no entry to delete
//     if (target == NULL)
//     {
//         return false;
//     }

//     if (*target == NULL)
//     {
//         target = NULL;
//         return false;
//     }

//     free_table_entry(target);
//     assert(*target == NULL);

//     return true;
// }

// // Entry_T *find_entry_to_delete(HashTable_T *table)
// // {

// //     int i = 0;
// //     int age;
// //     time_t current_time = time(NULL);
// //     unsigned long now = (unsigned long)current_time;

// //     // least recently retrieved
// //     unsigned long lrr_time = now;
// //     Entry_T *lrr_entry = NULL;
// //     Entry_T *backup = NULL;

// //     // iterate thru the entire table
// //     while (i < table->buckets)
// //     {
// //         Entry_T **target = &table->entries[i];

// //         if (*target != NULL)
// //         {
// //             // find the age of the entry

// //             printf("Considering deleting entry %s\n", (*target)->key);

// //             backup = *target;
// //             age = now - (*target)->insert_time;

// //             printf("Age of target: %d\n", age);
// //             printf("Max age of target: %d\n", (*target)->max_age);
// //             printf("Last accessed %d seconds age\n", now - (*target)->access_time);

// //             // if the entry is stale, return it (to be deleted)
// //             if (age >= (*target)->max_age)
// //             {
// //                 return *target;
// //             }

// //             // check and see if this entry is the least recently retrieved
// //             if ((*target)->access_time < lrr_time)
// //             {
// //                 lrr_time = (*target)->access_time;
// //                 lrr_entry = *target;
// //             }
// //         }

// //         i++;
// //     }

// //     if (lrr_entry != NULL)
// //     {
// //         return lrr_entry;
// //     }
// //     else
// //     {
// //         // just delete any old thing
// //         return backup;
// //     }

// //     // failed to find something to delete (should never happen)
// //     assert(false);
// // }