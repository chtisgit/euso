/*!
	\file client.c
	\author Christian Fiedler <e1363562@student.tuwien.ac.at>
	\date 22.10.2015

	\brief mastermind client with AI

	\details
		This program connects to a game server via TCP und plays
		mastermind with its integrated AI.
*/

#include "client.h"
#include "client-ai.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <poll.h>
#include <netdb.h>

volatile sig_atomic_t quit = 0;
static const char *progname;
static int sockfd = -1;

void deinit(void)
{
	if(sockfd >= 0)
		(void)close(sockfd);
	ai_deinit();
}

static void error(int exitcode, const char *fmt, ...)
{
	va_list ap;

	(void)fprintf(stderr, "%s error: ", progname);
	if (fmt != NULL) {
		va_start(ap, fmt);
		(void)vfprintf(stderr, fmt, ap);
		(void)fprintf(stderr, "\n");
		va_end(ap);
	}

	deinit();
	exit(exitcode);
}

void usage(void)
{
	(void)printf("Usage: %s <server-hostname> <server-port>\n\n", progname);
}

void establish_connection(const char *s_name, const char *s_port)
{
	struct addrinfo *res;
	struct addrinfo hint;

	memset(&hint, 0, sizeof(hint));
	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_flags = AI_PASSIVE;

	const int port = strtol(s_port, NULL, 10);
	if(port <= 0 || port >= 65536)
		error(EC_DEFAULT, "Use a valid TCP/IP port range (1-65535)");

	if(getaddrinfo(s_name, s_port, &hint, &res) != 0)
		error(EC_DEFAULT, "error: could not resolve hostname %s", s_name);

	const struct sockaddr_in s_addr = *((struct sockaddr_in*) res->ai_addr);
	const int addrlen = res->ai_addrlen;
	const int protocol = res->ai_protocol;
	(void)freeaddrinfo(res);
	
	sockfd = socket(AF_INET, SOCK_STREAM, protocol);

	if(sockfd == -1)
		error(EC_DEFAULT, "error: could not create socket");

	if(connect(sockfd, (struct sockaddr*) &s_addr, addrlen) != 0)
		error(EC_DEFAULT, "error: could not connect to server");

}

int parity(uint16_t msg)
{
	int p = 0;
	while(msg != 0){
		p ^= msg & 1;
		msg >>= 1;
	}
	return p;
}

uint16_t compose_msg(enum Color tip[SLOTS])
{
	uint16_t msg = 0;
	int i;
	for(i = 0; i < SLOTS; i++){
		const int c = (int) tip[i];
		assert(c >= 0 && c < COLORS);
		msg |= c << (i*3);
	}
	msg |= parity(msg) << 15;

	return msg;
}

void signal_stopserver(int sig)
{
	quit = 1;
}

int play(void)
{
	enum Color tip[SLOTS];
	int rounds = 0;

	while(quit == 0){
		rounds++;
		ai_guess(tip);
		uint16_t msg = compose_msg(tip);

#ifdef ENDEBUG
		(void)printf("AI guess: ");
		for(int i = 0; i < SLOTS; i++)
			(void)printf("%d ", tip[i]);
		(void)printf("\n");
#endif
	
		ssize_t bytessent = 0;
		do
			bytessent += send(sockfd, &msg+bytessent, sizeof(msg), 0);
		while(bytessent < 2 && quit == 0);

		uint8_t resp;

		while(recv(sockfd, &resp, sizeof(resp), 0) != 1 && quit == 0);
		
		const int red = resp & 7;
		const int white = (resp >> 3) & 7;
		const int status = (resp >> 6) & 3;
#ifdef ENDEBUG
		(void)printf("red: %d\nwhite: %d\nstatus: %d\n", red, white, status);
#endif	
		switch(status){
		case 1:
			error(EC_PARITY, "Parity error");
		case 2:
			error(EC_GAMELOST, "Game lost");
		case 3:
			error(EC_BOTH, "Parity error\nGame lost");
		}

		if(red == SLOTS)
			break;

		ai_response(tip, red, white);
	}
	return rounds;
}

int main(int argc, char **argv)
{
	int opt,i;

	(void)signal(SIGINT,signal_stopserver);
	(void)signal(SIGTERM,signal_stopserver);

	progname = argv[0];

	while ((opt = getopt(argc, argv, "")) != -1) {
		usage();
		return 1;
	}
	i = optind;

	/* check if exactly 2 parameters given */
	if(argc - i != 2){
		usage();
		return 1;
	}

	if(ai_init() == 0)
		return 1;

	establish_connection(argv[i], argv[i+1]);
	
	const int rounds = play();

	deinit();

	(void)printf("Runden: %d\n", rounds);

	return 0;
}

