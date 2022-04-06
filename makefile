# CC = gcc
# CFLAGS = -Wall -g

# # ****************************************************

# shell: shell.o command.o
# 	$(CC) $(CFLAGS) -o shell shell.o command.o

# shell.o: shell.c command.h
# 	$(CC) $(CFLAGS) -c shell.c

# command.o: command.c command.h
# 	$(CC) $(CFLAGS) -c command.c

# clean: 
# 	rm -f 


CC = gcc
CFLAGS = -Wall -g

# ****************************************************

# shell: client server
# 	$(CC) $(CFLAGS) -o shell client server

# server: server.o command.o
# 	$(CC) $(CFLAGS) -o server server.o command.o

# server.o: server.c command.h
# 	$(CC) $(CFLAGS) -c server.c

# client: client.o command.o
# 	$(CC) $(CFLAGS) -o client client.o command.o

# client.o: client.c command.h
# 	$(CC) $(CFLAGS) -c client.c

# command.o: command.c command.h
# 	$(CC) $(CFLAGS) -c command.c

all: server client
# 	$(CC) $(CFLAGS) -o all client server

server: server.c command.o
	$(CC) $(CFLAGS) -o server server.c command.o

client: client.c command.o
	$(CC) $(CFLAGS) -o client client.c command.o

command.o: command.c command.h
	$(CC) $(CFLAGS) -c command.c

clean: 
	rm -f 

