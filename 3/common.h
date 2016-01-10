/*!
	\file	common.h
	\author	Christian Fiedler <e1363562@student.tuwien.ac.at>
	\date	10.01.2015

	\brief	battleships common code (header)

	\details
		this is code, that both the server and
		the client application of the battleships
		project use.
		
*/

#ifndef _COMMON_
#define _COMMON_

#ifndef ENDEBUG
#define NDEBUG
#endif

#include <unistd.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>

#define LEN(A)	(sizeof(A)/sizeof(*(A)))

/*! \brief this will be set to argv[0] in main */
extern const char *progname;

/*! \brief this variable is set to nonzero on SIGINT and SIGTERM */
extern sig_atomic_t exitsig;

/* SHARED MEMORY */

#define SHMEM_NAME	"battleships-shm"

/*! \brief saves a 2D coordinate */
struct Coord{
	int x, y;
};

#define SHIP_COORDS	3
/*! \brief ship data structure */
struct Ship{
	/* \brief saves the coordinates of the ship */
	struct Coord c[SHIP_COORDS];

	/* \brief saves the dead coordinates of the ship */
	int dead[SHIP_COORDS];
};

#define FIELD_W		4
#define FIELD_H		4

/*! \brief game field data structure */
struct Field{
	/*! \brief 2D array that corresponds to the game board */
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

	/*! \brief number of players on the server */
	int players;

	/*! \brief player number that has won */
	int won;

	/*! \brief the two players' ships */
	struct Ship ship[2];

	/*! \brief a shot coordinate that is used when the players are shooting */
	struct Coord shot;

	/*! \brief the surrender flags */
	int surrender[2];

	/*! \brief hit will be set to nonzero if the previous shot hit*/
	int hit;
};
extern const char *SRV_MSG[EC_LAST];
extern struct SharedStructure *shared;

extern int shm_fd;

/* SEMAPHORES */

/* don't use SEM_LAST */
enum{ SEM_START, SEM_GLOBAL, SEM_1, SEM_2, SEM_SYNC, SEM_EXIT, SEM_LAST };

extern const char *SEM_NAME[SEM_LAST];
extern sem_t *sem[SEM_LAST];

/*! \brief prints the synopsis of the program */
void usage(void);

/*! \brief exits with an error message */
void bail_out(const char *s);

/*! 
	\brief allocates shared memory
	\param owner
		if nonzero, further initializations are performed (ftruncate)
	\return
		nonzero on success
*/
int allocate_shared(int owner);

/*! \brief initializes a ship datastructure with zero */
void ship_init(struct Ship *const ship);

/*!
	\brief checks a ship data structure for validness
	\param ship
		a const pointer to a ship data structure
	\return
		nonzero if ship data structure is valid

*/
int ship_check(const struct Ship *const ship);

/*!
	\brief initializes signal handler for SIGINT and SIGTERM
	\param handler
		a signal handler function to call
		Can be NULL
	\return
		nonzero if setup succeeds, zero otherwise
	\details 
		this function sets minimal_handler as signal handler
		handler is saved in signal_handler_cb
		when minimal_handler is called and signal_handler_cb is
		not NULL, it will be called.
*/
int setup_signal_handler(void (*handler)(int));

/*! \brief calls free_semaphores() and free_shared() with zero parameter */
void free_common_ressources(void);

/*! \brief calls free_semaphores() and free_shared() with nonzero parameter */
void free_common_ressources_owner(void);

/*!
	\brief calls sem_wait and checks for incoming signals
	\param s
		a semaphore
	\param cb
		a callback function
	\details
		sem_wait_cb calls sem_wait on s. After the call, if
		exitsig was set to nonzero and cb is not NULL, it will be
		called.
		This is repeated until sem_wait succeeds or fails with anything
		not equal to errno=EINTR.
*/
void sem_wait_cb(sem_t *s, void (*cb)(void));

#endif
