/*!
	\file	encr.h
	\author	Christian Fiedler <e1363562@student.tuwien.ac.at>
	\date	16.11.2015

	\brief	header file for main file encr.c

	\details
		this program hashes passwords
*/

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
	\brief	waits for children to exit (if any)
		and frees memory
	\details
		determines by checking the global variable child_process,
		if the parent or a child is exiting
		If it is the parent, cleanup waits for additional child 
		processes to exit.
		Then for both cases the childlist data structure is freed.
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
	\details
		uses the global variable childlist
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
	\details
		this function is called by main in a child process
		created by fork()
		The global variable child_process is set to 1 when
		this function is called
*/
void compute_pw(const char *pw, int sleep_time);

/*!
	\brief	sets up the signal handler signal_child for SIGCHLD
	\return	0 on failure and 1 otherwise
*/
int setup_signal_handler(void);

/*!
	\brief	main program
	\return	EXIT_SUCCESS on success and EXIT_FAILURE otherwise
	\details
		sets up the global variable progname to point to argv[0]
		initializes the global childlist variable
		sets child_process to 1 for child processes created by fork()
		after checking for correct usage of the program, main sets
		up cleanup() to be run at exit.

*/
int main(int argc, char **argv);

#endif
