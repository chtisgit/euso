#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

#include <common.h>

const char *progname;

void cleanup(void)
{
	free_common_ressources();
	exit(EXIT_SUCCESS);
}

int main(int argc, char** argv)
{
	atexit(free_common_ressources);

	sem[SEM_GLOBAL] = sem_open(SEM_NAME[SEM_GLOBAL], O_CREAT, O_RDWR, 1);
	sem[SEM_1] = sem_open(SEM_NAME[SEM_1], O_CREAT, O_RDWR, 0);
	sem[SEM_2] = sem_open(SEM_NAME[SEM_2], O_CREAT, O_RDWR, 0);

	for(int i = 0; i < LEN(sem); i++){
		if(sem[i] == SEM_FAILED)
			bail_out("sem_open");
	}

	if(allocate_shared() == 0)
		bail_out("allocate_shared");
	
	sem_wait_cb(sem[SEM_1], cleanup);
	sem_wait_cb(sem[SEM_2], cleanup);

	
	

	return EXIT_SUCCESS;
}
