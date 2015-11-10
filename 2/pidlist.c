#include "encr.h"
#include "pidlist.h"
#include <stdlib.h>
#include <string.h>

struct PidList* pidlist_new(size_t init_len)
{
	struct PidList *pl = malloc( sizeof(struct PidList) );
	if(pl == NULL)
		return NULL;

	pl->len = 0;
	pl->maxlen = init_len;
	pl->pids = malloc(sizeof(*pl->pids) * pl->maxlen);

	if(pl->pids == NULL){
		free(pl);
		return NULL;
	}

	return pl;
}

int pidlist_add(struct PidList *pl, pid_t pid)
{
	const int plus = 20;

	if(pl->len == pl->maxlen){
		pid_t *n = realloc(pl->pids, (pl->maxlen+plus) * sizeof(*n));
		if(n == NULL)
			return 0;
		pl->pids = n;
		pl->maxlen += plus;
	}

	pl->pids[pl->len++] = pid;
	DEBUG(fprintf(stderr, "new pid: %d (pl->len = %d)\n", pid,pl->len));
	return 1;
}

void pidlist_remove(struct PidList *pl, pid_t pid)
{
	for(int i = 0; i < pl->len; i++){
		if(pl->pids[i] == pid){
			const int len = pl->len - i - 1;
			if(len) memmove(&pl->pids[i], &pl->pids[i+1], len);

			pl->len--;
			i--;
		}
	}
}

void pidlist_delete(struct PidList *pl)
{
	free(pl->pids);
	free(pl);
}



