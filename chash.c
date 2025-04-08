#include "hash_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define MAX_COMMANDS 100
#define MAX_LINE_LENGTH 100

typedef struct {
    char operation[10];
    char name[50];
    uint32_t salary;
} Command;

Command commands[MAX_COMMANDS];
int command_count = 0;

void *execute_command(void *arg) {
    Command *cmd = (Command *)arg;

    if (strcmp(cmd->operation, "insert") == 0) {
        insert_record(cmd->name, cmd->salary);
    } else if (strcmp(cmd->operation, "delete") == 0) {
        delete_record(cmd->name);
    } else if (strcmp(cmd->operation, "search") == 0) {
        hashRecord *result = search_record(cmd->name);
        if (result) {
            printf("%u,%s,%u\n", result->hash, result->name, result->salary);
        } else {
            printf("No Record Found\n");
        }
    }
    return NULL;
}

int main() {
    FILE *fp = fopen("commands.txt", "r");
    if (!fp) {
        perror("Failed to open commands.txt");
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), fp)) {
        sscanf(line, "%[^,],%49[^,],%u", commands[command_count].operation, 
               commands[command_count].name, &commands[command_count].salary);
        command_count++;
    }
    fclose(fp);

    pthread_t threads[command_count];
    for (int i = 0; i < command_count; i++) {
        pthread_create(&threads[i], NULL, execute_command, &commands[i]);
    }
    for (int i = 0; i < command_count; i++) {
        pthread_join(threads[i], NULL);
    }

    print_table_to_file("output.txt");
    free_table();
    return 0;
}