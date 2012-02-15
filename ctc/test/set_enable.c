#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <libctc.h>

/* doing two levels of stringification allows us to stringify a macro */
#define my_stringify_l(x...)	#x
#define my_stringify(x...)	my_stringify_l(x)


#define PROGNAME	"set_status"
#define MODULE_NR 	0
#define CHANNEL_NR 	1

int fd;
int channel_nr = CHANNEL_NR;
int module_nr = MODULE_NR;

int status_read = 0;

extern char *optarg;

static const char usage_string[] =
	"Setup status on a channel \n"
	" " PROGNAME " [-h] [-c<CHAN>] [-m<LUN>] [-s<CLK>]";

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


int main(int argc, char *argv[])
{

	int hw_version;

	parse_args(argc, argv);
	
	fd = ctc_open(module_nr);	
		
	if (fd == 0) {
		fprintf(stderr, "Error opening the device: %s\n", strerror(errno));
		exit(-1);
	}

	if(ctc_get_hw_version(fd, &hw_version)) {
		fprintf(stderr, "Error get hw_version: %s\n", strerror(errno));
		goto out_disable;
	}
	
	printf("HW Version 0x%x\n", hw_version);

	status_read = -1;
	// Read the setup before change it
	if(ctc_chan_get_status(fd, channel_nr, &status_read)) {
		fprintf(stderr, "Error get status from channel %d: %s\n", channel_nr, strerror(errno));
		goto out_disable;
	}

	printf("Original -- Dev %d Channel %d Status %d\n", module_nr, channel_nr, status_read);

	// Enable the channel
	if(ctc_chan_enable(fd, channel_nr)) {
		fprintf(stderr, "Error enabling channel %d: %s\n", channel_nr, strerror(errno));
		goto out;
	}

	status_read = 0;
	// Read again the setup
	if(ctc_chan_get_status(fd, channel_nr, &status_read)) {
		fprintf(stderr, "Error get status from channel %d: %s\n", channel_nr, strerror(errno));
		goto out_disable;
	}

	printf("Enabled-- Dev %d Channel %d Status %d\n", module_nr, channel_nr, status_read);

out_disable:
	// Disable the channel
	if(ctc_chan_disable(fd, channel_nr)) {
		fprintf(stderr, "Error disabling channel %d: %s\n", channel_nr, strerror(errno));
		goto out;
	}

	status_read = 0;
	// Read again the setup
	if(ctc_chan_get_status(fd, channel_nr, &status_read)) {
		fprintf(stderr, "Error get status from channel %d: %s\n", channel_nr, strerror(errno));
		goto out_disable;
	}
	printf("Disabled -- Dev %d Channel %d Status %d\n", module_nr, channel_nr, status_read);
out:
	ctc_close(fd);
	return 0;
}
