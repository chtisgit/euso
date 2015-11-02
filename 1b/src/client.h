/*!
	\file client.h
	\author Christian Fiedler <e1363562@student.tuwien.ac.at>
	\date 22.10.2015

	\brief header file for client.c

*/


#ifndef __CLIENT_H__
#define __CLIENT_H__

#ifndef NDEBUG
#ifndef ENDEBUG

/*	
	this means we don't compile debug target, so
	assert calls will vanish 
	this must precede #include <assert.h>
*/
#define NDEBUG

#endif
#endif

#include <assert.h>
#include <stdint.h>

/* program specific Exit Codes */
#define EC_OKAY		0
#define EC_FAILURE	1
#define EC_PARITY	2
#define EC_GAMELOST	3
#define EC_BOTH		4


#define SLOTS	5
#define COLORS	8
/* Keep in sync! */
enum Color { beige, darkblue, green, orange, red, black, violet, white };


/*!
	\brief frees all allocated ressources
	\detail
		in particular, calls ai_deinit() and
		closes the socket, if it was opened
*/
void deinit(void);

/*!
	\brief prints synopsis of program
*/
void usage(void);

/*!
	\brief establishes the connection to a server
	\param s_name
		Server's hostname or IP address
	\param s_port
		Destination port in base 10
	\details
		This function initializes the global sockfd with a socket
		which is connected to the server or fails and calls
		error() with a specific error message
*/
void establish_connection(const char *s_name, const char *s_port);

/*!
	\brief composes the message, which can be then sent to the server
	\param tip
		an array of colors with length SLOTS
		where the leftmost color is represented by the
		first element in the array
	\return 
		a 16 byte unsigned integer is returned, which contains
		the message (all the colors + parity bit) to the server.
*/
uint16_t compose_msg(enum Color tip[SLOTS]);

/*!
	\brief calculates the odd parity for a uint16_t
	\param msg
		a uint16_t type of which the parity shall be calculated
	\return 
		1 ... if msg's binary representation contains an odd number of 1's
		0 ... if not
*/
int parity(uint16_t msg);

/*!
	\brief composes the message, which can be then sent to the server
	\param tip
		an array of colors with length SLOTS
		where the leftmost color is represented by the
		first element in the array
	\return 
		a 16 byte unsigned integer is returned, which contains
		the message (all the colors + parity bit) to the server.
*/
uint16_t compose_msg(enum Color tip[SLOTS]);

/*!
	\brief Signal handler for SIGINT and SIGTERM
	\param sig
		Type of signal
	\details This functions sets the global variable quit = 1
*/
void signal_stopserver(int sig);

/*!
	\brief this function is actually playing the game
	\return
		play() returns the number of rounds played
		and only returns if no error occurred
		(otherwise error() is called)
	\details
		This functions uses the sockfd which should
		already be set up, and plays the game
		against the server using the calls of the
		AI interface
*/
int play(void);

/*!
	\brief main function
	\return
		0 .. if the game was one
		1 .. in case of a syntax error in command args
		2 .. if a message sent by the client got the parity bit wrong
		3 .. if the client lost the game
		4 .. if parity bit is wrong and client lost the game
	\details
		main() sets up the signal handler signal_stopserver(),
		parses the command line options, initializes the AI,
		establishes the connection, prints the rounds played
		to screen and exits.
		(if no function called error())

*/
int main(int argc, char** argv);

#endif
