/*
 * version.c
 *
 * Retrieve the version of a CVORB module and library
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <libcvorb.h>
#include "my_stringify.h"

/* default module number (LUN) */
#define MODULE_NR	0

#define PROGNAME	"version"

static int	module_nr = MODULE_NR;
static cvorb_t *device;

extern char *optarg;

static const char usage_string[] =
	"Retrieve the version of a CVORB module and library\n"
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

int main(int argc, char *argv[])
{
	char *version;

	parse_args(argc, argv);

	/* Get a handle. Don't care which channel we choose */
	device = cvorb_open(module_nr, CVORB_CHANNEL_B);
	if (device == NULL) {
		cvorb_perror("cvorb_open");
		exit(EXIT_FAILURE);
	}

	version = cvorb_get_hw_version(device);
	if (version == NULL) {
		cvorb_perror("cvorb_get_hw_version");
		exit(EXIT_FAILURE);
	}
	printf("CVORB hardware %s\n", version);
	printf("libcvorb: %s\n",
		libcvorb_version ? libcvorb_version : "version unknown");

	exit(EXIT_SUCCESS);
}
