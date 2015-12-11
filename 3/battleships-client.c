#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

#include <ncurses.h>

#include <common.h>

const char *progname;

int main(int argc, char **argv)
{
	progname = argv[0];
	
	if(argc != 1){
		usage();
		return EXIT_FAILURE;
	}
	
	initscr();
	atexit((void* (void)) endwin);




	return EXIT_SUCCESS;
}
