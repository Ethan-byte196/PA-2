#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>  // Windows-specific headers
#include <stdint.h>
#include <time.h>     // For time-related functions (GetSystemTime)

#define TABLE_SIZE 100
#define MAX_LINE 256
#define OUTPUT_FILE "output.txt"

typedef struct hash_struct {
    uint32_t hash;
    char name[50];
    uint32_t salary;
    struct hash_struct *next;
} hashRecord;

hashRecord *hashTable[TABLE_SIZE];
CRITICAL_SECTION locks[TABLE_SIZE];   // For thread synchronization
CONDITION_VARIABLE insert_done;       // Condition variable for inserts
int inserts_completed = 0;
int total_inserts = 0;
int lock_acquisitions = 0, lock_releases = 0;

uint64_t get_time_in_microseconds() {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER ull;
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;
    return ull.QuadPart / 10;  // Convert to microseconds
}

void log_message(const char *message) {
    FILE *file = fopen(OUTPUT_FILE, "a");
    if (file) {
        fprintf(file, "%llu: %s\n", get_time_in_microseconds(), message);
        fclose(file);
    }
}

uint32_t jenkins_one_at_a_time_hash(const uint8_t *key, size_t length) {
    size_t i = 0;
    uint32_t hash = 0;
    while (i != length) {
        hash += key[i++];
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash % TABLE_SIZE;
}

void insert(const char *name, uint32_t salary) {
    uint32_t hash = jenkins_one_at_a_time_hash((const uint8_t *)name, strlen(name));
    log_message("WRITE LOCK ACQUIRED");
    EnterCriticalSection(&locks[hash]);
    lock_acquisitions++;

    hashRecord *current = hashTable[hash];
    while (current) {
        if (strcmp(current->name, name) == 0) {
            current->salary = salary;
            LeaveCriticalSection(&locks[hash]);
            log_message("WRITE LOCK RELEASED");
            lock_releases++;
            return;
        }
        current = current->next;
    }

    hashRecord *newRecord = (hashRecord *)malloc(sizeof(hashRecord));
    newRecord->hash = hash;
    strcpy(newRecord->name, name);
    newRecord->salary = salary;
    newRecord->next = hashTable[hash];
    hashTable[hash] = newRecord;

    LeaveCriticalSection(&locks[hash]);
    log_message("WRITE LOCK RELEASED");
    lock_releases++;

    EnterCriticalSection(&locks[hash]);
    inserts_completed++;
    if (inserts_completed == total_inserts) {
        log_message("DELETE AWAKENED");
        WakeConditionVariable(&insert_done);
    }
    LeaveCriticalSection(&locks[hash]);
}

void delete(const char *name) {
    EnterCriticalSection(&locks[0]);  // Lock on a global critical section (or the relevant lock for your operation)
    while (inserts_completed < total_inserts) {
        log_message("WAITING ON INSERTS");
        SleepConditionVariableCS(&insert_done, &locks[0], INFINITE);
    }
    LeaveCriticalSection(&locks[0]);

    uint32_t hash = jenkins_one_at_a_time_hash((const uint8_t *)name, strlen(name));
    log_message("WRITE LOCK ACQUIRED");
    EnterCriticalSection(&locks[hash]);
    lock_acquisitions++;

    hashRecord *current = hashTable[hash];
    hashRecord *prev = NULL;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            if (prev) prev->next = current->next;
            else hashTable[hash] = current->next;
            free(current);
            LeaveCriticalSection(&locks[hash]);
            log_message("WRITE LOCK RELEASED");
            lock_releases++;
            return;
        }
        prev = current;
        current = current->next;
    }

    LeaveCriticalSection(&locks[hash]);
    log_message("WRITE LOCK RELEASED");
    lock_releases++;
}

