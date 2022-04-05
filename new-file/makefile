CC = gcc
CFLAGS = -Wall -g

# ****************************************************

# shell: LABserver.o LABclient.o command.o 
# 	$(CC) $(CFLAGS) -o LABserver LABserver.o command.o


# shell: LABclient.o
# 	$(CC) $(CFLAGS) -o LABclient LABclient.o


# shell.o: LABserver.c LABclient.c command.h
# 	$(CC) $(CFLAGS) -c LABserver.c LABclient.c

# all: server client

server: LABserver.c command.o
	$(CC) -o server LABserver.c command.o

client: LABclient.c command.o
	$(CC) -o client LABclient.c command.o

command.o: command.c command.h
	$(CC) $(CFLAGS) -c command.c

clean: 
	rm -f 

