#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <libctc.h>

/* doing two levels of stringification allows us to stringify a macro */
#define my_stringify_l(x...)	#x
#define my_stringify(x...)	my_stringify_l(x)


#define PROGNAME	"set_clk"
#define MODULE_NR 	0
#define CHANNEL_NR 	1
#define MODE 		1
#define DIRECTION	1

int fd;
int mode = MODE;
int direction = DIRECTION;
int channel_nr = CHANNEL_NR;
int module_nr = MODULE_NR;

int mode_read = 0;
int direction_read = 0;

extern char *optarg;

static const char usage_string[] =
	"Setup mode  on a channel \n"
	" " PROGNAME " [-h] [-c<CHAN>] [-m<LUN>] [-s<MODE>] [-d<DIR>";

static const char commands_string[] =
	"options:\n"
	" -c = channel number (default: " my_stringify(CHANNEL_NR) ")\n"
	" -h = show this help text\n"
	" -m = Module number (default: " my_stringify(MODULE_NR) ")\n"
	" -s = Set mode (default: " my_stringify(MODE) ")\n"
	" -d = Set direction (default: " my_stringify(DIRECTION) ")";

static void usage_complete(void)
{
	printf("%s\n", usage_string);
	printf("%s\n", commands_string);
}

static void parse_args(int argc, char *argv[])
{
	int c;

	for (;;) {
		c = getopt(argc, argv, "c:hm:s:d:");
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
		case 's':
			mode = atoi(optarg);
			break;
		case 'd':
			direction = atoi(optarg);
			break;

		}
	}
}


int main(int argc, char *argv[])
{

	parse_args(argc, argv);
	
	fd = ctc_open(module_nr);	
		
	if (fd == 0) {
		fprintf(stderr, "Error opening the device: %s\n", strerror(errno));
		exit(-1);
	}

	// Read the setup before change it
	if(ctc_chan_get_mode(fd, channel_nr, &mode_read, &direction_read)) {
		fprintf(stderr, "Error get  mode  from channel %d: %s\n", channel_nr, strerror(errno));
		goto out;
	}

	printf("Original -- Dev %d Channel %d mode %d direction %d\n", module_nr, channel_nr, mode_read, direction_read);


	printf("Writing -- Dev %d Channel %d mode %d direction %d\n", module_nr, channel_nr, mode, direction);
	// Write the setup 
	if(ctc_chan_set_mode(fd, channel_nr, mode, direction)) {
		fprintf(stderr, "Error set %d  mode  to channel %d: %s\n", mode, channel_nr, strerror(errno));
		goto out;
	}

	mode_read = 0;
	direction_read = 0;
	// Read again the setup
	if(ctc_chan_get_mode(fd, channel_nr, &mode_read, &direction_read)) {
		fprintf(stderr, "Error get  mode  from channel %d: %s\n", channel_nr, strerror(errno));
		goto out;
	}

	printf("Read -- Dev %d Channel %d mode %d direction %d\n", module_nr, channel_nr, mode_read, direction_read);

out:
	ctc_close(fd);
	return 0;
}
