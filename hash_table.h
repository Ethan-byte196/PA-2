#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct hash_struct {
    uint32_t hash;
    char name[50];
    uint32_t salary;
    struct hash_struct *next;
} hashRecord;

void insert_record(const char *name, uint32_t salary);
void delete_record(const char *name);
uint32_t search_record(const char *name);
void print_final_summary();

#endif
