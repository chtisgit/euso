/*!
	\file client-ai.c
	\author Christian Fiedler <e1363562@student.tuwien.ac.at>
	\date 22.10.2015

	\brief Artificial "Intelligence" for Mastermind

	\details
		This module exposes an API for a mastermind AI
		through client-ai.h and implements it in client-ai.c
*/

#include "client.h"
#include "client-ai.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FIRST_GUESS	0

static int *all = NULL;
static int all_size = 0;
static int next_guess = FIRST_GUESS;

/*! calculates COLORS to the power of SLOTS */
inline const int calc_all_size()
{
	int size = COLORS;
	for(int i = 1; i <= SLOTS-1; i++){
		size *= COLORS;
	}
	return size;
}

int ai_init(void)
{
	all_size = calc_all_size();
	all = calloc( sizeof(*all), all_size );
	if(all == NULL)
		return 0;
	return 1;
}

void ai_deinit(void)
{
	if(all != NULL)
		free(all);
}

static void decode(int coded, enum Color tip[SLOTS])
{
	for(int j = 0; j < SLOTS; j++){
		tip[j] = (enum Color) (coded % COLORS);
		coded /= COLORS;
	}
}

void ai_guess(enum Color tip[SLOTS])
{
	decode(next_guess, tip);
}

static int possible(int coded, enum Color tip[SLOTS], int red, int white)
{
	enum Color tip2[SLOTS];
	decode(coded, tip2);

	int redc = 0, i;
	for(i = 0; i < SLOTS; i++){
		if(tip[i] == tip2[i]) redc++;
	}
	if(redc != red) return 0;

	int whitec = 0;
	for(i = 0; i < SLOTS; i++){
		if(tip[i] != tip2[i]){
			for(int j = 0; j < SLOTS; j++){
				if(tip2[i] == tip[j]){
					whitec++;
					break;
				}
			}
		}
	}
	if(whitec != white) return 0;

	return 1;
}

void ai_response(enum Color tip[SLOTS], int red, int white)
{
	next_guess = FIRST_GUESS;
	for(int i = 0; i < all_size; i++){
		if(!possible(i, tip, red, white))
			all[i] = 1;
		if(all[i] == 0 && next_guess == FIRST_GUESS)
			next_guess = i;
	}
	
}


