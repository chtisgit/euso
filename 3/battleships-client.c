#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>

#include <curses.h>

#include <common.h>

const char *progname;
static int player_nr = 0;
static int mysem = -1;

static const char *STAGE_STR[] = {
	"Waiting for player 2...",
	"Position your ship! Use Space to toggle blocks and Enter to confirm!",
	"It's your turn!",
	"It's your opponent's turn!",
	"Server shutting down"
};

static const char* map_stage_str(void)
{
	assert(player_nr == 1 || player_nr == 2);
	assert(shared != NULL);
	assert(shared->stage >= 0 && shared->stage < STAGE_LAST);

	if(player_nr == 2){
		if(shared->stage == STAGE_TURN1)
			return STAGE_STR[STAGE_TURN2];
		else if(shared->stage == STAGE_TURN2)
			return STAGE_STR[STAGE_TURN1];
	}
	return STAGE_STR[shared->stage];
}

static void cleanup_curses(void)
{
	if(isendwin() != TRUE)
		(void)endwin();
}

#define GFX_W	(FIELD_W*2+1+2)
#define GFX_H	(FIELD_H+4)

static void draw_game_field(WINDOW *win, const struct Field *const gamef, int cursor_x, int cursor_y)
{
	int x, y;
	(void)wclear(win);
	(void)box(win, '|', '-');

	for(y = 0; y < FIELD_H; y++){
		for(x = 0; x < FIELD_W; x++){
			(void)mvwaddch(win, y+2, x*2+2, FIELD_CHAR[gamef->buf[x][y]]);
		}
	}
	
	cursor_y += 2;
	cursor_x = cursor_x*2+2;

	(void)mvwaddch(win, cursor_y, cursor_x-1, '>');
	(void)mvwaddch(win, cursor_y, cursor_x+1, '<');

	(void)touchwin(win);
	(void)refresh();
	(void)wrefresh(win);

	if(player_nr == 1)
		(void)mvaddstr(0, 2, "PLAYER 1");
	else if(player_nr == 2)
		(void)mvaddstr(0, 2, "PLAYER 2");
	else
		assert(0);
	
	if(shared != NULL)
		(void)mvaddstr(0, 2, map_stage_str());

	(void)move(0,0);
}

static int build_ship(const struct Field *const gamef)
{
	struct Ship *const myship = &shared->ship[player_nr-1];
	int coordn = 0, x, y;
	for(y = 0; y < FIELD_H; y++){
		for(x = 0; x < FIELD_W; x++){
			if(gamef->buf[x][y] != FIELD_NIL){
				if(coordn == SHIP_COORDS) return 0;
				myship->c[coordn].x = x;
				myship->c[coordn].y = y;
				coordn++;
			}
		}
	}
	return ship_check(myship);
}

static void set_ship(WINDOW *win)
{
	struct Field gamef = {{{0}}};
	int enter = 0, x,y;

	for(y = 0; y < FIELD_H; y++){
		for(x = 0; x < FIELD_W; x++){
			gamef.buf[x][y] = FIELD_NIL;
		}
	}

	x = y = 0;

	do{
		draw_game_field(win, &gamef, x, y);
		int ch = getch();
		switch(ch){
		case KEY_ENTER:
			enter = build_ship(&gamef);
			break;
		case KEY_LEFT:
		case 'A': case 'a':
			if(x > 0) x--;
			break;
		case KEY_RIGHT:
		case 'D': case 'd':
			if(x < FIELD_W-1) x++;
			break;
		case KEY_UP:
		case 'W': case 'w':
			if(y > 0) y--;
			break;
		case KEY_DOWN:
		case 'S': case 's':
			if(y < FIELD_H-1) y++;
			break;
		case ' ':
			if(gamef.buf[x][y] == FIELD_NIL)
				gamef.buf[x][y] = FIELD_HIT;
			else
				gamef.buf[x][y] = FIELD_NIL;
			break;
		}
	}while(enter == 0);

}

int main(int argc, char **argv)
{
	struct Field gamef = {{{0}}};
	int cursor_x, cursor_y;

	progname = argv[0];
	if(argc != 1){
		usage();
		return EXIT_FAILURE;
	}

	atexit(free_common_ressources);

	sem[SEM_GLOBAL] = sem_open(SEM_NAME[SEM_GLOBAL], 0);
	sem[SEM_1] = sem_open(SEM_NAME[SEM_1], 0);
	sem[SEM_2] = sem_open(SEM_NAME[SEM_2], 0);
	sem[SEM_SHUTDOWN] = sem_open(SEM_NAME[SEM_SHUTDOWN], 0);
	sem[SEM_SYNC] = sem_open(SEM_NAME[SEM_SYNC], 0);

	for(int i = 0; i < LEN(sem); i++){
		if(sem[i] == SEM_FAILED)
			bail_out("sem_open");
	}

	if(allocate_shared(0) == 0)
		bail_out("allocate_shared");

	(void)initscr();
	(void)atexit( cleanup_curses );
	WINDOW *win = newwin(GFX_H,GFX_W,3,2);
	cursor_x = cursor_y = 0;

	/* requesting access and setting player number */
	sem_wait(sem[SEM_GLOBAL]);
	player_nr = ++shared->players;
	if(player_nr == 1)
		mysem = SEM_1;
	else if(player_nr == 2)
		mysem = SEM_2;
	else
		assert(0);

	draw_game_field(win, &gamef, cursor_x, cursor_y);
	sem_post(sem[mysem]);

	/* waiting for SET stage*/
	sem_wait(sem[SEM_SYNC]);
	assert(shared->stage == STAGE_SET);

	set_ship(win);


	


	getch();

	delwin(win); 

	return EXIT_SUCCESS;
}
