#ifndef __ENCR_H__
#define __ENCR_H__

#ifdef ENDEBUG
/* DEBUG MODE */
#define DEBUG(X)	X
#include <stdio.h>

#else
/* RELEASE MODE */
#define NDEBUG
#define DEBUG(X)	

#endif

#include <assert.h>

/*!
	\brief	waits for children to exit
		and frees memory
*/
void cleanup(void);

/*!	
	\brief	shows an error message, calls cleanup 
		and exits with EXIT_FAILURE
	\param	s
		name of the function that failed, which
		will be displayed by the error message
*/
void error(const char *f);

/*!
	\brief	waits for a child process to exit
		and marks it as inactive in childlist
*/
void wait_child(void);

/*! \brief	signal handler for SIGCHLD. Calls wait_child(). */
void signal_child(int s);

/*!
	\brief	removes inactive children from childlist
*/
int check_childlist(void);

/*! \brief	prints synopsis to screen */
void usage(void);

/*!
	\brief	reads an arbitrarily long string from the command line. 
		stops reading at EOF or newline
	\return	pointer to a dynamically allocated string, which contains
		the user input. 
*/
char* read_from_stdin(void);

/*!
	\brief	hashes password, waits some seconds and
		exits with EXIT_SUCCESS
	\param	pw
		Password that shall be hashed
	\param	sleep_time
		time in seconds that the process will sleep before
		printing the hash to screen
*/
void compute_pw(const char *pw, const int sleep_time);

/*!
	\brief	sets up the signal handler signal_child for SIGCHLD
	\return	0 on failure and 1 otherwise
*/
int setup_signal_handler(void);


#endif
