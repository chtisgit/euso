#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include <common.h>

const char *progname;

void cleanup(void)
{
	if(shared->errorcode == EC_NONE){
		if(exitsig != 0)
			shared->errorcode = EC_SERVERSHUTDOWN;
		else
			shared->errorcode = EC_INTERNALERROR;
	}

	shared->stage = STAGE_SHUTDOWN;
	sem_post(sem[SEM_GLOBAL]);
	sem_post(sem[SEM_1]);
	sem_wait(sem[SEM_SHUTDOWN]);
	if(shared->players > 0){
		sem_post(sem[SEM_2]);
		sem_wait(sem[SEM_SHUTDOWN]);
	}
	assert(shared->players == 0);
	free_common_ressources();
	exit(EXIT_SUCCESS);
}

int main(int argc, char** argv)
{
	atexit(free_common_ressources);

	sem[SEM_GLOBAL] = sem_open(SEM_NAME[SEM_GLOBAL], O_CREAT, O_RDWR, 1);
	sem[SEM_1] = sem_open(SEM_NAME[SEM_1], O_CREAT, O_RDWR, 0);
	sem[SEM_2] = sem_open(SEM_NAME[SEM_2], O_CREAT, O_RDWR, 0);
	sem[SEM_SHUTDOWN] = sem_open(SEM_NAME[SEM_SHUTDOWN], O_CREAT, O_RDWR, 0);
	sem[SEM_SYNC] = sem_open(SEM_NAME[SEM_SYNC], O_CREAT, O_RDWR, 0);

	for(int i = 0; i < LEN(sem); i++){
		if(sem[i] == SEM_FAILED)
			bail_out("sem_open");
	}

	if(allocate_shared() == 0)
		bail_out("allocate_shared");

	/* waiting for the two players */

	shared->players = 0;
	shared->stage = STAGE_WAIT;

	memset(shared->ship, 0, sizeof(shared->ship));
	printf("sizeof(shared->ship) : %d\n",sizeof(shared->ship));


	if(sem_wait_sigsafe(sem[SEM_1]) == 0) cleanup();
	
	sem_post(sem[SEM_GLOBAL]);

	if(sem_wait_sigsafe(sem[SEM_2]) == 0) cleanup();

	assert(shared->players == 2);
	
	/* now the players may position their ships */
	
	shared->stage = STAGE_SET;

	if(sem_wait_sigsafe(sem[SEM_1]) == 0) cleanup();
	if(sem_wait_sigsafe(sem[SEM_2]) == 0) cleanup();

	if(ship_check(&shared->ship[0]) == 0) cleanup();
	if(ship_check(&shared->ship[1]) == 0) cleanup();

	sem_post(sem[SEM_GLOBAL]);
	sem_post(sem[SEM_GLOBAL]);


	for(;;){
		shared->stage = STAGE_TURN1;
		sem_post(sem[SEM_1]);
		/* player 1 makes his turn (sets shared->shot)*/
		sem_wait(sem[SEM_SYNC]);

		shared->stage = STAGE_TURN2;
		sem_post(sem[SEM_2]);
		/* player 2 makes his turn (sets shared->shot)*/
		sem_wait(sem[SEM_SYNC]);
	}

	return EXIT_SUCCESS;
}
