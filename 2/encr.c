#include "encr.h"
#include "pidlist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

volatile sig_atomic_t pidlist_dirty = 0;
struct PidList *pidlist = NULL;

void signal_child(int s)
{
	assert(pidlist != NULL);
	pidlist_dirty = 1;
}

void check_pidlist(void)
{
	int status;
	struct PidList *pl = pidlist;

	fprintf(stderr, "pidlist dirty\n");

	for(int i = 0; i < pidlist_len(pl); i++){
		const pid_t current = pidlist_get(pl, i);
		const pid_t tmp = waitpid(current, &status, WNOHANG);
		if(tmp == current){
			//DEBUG(fprintf(stderr,"received exit status from pid %d : %d\n", current, status));
			pidlist_remove(pl, current);
			i--;
		}
	}
	pidlist_dirty = 0;
}
void wait_pidlist(void)
{
	int status;
	struct PidList *pl = pidlist;

	for(int i = 0; i < pidlist_len(pl); i++){
		const pid_t current = pidlist_get(pl, i);
		(void)waitpid(current, &status, 0);
	}
}

void error(const char *f)
{
	(void)fprintf(stderr, "error in function %s\n", f);
	exit(EXIT_FAILURE);
}

void usage(void)
{
	(void)fprintf(stderr, "usage: encr\n\n");
}

char* read_from_stdin(void)
{
	size_t maxlen = 30, len = 0;
	char *s = malloc(maxlen);
	
	if(s == NULL)
		return NULL;

	do{
		
		s[len++] = getchar();
		if(s[len-1] == EOF)
			return NULL;
#if 0
		check = fgets(s+len, maxlen, stdin);
		if(pidlist_dirty != 0)
			check_pidlist();

		if(check == NULL)
			return NULL;
		
		len = strlen(s);
#endif
		if(s[len-1] != '\n' && len == maxlen){
			char *n = realloc(s, maxlen += 20);
			if(n == NULL){
				free(s);
				error("realloc");
			}
			s = n;
		}

	}while(s[len-1] != '\n');
	s[len-1] = '\0';
	fprintf(stderr, "%s\n", s);

	return s;
}

void compute_pw(const char *pw)
{
	const char *result = crypt(pw, "aC");
	
	srand(time(NULL));
	sleep( rand() % 4 + 2 );

	printf("encr: %s -> %s\n", pw, result);
	free((char*)pw);

	exit(EXIT_SUCCESS);
}

int setup_signal_handler(void)
{
	struct sigaction sa;
	sa.sa_handler = signal_child;
	if(sigfillset(&sa.sa_mask) < 0)
		error("sigfillset");
	sa.sa_flags = SA_RESTART;

	return sigaction(SIGCHLD, &sa, NULL) == -1 ? 0 : 1;
}

void cleanup(void)
{
	if(pidlist != NULL){
		if(pidlist_len(pidlist) > 0)
			wait_pidlist();
		pidlist_delete(pidlist);
	}
}

int main(int argc, char **argv)
{
	if(argc != 1){
		usage();
		return EXIT_FAILURE;
	}
	DEBUG(fprintf(stderr, "--- DEBUG MODE ---\n"));

	pidlist = pidlist_new(30);

	atexit(cleanup);

	if(setup_signal_handler() == 0)
		error("sigaction");

	for(;;){
		const char *pw = read_from_stdin();
		if(pw == NULL)
			break;

		check_pidlist();

		const pid_t child = fork();

		switch(child){
		case -1:
			free(pw);
			error("fork");
			break;
		case 0:
			compute_pw(pw);
			break;
		default:
			pidlist_add(pidlist, child);
		}

	}

	return EXIT_SUCCESS;
}
