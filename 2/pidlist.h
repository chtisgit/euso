#ifndef __PIDLIST_H__
#define __PIDLIST_H__
#include <unistd.h>

struct PidList{
	pid_t *pids;
	size_t maxlen, len;
};


struct PidList* pidlist_new(size_t init_len);

int pidlist_add(struct PidList *pl, pid_t pid);

void pidlist_remove(struct PidList *pl, pid_t pid);

void pidlist_delete(struct PidList *pl);

static inline int pidlist_len(struct PidList *pl)
{
	return pl->len;
}

static inline pid_t pidlist_get(struct PidList *pl, int pos)
{
	assert(pos >= 0 && pos < pl->len);
	return pl->pids[pos];
}





#endif

