CC = clang
CFLAGS = -ansi -Wall -O2 -g
OBJS = telnetd.o

all: telnetd
	
telnetd: $(OBJS)
	$(CC) -o $@ $(OBJS)

%.o:%.c
	$(CC) $(CFLAGS)  -c $<
	
clean:
	rm -f telnetd *.o

.PHONY: all clean
