/*
 * Copyright (c) 2012-2015 OSUE Team <osue-team@vmars.tuwien.ac.at>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * gcc -std=c99 -Wall -g -pedantic -DENDEBUG \
 *      -D_BSD_SOURCE -D_XOPEN_SOURCE=500 -o server server.c
 */

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


/* === Constants === */

#define MAX_TRIES (35)
#define SLOTS (5)
#define COLORS (8)

#define READ_BYTES (2)
#define WRITE_BYTES (1)
#define BUFFER_BYTES (2)
#define SHIFT_WIDTH (3)
#define PARITY_ERR_BIT (6)
#define GAME_LOST_ERR_BIT (7)

#define EXIT_PARITY_ERROR (2)
#define EXIT_GAME_LOST (3)
#define EXIT_MULTIPLE_ERRORS (4)

#define BACKLOG (5)


/* === Macros === */

#ifdef ENDEBUG
#define DEBUG(...) do { fprintf(stderr, __VA_ARGS__); } while(0)
#else
#define DEBUG(...)
#endif

/* Length of an array */
#define COUNT_OF(x) (sizeof(x)/sizeof(x[0]))

/* === Global Variables === */

/* Name of the program */
static const char *progname = "server"; /* default name */

/* File descriptor for server socket */
static int sockfd = -1;

/* File descriptor for connection socket */
static int connfd = -1;

/* This variable is set upon receipt of a signal */
volatile sig_atomic_t quit = 0;


/* === Type Definitions === */

struct opts {
	long int portno;
	uint8_t secret[SLOTS];
};


/* === Prototypes === */

/**
 * @brief Parse command line options
 * @param argc The argument counter
 * @param argv The argument vector
 * @param options Struct where parsed arguments are stored
 */
static void parse_args(int argc, char **argv, struct opts *options);

/**
 * @brief Read message from socket
 *
 * This code *illustrates* one way to deal with partial reads
 *
 * @param sockfd_con Socket to read from
 * @param buffer Buffer where read data is stored
 * @param n Size to read
 * @return Pointer to buffer on success, else NULL
 */
static uint8_t *read_from_client(int sockfd_con, uint8_t *buffer, size_t n);

/**
 * @brief Compute answer to request
 * @param req Client's guess
 * @param resp Buffer that will be sent to the client
 * @param secret The server's secret
 * @return Number of correct matches on success; -1 in case of a parity error
 */
static int compute_answer(uint16_t req, uint8_t *resp, uint8_t *secret);

/**
 * @brief terminate program on program error
 * @param exitcode exit code
 * @param fmt format string
 */
static void bail_out(int exitcode, const char *fmt, ...);

/**
 * @brief Signal handler
 * @param sig Signal number catched
 */
static void signal_handler(int sig);

/**
 * @brief free allocated resources
 */
static void free_resources(void);


/* === Implementations === */

static uint8_t *read_from_client(int fd, uint8_t *buffer, size_t n)
{
	/* loop, as packet can arrive in several partial reads */
	size_t bytes_recv = 0;
	do {
		ssize_t r;
		r = recv(fd, buffer + bytes_recv, n - bytes_recv, 0);
		if (r <= 0) {
			return NULL;
		}
		bytes_recv += r;
	} while (bytes_recv < n);

	if (bytes_recv < n) {
		return NULL;
	}
	return buffer;
}

