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

static struct ChildList *childlist = NULL;


void error(const char *f)
{
	(void)fprintf(stderr, "error in function %s\n", f);
	cleanup();
	exit(EXIT_FAILURE);
}

void wait_child(void)
{
	int status;
	pid_t exited;

	do
		exited = wait(&status);
	while(exited == -1 && errno == EINTR);
	//assert(exited > 0);

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
		
		if(info->active == 0){
			DEBUG(fprintf(stderr, "pid %d inactive. removing.\n",info->pid));
			childlist_remove(childlist, i--);
		}
	}

	return childlist_len(childlist);
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

		if(s[len-1] == EOF){
			free(s);
			s = NULL;
			break;
		}

		if(s[len-1] != '\n' && len == maxlen){
			char *n = realloc(s, maxlen += 20);
			if(n == NULL){
				free(s);
				error("realloc");
			}
			s = n;
		}

	}while(s[len-1] != '\n');
	
	if(s != NULL) s[len-1] = '\0';

	return s;
}

void compute_pw(const char *pw, const int sleep_time)
{
	const char *result = crypt(pw, "aC");

	sleep( sleep_time );

	printf("encr: %s -> %s\n", pw, result);
	free((void*)pw);

	childlist_delete(childlist);

	exit(EXIT_SUCCESS);
}

int setup_signal_handler(void)
{
	struct sigaction sa;
	sa.sa_handler = signal_child;
	if(sigfillset(&sa.sa_mask) < 0)
		error("sigfillset");
	sa.sa_flags = SA_RESTART|SA_NOCLDSTOP;

	return sigaction(SIGCHLD, &sa, NULL) == -1 ? 0 : 1;
}

void cleanup(void)
{
	if(childlist != NULL){
		while(check_childlist() > 0){
			wait_child();
			DEBUG(fprintf(stderr,"childlist_len: %d\n",childlist_len(childlist)));
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
	DEBUG(fprintf(stderr, "--- DEBUG MODE ---\n"));

	childlist = childlist_new(30);

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
			compute_pw(child.pw, sleep_time);
			assert(0);
		default:
			childlist_add(childlist, &child);
		}

	}

	cleanup();
	return EXIT_SUCCESS;
}
