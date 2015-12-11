#ifndef _COMMON_
#define _COMMON_

#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>

#define LEN(A)	(sizeof(A)/sizeof(*(A)))

extern const char *progname;
extern sig_atomic_t exit;

/* SHARED MEMORY */

#define SHMEM_NAME	"battleships-shm"

struct SharedStructure{
	enum{
		STAGE_WAIT,
		STAGE_SET1,
		STAGE_SET2,
		STAGE_TURN1,
		STAGE_TURN2
	} stage;
	int players;
};
extern struct SharedStructure *shared;

extern int shm_fd;

/* SEMAPHORES */

enum{ SEM_GLOBAL, SEM_1, SEM_2 };

extern const char *SEM_NAME[3];
extern sem_t *SEM[3];

void free_common_ressources(void);
void usage(void);
void bail_out(const char *s);
void sem_wait_cb(const sem_t *const s, void (callback*)(void));

#endif
