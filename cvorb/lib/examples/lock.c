/*
 * lock.c
 *
 * Claim ownership of a module. The module is unlocked when the process is
 * killed.
 */

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <libcvorb.h>
#include "my_stringify.h"

/* default module number (LUN) */
#define MODULE_NR	0

#define PROGNAME	"lock"

static int	module_nr = MODULE_NR;
static cvorb_t *device;

extern char *optarg;

static const char usage_string[] =
	"Claim ownership of a CVORB module\n"
	" " PROGNAME " [-h] [-m<LUN>]";

static const char commands_string[] =
	"options:\n"
	" -h = show this help text\n"
	" -m = Module number (default: " my_stringify(MODULE_NR) ")";

static void usage_complete(void)
{
	printf("%s\n", usage_string);
	printf("%s\n", commands_string);
}

static void parse_args(int argc, char *argv[])
{
	int c;

	for (;;) {
		c = getopt(argc, argv, "hm:");
		if (c < 0)
			break;
		switch (c) {
		case 'h':
			usage_complete();
			exit(EXIT_SUCCESS);
		case 'm':
			module_nr = atoi(optarg);
			break;
		}
	}
}

static void sighandler(int sig)
{
	if (!device)
		exit(EXIT_FAILURE);


	/* unlock the device */
/*
	if (cvorb_unlock(device) < 0) {
		cvorb_perror("cvorb_unlock");
		exit(EXIT_FAILURE);
	}
*/
	/* close the handle */
	if (cvorb_close(device)) {
		cvorb_perror("cvorb_close");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}

static void sighandler_register(void)
{
	struct sigaction act;

	act.sa_handler = sighandler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGTERM, &act, 0);
	sigaction(SIGINT, &act, 0);
}

int main(int argc, char *argv[])
{
	parse_args(argc, argv);

	/* Get a handle. Don't care which channel we choose */
	device = cvorb_open(module_nr);
	if (device == NULL) {
		cvorb_perror("cvorb_open");
		exit(EXIT_FAILURE);
	}
/*
	if (cvorb_lock(device) < 0) {
		cvorb_perror("cvorb_lock");
		exit(EXIT_FAILURE);
	}
*/
	sighandler_register();

	while (1)
		sleep(10);

	exit(EXIT_FAILURE);
}
