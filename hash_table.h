#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdint.h>
#include <pthread.h>

// Structure for each node in the hash table (linked list)
typedef struct hash_struct {
    uint32_t hash;
    char name[50];
    uint32_t salary;
    struct hash_struct *next;
} hashRecord;

// Global variables for synchronization
extern pthread_rwlock_t rwlock;
extern pthread_mutex_t insert_mutex;
extern pthread_cond_t insert_done;
extern int insert_count;
extern int lock_acquired;
extern int lock_released;

// Function prototypes
uint32_t jenkins_one_at_a_time_hash(const char *key);
void insert_record(const char *name, uint32_t salary);
void delete_record(const char *name);
hashRecord* search_record(const char *name);
void print_table_to_file(const char *filename);
void free_table();

#endif
