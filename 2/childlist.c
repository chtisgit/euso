#include "encr.h"
#include "childlist.h"
#include <stdlib.h>
#include <string.h>

struct ChildList* childlist_new(size_t init_len)
{
	struct ChildList *pl = malloc( sizeof(struct ChildList) );
	if(pl == NULL)
		return NULL;

	pl->len = 0;
	pl->maxlen = init_len;
	pl->buf = malloc(sizeof(*pl->buf) * pl->maxlen);

	if(pl->buf == NULL){
		free(pl);
		return NULL;
	}

	return pl;
}

int childlist_add(struct ChildList *pl, const struct ChildInfo *const info)
{
	const int plus = 20;

	if(pl->len == pl->maxlen){
		void *n = realloc(pl->buf, (pl->maxlen+plus) * sizeof(*pl->buf));
		if(n == NULL)
			return 0;
		
		pl->buf = n;
		pl->maxlen += plus;
	}

	pl->buf[pl->len++] = *info;
	DEBUG(fprintf(stderr, "new pid: %d (pl->len = %d)\n", info->pid, pl->len));
	return 1;
}

void childlist_remove(struct ChildList *pl, size_t pos)
{
	const size_t len = (pl->len - pos - 1) * sizeof(*pl->buf);
	if(len) memmove(&pl->buf[pos], &pl->buf[pos+1], len);
	pl->len--;
}
struct ChildInfo childlist_get(struct ChildList *pl, size_t pos)
{
	assert(0 <= pos && pos < pl->len);
	return pl->buf[pos];
}

const struct ChildInfo* childlist_get_ro(struct ChildList *pl, size_t pos)
{
	assert(0 <= pos && pos < pl->len);
	return &pl->buf[pos];
}

void childlist_mark_inactive(struct ChildList *pl, pid_t pid, int status)
{
	for(int i = 0; i < pl->maxlen; i++){
		struct ChildInfo *const info = &pl->buf[i];
		if(info->pid == pid){
			info->active = 0;
			info->status = status;
			return;
		}
	}
}

void childlist_delete(struct ChildList *pl)
{
	free(pl->buf);
	free(pl);
}



