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
#define CLK_SRC 	1

int fd;
int clk_source = CLK_SRC;
int channel_nr = CHANNEL_NR;
int module_nr = MODULE_NR;

int clk_read = 0;

extern char *optarg;

static const char usage_string[] =
	"Setup clk source on a channel \n"
	" " PROGNAME " [-h] [-c<CHAN>] [-m<LUN>] [-s<CLK>]";

static const char commands_string[] =
	"options:\n"
	" -c = channel number (default: " my_stringify(CHANNEL_NR) ")\n"
	" -h = show this help text\n"
	" -m = Module number (default: " my_stringify(MODULE_NR) ")\n"
	" -s = Set source clock (default: " my_stringify(CLK_SRC) ")";

static void usage_complete(void)
{
	printf("%s\n", usage_string);
	printf("%s\n", commands_string);
}

static void parse_args(int argc, char *argv[])
{
	int c;

	for (;;) {
		c = getopt(argc, argv, "c:hm:s:");
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
			clk_source = atoi(optarg);
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

	// Enable the channel
	if(ctc_chan_enable(fd, channel_nr)) {
		fprintf(stderr, "Error enabling channel %d: %s\n", channel_nr, strerror(errno));
		goto out;
	}

	// Read the setup before change it
	if(ctc_chan_get_clk_counter1(fd, channel_nr, &clk_read)) {
		fprintf(stderr, "Error get clk source from channel %d: %s\n", channel_nr, strerror(errno));
		goto out_disable;
	}

	printf("Original -- Dev %d Channel %d Clk Source %d\n", module_nr, channel_nr, clk_read);


	printf("Writing -- Dev %d Channel %d Clk Source %d\n", module_nr, channel_nr, clk_source);
	// Write the setup 
	if(ctc_chan_set_clk_counter1(fd, channel_nr, clk_source)) {
		fprintf(stderr, "Error set %d clk source to channel %d: %s\n", clk_source, channel_nr, strerror(errno));
		goto out_disable;
	}

	clk_read = 0;
	// Read again the setup
	if(ctc_chan_get_clk_counter1(fd, channel_nr, &clk_read)) {
		fprintf(stderr, "Error get clk source from channel %d: %s\n", channel_nr, strerror(errno));
		goto out_disable;
	}

	printf("Read -- Dev %d Channel %d Clk Source %d\n", module_nr, channel_nr, clk_read);

out_disable:
	// Disable the channel
	if(ctc_chan_disable(fd, channel_nr)) {
		fprintf(stderr, "Error disabling channel %d: %s\n", channel_nr, strerror(errno));
		goto out;
	}

out:
	ctc_close(fd);
	return 0;
}
