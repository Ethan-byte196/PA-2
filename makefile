CC = gcc
CFLAGS = -pthread -Wall -Wextra

OBJS = chash.o hash_table.o command_processor.o utils.o

chash: $(OBJS)
	$(CC) $(CFLAGS) -o chash $(OBJS)

chash.o: chash.c command_processor.h
	$(CC) $(CFLAGS) -c chash.c

hash_table.o: hash_table.c hash_table.h utils.h
	$(CC) $(CFLAGS) -c hash_table.c

command_processor.o: command_processor.c command_processor.h hash_table.h
	$(CC) $(CFLAGS) -c command_processor.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

clean:
	rm -f chash $(OBJS) output.txt
