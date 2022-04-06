# Phase 1: LOCAL

# CC = gcc
# CFLAGS = -Wall -g

# shell: shell.o command.o
# 	$(CC) $(CFLAGS) -o shell shell.o command.o

# shell.o: shell.c command.h
# 	$(CC) $(CFLAGS) -c shell.c

# command.o: command.c command.h
# 	$(CC) $(CFLAGS) -c command.c

# clean: 
# 	rm -f 

# Phase 2: REMOTE

CC = gcc
CFLAGS = -Wall -g

server: server.c command.o
	$(CC) $(CFLAGS) -o server server.c command.o

client: client.c command.o
	$(CC) $(CFLAGS) -o client client.c command.o

command.o: command.c command.h
	$(CC) $(CFLAGS) -c command.c

clean: 
	rm -f 

