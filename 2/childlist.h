#ifndef __PIDLIST_H__
#define __PIDLIST_H__
#include <unistd.h>

struct ChildInfo{
	pid_t pid;
	char *pw;
	int active;
	int status;
};

struct ChildList{
	struct ChildInfo *buf;
	size_t maxlen, len;
};

struct ChildList* childlist_new(size_t init_len);

int childlist_add(struct ChildList *pl, const struct ChildInfo *const info);

void childlist_remove(struct ChildList *pl, size_t pos);

void childlist_delete(struct ChildList *pl);

struct ChildInfo childlist_get(struct ChildList *pl, size_t pos);

const struct ChildInfo* childlist_get_ro(struct ChildList *pl, size_t pos);

void childlist_mark_inactive(struct ChildList *pl, pid_t pid, int status);

static inline int childlist_len(struct ChildList *pl)
{
	return pl->len;
}







#endif