static int compute_answer(uint16_t req, uint8_t *resp, uint8_t *secret)
{
	int colors_left[COLORS];
	int guess[COLORS];
	uint8_t parity_calc, parity_recv;
	int red, white;
	int j;

	parity_recv = (req >> 15) & 1;

	/* extract the guess and calculate parity */
	parity_calc = 0;
	for (j = 0; j < SLOTS; ++j) {
		int tmp = req & 0x7;
		parity_calc ^= tmp ^ (tmp >> 1) ^ (tmp >> 2);
		guess[j] = tmp;
		req >>= SHIFT_WIDTH;
	}
	parity_calc &= 0x1;

	/* marking red and white */
	(void) memset(&colors_left[0], 0, sizeof(colors_left));
	red = white = 0;
	for (j = 0; j < SLOTS; ++j) {
		/* mark red */
		if (guess[j] == secret[j]) {
			red++;
		} else {
			colors_left[secret[j]]++;
		}
	}
	for (j = 0; j < SLOTS; ++j) {
		/* not marked red */
		if (guess[j] != secret[j]) {
			if (colors_left[guess[j]] > 0) {
				white++;
				colors_left[guess[j]]--;
			}
		}
	}

	/* build response buffer */
	resp[0] = red;
	resp[0] |= (white << SHIFT_WIDTH);
	if (parity_recv != parity_calc) {
		resp[0] |= (1 << PARITY_ERR_BIT);
		return -1;
	} else {
		return red;
	}
}

static void bail_out(int exitcode, const char *fmt, ...)
{
	va_list ap;

	(void) fprintf(stderr, "%s: ", progname);
	if (fmt != NULL) {
		va_start(ap, fmt);
		(void) vfprintf(stderr, fmt, ap);
		va_end(ap);
	}
	if (errno != 0) {
		(void) fprintf(stderr, ": %s", strerror(errno));
	}
	(void) fprintf(stderr, "\n");

	free_resources();
	exit(exitcode);
}

static void free_resources(void)
{
	/* clean up resources */
	DEBUG("Shutting down server\n");
	if(connfd >= 0) {
		(void) close(connfd);
	}
	if(sockfd >= 0) {
		(void) close(sockfd);
	}
}

static void signal_handler(int sig)
{
	quit = 1;
}

static void init_sockets(const struct opts *const options)
{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1)
		bail_out(EXIT_FAILURE, "socket");

	const int one = 1;
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) != 0)
		bail_out(EXIT_FAILURE, "setsockopt");

	const struct sockaddr_in sa_struct = { 
		.sin_family = AF_INET,
		.sin_port = htons(options->portno),
		.sin_addr = { .s_addr = INADDR_ANY }
	};

	if(bind(sockfd, (const struct sockaddr*) &sa_struct, sizeof(sa_struct)) != 0)
		bail_out(EXIT_FAILURE, "bind");
	

	if(listen(sockfd, 5) != 0)
		bail_out(EXIT_FAILURE, "listen");

/*
	struct pollfd sockfd_poll = {
		.fd = sockfd,
		.events = POLLIN,
		.revents = 0
	};

	int pollr;
	do{
		pollr = poll(&sockfd_poll, 1, 500);
		
		if(pollr < 0)
			bail_out(EXIT_FAILURE, "poll");	

	}while(pollr == 0 && quit == 0);
*/

	connfd = accept(sockfd, NULL, NULL);
	if(connfd < 0)
		bail_out(EXIT_FAILURE, "accept");
}

/**
 * @brief Program entry point
 * @param argc The argument counter
 * @param argv The argument vector
 * @return EXIT_SUCCESS on success, EXIT_PARITY_ERROR in case of an parity
 * error, EXIT_GAME_LOST in case client needed to many guesses,
 * EXIT_MULTIPLE_ERRORS in case multiple errors occured in one round
 */
