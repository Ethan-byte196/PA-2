#include "utils.h"

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
    return hash;
}

void log_event(const char *event) {
    FILE *file = fopen("output.txt", "a");
    fprintf(file, "%lu: %s\n", time(NULL), event);
    fclose(file);
}

void log_command(const char *command, const char *name, uint32_t salary) {
    FILE *file = fopen("output.txt", "a");
    fprintf(file, "%lu, %s, %s, %u\n", time(NULL), command, name, salary);
    fclose(file);
}
