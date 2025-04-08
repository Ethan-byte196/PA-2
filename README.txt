# Concurrent Hash Table Project

## Overview
This program implements a thread-safe concurrent hash table using a linked list. It supports inserting, searching, and deleting records with thread synchronization.

## Features
- Uses **Jenkins's One-at-a-Time Hash** function for key hashing.
- Implements **read-write locks (pthread_rwlock_t)** for safe multi-threading.
- Ensures **all inserts complete before deletes** using **condition variables**.
- Outputs execution logs, including **timestamps**, **locks**, and **condition variable wait signals**.
- Writes the final hash table contents to `output.txt`.

## Files
- **chash.c**: Main program that reads `commands.txt`, creates threads, and manages execution.
- **hash_table.c**: Implements the concurrent hash table functions (insert, delete, search, print).
- **hash_table.h**: Header file defining structures and function prototypes.
- **Makefile**: Automates compilation of the project.
- **README.txt**: This file.

## Compilation
To compile the program, run:
make

This produces an executable called `chash`.

## Running the Program
Once compiled, run:
./chash

This reads commands from `commands.txt` and writes output to `output.txt`.

## Cleaning Up
To remove compiled files and output:
make clean


## AI Usage
Some portions of this project were generated with AI assistance for modular design, thread synchronization, and proper C implementation.