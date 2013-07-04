/*
 * status.c
 *
 * Get the status of a CVORB channel.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <libcvorb.h>
#include "my_stringify.h"

/* default module number (LUN) */
#define MODULE_NR	0

/* default channel number */
#define CHANNEL_NR	CVORB_CHANNEL_A

static int	module_nr = MODULE_NR;
static int	channel_nr = CHANNEL_NR;
extern char *optarg;

static const char usage_string[] =
	"Retrieve the status of a CVORB channel\n"
	" status [-h] [-c<CHAN>] [-m<LUN>]";

static const char commands_string[] =
	"options:\n"
	" -c = channel number (default: " my_stringify(CHANNEL_NR) ")\n"
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
		c = getopt(argc, argv, "c:hm:");
		if (c < 0)
			break;
		switch (c) {
		case 'c':
			channel_nr = atoi(optarg);
			if (!channel_nr) {
				fprintf(stderr, "Invalid channel_nr\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 'h':
			usage_complete();
			exit(EXIT_SUCCESS);
		case 'm':
			module_nr = atoi(optarg);
			break;
		}
	}
}

static void print_chanstat(unsigned int chanstat)
{
	if (chanstat & CVORB_CHANSTAT_BUSY)
		printf("Busy\n");

	if (chanstat & CVORB_CHANSTAT_SRAM_BUSY)
		printf("SRAM Busy\n");

	if (chanstat & CVORB_CHANSTAT_ERR) {
		printf("Module in Error:\n");

		if (chanstat & CVORB_CHANSTAT_ERR_CLK)
			printf("Sampling clock not present or unstable\n");
		if (chanstat & CVORB_CHANSTAT_ERR_TRIG)
			printf("Start trigger before channel is ready\n");
		if (chanstat & CVORB_CHANSTAT_ERR_SYNC)
			printf("Sync. FIFO empty before end of waveform\n");
	}

	if (chanstat & CVORB_CHANSTAT_CLKOK)
		printf("\tSampling Clock present and stable\n");

	if (chanstat & CVORB_CHANSTAT_STARTABLE)
		printf("\tReady to accept a 'Start' trigger\n");

	if (chanstat & CVORB_CHANSTAT_OUT_ENABLED)
		printf("\tOutput enabled\n");
	else
		printf("\tOutput disabled\n");
}

int main(int argc, char *argv[])
{
	cvorb_t *device;
	unsigned int chanstat;

	parse_args(argc, argv);

	/* get a handle */
	device = cvorb_open(module_nr);
	if (device == NULL) {
		cvorb_perror("cvorb_open");
		exit(EXIT_FAILURE);
	}

	if (cvorb_channel_get_status(device, &chanstat) < 0) {
		cvorb_perror("cvorb_channel_get_status");
		exit(EXIT_FAILURE);
	}

	print_chanstat(chanstat);

	/* close the handle */
	if (cvorb_close(device)) {
		cvorb_perror("cvorb_close");
		exit(EXIT_FAILURE);
	}

	return 0;
}
