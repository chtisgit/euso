#include <stdio.h>
#include <stdlib.h>
#include <common.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

sig_atomic_t exitsig = 0;
const char *progname = NULL;

int shm_fd = -1;
struct SharedStructure *shared = NULL;

const char *SEM_NAME[] = { "/battlesA", "/battles1", "/battles2", "/battlesZ" };
sem_t *sem[] = { SEM_FAILED, SEM_FAILED, SEM_FAILED, SEM_FAILED };

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
	fprintf(stderr, "error: function '%s'\n",s);
	exit(EXIT_FAILURE);
}

int allocate_shared(int owner)
{
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

static void (*signal_handler_cb)(int);

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

void free_common_ressources(void)
{
	int i;
	for(i = 0; i < LEN(sem); i++){
		if(sem[i] != NULL){
			(void)sem_close(sem[i]);
			(void)sem_unlink(SEM_NAME[i]);
			sem[i] = SEM_FAILED;
		}
	}

	if(shm_fd != -1){
		if(shared != NULL){
			munmap(shared, sizeof(*shared));
			shared = NULL;
		}
		(void)shm_unlink(SHMEM_NAME);
	}	
}

void sem_wait_cb(sem_t *s, void (*cb)(void))
{
	int x;
	do{
		x = sem_wait(s);
		if(cb != NULL && exitsig != 0) cb();
	}while(x == -1 && errno == EINTR);
}

