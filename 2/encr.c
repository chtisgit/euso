#include "subproc.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void usage()
{
	(void)printf("usage: encr\n\n");
}

int main(int argc, char **argv)
{
	int opt;
	while ((opt = getopt(argc, argv, "")) != -1) {
		usage();
		return 1;
	}
	if(optind != 1){
		usage();
		return 1;
	}
	
	struct SubProc *sp_top;	
}
