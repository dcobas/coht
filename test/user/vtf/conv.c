/* gcc -g -Wall -lm conv.c -o conv */
/*
  Converts physical values into 16-bits digital values

  First -- CSV2csv.sed should run on *.CSV files, produced by
  oscilloscope. It will get rid of unused zeroes.

  Second -- this program should be run on *.csv files.

  Produced *.vft files (Vector Table File) are CVORB data table functions
  (time / 16-bit value).

  They are used by the test program to load them into SRAM.
*/
#define _GNU_SOURCE /* asprintf rocks */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <term.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <math.h>

#define WITHIN_RANGE(MIN,ARG,MAX) ( ((MAX) >= (ARG)) && ((MIN) <= (ARG)) )

struct {
	double t;
	ushort v;
} data[2048] = { { 0 } };

/* usage: conv minV maxV fname */
int main(int argc, char *argv[], char *envp[])
{
	int min, max;
	FILE *fd, *newfd;
	char *fname = NULL, *newfname = NULL;
	int t, newval;
	float val;
	int zero_val;
	int i, cntr = 0;
	int rc;
	char str[128];
	int step;

	if (argc != 4) {
		printf("Wrong arguments!\n"
		       "Usage: conv minV maV filename\n"
		       "Bye!\n");
		exit(EXIT_FAILURE);
	}

	min = atoi(argv[1]);
	max = atoi(argv[2]);
	asprintf(&fname, "%s", argv[3]);

	fd = fopen(fname, "r");
	if (!fd) {
		printf("ERROR. Failed to open '%s' file\n", fname);
		free(fname);
		exit(EXIT_FAILURE);
	}

	if (min < 0)
		zero_val = 32768;
	else
		zero_val = 0;

	/* get data from *.csv file into local structure */
	while ( (rc = fscanf(fd, "%d %f\n", &t, &val)) != EOF) {
		if (!rc) { /* skip matching failure (normally a comment) */
			rc = fscanf(fd, "%*[^\n]");
			continue;
		}

		if (!WITHIN_RANGE(min, val, max))
			continue;
		newval = (zero_val*abs(val))/max;
		data[cntr].t = t;
		if (min < 0) {
			if (val < 0)
				data[cntr].v = zero_val - newval;
			else
				data[cntr].v = zero_val + newval - 1;
		}
		++cntr;
	}

	/* store converted 16-bit values it in the *.vtf file */
	newfname = index(fname, '.');
	i = newfname - fname;
	asprintf(&newfname, "%.*s.vft", i, fname);
	newfd = fopen(newfname, "w");

	step = ceil((double)cntr/(double)679);
	for (i = 0; i < cntr; i += step) {
		sprintf(str, "%hu %hu\n", data[i].t, data[i].v);
		fwrite(str, 1, strlen(str), newfd);
	}

	fclose(fd);
	if (newfd) fclose(newfd);
	if (fname) free(fname);
	if (newfname) free(newfname);
	exit(EXIT_SUCCESS);		/* 0 */
}
