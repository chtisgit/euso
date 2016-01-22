#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


#define LINE_MAX	500
#define min(a,b)	((a) <= (b) ? a : b)
#define max(a,b)	((a) >= (b) ? a : b)

char** build_array(const char *s, const char **s_out)
{
	size_t buf_cap=30, buf_len=0;
	size_t arr_cap=10, arr_len=0;
	char *buf = malloc(buf_cap * sizeof(*buf));
	if(buf == NULL)
		return NULL;

	char **arr = malloc(arr_cap * sizeof(*arr));
	if(arr == NULL){
		free(buf);
		return NULL;
	}

	const char *begin = s;
	int i;

	do{
		for(i = 0; begin[i] != ' ' && begin[i] != '|' && begin[i] != '\0'; i++);
		if(i > 0){
			/* new argument length is i (excluding \0) */
			if(buf_cap - buf_len < i+1){
				void *n = realloc(buf, (buf_cap += min(30,i+1)) * sizeof(*buf));
				if(n == NULL)
					goto fail;
				buf = n;
			}
			char *buf_str = &buf[buf_len];
			memcpy(buf_str, begin, i);
			buf_len += i;
			buf[buf_len++] = '\0';

			if(arr_len == arr_cap-1){
				void *n = realloc(arr, (arr_cap += 8) * sizeof(*arr));
				if(n == NULL)
					goto fail;
				arr = n;
			}
			arr[arr_len++] = buf_str;
		}
		while(isspace(begin[i])) i++;

		begin = &begin[i];
	}while(*begin != '\0' && *begin != '|');
	arr[arr_len] = NULL;
	*s_out = begin;
	return arr;

fail:
	free(buf);
	free(arr);
	return NULL;
}

void delete_array(char **arr)
{
	void *buf = arr[0];
	free(buf);
	free(arr);
}

int run(int lev, const char *s)
{
	char **arr = build_array(s, &s);
	int status, in[2];
	pipe(in);

	pid_t p = fork();

	if(p < 0){
		fprintf(stderr, "error!");
		return 0;
	}

	if(p == 0){
		close(in[0]);
		if(*s == '|'){
			if(lev > 0)
				dup2(in[1], fileno(stdout));
			
			for(++s; isspace(s); s++);
			run(lev+1, s);
		}

		execvp(arr[0], arr);
		close(in[1]);
		exit(0);
	}else{
		close(in[1]);
		if(*s == '|' && lev > 0)
			dup2(in[0], fileno(stdout));

		wait(&status);
		close(in[0]);
	}

	delete_array(arr);

	return status;
}


void cut_newline(char *s)
{
	while(*s != '\n' && *s != '\0') s++;
	*s = '\0';
}

int main(int argc, char **argv)
{
	char line[LINE_MAX];
	char *valid;
	int status = 0;

	do{
		printf("$hit$shell $ ");
		fflush(stdout);
		
		valid = fgets(line, LINE_MAX, stdin);

		if(line[0] != '\0'){
			cut_newline(line);
			pid_t p = fork();
			if(p < 0){
				fprintf(stderr, "error!\n");
			}else if(p == 0){
				exit(run(0, line));
			}else{
				wait(&status);
			}
		}
		
	}while(valid != NULL);

	printf("\n");

	return 0;
}

