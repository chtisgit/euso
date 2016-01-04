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

struct Field{
	enum{
		FIELD_UNKNOWN,
		FIELD_WATER,
		FIELD_HIT,
		FIELD_NIL,
		FIELD_LAST
	} buf[FIELD_W][FIELD_H];
};
extern const char FIELD_CHAR[FIELD_LAST];


struct SharedStructure{
	enum{
		EC_NONE,
		EC_SERVERSHUTDOWN,
		EC_CLIENTDISCONNECT,
		EC_INTERNALERROR,
		EC_GAMEOVER,
		EC_LAST
	} errorcode;

	enum{
		STAGE_WAIT,
		STAGE_SET,
		STAGE_TURN1,
		STAGE_TURN2,
		STAGE_SHUTDOWN,
		STAGE_LAST
	} stage;
	int players;	/* number of players on the server */
	int won;	/* player number that has won */
	struct Ship ship[2];
	struct Coord shot;
	int hit;
};
extern const char *SRV_MSG[EC_LAST];
extern struct SharedStructure *shared;

extern int shm_fd;

/* SEMAPHORES */

/* don't use SEM_LAST */
enum{ SEM_GLOBAL, SEM_1, SEM_2, SEM_SYNC, SEM_LAST };

extern const char *SEM_NAME[SEM_LAST];
extern sem_t *sem[SEM_LAST];

int setup_signal_handler(void (*handler)(int));
int allocate_shared(int owner);
void free_common_ressources(void);
void usage(void);
int ship_check(const struct Ship *const ship);
void bail_out(const char *s);
void sem_wait_cb(sem_t *s, void (*cb)(void));

#define sem_wait_atom(X)	sem_wait_cb((X), NULL)

#endif
