#ifndef __SUBPROC_H__
#define __SUBPROC_H__

struct SubProc{
	const char *data;
	int pipe[2];
	struct SubProc *next;
};

int subproc_create(struct SubProc **dst_list, const char *data);

void subproc_destroy(struct SubProc *sp);

void subproc_destroylist(struct SubProc **list);

int subproc_remove(struct SubProc **list, struct SubProc *sp);

#endif