void search(const char *name) {
    uint32_t hash = jenkins_one_at_a_time_hash((const uint8_t *)name, strlen(name));
    log_message("READ LOCK ACQUIRED");
    EnterCriticalSection(&locks[hash]);
    lock_acquisitions++;

    FILE *file = fopen(OUTPUT_FILE, "a");
    if (!file) return;

    hashRecord *current = hashTable[hash];
    while (current) {
        if (strcmp(current->name, name) == 0) {
            fprintf(file, "%u,%s,%u\n", current->hash, name, current->salary);
            LeaveCriticalSection(&locks[hash]);
            log_message("READ LOCK RELEASED");
            lock_releases++;
            fclose(file);
            return;
        }
        current = current->next;
    }

    fprintf(file, "SEARCH: NOT FOUND\n");
    fclose(file);
    LeaveCriticalSection(&locks[hash]);
    log_message("READ LOCK RELEASED");
    lock_releases++;
}

void print_table() {
    FILE *file = fopen(OUTPUT_FILE, "a");
    if (!file) return;

    for (int i = 0; i < TABLE_SIZE; i++) {
        EnterCriticalSection(&locks[i]);
        hashRecord *current = hashTable[i];
        while (current) {
            fprintf(file, "%u,%s,%u\n", current->hash, current->name, current->salary);
            current = current->next;
        }
        LeaveCriticalSection(&locks[i]);
    }
    fclose(file);
}

void *process_command(void *arg) {
    char *command = (char *)arg;
    char operation[10], name[50];
    uint32_t salary;

    if (sscanf(command, "%9[^,],%49[^,],%u", operation, name, &salary) < 2) return NULL;

    char log_entry[MAX_LINE];
    sprintf(log_entry, "%s,%s,%u", operation, name, salary);
    log_message(log_entry);

    if (strcmp(operation, "insert") == 0) insert(name, salary);
    else if (strcmp(operation, "delete") == 0) delete(name);
    else if (strcmp(operation, "search") == 0) search(name);
    else if (strcmp(operation, "print") == 0) print_table();

    return NULL;
}

int main() {
    FILE *file = fopen("commands.txt", "r");
    if (!file) return 1;

    char line[MAX_LINE];
    int thread_count = 0;
    HANDLE *threads = NULL;  // Use HANDLE for Windows threads
    char *commands[100];
    int insert_count = 0, non_insert_count = 0;

    // Read commands from file and determine number of threads
    while (fgets(line, MAX_LINE, file)) {
        line[strcspn(line, "\n")] = 0;
        if (strncmp(line, "threads,", 8) == 0) {
            sscanf(line, "threads,%d,0", &thread_count);
            threads = malloc(thread_count * sizeof(HANDLE));  // Allocate memory for thread handles
            continue;
        }
        commands[insert_count + non_insert_count] = strdup(line);
        if (strncmp(line, "insert", 6) == 0) insert_count++;
        else non_insert_count++;
    }
    fclose(file);
    total_inserts = insert_count;

    // Create threads for insert operations
    for (int i = 0; i < insert_count; i++) {
        threads[i] = (HANDLE)_beginthreadex(NULL, 0, (unsigned (__stdcall *)(void*))process_command, commands[i], 0, NULL);
    }
    for (int i = 0; i < insert_count; i++) {
        WaitForSingleObject(threads[i], INFINITE);  // Wait for thread completion
        CloseHandle(threads[i]);  // Close thread handle
    }

    // Create threads for non-insert operations
    for (int i = insert_count; i < insert_count + non_insert_count; i++) {
        threads[i] = (HANDLE)_beginthreadex(NULL, 0, (unsigned (__stdcall *)(void*))process_command, commands[i], 0, NULL);
    }
    for (int i = insert_count; i < insert_count + non_insert_count; i++) {
        WaitForSingleObject(threads[i], INFINITE);  // Wait for thread completion
        CloseHandle(threads[i]);  // Close thread handle
    }

    log_message("Finished all threads.");
    free(threads);  // Free memory
    return 0;
}