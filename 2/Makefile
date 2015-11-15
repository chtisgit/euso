# Makefile for
# Project: mastermind
# Author: Christian Fiedler <e1363562@student.tuwien.ac.at>

CC=clang
CFLAGS=-std=c99 -pedantic -Wall -c -g -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE=200112L 
LFLAGS=-lcrypt
OBJS=encr.o childlist.o
DOXYFILE=./Doxyfile
DOXY_TRASH=html latex doxygen_sqlite3.db

# all = release target
all: release

release: CFLAGS+= -O3
release: encr

# debug -> debug mode 
debug: CFLAGS+= -DENDEBUG -g -O0
debug: clean encr
 
encr: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o encr

%.o: %.c
	$(CC) $(CFLAGS) $^

clean:
	rm -f *.o encr
	rm -Rf $(DOXY_TRASH)

docs:
	doxygen $(DOXYFILE)

.PHONY: all docs release debug clean

