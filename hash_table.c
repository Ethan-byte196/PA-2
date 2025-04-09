#include "hash_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Head of the linked list
static hashRecord *head = NULL;

// Synchronization primitives
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
pthread_mutex_t insert_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t insert_done = PTHREAD_COND_INITIALIZER;
int insert_count = 0;

// Utility to get timestamp
long get_timestamp() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;  // Microseconds
}

// Jenkins's One-at-a-Time Hash Function
uint32_t jenkins_one_at_a_time_hash(const char *key) {
    uint32_t hash = 0;
    while (*key) {
        hash += (unsigned char)(*key);
        hash += (hash << 10);
        hash ^= (hash >> 6);
        key++;
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

// Insert or update a record in the hash table
void insert_record(const char *name, uint32_t salary) {
    pthread_rwlock_wrlock(&rwlock);
    long timestamp = get_timestamp();
    printf("%ld,WRITE LOCK ACQUIRED\n", timestamp);

    uint32_t hash = jenkins_one_at_a_time_hash(name);
    hashRecord *curr = head, *prev = NULL;

    while (curr && curr->hash < hash) {
        prev = curr;
        curr = curr->next;
    }

    if (curr && curr->hash == hash) {  // Update existing
        curr->salary = salary;
    } else {  // Insert new record
        hashRecord *new_node = malloc(sizeof(hashRecord));
        new_node->hash = hash;
        strncpy(new_node->name, name, 50);
        new_node->salary = salary;
        new_node->next = curr;

        if (prev) {
            prev->next = new_node;
        } else {
            head = new_node;
        }
    }

    timestamp = get_timestamp();
    printf("%ld,WRITE LOCK RELEASED\n", timestamp);
    pthread_rwlock_unlock(&rwlock);

    pthread_mutex_lock(&insert_mutex);
    insert_count--;
    if (insert_count == 0) {
        pthread_cond_broadcast(&insert_done);
    }
    pthread_mutex_unlock(&insert_mutex);
}

// Delete a record
void delete_record(const char *name) {
    pthread_mutex_lock(&insert_mutex);
    while (insert_count > 0) {
        long timestamp = get_timestamp();
        printf("%ld: WAITING ON INSERTS\n", timestamp);
        pthread_cond_wait(&insert_done, &insert_mutex);
    }
    pthread_mutex_unlock(&insert_mutex);

    pthread_rwlock_wrlock(&rwlock);
    long timestamp = get_timestamp();
    printf("%ld,WRITE LOCK ACQUIRED\n", timestamp);

    uint32_t hash = jenkins_one_at_a_time_hash(name);
    hashRecord *curr = head, *prev = NULL;

    while (curr && curr->hash != hash) {
        prev = curr;
        curr = curr->next;
    }

    if (curr) {
        if (prev) {
            prev->next = curr->next;
        } else {
            head = curr->next;
        }
        free(curr);
    }

    timestamp = get_timestamp();
    printf("%ld,WRITE LOCK RELEASED\n", timestamp);
    pthread_rwlock_unlock(&rwlock);
}

// Search for a record
hashRecord* search_record(const char *name) {
    pthread_rwlock_rdlock(&rwlock);
    long timestamp = get_timestamp();
    printf("%ld,READ LOCK ACQUIRED\n", timestamp);

    uint32_t hash = jenkins_one_at_a_time_hash(name);
    hashRecord *curr = head;

    while (curr) {
        if (curr->hash == hash) {
            pthread_rwlock_unlock(&rwlock);
            return curr;
        }
        curr = curr->next;
    }

    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

// Print the hash table to a file
void print_table_to_file(const char *filename) {
    pthread_rwlock_rdlock(&rwlock);
    FILE *fp = fopen(filename, "w");

    hashRecord *curr = head;
    while (curr) {
        fprintf(fp, "%u,%s,%u\n", curr->hash, curr->name, curr->salary);
        curr = curr->next;
    }
    fclose(fp);
    pthread_rwlock_unlock(&rwlock);
}

// Free the hash table memory
void free_table() {
    hashRecord *curr = head;
    while (curr) {
        hashRecord *temp = curr;
        curr = curr->next;
        free(temp);
    }
}