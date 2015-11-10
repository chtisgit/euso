#include "subproc.h"

#include <stdlib.h>

struct SubProc* subproc_append(struct SubProc **dst_list, const char *data)
{
	struct SubProc *sp = malloc( sizeof(SubProc) );

	if(sp == NULL)
		return NULL;

	sp->data = malloc( strlen(data) );
	if(sp->data == NULL){
		(void)free(sp);
		return NULL;
	}
	strcpy(sp->data, data);
	if(pipe(sp->pipe) == -1){
		(void)free(sp->data);
		(void)free(sp);
		return NULL;
	}

	if(*dst_list == NULL){
		sp->next = NULL;
		*dst_list = sp;
	}else{
		sp->next = *dst_list;
		*dst_list = sp;
	}
	return sp;
}

void subproc_destroy(struct SubProc *sp)
{
	(void)close(sp->pipe[0]);
	(void)close(sp->pipe[1]);
	(void)free(sp->data);
	(void)free(sp);
}

void subproc_destroylist(struct SubProc **list)
{
	if(*list == NULL)
		return;

	subproc_destroylist(&(*list)->next);

	subproc_destroy(*list);
}

int subproc_remove(struct SubProc **list, struct SubProc *sp)
{
	if(*list == NULL){
		return 0;
	}else if(*list == sp){
		struct SubProc *tmp = *list;
		*list = (*list)->next;
		subproc_destroy(tmp);
		return 1;
	}else{
		return subproc_remove(&(*list)->next, sp);
	}
}
