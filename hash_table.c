#include "hash_table.h"
#include "utils.h"
#include <windows.h>  // For SRWLOCK

hashRecord *hashTable = NULL;
SRWLOCK table_lock;  // Windows Slim Reader-Writer Lock

void initialize_table_lock() {
    InitializeSRWLock(&table_lock);
}

void insert_record(const char *name, uint32_t salary) {
    uint32_t hash = jenkins_one_at_a_time_hash((const uint8_t *)name, strlen(name));

    AcquireSRWLockExclusive(&table_lock);
    log_event("WRITE LOCK ACQUIRED");

    hashRecord *current = hashTable;
    while (current) {
        if (current->hash == hash) {
            current->salary = salary;
            log_command("INSERT", name, salary);
            ReleaseSRWLockExclusive(&table_lock);
            log_event("WRITE LOCK RELEASED");
            return;
        }
        current = current->next;
    }

    hashRecord *new_node = malloc(sizeof(hashRecord));
    new_node->hash = hash;
    strncpy(new_node->name, name, 50);
    new_node->salary = salary;
    new_node->next = hashTable;
    hashTable = new_node;

    log_command("INSERT", name, salary);
    ReleaseSRWLockExclusive(&table_lock);
    log_event("WRITE LOCK RELEASED");
}

void delete_record(const char *name) {
    uint32_t hash = jenkins_one_at_a_time_hash((const uint8_t *)name, strlen(name));

    AcquireSRWLockExclusive(&table_lock);
    log_event("WRITE LOCK ACQUIRED");

    hashRecord *prev = NULL, *current = hashTable;
    while (current) {
        if (current->hash == hash) {
            if (prev) prev->next = current->next;
            else hashTable = current->next;
            free(current);
            log_command("DELETE", name, 0);
            ReleaseSRWLockExclusive(&table_lock);
            log_event("WRITE LOCK RELEASED");
            return;
        }
        prev = current;
        current = current->next;
    }

    ReleaseSRWLockExclusive(&table_lock);
    log_event("WRITE LOCK RELEASED");
}

uint32_t search_record(const char *name) {
    uint32_t hash = jenkins_one_at_a_time_hash((const uint8_t *)name, strlen(name));

    AcquireSRWLockShared(&table_lock);
    log_event("READ LOCK ACQUIRED");

    hashRecord *current = hashTable;
    while (current) {
        if (current->hash == hash) {
            log_command("SEARCH", name, current->salary);
            ReleaseSRWLockShared(&table_lock);
            log_event("READ LOCK RELEASED");
            return current->salary;
        }
        current = current->next;
    }

    log_command("SEARCH", name, 0);
    ReleaseSRWLockShared(&table_lock);
    log_event("READ LOCK RELEASED");
    return 0;
}
