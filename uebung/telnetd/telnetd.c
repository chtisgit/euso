#define _POSIX_SOURCE

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#define info(...)	fprintf(stderr, __VA_ARGS__);
#define BACKLOG_NUM	50

static const char *server_app;
static const char *progname;

static void die(const char* s)
{
	fprintf(stderr, "error: %s\n\n", s);
	exit(EXIT_FAILURE);
}

static void usage(void)
{
	fprintf(stderr, "USAGE: %s port server-app\n\n", progname);
}

static int init(const char* sport)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	const int port = atoi(sport);
	
	if(port < 1 || port > 65535){
		die("port out of range");
	}
	

	if(sock == -1){
		die("socket() failed");
	}

	int enable = 1;
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) != 0){
		die("setsockopt() failed");
	}
	
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(port),
		.sin_addr = { .s_addr = INADDR_ANY }
	};

	if(bind(sock, (struct sockaddr*) &addr, sizeof(addr)) != 0){
		die("bind() failed");
	}

	if(listen(sock, BACKLOG_NUM) != 0){
		die("listen() failed");
	}

	return sock;
}

struct Data{
	int sock;
	struct sockaddr addr;
};

void manager_process(struct Data *d)
{
	int prog_stdout[2];
	int prog_stdin[2];
	int run;

	if(pipe(prog_stdout) != 0){
		die("pipe() failed");
	}
	if(pipe(prog_stdin) != 0){
		die("pipe() failed");
	}

	pid_t pid = fork();

	if(pid == -1){
		die("fork() error");
	}

	if(pid == 0){
		/* CHILD */
		close(prog_stdout[0]);
		close(prog_stdin[1]);
		
		if(dup2(prog_stdout[1], STDOUT_FILENO) == -1){
			exit(EXIT_FAILURE);
		}
		close(prog_stdout[1]);

		if(dup2(prog_stdin[0], STDIN_FILENO) == -1){
			exit(EXIT_FAILURE);
		}
		close(prog_stdin[0]);

		info("running '%s' ...\n", server_app);
		execlp(server_app, server_app, NULL);
		assert(0);

	}else{
		/* PARENT */
		close(prog_stdout[1]);
		close(prog_stdin[0]);

		FILE *prog_out, *prog_in;
		unsigned i;
		size_t off, len;
		char buf[512];

		run = 1;

		prog_in = fdopen(prog_stdin[1], "w");
		if(prog_in == NULL){
			run = 0;
			die("fdopen() failed");
		}

		prog_out = fdopen(prog_stdout[0], "r");
		if(prog_out == NULL){
			run = 0;
			fprintf(prog_in, "\n%c", EOF);
			wait(NULL);
		}

		for(i = 0; run; i++){
			if(waitpid(pid, NULL, WNOHANG) == pid){
				run = 0;
				break;
			}

			if((i & 1) == 0){
				const char *s = fgets(buf, 512, prog_out);
				if(s == NULL) continue;

				off = 0;
				do{
					len = strlen(buf);
					ssize_t s = send(d->sock, buf+off, len, MSG_NOSIGNAL);
					if(s == -1) die("send() failed");
					off += (size_t)s;
				}while(off < len);
				buf[0] = '\0';
				
			}else{

			}
		}

	}
}

int main(int argc, char *argv[])
{
	progname = argv[0];

	if(argc != 3){
		usage();
		return EXIT_FAILURE;
	}

	server_app = argv[2];

	int servsock = init(argv[1]);
	struct Data *d;

	for(;;){
		d = malloc(sizeof(*d));

		do{
			socklen_t sl = sizeof(d->addr);

			errno = 0;
			d->sock = accept(servsock, &d->addr, &sl);
		}while(errno == EINTR && d->sock == -1);

		if(d->sock == -1){
			free(d);
			die("accept() error");
		}

		info("accepted new connection\n");		

		pid_t pid = fork();

		if(pid == 0){
			/* CHILD */
			manager_process(d);
			exit(EXIT_SUCCESS);
		}else{
			/* PARENT */
			free(d);
			if(pid == -1){
				info("non-fatal error: could not fork\n");
			}
		}


	}

	return EXIT_SUCCESS;
}
