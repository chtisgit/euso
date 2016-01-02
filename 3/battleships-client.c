#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include <ncurses.h>

#include <common.h>

const char *progname;

static void cleanup_curses(void)
{
	(void)endwin();
}

#define GFX_W	(FIELD_W*2+1+2)
#define GFX_H	(FIELD_H+4)

int main(int argc, char **argv)
{
	progname = argv[0];
	
	if(argc != 1){
		usage();
		return EXIT_FAILURE;
	}

	struct FieldType gamef = {{{0}}};

	(void)initscr();
	(void)atexit( cleanup_curses );


	WINDOW *win = newwin(GFX_H,GFX_W,2,2);

	box(win, '*', '*');
	touchwin(win);
	wrefresh(win);

	getchar();	




	return EXIT_SUCCESS;
}
