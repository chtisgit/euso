/*!
	\file client-ai.h
	\author Christian Fiedler <e1363562@student.tuwien.ac.at>
	\date 22.10.2015

	\brief header file for client-ai.c (exposed functions)

	\details
		This module exposes an API for a mastermind AI
		through client-ai.h and implements it in client-ai.c
*/
#ifndef __CLIENT_AI_H__
#define __CLIENT_AI_H__

/*!
	\brief does initialization of ai unit
	\return	1 on success, 0 otherwise

	This function initializes the AI. It must be called before 
	using ai_guess and ai_response.
*/
int ai_init(void);


/*!
	\brief does deinitialization of ai unit

	This function deinitializes the AI. It should be called
	after a game is over. After that you can call ai_init again
*/
void ai_deinit(void);

/*!
	\brief guesses a color combination
	\param tip
		array of colors with length SLOTS
	
	The array tip is filled with the AI's next guess
*/
void ai_guess(enum Color tip[SLOTS]);


/*!
	\brief tells the AI the result of its last guess
	\param tip
		array of colors with length SLOTS (which the AI 
		provided when ai_guess was called last)
	
	When this function is called, the AI will rate its guess
	and prepare for the next one.
*/
void ai_response(enum Color tip[SLOTS], int red, int white);



#endif

