/*!
	\file	encr.c
	\author	Christian Fiedler <e1363562@student.tuwien.ac.at>
	\date	16.11.2015

	\brief	main file of the project

	\details
		this program hashes passwords
*/

#include "encr.h"
#include "childlist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

const char *progname = NULL;
static struct ChildList *childlist = NULL;
int child_process = 0;

void error(const char *f)
{
	(void)fprintf(stderr, "%s: error in function %s\n", progname, f);
	exit(EXIT_FAILURE);
}

void wait_child(void)
{
	int status;
	pid_t exited;

	do
		exited = wait(&status);
	while(exited == -1 && errno == EINTR);

	if(exited > 0)
		childlist_mark_inactive(childlist, exited, status);
}

void signal_child(int s)
{
	wait_child();
}

int check_childlist(void)
{
	for(int i = 0; i < childlist_len(childlist); i++){
		const struct ChildInfo *const info = childlist_get_ro(childlist, i);
		
		if(info->active == 0)
			childlist_remove(childlist, i--);
	}

	return childlist_len(childlist);
}

void usage(void)
{
	(void)fprintf(stderr, "usage: encr\n\n");
}

char* read_from_stdin(void)
{
	const int LENGTH_INC = 20;

	size_t maxlen = 30, len = 0;
	char *s = malloc(maxlen);
	
	if(s == NULL)
		return NULL;

	do{
		s[len++] = getchar();

		if(s[len-1] == EOF){
			free(s);
			return NULL;
		}

		if(s[len-1] != '\n' && len == maxlen){
			char *n = realloc(s, maxlen += LENGTH_INC);
			if(n == NULL){
				free(s);
				error("realloc");
			}
			s = n;
		}

	}while(s[len-1] != '\n');
	
	s[len-1] = '\0';

	return s;
}

void compute_pw(const char *pw, int sleep_time)
{
	const char *result = crypt(pw, "aC");

	do
		sleep_time = sleep( sleep_time );
	while(sleep_time > 0);

	printf("encr: %s -> %s\n", pw, result);

	free((void*)pw);

	exit(EXIT_SUCCESS);
}

int setup_signal_handler(void)
{
	struct sigaction sa = {
		.sa_handler = signal_child,
		.sa_flags = SA_RESTART|SA_NOCLDSTOP
	};

	if(sigfillset(&sa.sa_mask) < 0)
		error("sigfillset");

	return sigaction(SIGCHLD, &sa, NULL) == -1 ? 0 : 1;
}

void cleanup(void)
{
	if(childlist != NULL){
		if(child_process == 0){
			while(check_childlist() > 0)
				wait_child();
		}

		childlist_delete(childlist);
		childlist = NULL;
	}
}

int main(int argc, char **argv)
{
	if(argc != 1){
		usage();
		return EXIT_FAILURE;
	}
	progname = argv[0];

	DEBUG(fprintf(stderr, "--- DEBUG MODE ---\n"));

	atexit(cleanup);

	childlist = childlist_new(30);
	if(childlist == NULL)
		error("childlist_new");

	if(setup_signal_handler() == 0)
		error("sigaction");

	srand(time(NULL));
	for(;;){
		struct ChildInfo child;
	
		child.active = 1;
		child.pw = read_from_stdin();
		if(child.pw == NULL)
			break;

		check_childlist();
		
		const int sleep_time = rand() % 4 + 2;
		child.pid = fork();

		switch(child.pid){
		case -1:
			free(child.pw);
			error("fork");
			assert(0);
		case 0:
			child_process = 1;
			compute_pw(child.pw, sleep_time);
			assert(0);
		default:
			if(childlist_add(childlist, &child) == 0)
				error("childlist_add");
		}

	}

	return EXIT_SUCCESS;
}
