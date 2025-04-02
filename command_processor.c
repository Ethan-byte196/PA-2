#include "command_processor.h"
#include "hash_table.h"

void *process_command(void *arg) {
    char *command = (char *)arg;
    char action[10], name[50];
    uint32_t salary;

    sscanf(command, "%[^,],%[^,],%u", action, name, &salary);

    if (strcmp(action, "insert") == 0) {
        insert_record(name, salary);
    } else if (strcmp(action, "delete") == 0) {
        delete_record(name);
    } else if (strcmp(action, "search") == 0) {
        search_record(name);
    }

    free(command);
    return NULL;
}
