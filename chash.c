#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>  // For Windows threading
#include "command_processor.h"

int main() {
    FILE *file = fopen("commands.txt", "r");
    if (!file) {
        perror("Error opening commands.txt");
        return EXIT_FAILURE;
    }

    initialize_table(); // Initialize the hash table

    int num_threads;
    fscanf(file, "threads,%d,0\n", &num_threads); // Read thread count

    HANDLE *threads = malloc(num_threads * sizeof(HANDLE));  // Dynamic memory allocation for thread handles
    if (!threads) {
        perror("Memory allocation failed");
        fclose(file);
        return EXIT_FAILURE;
    }

    char command[100];
    int thread_index = 0;

    while (fgets(command, sizeof(command), file)) {
        command[strcspn(command, "\n")] = 0; // Remove newline

        // Create a new thread for each command
        threads[thread_index] = (HANDLE)_beginthreadex(NULL, 0, (unsigned (__stdcall *)(void *))process_command, strdup(command), 0, NULL);
        if (threads[thread_index] == 0) {
            perror("Thread creation failed");
            fclose(file);
            free(threads);
            return EXIT_FAILURE;
        }

        thread_index++;
    }

    fclose(file);

    // Wait for all threads to complete
    WaitForMultipleObjects(thread_index, threads, TRUE, INFINITE);

    // Close thread handles and free memory
    for (int i = 0; i < thread_index; i++) {
        CloseHandle(threads[i]);
    }
    free(threads);

    print_final_summary(); // Print final hash table state

    return 0;
}