int main(int argc, char *argv[])
{
	struct opts options;
	int round;
	int ret;

	parse_args(argc, argv, &options);

	/* setup signal handlers */
	const int signals[] = {SIGINT, SIGTERM};
	struct sigaction s;

	s.sa_handler = signal_handler;
	s.sa_flags   = 0;
	if(sigfillset(&s.sa_mask) < 0) {
		bail_out(EXIT_FAILURE, "sigfillset");
	}
	for(int i = 0; i < COUNT_OF(signals); i++) {
		if (sigaction(signals[i], &s, NULL) < 0) {
			bail_out(EXIT_FAILURE, "sigaction");
		}
	}



	/* Create a new TCP/IP socket `sockfd`, and set the SO_REUSEADDR
	   option for this socket. Then bind the socket to localhost:portno,
	   listen, and wait for new connections, which should be assigned to
	   `connfd`. Terminate the program in case of an error.
	 */
	init_sockets(&options);

	/* accepted the connection */
	ret = EXIT_SUCCESS;
	for (round = 1; round <= MAX_TRIES && !quit; ++round) {
		uint16_t request;
		static uint8_t buffer[BUFFER_BYTES];
		int correct_guesses;
		int error = 0;

		/* read from client */
		if (read_from_client(connfd, &buffer[0], READ_BYTES) == NULL) {
			if (quit) break; /* caught signal */
			bail_out(EXIT_FAILURE, "read_from_client");
		}
		request = (buffer[1] << 8) | buffer[0];
		DEBUG("Round %d: Received 0x%x\n", round, request);

		/* compute answer */
		correct_guesses = compute_answer(request, buffer, options.secret);
		if (round == MAX_TRIES && correct_guesses != SLOTS) {
			buffer[0] |= 1 << GAME_LOST_ERR_BIT;
		}

		DEBUG("Sending byte 0x%x\n", buffer[0]);

		/* send message to client */
		
		send(connfd, &buffer[0], sizeof(buffer[0]), 0);
		
		/* We sent the answer to the client; now stop the game
		   if its over, or an error occured */
		if (*buffer & (1<<PARITY_ERR_BIT)) {
			(void) fprintf(stderr, "Parity error\n");
			error = 1;
			ret = EXIT_PARITY_ERROR;
		}
		if (*buffer & (1 << GAME_LOST_ERR_BIT)) {
			(void) fprintf(stderr, "Game lost\n");
			error = 1;
			if (ret == EXIT_PARITY_ERROR) {
				ret = EXIT_MULTIPLE_ERRORS;
			} else {
				ret = EXIT_GAME_LOST;
			}
		}
		if (error) {
			break;
		} else if (correct_guesses == SLOTS) {
			/* won */
			(void) printf("Runden: %d\n", round);
			break;
		}
	}

	/* we are done */
	free_resources();
	return ret;
}

static void parse_args(int argc, char **argv, struct opts *options)
{
	int i;
	char *port_arg;
	char *secret_arg;
	char *endptr;
	enum { beige, darkblue, green, orange, red, black, violet, white };

	if(argc > 0) {
		progname = argv[0];
	}
	if (argc != 3) {
		bail_out(EXIT_FAILURE,
				"Usage: %s <server-port> <secret-sequence>", progname);
	}
	port_arg = argv[1];
	secret_arg = argv[2];

	errno = 0;
	options->portno = strtol(port_arg, &endptr, 10);

	if ((errno == ERANGE &&
				(options->portno == LONG_MAX || options->portno == LONG_MIN))
			|| (errno != 0 && options->portno == 0)) {
		bail_out(EXIT_FAILURE, "strtol");
	}

	if (endptr == port_arg) {
		bail_out(EXIT_FAILURE, "No digits were found");
	}

	/* If we got here, strtol() successfully parsed a number */

	if (*endptr != '\0') { /* In principle not necessarily an error... */
		bail_out(EXIT_FAILURE,
				"Further characters after <server-port>: %s", endptr);
	}

	/* check for valid port range */
	if (options->portno < 1 || options->portno > 65535)
	{
		bail_out(EXIT_FAILURE, "Use a valid TCP/IP port range (1-65535)");
	}

	if (strlen(secret_arg) != SLOTS) {
		bail_out(EXIT_FAILURE,
				"<secret-sequence> has to be %d chars long", SLOTS);
	}

	/* read secret */
	for (i = 0; i < SLOTS; ++i) {
		uint8_t color;
		switch (secret_arg[i]) {
		case 'b':
			color = beige;
			break;
		case 'd':
			color = darkblue;
			break;
		case 'g':
			color = green;
			break;
		case 'o':
			color = orange;
			break;
		case 'r':
			color = red;
			break;
		case 's':
			color = black;
			break;
		case 'v':
			color = violet;
			break;
		case 'w':
			color = white;
			break;
		default:
			bail_out(EXIT_FAILURE,
					"Bad Color '%c' in <secret-sequence>", secret_arg[i]);
		}
		options->secret[i] = color;
	}
}
