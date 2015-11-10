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


#endif
