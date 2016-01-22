CC=gcc
CFLAGS=-Wall -std=c99 -g -O2 

all: shitshell

shitshell: shitshell.o
	$(CC) $^ -o $@

%.o: %.c
	$(CC) -c $^ -o $@

clean:
	rm -f *.o

.PHONY: all clean
