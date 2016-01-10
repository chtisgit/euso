/*!
	\file	common.c
	\author	Christian Fiedler <e1363562@student.tuwien.ac.at>
	\date	10.01.2015

	\brief	battleships common code

	\details
		this is code, that both the server and
		the client application of the battleships
		project use.
		
*/

#include <common.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

sig_atomic_t exitsig = 0;

const char *progname = NULL;

int shm_fd = -1;
struct SharedStructure *shared = NULL;

const char *SEM_NAME[] = { "/battlesS", "/battlesA", "/battles1", "/battles2", "/battlesZ", "/battlesE" };
sem_t *sem[] = { SEM_FAILED, SEM_FAILED, SEM_FAILED, SEM_FAILED, SEM_FAILED, SEM_FAILED };

const char FIELD_CHAR[] = { '#' , '~', 'x', ' ' };
const char *SRV_MSG[] = {
	"", "",
	"Your opponent left the game!",
	"Internal Error!",
	"Game over!",
};

void usage(void)
{
	(void)fprintf(stderr, "USAGE: %s\n", progname);
}

void bail_out(const char *s)
{
	(void)fprintf(stderr, "error: function '%s'\n",s);
	exit(EXIT_FAILURE);
}

int allocate_shared(int owner)
{
	assert(LEN(sem) == LEN(SEM_NAME) && LEN(SEM_NAME) == SEM_LAST);

	const int oflag = (owner == 0 ? 0 : O_CREAT) | O_RDWR;
	shm_fd = shm_open(SHMEM_NAME, oflag, 0600);
	if(shm_fd == -1) return 0;

	if(owner != 0){
		if(ftruncate(shm_fd, sizeof(*shared)) == -1) return 0;
	}
	shared = mmap(NULL, sizeof(*shared), PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd, 0);
	(void)close(shm_fd);
	
	if(shared == MAP_FAILED){
		shared = NULL;
		return 0;
	}
	return 1;
}

void ship_init(struct Ship *const ship)
{
	int i;
	for(i = 0; i < SHIP_COORDS; i++){
		ship->c[i].x = ship->c[i].y = 0;
		ship->dead[i] = 0;
	}
}

int ship_check(const struct Ship *const ship)
{
	int i, j, k, n, correct;
	const struct Coord *c = ship->c;

	for(i = 0; i < FIELD_W; i++){
		n = 0;
		for(j = 0; j < FIELD_H; j++){
			correct = 0;
			for(k = 0; k < SHIP_COORDS; k++){
				if(i == c[k].x && j == c[k].y){
					correct = 1;
					break;
				}
			}
			if(correct != 0){
				n++;
				if(n == SHIP_COORDS)
					return 1;
			}else{
				n = 0;
			}
		}
	}
	for(j = 0; j < FIELD_H; j++){
		n = 0;
		for(i = 0; i < FIELD_W; i++){
			correct = 0;
			for(k = 0; k < SHIP_COORDS; k++){
				if(i == c[k].x && j == c[k].y){
					correct = 1;
					break;
				}
			}
			if(correct != 0){
				n++;
				if(n == SHIP_COORDS)
					return 1;
			}else{
				n = 0;
			}
		}
	}
	for(i = SHIP_COORDS-FIELD_H; i < FIELD_W-SHIP_COORDS+1; i++){
		n = 0;
		for(j = 0; j < FIELD_H; j++){
			correct = 0;
			for(k = 0; k < SHIP_COORDS; k++){
				if(i+j >= 0 && i+j < FIELD_W && i+j == c[k].x && j == c[k].y){
					correct = 1;
					break;
				}
			}
			if(correct != 0){
				n++;
				if(n == SHIP_COORDS)
					return 1;
			}else{
				n = 0;
			}
		}
	}
	for(i = SHIP_COORDS-FIELD_H; i < FIELD_W-SHIP_COORDS+1; i++){
		n = 0;
		for(j = 0; j < FIELD_H; j++){
			correct = 0;
			for(k = 0; k < SHIP_COORDS; k++){
				const int x = i+j;
				const int y = FIELD_H - j - 1;
				if(y >= 0 && y < FIELD_H && x >= 0 && x < FIELD_W && x == c[k].x && y == c[k].y){
					correct = 1;
					break;
				}
			}
			if(correct != 0){
				n++;
				if(n == SHIP_COORDS)
					return 1;
			}else{
				n = 0;
			}
		}
	}
	
	return 0;
}

/*! \brief see setup_signal_handler */
static void (*signal_handler_cb)(int);

/*! \brief see setup_signal_handler */
static void minimal_handler(int signr){
	exitsig = 1;
	if(signal_handler_cb != NULL)
		signal_handler_cb(signr);
}

int setup_signal_handler(void (*handler)(int))
{
	struct sigaction sa = {
		.sa_handler = minimal_handler,
		.sa_flags = SA_NOCLDSTOP
	};

	signal_handler_cb = handler;

	if(sigfillset(&sa.sa_mask) < 0)
		return 0;

	return sigaction(SIGINT, &sa, NULL) == -1 || sigaction(SIGTERM, &sa, NULL) == -1 ? 0 : 1;
}

/*!
	\brief frees the sem[] array
	\param owner
		if owner is nonzero, further deinitialization (sem_unlink)
		is performed
	\details
		Every element of sem[] will be SEM_FAILED after this function
		is called. If owner is nonzero, the semaphores will be unlinked
*/
static void free_semaphores(int owner)
{
	int i;
	for(i = 0; i < LEN(sem); i++){
		if(sem[i] != SEM_FAILED){
			(void)sem_close(sem[i]);
			sem[i] = SEM_FAILED;
		}
		if(owner != 0) 
			(void)sem_unlink(SEM_NAME[i]);
	}
}

/*!
	\brief frees the shared memory
	\param owner
		if owner is nonzero, further deinitialization (shm_unlink)
		is performed
	\details
		shared will be set to NULL after this call, if shm_fd was 
		not -1.
*/
static void free_shared(int owner)
{
	if(shm_fd != -1){
		if(shared != NULL){
			munmap(shared, sizeof(*shared));
			shared = NULL;
		}
		if(owner != 0)
			(void)shm_unlink(SHMEM_NAME);
	}	
}

void free_common_ressources_owner(void)
{
	free_semaphores(1);
	free_shared(1);
}

void free_common_ressources(void)
{
	free_semaphores(0);
	free_shared(0);
}

void sem_wait_cb(sem_t *s, void (*cb)(void))
{
	int x;
	do{
		x = sem_wait(s);
		if(cb != NULL && exitsig != 0) cb();
	}while(x == -1 && errno == EINTR);
}

