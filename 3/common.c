#include <stdio.h>
#include <stdlib.h>
#include <common.h>

sig_atomic_t exit = 0;

int shm_fd = -1;
struct SharedStructure *shared = NULL;

const char *SEM_NAME[] = { "/battlesA", "/battles1", "/battles2" };
sem_t *SEM[] = { SEM_FAILED, SEM_FAILED, SEM_FAILED };

void usage(void)
{
	(void)fprintf(stderr, "USAGE: %s\n", progname);
}

void bail_out(const char *s)
{
	fprintf(stderr, "error: function '%s'\n",s);
	exit(EXIT_FAILURE);
}

int allocate_shared(void)
{
	shm_fd = shm_open(SHMEM_NAME, O_CREAT|O_RDWR, 0600);
	if(shm_fd == -1) return 0;

	if(ftrunctate(shm_fd, sizeof(*shared)) == -1) return 0;
	
	shared = mmap(NULL, sizeof(*shared), PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd, 0);
	(void)close(shm_fd);
	
	if(shared == MAP_FAILED){
		shared = NULL;
		return 0;
	}
	return 1;
}

void free_common_ressources(void)
{
	int i;
	for(i = 0; i < LEN(SEM); i++){
		if(SEM[i] != NULL){
			(void)sem_close(sem[i]);
			(void)sem_unlink(SEM_NAME[i]);
			sem[i] = SEM_FAILED;
		}
	}

	if(shared_mem != -1){
		if(shared != NULL){
			munmap(shared, sizeof(*shared));
			shared = NULL;
		}
		(void)shm_unlink(shm_fd);
	}
	
}

void sem_wait_cb(const sem_t *const s, void (callback*)(void))
{
	int x;
	do{
		x = sem_wait(s);
		if(exit != 0)
			callback();
	}while(x == -1 && errno == EINTR);
}

