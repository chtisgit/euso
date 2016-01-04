#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include <common.h>

void cleanup(void)
{
	if(shared->errorcode == EC_NONE){
		if(exitsig != 0)
			shared->errorcode = EC_SERVERSHUTDOWN;
		else
			shared->errorcode = EC_INTERNALERROR;
	}
	shared->stage = STAGE_SHUTDOWN;
	sem_post(sem[SEM_1]);
	sem_post(sem[SEM_2]);
	sem_post(sem[SEM_GLOBAL]);
	sem_post(sem[SEM_GLOBAL]);
	sem_post(sem[SEM_SYNC]);
	sem_post(sem[SEM_SYNC]);

	free_common_ressources();
	exit(EXIT_SUCCESS);
}


static int has_shot_hit(struct Ship *const ship)
{
	int i;

	for(i = 0; i < SHIP_COORDS; i++){
		if(ship->c[i].x == shared->shot.x && ship->c[i].y == shared->shot.y){
			ship->dead[i] = 1;
			return 1;
		}
	}
	return 0;
}

static int is_ship_dead(const struct Ship *const ship){
	int i;
	for(i = 0; i < SHIP_COORDS; i++){
		if(ship->dead[i] == 0)
			return 0;
	}
	return 1;
}

int main(int argc, char** argv)
{
	progname = argv[0];
	if(argc != 1){
		usage();
		return EXIT_FAILURE;
	}

	if(setup_signal_handler(NULL) == 0)
		bail_out("setup_signal_handler");

	atexit(free_common_ressources);
	
	const int oflag = O_CREAT | O_EXCL;
	sem[SEM_GLOBAL] = sem_open(SEM_NAME[SEM_GLOBAL], oflag, S_IRWXU, 1);
	sem[SEM_1] = sem_open(SEM_NAME[SEM_1], oflag, S_IRWXU, 0);
	sem[SEM_2] = sem_open(SEM_NAME[SEM_2], oflag, S_IRWXU, 0);
	sem[SEM_SYNC] = sem_open(SEM_NAME[SEM_SYNC], oflag, S_IRWXU, 0);

	for(int i = 0; i < LEN(sem); i++){
		if(sem[i] == SEM_FAILED)
			bail_out("sem_open");
	}

	if(allocate_shared(1) == 0)
		bail_out("allocate_shared");

	/* waiting for the two players */

	shared->players = 0;
	shared->stage = STAGE_WAIT;

	sem_wait_cb(sem[SEM_1], cleanup);
	
	sem_post(sem[SEM_GLOBAL]);

	sem_wait_cb(sem[SEM_2], cleanup);

	assert(shared->players == 2);
	
	/* now the players may position their ships */
	
	shared->stage = STAGE_SET;

	sem_post(sem[SEM_SYNC]);
	sem_post(sem[SEM_SYNC]);
	sem_wait_cb(sem[SEM_1], cleanup);
	sem_wait_cb(sem[SEM_2], cleanup);

	if(ship_check(&shared->ship[0]) == 0) cleanup();
	if(ship_check(&shared->ship[1]) == 0) cleanup();

	sem_post(sem[SEM_GLOBAL]);
	sem_post(sem[SEM_GLOBAL]);

	for(;;){
		shared->stage = STAGE_TURN1;
		sem_post(sem[SEM_1]);
		/* player 1 makes his turn (sets shared->shot)*/
		sem_wait_cb(sem[SEM_SYNC], cleanup);
		shared->hit = has_shot_hit(&shared->ship[1]);
		if(is_ship_dead(&shared->ship[0]) != 0){
			shared->errorcode = EC_GAMEOVER;
			shared->won = 1;
			cleanup();
		}
		sem_post(sem[SEM_1]);
		sem_wait_cb(sem[SEM_SYNC], cleanup);

		shared->stage = STAGE_TURN2;
		sem_post(sem[SEM_2]);
		/* player 2 makes his turn (sets shared->shot)*/
		sem_wait_cb(sem[SEM_SYNC], cleanup);
		shared->hit = has_shot_hit(&shared->ship[0]);
		if(is_ship_dead(&shared->ship[0]) != 0){
			shared->errorcode = EC_GAMEOVER;
			shared->won = 2;
			cleanup();
		}
		sem_post(sem[SEM_2]);
		sem_wait_cb(sem[SEM_SYNC], cleanup);
	}

	return EXIT_SUCCESS;
}
