# Makefile for
# Project: battleships
# Author: Christian Fiedler <e1363562@student.tuwien.ac.at>

PROJ=battleships
EXE_1=$(PROJ)-client
EXE_2=$(PROJ)-server
PROJFILES=*.c *.h Makefile Doxyfile
PROJARCHIVE="$(PROJ).tgz"

CC=clang
CFLAGS=-std=c99 -I. -pedantic -Wall -c -D_XOPEN_SOURCE=500 -D_BSD_SOURCE
LFLAGS=-pthread -lncurses -lrt

OBJS_1=battleships-client.o common.o
OBJS_2=battleships-server.o common.o

DOXYFILE=./Doxyfile
DOXY_TRASH=html latex doxygen_sqlite3.db



# all = release target
all: release

release: CFLAGS+= -O3
release: _all

# debug -> debug mode 
debug: CFLAGS+= -DENDEBUG -g -O0
debug: _all
 
_all: $(EXE_1) $(EXE_2)

$(EXE_1): $(OBJS_1)
	$(CC) $(LFLAGS) $(OBJS_1) -o $(EXE_1)

$(EXE_2): $(OBJS_2)
	$(CC) $(LFLAGS) $(OBJS_2) -o $(EXE_2)

%.o: %.c
	$(CC) $(CFLAGS) $^

clean:
	rm -f *.o $(EXE_1) $(EXE_2) $(PROJARCHIVE)
	rm -Rf $(DOXY_TRASH)

docs:
	doxygen $(DOXYFILE)

export:
	tar cfz $(PROJARCHIVE) $(PROJFILES)

.PHONY: all docs release debug clean _all

