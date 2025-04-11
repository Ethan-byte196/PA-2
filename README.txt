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
Portions of this project were generated with AI assistance for modular design, thread synchronization, and proper C implementation.
With AI being encouraged as part of the deveopment process for this assignment, we wanted (to the best of our ability) to craft a high-quality prompt that would maximize the probability of code generation meeting
the assignment criteria. From experience using it we have gathered that this typically entails a multi-stage approach to providing the model (in our case, ChatGPT 4o) sufficient context to grasp the assignment purpose,
key goals, and more specific requirements in order to meet the deliverable standard. We will now describe the development flow we utilized, as well as providing some snippets of our conversations with the model, to give some insight into the extent to which it was involved in helping us complete the project. 

We found that it yields good results to prompt the model with a rough outline of our tasks, goals, and purpose, before diving into specifics. The quality of prompts, especially near the beginning of the conversation, is
highly important as the model is still fresh and has not acquired enough context / information to lead itself into a rabit hole. Once the foundation of what we hope to achieve has been laid out, we asked the model if it
had a good understanding of its role in this process, to verify that we are on the right track. Once acknowledged, we then provide snippets from the webcourse assignment so that it can fill in any gaps in it's understanding. Crucially, the first prompt asked the model to refrain from trying to generate code or solve the problem until we were on the same page. This is important as once it starts to code, it has a habit of attempting to modify the existing codebase as opposed to starting from scratch when prompted to fix any errors. This is why the beginning of the conversation is the most important time to verify the model has the same
understanding of what needs to be accomplished as our team. Once all the basic and specific information has been passed into the model, we asked that it please provide us with any questions or concerns it has so that we 
may address them prior to code generation. This methodology resulted in code that accomplished roughly 85% of the target deliverables on the first pass. From here, we downloaded / copied the code in Visual Studio Code in 
order to structure our project and run test cases to validate results. If errors were found, we would take note of what had occurred, as well as what we thought the solution might be. We'd prompt GPT with our data and 
ask that it please implement these changes if it found them likely to be accurate, and if not, provide a counter argument. Once a path forward was decided on, we'd prompt the model to correct the code errors before modifying our own copy and retesting. This iterative process is how we went about integrating AI into our workflow and resulted in successfully meeting the demands of the project. Minor tweaks were hand-coded at times, such
as when instead of printing read and write locks to the output file, they were printed to the terminal. 

Below you can find some snippets of the prompts we gave GPT:

(1) Us -> GPT
I have an assignment for my operating systems course in which we must create a concurrent hash table. The point of the assignment is to prompt you to generate a working code. I am happy to provide you the instructions, and I'd like for you to ask me any additional questions you may have after reviewing the instructions to make sure you have any and all information you need to successfully complete the assignment. How does this sound? 

(2) GPT -> Us
Thanks for sharing the assignment details! I have a few clarifying questions to ensure I implement the concurrent hash table correctly:

Clarifications on the Hash Table
Collision Handling: The instructions state that we do not need to handle collisions. Just to confirm, does this mean that each node in the linked list directly corresponds to a unique hash value, with no chaining or probing necessary?
Data Storage: The hash table is a linked list of hashRecord nodes. Does this mean the table itself consists of a single linked list (rather than an array of lists)?
Sorting on Output: The final print of the hash table should be sorted by hash value. Should this sorting be performed dynamically as elements are inserted, or should we sort only when printing?

Concurrency & Synchronization
Locking Mechanism:
The insert and delete functions require a write lock. Do we use a single global mutex for the entire hash table, or should we use finer-grained locking at the node level?
The search function uses a read lock. Do we need a shared reader-writer lock, or should each node have its own lock?
Condition Variables: The instructions mention that DELETE operations should wait until all INSERT operations are completed. Does this mean:
All inserts must finish before any delete starts? (i.e., delete operations should be queued until inserts are done.)
Or does it mean a specific delete operation should wait if its key is currently being inserted?
Thread Execution Order: Since commands run in separate threads, should we ensure that threads, X, 0 determines the exact number of parallel worker threads executing operations, or is it simply the number of commands in commands.txt?

File Handling & Output
Command File Processing:
Should we read the entire commands.txt file into memory first, or process commands one at a time as threads become available?
Do we need to handle invalid command formats (e.g., insert, John Doe without a salary)?
Output Requirements: The example output shows timestamps. Should these timestamps be Unix epoch time in seconds, or should they include microseconds for finer granularity?

Once I have these answers, I can start structuring the implementation.

(3) Us -> GPT
Clarifications on the Hash Table:
1. No, you do not need to worry about the collisions as they will make sure the data tested on our program does not have any collisions. Still, you should use Jenkins's one at a time hash function, which has a very low collision rate for the number of hashes we will be working with. 
2. I believe that the table itself consists of a single linked list as opposed to an array of lists, but I'm not certain, so use your best judgement here.
3. Again, not sure which approach would be better, but from experience it might be less complicated to just sort the data when it comes time to print. If you disagree, you're all good to continue how you please. 
4. I'm not certain what kinds of locks we need to use, based on the sample code you got in the instructions I'd like you to use your best judgement on how this is to be achieved.
5. All inserts must finish before any delete starts.
6. If it doesn't impact the final print or results, I think you can decide to do it how you wish. Again, use your best judgement. 
7. Don't worry about invalid commands, and I would say that you should read the entire file into memory first. 
8. Just match the timestamps present in the example output.

(4) GPT -> Us
Got it! Thanks for the clarifications. Based on your responses, here's how I plan to approach the implementation:

Hash Table Structure
Implement as a single linked list (since we're not sure, but that seems to align with the struct definition).
Use Jenkins’s one-at-a-time hash function for hashing.
Sorting will be done only when printing rather than maintaining sorted order during insertion.

Concurrency & Synchronization
Read/Write Locks: Use a single read-write lock (pthread_rwlock_t) to protect the entire linked list.
Condition Variable: Use a pthread_cond_t to ensure that delete operations do not start until all inserts are complete.
Thread Execution: Create N threads (based on threads, X, 0 line), and have them execute commands asynchronously while ensuring proper ordering constraints (e.g., all inserts before deletes).

File Handling & Output
Read the entire commands.txt file into memory first.
Execute operations in threads based on the parsed commands.
Ensure output formatting (timestamps, locks, etc.) follows the example output file.

I’ll start implementing the structure, thread management, and locking logic. Let me know if you think of anything else!

(5) Us -> GPT
Sounds good, go ahead and start coding. Let me know if you need anything.
