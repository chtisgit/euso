#ifndef _COMMON_
#define _COMMON_

#include <unistd.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>

#define LEN(A)	(sizeof(A)/sizeof(*(A)))

extern const char *progname;
extern sig_atomic_t exitsig;

/* SHARED MEMORY */

#define SHMEM_NAME	"battleships-shm"

struct Coord{
	int x, y;
};

#define SHIP_COORDS	3
struct Ship{
	struct Coord c[SHIP_COORDS];
	int dead[SHIP_COORDS];
};

#define FIELD_W		4
#define FIELD_H		4

struct FieldType{
	int buf[FIELD_W][FIELD_H];

};

struct SharedStructure{
	enum{
		EC_NONE,
		EC_SERVERSHUTDOWN,
		EC_CLIENTDISCONNECT,
		EC_INTERNALERROR,
		EC_GAMEOVER,
	} errorcode;

	enum{
		STAGE_WAIT,
		STAGE_SET,
		STAGE_TURN1,
		STAGE_TURN2,
		STAGE_SHUTDOWN
	} stage;
	int players;	/* number of players on the server */
	int won;	/* player number that has won */
	struct Ship ship[2];
	struct Coord shot;
};
extern struct SharedStructure *shared;

extern int shm_fd;

/* SEMAPHORES */

enum{ SEM_GLOBAL, SEM_1, SEM_2, SEM_SHUTDOWN, SEM_SYNC };

extern const char *SEM_NAME[3];
extern sem_t *sem[3];

void free_common_ressources(void);
void usage(void);
void bail_out(const char *s);
int sem_wait_sigsafe(const sem_t *const s);

#endif
