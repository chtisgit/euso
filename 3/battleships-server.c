/*!
	\file	battleships-server.c
	\author	Christian Fiedler <e1363562@student.tuwien.ac.at>
	\date	10.01.2015

	\brief	battleships game server

	\details
		with this program two players can play
		battleships over shared memory, yay!
		
*/

#include <common.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

/*!
	\brief shuts down the server
	\details
		shared->errorcode will be set to a valid and appropriate value
		shared->stage will be set to STAGE_SHUTDOWN
		all semaphores that may block clients will be posted to
*/
void shutdown(void)
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
}

/*! \brief calls shutdown() and then exit with EXIT_SUCCESS */
void cleanup(void)
{
	shutdown();
	exit(EXIT_SUCCESS);
}

/*!
	\brief checks if the given ship is dead
	\param ship
		a const pointer to the given ship
	\return
		zero if it is alive, nonzero if it is dead
*/
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

/*!
	\brief checks if the given ship is dead
	\param ship
		a const pointer to the given ship
	\return
		zero if it is alive, nonzero if it is dead
*/
static int is_ship_dead(const struct Ship *const ship){
	int i;
	for(i = 0; i < SHIP_COORDS; i++){
		if(ship->dead[i] == 0)
			return 0;
	}
	return 1;
}

/*!
	\brief checks the surrender flags in the shared structure
	\param errorcode
		an error code (typically EC_GAMEOVER);
	\return zero if the surrender flags are not set, nonzero if they are
	\details
		if one of the surrender flags is set, then the player, that
		is still ingame, will win.
		shutdown() will be called and nonzero will be returned
*/
static int check_surrender(int errorcode)
{
	assert(shared != NULL);
	if(shared->surrender[0] != 0){
		shared->errorcode = errorcode;
		shared->won = 2;
		shutdown();
		return 1;
	}else if(shared->surrender[1] != 0){
		shared->errorcode = errorcode;
		shared->won = 1;
		return 1;
	}
	return 0;
}

/*!
	\brief this function initializes and plays the game with the clients
	\details
		the function returns :
		- if the game is over,
		- if the server is shut down, or
		- if a player left
*/
static void game(void)
{
	shared->errorcode = EC_NONE;
	shared->players = 0;
	shared->players_gone = 0;
	shared->won = 0;
	shared->surrender[0] = shared->surrender[1] = 0;
	shared->stage = STAGE_WAIT;
	ship_init(&shared->ship[0]);
	ship_init(&shared->ship[1]);

	/* waiting for the two players */
	sem_post(sem[SEM_START]);
	sem_wait_cb(sem[SEM_1], cleanup);
	if(check_surrender(EC_CLIENTDISCONNECT) != 0) return;

	sem_post(sem[SEM_START]);
	sem_wait_cb(sem[SEM_2], cleanup);
	if(check_surrender(EC_CLIENTDISCONNECT) != 0) return;

	if(shared->players != 2 || shared->players_gone != 0){
		shutdown();
		return;
	}
	
	/* now the players may position their ships */
	
	shared->stage = STAGE_SET;

	sem_post(sem[SEM_SYNC]);
	sem_post(sem[SEM_SYNC]);
	sem_wait_cb(sem[SEM_1], cleanup);
	if(check_surrender(EC_CLIENTDISCONNECT) != 0) return;

	sem_wait_cb(sem[SEM_2], cleanup);
	if(check_surrender(EC_CLIENTDISCONNECT) != 0) return;

	if(ship_check(&shared->ship[0]) == 0 || ship_check(&shared->ship[1]) == 0){
		cleanup();
		return;
	}

	sem_post(sem[SEM_GLOBAL]);
	sem_post(sem[SEM_GLOBAL]);

	for(;;){
		shared->stage = STAGE_TURN1;
		sem_post(sem[SEM_1]);
		/* player 1 makes his turn (sets shared->shot)*/
		sem_wait_cb(sem[SEM_SYNC], cleanup);
		if(check_surrender(EC_GAMEOVER) != 0) return;

		shared->hit = has_shot_hit(&shared->ship[1]);
		if(is_ship_dead(&shared->ship[1]) != 0){
			shared->errorcode = EC_GAMEOVER;
			shared->won = 1;
			shutdown();
			return;
		}
		sem_post(sem[SEM_1]);
		sem_wait_cb(sem[SEM_SYNC], cleanup);
		if(check_surrender(EC_GAMEOVER) != 0) return;

		shared->stage = STAGE_TURN2;
		sem_post(sem[SEM_2]);
		/* player 2 makes his turn (sets shared->shot)*/
		sem_wait_cb(sem[SEM_SYNC], cleanup);
		if(check_surrender(EC_GAMEOVER) != 0) return;

		shared->hit = has_shot_hit(&shared->ship[0]);
		if(is_ship_dead(&shared->ship[0]) != 0){
			shared->errorcode = EC_GAMEOVER;
			shared->won = 2;
			shutdown();
			return;
		}
		sem_post(sem[SEM_2]);
		sem_wait_cb(sem[SEM_SYNC], cleanup);
		if(check_surrender(EC_GAMEOVER) != 0) return;
	}

}

/*! \brief does everything */
int main(int argc, char** argv)
{
	progname = argv[0];
	if(argc != 1){
		usage();
		return EXIT_FAILURE;
	}

	if(setup_signal_handler(NULL) == 0)
		bail_out("setup_signal_handler");

	atexit(free_common_ressources_owner);
	
	const int oflag = O_CREAT;
	int shm_owner = 1;
	for(;;){
		sem[SEM_START] = sem_open(SEM_NAME[SEM_START], oflag, S_IRWXU, 0);
		sem[SEM_EXIT] = sem_open(SEM_NAME[SEM_EXIT], oflag, S_IRWXU, 0);
		sem[SEM_GLOBAL] = sem_open(SEM_NAME[SEM_GLOBAL], oflag, S_IRWXU, 0);
		sem[SEM_1] = sem_open(SEM_NAME[SEM_1], oflag, S_IRWXU, 0);
		sem[SEM_2] = sem_open(SEM_NAME[SEM_2], oflag, S_IRWXU, 0);
		sem[SEM_SYNC] = sem_open(SEM_NAME[SEM_SYNC], oflag, S_IRWXU, 0);

		for(int i = 0; i < LEN(sem); i++){
			if(sem[i] == SEM_FAILED)
				bail_out("sem_open");

			int val;
			while(sem_getvalue(sem[i],&val), val > 0)
				sem_wait(sem[i]);
		}

		if(allocate_shared(shm_owner) == 0)
			bail_out("allocate_shared");
		shm_owner = 0;

		(void)printf("Starting new game...\n");
		game();
		(void)printf("Game Over. Waiting for players to exit...\n");

		while(shared != NULL && shared->players_gone < 2){
			sem_wait_cb(sem[SEM_EXIT], cleanup);
		}
		/* NOT free_common_ressources_owner() ! */
		free_common_ressources();
	}

	return EXIT_SUCCESS;
}
