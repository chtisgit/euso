# Makefile for
# Project: mastermind
# Author: Christian Fiedler <e1363562@student.tuwien.ac.at>

CC=gcc
CFLAGS=-O3 -std=c99 -pedantic -Wall -c -g -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE=200112L 
LFLAGS=-lm

# all = release target
all: server client

# debug -> debug mode 
debug: CFLAGS+= -DENDEBUG -g 
debug: server client
 
server: server.o
	$(CC) $(LFLAGS) server.o -o server

client:	client.o client-ai.o
	$(CC) $(LFLAGS) client.o client-ai.o -o client

%.o: %.c
	$(CC) $(CFLAGS) $^

clean:
	rm -f *.o server client

.PHONY: all debug clean

