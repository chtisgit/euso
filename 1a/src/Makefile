# Makefile for
# Project: postfixcalc
# Author: Christian Fiedler <e1363562@student.tuwien.ac.at>

CC=gcc
CFLAGS=-O3 -std=c99 -pedantic -Wall -c -g -D_XOPEN_SOURCE=500
LFLAGS=-lm

all: calc
 
calc: calc.o
	$(CC) $(LFLAGS) -o calc calc.o
 
calc.o: calc.c
	$(CC) $(CFLAGS) calc.c

clean:
	rm -f *.o calc

.PHONY: all clean

