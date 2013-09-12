/*
 * cvorbtest.c
 * cvorb interactive test-program
 * derived from sis33 interactive test-program
 * Copyright (c) 2012 Michel Arruat <michel.arruat@cern.ch>
 * Copyright (c) 2010-2011 Samuel Iglesias Gonsalvez <siglesia@cern.ch>
 * Copyright (c) 2009 Emilio G. Cota <cota@braap.org>
 * Released under the GPL v2. (and only v2, not any later version)
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <sys/dir.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>

#include <extest/general_usr.h>  /* for handy definitions (mperr etc..) and macros coming from general_both.h */
#include <extest/extest.h>
#include <cvorb/libcvorb.h>
#include <cvorbdev.h>

#include "cvorbtest.h"
#if __CVORB_DEBUG__
#include "time_stamp_counter.h"
#endif
/* mandatory external global variables */
int use_builtin_cmds = 0;
char xmlfile[128] = "cvorb.xml";

/* private global variables */
static int current_ch = 1;
static int current_dev_index = 0;
static cvorb_t *current_dev=NULL;
static int *indexes=NULL;
static int ndevs;
static struct cvorb_vector_fcn fv[CVORB_MAX_VECTOR];
#define MAX_VTF_FILES 32
#define MAX_STR_SZ 32
static char vtf[MAX_VTF_FILES][MAX_STR_SZ] = { { 0 } }; /* search results */
static char path[CVORB_PATH_SIZE] = "\0";
static int file_count = 0;

int h_ch(struct cmd_desc *cmdd, struct atom *atoms)
{
	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("%s - Get/Set channel\n"
			"%s - display current channel\n"
			"%s channel_nr [1..16] - set current channel\n",
			cmdd->name, cmdd->name, cmdd->name);
		return 1;
	}
	++atoms;
	if (atoms->type == Terminator) {
		printf("%d\n", current_ch);
		return 1;
	}

	if (atoms->type != Numeric || !WITHIN_RANGE(1, atoms->val, CVORB_MAX_CH_NR))
		return -TST_ERR_WRONG_ARG;

	current_ch = atoms->val;
	return 1;
}

int h_ch_next(struct cmd_desc *cmdd, struct atom *atoms)
{
	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("%s - Select Next Channel\n", cmdd->name);
		return 1;
	}
	if (current_ch == CVORB_MAX_CH_NR)
		current_ch =1;
	else
		current_ch += 1;
	return 1;
}

int h_ch_status(struct cmd_desc *cmdd, struct atom *atoms)
{
	unsigned int ch_status, ch_state;

	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("%s - get the current channel's status\n", cmdd->name);
		return 1;
	}

	/* Get and display current channel status*/
	if (cvorb_ch_get_status(current_dev, current_ch, &ch_status) < 0) {
		mperr("get_attr channel%d status failed\n", current_ch);
		return -TST_ERR_SYSCALL;
	}
	printf("channel%d status: 0x%08x\n", current_ch, ch_status);

	if (ch_status & CVORB_CH_BUSY)
		printf("\tChannel is busy generating a waveform\n");
	if (ch_status & CVORB_CH_FCN_PAUSED)
		printf("\tWaveform paused\n");
	if (ch_status & CVORB_CH_SERIAL_LINK_ERR)
		printf("\tSerial link in fault\n");
	printf("\tChannel state machine details:\n");
	/* Extract the bit field channel state in order to use it as an index in the state messages*/
	ch_state = (ch_status >> CVORB_CH_STATE_SHIFT) & CVORB_CH_STATE_MASK;
	if (ch_state < CVORB_CH_MAX_STATES)
		printf("\t\t%s\n", channel_state_machine[ch_state]);
	else
		printf("\t\tUnknown channel state\n");
	return 0;
}

int h_ch_enable(struct cmd_desc *cmdd, struct atom *atoms)
{
	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("%s - Enable the current channel\n", cmdd->name);
		return 1;
	}

	/* Get and display current channel status*/
	if (cvorb_ch_enable(current_dev, current_ch) < 0) {
		mperr("get_attr channel%d enable failed\n", current_ch);
		return -TST_ERR_SYSCALL;
	}
	return 0;
}

int h_ch_disable(struct cmd_desc *cmdd, struct atom *atoms)
{
	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("%s - Disable the current channel\n", cmdd->name);
		return 1;
	}

	/* Get and display current channel status*/
	if (cvorb_ch_disable(current_dev, current_ch) < 0) {
		mperr("get_attr channel%d enable failed\n", current_ch);
		return -TST_ERR_SYSCALL;
	}
	return 0;
}

static void show_devices(void)
{
	char attr_path[CVORB_PATH_SIZE];
	char value[256];
	int i, len;
	cvorb_t *dev;


	for (i = 0; i < ndevs; i++) {
		char devname[32];
		int index = indexes[i];

		if (index == current_dev_index)
			printf("current");
		else
			printf("-");

		printf("\t%2d", index);
		cvorbdev_get_devname(index, devname, sizeof(devname));
		printf("\t%8s", devname);
		dev = cvorb_open(index);
		len = snprintf(attr_path, CVORB_PATH_SIZE, "%s/description", dev->sysfs_path);
		attr_path[len] = '\0';
		if (cvorbdev_get_attr_char(attr_path, value, sizeof(value))) {
			fprintf(stderr, "dev%d: retrieval of 'description' failed\n", index);
			continue;
		}
		printf("\t%s", value);

		printf("\n");
		cvorb_close(dev);
	}
}

static int __h_device(int index)
{
	if (!cvorbdev_device_exists(index)) {
		fprintf(stderr, "Device with index %d doesn't exist\n", index);
		return -TST_ERR_WRONG_ARG;
	}

	if (current_dev != NULL) {
		if (cvorb_close(current_dev))
			mperr("cvorb_close");
		current_dev = NULL;
	}

	current_dev = cvorb_open(index);
	if (current_dev == NULL) {
		fprintf(stderr, "cvorb_open failed. Check if you have root access privileges. Fatal error(exit)\n");
		/* fatal error */
		exit (-1);
	}
	current_dev_index = index;
	return 1;
}

int h_device(struct cmd_desc *cmdd, struct atom *atoms)
{
	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("%s - Set device\n"
			"%s - Show available devices\n"
			"Format:\n"
			"curr\tindex\tdev_name\tdetailed_name\n"
			"%s index - Set current device to that given by the index\n"
			"\tindex: 0-n (first CVORB board corresponds to the index 0)\n",
			cmdd->name, cmdd->name, cmdd->name);
		return 1;
	}

	++atoms;
	if (atoms->type == Terminator) {
		show_devices();
		return 1;
	}

	if (atoms->type != Numeric)
		return -TST_ERR_WRONG_ARG;

	return __h_device(atoms->val);
}

int h_device_next(struct cmd_desc *cmdd, struct atom *atoms)
{
	int *indexes;
	int ndevs;
	int ret;
	int i;

	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("%s - Set next available device\n", cmdd->name);
		return 1;
	}

	ndevs = cvorbdev_get_nr_devices();
	if (!ndevs)
		return 1;
	indexes = calloc(ndevs, sizeof(*indexes));
	if (indexes == NULL) {
		mperr("calloc");
		return -TST_ERR_SYSCALL;
	}
	if (cvorbdev_get_device_list(indexes, ndevs) < 0) {
		mperr("sis33dev_get_device_list");
		ret = -TST_ERR_SYSCALL;
		goto out;
	}

	for (i = 0; i < ndevs; i++) {
		if (indexes[i] == current_dev_index)
			break;
	}
	if (i == ndevs) {
		fprintf(stderr, "curr_index %d not in the devices' list. Has it been removed?\n", current_dev_index);
		ret = -TST_ERR_WRONG_ARG;
		goto out;
	}
	if (i == ndevs - 1)
		ret = 1;
	else
		ret = __h_device(indexes[i + 1]);
 out:
	free(indexes);
	return ret;
}

int h_hw_version(struct cmd_desc *cmdd, struct atom *atoms)
{
	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("%s - Print the HW version of the current device\n", cmdd->name);
		return 1;
	}

	printf ("%s\n", current_dev->hw_version);
	return 1;
}

int h_pcb(struct cmd_desc *cmdd, struct atom *atoms)
{
	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("%s - Print the PCB Serial Number of the current device\n", cmdd->name);
		return 1;
	}

	printf ("0x%016llx\n", (unsigned long long)current_dev->pcb_id);
	return 1;
}

static void show_submodule_status(unsigned int status)
{
	if (status & CVORB_SUBMOD_READY)
		printf("\tSubmodule is ready\n");
	if (status & CVORB_SUBMOD_SRAM_BUSY)
		printf("\tSRAM Busy (SRAM being accessed by the module)\n");
	if (status & CVORB_SUBMOD_BUSY)
		printf("\tSubmodule is Busy generating a waveform\n");
	if (status & CVORB_SUBMOD_PAUSE)
		printf("\tSubmodule is idle (no waveform generation)\n");
}

int h_submodule_status(struct cmd_desc *cmdd, struct atom *atoms)
{
	/* Get the two submodules status */
	unsigned int submod_status;
	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("%s - get the two submodule's status\n", cmdd->name);
		return 1;
	}

	/* Get and display submodule 0 status*/
	if (cvorb_sm_get_status(current_dev, 0, &submod_status) < 0) {
		mperr("get_attr submodule0 status failed\n");
		return -TST_ERR_SYSCALL;
	}
	printf("submodule 0 status: 0x%08x\n",submod_status);
	show_submodule_status(submod_status);

	/* Get and display submodule 1 status*/
	if (cvorb_sm_get_status(current_dev, 1, &submod_status) < 0) {
		mperr("get_attr submodule1 status failed\n");
		return -TST_ERR_SYSCALL;
	}
	printf("submodule 1 status: 0x%08x\n",submod_status);
	show_submodule_status(submod_status);

	return 1;
}

#define SHOW_POL(pol) ((pol)?"negative pulse":"positive pulse")
static int show_polarity(int smnr)
{
	enum cvorb_input_polarity polarity;
	if (smnr==-1 || smnr==0)
	{
		if (cvorb_sm_get_input_polarity(current_dev, 0, &polarity)<0) {
			mperr("get_attr submodule0 input_polarity failed\n");
			return -TST_ERR_SYSCALL;
		}
		printf("Submodule 0 input's pulses polarity: %d (%s)\n", polarity,
				SHOW_POL(polarity));
	}
	if (smnr==-1 || smnr==1)
	{
		if (cvorb_sm_get_input_polarity(current_dev, 1, &polarity)<0) {
			mperr("get_attr submodule0 input_polarity failed\n");
			return -TST_ERR_SYSCALL;
		}
		printf("Submodule 1 input's pulses polarity: %d (%s)\n", polarity,
				SHOW_POL(polarity));
	}
	return 1;
}

int h_submodule_inpol(struct cmd_desc *cmdd, struct atom *atoms)
{
	unsigned int submodule, polarity;
	/* Get the two submodules status */
	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("%s - Submodule input's pulses polarity of teh current device\n"
		       "%s - show the input polarity for the two submodules\n"
		       "%s submodule_nr[0,1] - show the input polarity for the given submodule\n"
		       "%s submodule_nr[0,1] polarity - set the desired input polarity for the given submodule\n"
		       "\tpolarity: 0=positive pulses, 1=negative pulses\n",
		       cmdd->name, cmdd->name, cmdd->name, cmdd->name);
		return 1;
	}
	++atoms;
	if (atoms->type == Terminator)
		return show_polarity(-1);
	if (atoms->type != Numeric)
		return -TST_ERR_WRONG_ARG;
	if (!WITHIN_RANGE(0, atoms->val, 1))
		return -TST_ERR_WRONG_ARG;
	submodule = atoms->val;

	++atoms;
	if (atoms->type == Terminator)
		return show_polarity(submodule);
	if (atoms->type != Numeric)
		return -TST_ERR_WRONG_ARG;
	if (!WITHIN_RANGE(0, atoms->val, 1))
		return -TST_ERR_WRONG_ARG;
	polarity = atoms->val;
	if ( cvorb_sm_set_input_polarity(current_dev, submodule, polarity) < 0 ) {
		mperr("set_attr submodule %d input_polarity failed\n", submodule);
		return -TST_ERR_SYSCALL;
	}
	printf("Polarity changed:\n");
	return show_polarity(submodule);
}

int h_temp(struct cmd_desc *cmdd, struct atom *atoms)
{
	int temp;

	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("%s - Get the board's temperature\n",
		       cmdd->name);
		return 1;
	}
	if (cvorb_get_temperature(current_dev, &temp) < 0) {
		mperr("get_attr temperature failed\n");
		return -TST_ERR_SYSCALL;
	}
	printf("%d\n", temp);
	return 1;
}

int h_rst(struct cmd_desc *cmdd, struct atom *atoms)
{
	char attr_path[CVORB_PATH_SIZE];
	int len;

	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("%s - Board full Software reset (FPGA)\n",
		       cmdd->name);
		return 1;
	}

	len = snprintf(attr_path, CVORB_PATH_SIZE, "%s/reset", current_dev->sysfs_path);
	attr_path[len] = '\0';
	if (cvorbdev_set_attr_uint32(attr_path, 1) < 0 ) {
		mperr("set_attr board reset failed\n");
		return -TST_ERR_SYSCALL;
	}
	printf("Done\n");
	return 1;
}

int h_submodule_rst(struct cmd_desc *cmdd, struct atom *atoms)
{
	char attr_path[CVORB_PATH_SIZE];
	int len;
	int n_sm, sm_nr[2], i;

	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("%s - software reset of the submodule\n"
		       "%s - send reset to the two submodules\n"
		       "%s submodule_nr [0/1] - desired submodule to reset\n",
		       cmdd->name, cmdd->name, cmdd->name);
		return 1;
	}
	++atoms;
	if (atoms->type == Terminator){
		/* Reset the two submodules */
		n_sm =2;
		sm_nr[0] = 0;
		sm_nr[1] = 1;
	}
	else {
		if (atoms->type != Numeric || !WITHIN_RANGE(0, atoms->val, 1)) {
			printf("Invalid submodule number\n");
			return -TST_ERR_WRONG_ARG;
		}
		n_sm =1;
		sm_nr[0] = atoms->val;
	}
	for (i=0; i<n_sm; ++i) {
		len = snprintf(attr_path, CVORB_PATH_SIZE, "%s/submodule.%d/reset", current_dev->sysfs_path, sm_nr[i]);
		attr_path[len] = '\0';
		if (cvorbdev_set_attr_uint32(attr_path, 1) < 0) {
			mperr("set_attr board reset failed\n");
			return -TST_ERR_SYSCALL;
		}
	}
	printf("Done\n");
	return 1;
}

int h_submodule_trigger(struct cmd_desc *cmdd, struct atom *atoms)
{
	int submodule;
	uint32_t trig;
#if __CVORB_DEBUG__
	double start, stop;
#endif

	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("%s - send a software trigger to the submodule corresponding to the current channel\n"
		       "%s trigger \n"
		       "\ttrigger - send the trigger given by \"string\":\n"
		       "\t\t\"start\": Start\n"
		       "\t\t\"stop\": Stop\n"
		       "\t\t\"evstart\": Event Start\n"
		       "\t\t\"evstop\": Event Stop\n",
		       cmdd->name, cmdd->name);
		return 1;
	}

	++atoms;
	if (atoms->type != String) {
		printf("Invalid trigger name\n");
		return -TST_ERR_WRONG_ARG;
	}
	if (!strncmp(atoms->text, "start", MAX_ARG_LENGTH))
		trig = CVORB_START;
	else if (!strncmp(atoms->text, "stop", MAX_ARG_LENGTH))
		trig = CVORB_STOP;
	else if (!strncmp(atoms->text, "evstart", MAX_ARG_LENGTH))
		trig = CVORB_EVT_START;
	else if (!strncmp(atoms->text, "evstop", MAX_ARG_LENGTH))
		trig = CVORB_EVT_STOP;
	else {
		printf("Invalid trigger name\n");
		return -TST_ERR_WRONG_ARG;
	}
	// Submodule is deduced from the current channel
	submodule = (current_ch > 8) ? 1 : 0;
#if __CVORB_DEBUG__
	start = ts_getValue(YS_UNIT);
#endif
	if (cvorb_sm_set_trigger(current_dev, submodule, trig) < 0) {
		mperr("set_attr trigger failed\n");
		return -TST_ERR_SYSCALL;
	}
#if __CVORB_DEBUG__
	stop = ts_getValue(YS_UNIT);
	fprintf(stderr, "Trigger sent(us): %g\n", stop - start);
#endif
	return 1;
}

int h_submodule_enopt(struct cmd_desc *cmdd, struct atom *atoms)
{
	printf("Not yet implemented \n");
	return 1;
}

int h_ch_src(struct cmd_desc *cmdd, struct atom *atoms)
{
	int submodule_nr, len;
	char attr_path[CVORB_PATH_SIZE];

	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("%s - Connect the current channel to the given resource\n"
		       "%s resource \n"
		       "\tresource - the resource given by \"string\":\n"
		       "\t\t\"dac\": on board DAC\n"
		       "\t\t\"opt\": Optical output\n"
		       "\t\t\"led\": Front panel led\n",
		       cmdd->name, cmdd->name);
		return 1;
	}

	submodule_nr = (current_ch>8) ? 1 : 0;

	++atoms;
	if (atoms->type != String) {
		printf("Invalid resource name\n");
		return -TST_ERR_WRONG_ARG;
	}
	if (!strncmp(atoms->text, "dac", MAX_ARG_LENGTH))
		len = snprintf(attr_path, CVORB_PATH_SIZE, "%s/submodule.%d/dac_source", current_dev->sysfs_path, submodule_nr);
	else if (!strncmp(atoms->text, "opt", MAX_ARG_LENGTH))
		len = snprintf(attr_path, CVORB_PATH_SIZE, "%s/submodule.%d/optical_source", current_dev->sysfs_path, submodule_nr);
	else if (!strncmp(atoms->text, "led", MAX_ARG_LENGTH))
		len = snprintf(attr_path, CVORB_PATH_SIZE, "%s/submodule.%d/led_source", current_dev->sysfs_path, submodule_nr);
	else {
		printf("Invalid resource name\n");
		return -TST_ERR_WRONG_ARG;
	}
	attr_path[len] = '\0';
	if (cvorbdev_set_attr_uint32(attr_path, current_ch) < 0) {
		mperr("set_attr %s submodule %d failed\n", atoms->text, submodule_nr);
		return -TST_ERR_SYSCALL;
	}
	return 1;
}

int h_ch_repeat(struct cmd_desc *cmdd, struct atom *atoms)
{
	unsigned int repeat_count;
	/* Get/Set repeat count */
	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("%s - Get/Set number of times the selected function will be generated for the current channel\n"
		       "%s - show the repeat count\n"
		       "%s count[0,2^32-1] - set repeat count(0 means infinite number)\n",
		       cmdd->name, cmdd->name, cmdd->name);
		return 1;
	}
	++atoms;
	if (atoms->type == Terminator)
	/* display current repeat count*/
	{
		if (cvorb_ch_get_repeat_count(current_dev, current_ch, &repeat_count) < 0)
		{
			mperr("set_attr trigger failed\n");
			return -TST_ERR_SYSCALL;
		}
		printf("Channel %d repeat count: %d\n", current_ch, repeat_count);
		return 1;
	}
	if (atoms->type != Numeric)
		return -TST_ERR_WRONG_ARG;
	if (!WITHIN_RANGE(0, atoms->val, (int)(pow(2, 32)-1)))
		return -TST_ERR_WRONG_ARG;
	repeat_count = atoms->val;
	if (cvorb_ch_set_repeat_count(current_dev, current_ch, repeat_count) < 0)
	{
		mperr("set_attr trigger failed\n");
		return -TST_ERR_SYSCALL;
	}
	return 1;
}

static int get_fcn_list(int display) {
	DIR *dir;
	struct direct *ent;
	unsigned int i;

	if (file_count == 0) {
		/* First try to open local vtf directory */
		snprintf(path, CVORB_PATH_SIZE, "./vtf");
		dir = opendir(path);
		if (!dir) {
			/* No local directory try oper one*/
			snprintf(path, CVORB_PATH_SIZE, "/usr/local/cvorb/vtf");
			dir = opendir(path);
		}

		if (!dir) {
			printf("Can't locate vtf files neither locally nor under /usr/local/drivers/cvorb/vtf\n");
			return -1;
		}

		/* scan dir for .vtf files */
		while ( (ent = readdir(dir)) && file_count < MAX_VTF_FILES)
			if (strstr(ent->d_name, ".vtf"))
				strncpy(vtf[file_count++], ent->d_name, sizeof(vtf[0]));
		closedir(dir);
	}
	if (display)
		for (i=0; i<file_count; ++i)
			printf("\t\t- %s\n", vtf[i]);
	return 0;
}

static int compute_vector_table(char* fcn_name, int* vect_count) {
	FILE *fd;
	int rc=0, i=0;

	fd = fopen(fcn_name, "r");
	if (!fd) {
		mperr("Can't open %s Vecto Table File \n", fcn_name);
		return -1;
	}

	bzero(fv, sizeof(struct cvorb_vector_fcn)*CVORB_MAX_VECTOR);
	/* get data from *.vtf file into local structure */
	while ( (rc = fscanf(fd, "%lf %hu\n", &fv[i].t, &fv[i].v)) != EOF) {
		if (!rc) { /* skip matching failure */
			rc = fscanf(fd, "%*[^\n]");
			continue;
		}
		++i;
		if (i == CVORB_MAX_VECTOR)
			break;
	}

	fclose(fd);
	*vect_count = i;
	return 0;
}

int h_fcn_load(struct cmd_desc *cmdd, struct atom *atoms)
{
	int i=0, fcn_nr, vect_count /*, driver_call=0 */;
	char fcn_name[CVORB_PATH_SIZE];
#if __CVORB_DEBUG__
	double start, stop;
#endif
/*
	struct timeval start_tv, stop_tv;
	unsigned long long delay;
*/
	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("%s - Load the current channel with a predefined function\n"
		       "%s function_nr function_type  \n"
		       "\tfunction_nr - function number [1,64]\n"
/*
		       "\tdriver call - driver call type (0: ioctl, 1: sysfs)"
*/
		       "\tfunction_type - function to load among a list of predefined ones given by \"string\":\n",
		       cmdd->name, cmdd->name);
		return (get_fcn_list(1));
	}

	++atoms;
	if (atoms->type != Numeric || !WITHIN_RANGE(1, atoms->val, 64)) {
		printf("Invalid function number\n");
		return -TST_ERR_WRONG_ARG;
	}
	fcn_nr = atoms->val;
/*
	++atoms;
	if (atoms->type != Numeric || !WITHIN_RANGE(0, atoms->val, 1)) {
		printf("Invalid resource name\n");
		return -TST_ERR_WRONG_ARG;
	}
	driver_call = atoms->val;
*/
	++atoms;
	if (atoms->type != String) {
		printf("Invalid resource name\n");
		return -TST_ERR_WRONG_ARG;
	}

	/* Initialize the vtf table if it's not already done */
	if ( (file_count == 0) && (get_fcn_list(0) < 0) ) {
		return -1;
	}

	/* Check if the name is valid */
	for (i=0; i<file_count; ++i) {
		if (!strcmp(vtf[i], atoms->text))
			break; /*valid file name */
	}
	if (i == file_count) {
		printf("Invalid CVORB Vector Table File name\n");
		return -1;
	}
	bzero(fv, sizeof(struct cvorb_vector_fcn)*CVORB_MAX_VECTOR);
	/* Translate the VTF file into a vector table */
	snprintf(fcn_name, CVORB_PATH_SIZE, "%s/%s", path, atoms->text);
	if (compute_vector_table(fcn_name, &vect_count) < 0)
		return -1;
/*
	if (driver_call == 0)
	{
*/
	       /* Just to check if the counter based on the CPU clock is correct
		* gettimeofday(&start_tv, 0);
		*/
#if __CVORB_DEBUG__
		start = ts_getValue(YS_UNIT);
#endif
/*
		if (cvorb_ch_ioctl_set_fcn(current_dev, current_ch, fcn_nr, fv, vect_count) == -1) {
*/
		if (cvorb_ch_set_fcn(current_dev, current_ch, fcn_nr, fv, vect_count) == -1) {
			printf("Load function failed\n");
		}
#if __CVORB_DEBUG__
		stop = ts_getValue(YS_UNIT);
		/* Just to check if the counter baced on the CPU clock is correct
		 * gettimeofday(&stop_tv, 0);
		 * delay = (unsigned long long) ((stop_tv.tv_sec * 1000000 + stop_tv.tv_usec) - (start_tv.tv_sec * 1000000 + start_tv.tv_usec));
		 * fprintf(stderr, "ioctl duration(us) to write HW: %g, gettimeofday:%llu\n", stop - start, delay);
		 */
		fprintf(stderr, "ioctl duration(us) to write HW: %g\n", stop - start);
#endif
/*
	}
	else
	{
#if __CVORB_DEBUG__
		start = ts_getValue(YS_UNIT);
#endif
		if (cvorb_ch_sysfs_set_fcn(current_dev, current_ch, fcn_nr, fv, vect_count) == -1) {
			printf("Load function failed\n");
		}
#if __CVORB_DEBUG__
		stop = ts_getValue(YS_UNIT);
		fprintf(stderr, "sysfs duration(us): %g\n", stop - start);
#endif
	}
*/
	return 1;
}

static int write_file(const char *filename, struct cvorb_vector_fcn *fv, unsigned int n_vector)
{
	FILE *filp;
	int i;

	int mask = umask(0); /* set umask to have a file with 0666 mode */
	filp = fopen(filename, "w");
	umask(mask); /* restore previous umask */
	if (filp == NULL) {
		mperr("fopen");
		return -1;
	}
	for (i=0; i < n_vector; i++)
		fprintf(filp, "%g %d\n", fv[i].t, fv[i].v);
	if (fclose(filp) < 0)
		return -1;
	return 0;
}

int h_fcn_read(struct cmd_desc *cmdd, struct atom *atoms)
{
	unsigned int fcn_nr, n_vector, display_type=0;
	char devname[32];
	char filename[64];
	char cmd[256];
	/* the following variables are just used to manage the dump of the vector table on the screen (5 columns)*/
	int i, noc=5/*number of columns */, nol/*number of lines*/, mod /*modulo noc*/;

	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("%s - Read and display the current channel function from the module (SRAM)\n"
		       "%s function_nr display\n"
		       "\tfunction_nr - function number [1,64]\n"
		       "\tdisplay - display type\n"
		       "\t\t0: table of vectors (default display)\n"
		       "\t\t1: X term plot (using X display)\n"
		       "\t\t2: TTY plot (no X display)\n",
		       cmdd->name, cmdd->name);
		return 1;
	}

	++atoms;
	if (atoms->type != Numeric || !WITHIN_RANGE(1, atoms->val, 64)) {
		printf("Invalid function number\n");
		return -TST_ERR_WRONG_ARG;
	}
	fcn_nr = atoms->val;

	++atoms;
	if (atoms->type == Numeric)
		display_type = atoms->val;

	bzero(fv, sizeof(struct cvorb_vector_fcn)*CVORB_MAX_VECTOR);
	if(cvorb_ch_get_fcn(current_dev, current_ch, fcn_nr, fv, CVORB_MAX_VECTOR, &n_vector) < 0) {
		printf("Read function failed\n");
		return -1;
	}
	switch (display_type) {
	case 0:
		printf("Channel(%d) function(%d) read : number of vectors is %d\n", current_ch, fcn_nr, n_vector);
		nol = (int)(n_vector/noc);
		mod = n_vector%noc;
		/* print the 5 column's headers */
		for (i=0; i<5; ++i)
			printf("%3s%15s%10s | ", "nb", "time(ms)", "amplitude");
		printf("\n");
		/* print the vector table */
		for (i=0; i<nol; i++) {
			printf("%3d:%13.3f %10hu | \%3d:%13.3f %10hu | %3d:%13.3f %10hu | %3d:%13.3f %10hu | %3d:%13.3f %10hu\n",
					i, fv[i].t, fv[i].v,
					i+mod+nol, fv[i+mod+nol].t, fv[i+mod+nol].v,
					i+mod+(2*nol), fv[i+mod+(2*nol)].t, fv[i+mod+(2*nol)].v,
					i+mod+(3*nol), fv[i+mod+(3*nol)].t, fv[i+mod+(3*nol)].v,
					i+mod+(4*nol), fv[i+mod+(4*nol)].t, fv[i+mod+(4*nol)].v);
		}
		/* print the incomplete line if any */
		for (i=1; i<=mod; i++)
			printf("%3d:%13.3f %10hu | ", (i*nol)+i, fv[(i*nol)+i].t, fv[(i*nol)+i].v);
		printf("\n");
		break;
	case 1:
	case 2:
		cvorbdev_get_devname(current_dev_index, devname, sizeof(devname));
		snprintf(filename, sizeof(filename), "/tmp/%s.ch%d.fcn%d.dat", devname, current_ch, fcn_nr);
		if (write_file(filename, fv, n_vector) < 0) {
			printf("Read function failed\n");
			return -1;
		}
		snprintf(cmd, sizeof(cmd), "echo \"%s plot '%s' with lines \" | gnuplot -persist",
					(display_type ==1) ? "set term x11; " : "set term dumb; ", filename);
		cmd[sizeof(cmd) - 1] = '\0';
		system(cmd);

		break;
	default:
		break;
	}
	return 1;
}

int h_fcn_enable(struct cmd_desc *cmdd, struct atom *atoms)
{
	unsigned int fcnnr;

	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("%s - Enable current channel's function\n"
		       "%s function number\n"
		       "\tfunction_nr - function number [1,64]\n",
		       cmdd->name, cmdd->name);
		return 1;
	}
	++atoms;
	if (atoms->type != Numeric)
		return -TST_ERR_WRONG_ARG;
	if (!WITHIN_RANGE(1, atoms->val, 64))
		return -TST_ERR_WRONG_ARG;
	fcnnr = atoms->val;

	if ( cvorb_ch_enable_fcn(current_dev, current_ch, fcnnr) < 0 ) {
		mperr("set_attr channel %d enable_fcn failed\n", current_ch);
		return -TST_ERR_SYSCALL;
	}
	return 1;
}

int h_fcn_disable(struct cmd_desc *cmdd, struct atom *atoms)
{
	unsigned int fcnnr;

	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("%s - Disable current channel's function\n"
		       "%s function number\n"
		       "\tfunction_nr - function number [1,64]\n",
		       cmdd->name, cmdd->name);
		return 1;
	}
	++atoms;
	if (atoms->type != Numeric)
		return -TST_ERR_WRONG_ARG;
	if (!WITHIN_RANGE(1, atoms->val, 64))
		return -TST_ERR_WRONG_ARG;
	fcnnr = atoms->val;

	if ( cvorb_ch_disable_fcn(current_dev, current_ch, fcnnr) < 0 ) {
		mperr("set_attr channel %d disable_fcn failed\n", current_ch);
		return -TST_ERR_SYSCALL;
	}
	return 1;
}

int h_fcn_enable_mask(struct cmd_desc *cmdd, struct atom *atoms)
{
	uint64_t enbl_mask;

	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("%s - Get current channel's function enable mask\n",
		       cmdd->name);
		return 1;
	}

	if ( cvorb_ch_get_enable_fcn_mask(current_dev, current_ch, &enbl_mask) < 0 ) {
		mperr("set_attr channel %d enable_mask_fcn failed\n", current_ch);
		return -TST_ERR_SYSCALL;
	}
	printf("0x%llx\n", enbl_mask);
	return 1;
}

int h_fcn_select(struct cmd_desc *cmdd, struct atom *atoms)
{
	unsigned int fcnnr;

	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("%s - Select current channel's function\n"
		       "%s function number\n"
		       "\tfunction_nr - function number [1,64]\n",
		       cmdd->name, cmdd->name);
		return 1;
	}
	++atoms;
	if (atoms->type != Numeric)
		return -TST_ERR_WRONG_ARG;
	if (!WITHIN_RANGE(1, atoms->val, 64))
		return -TST_ERR_WRONG_ARG;
	fcnnr = atoms->val;

	if ( cvorb_ch_select_fcn(current_dev, current_ch, fcnnr) < 0 ) {
		mperr("set_attr channel %d select_fcn failed\n", current_ch);
		return -TST_ERR_SYSCALL;
	}
	return 1;
}

int h_fcn_play(struct cmd_desc *cmdd, struct atom *atoms)
{
	int res =0;
	struct atom trigatoms[3];

	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("%s - Play a function already loaded into the current channel\n"
		       "%s function_nr\n"
		       "\tfunction_nr - function number [1,64]\n",
		       cmdd->name, cmdd->name);
		return (get_fcn_list(1));
	}

	/*Chain automatically the full sequence, that is to say */
	res = h_ch_enable(cmdd, atoms);
	if (res == -1) {
		printf("Enable channel failed\n");
		return -1;
	}

	res = h_fcn_enable(cmdd, atoms);
	if (res == -1) {
		printf("Enable function failed\n");
		return -1;
	}

	res = h_fcn_select(cmdd, atoms);
	if (res == -1) {
		printf("Load function failed\n");
		return -1;
	}

	/* Send start event trigger */
	/* header function name*/
	strncpy(trigatoms[0].text, "sm_trig", strlen("sm_trig"));
	trigatoms[0].text[strlen("sm_trig")] = '\0';
	trigatoms[0].type = String;
	/* trigger name atom */
	strncpy(trigatoms[1].text, "start", strlen("start"));
	trigatoms[1].text[strlen("start")] = '\0';
	trigatoms[1].type = String;
	/* terminator */
	trigatoms[2].type = Terminator;
	trigatoms[2].text[0] = '\0';
	res = h_submodule_trigger(cmdd, trigatoms);
	return 1;
}

int h_load_and_play(struct cmd_desc *cmdd, struct atom *atoms)
{
	int res =0;
	struct atom trigatoms[3];

	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("%s - Load/Enable/Play the current channel with a predefined function\n"
		       "%s function_nr function_type  \n"
		       "\tfunction_nr - function number [1,64]\n"
		       "\tfunction_type - function to load among a list of predefined ones given by \"string\":\n",
		       cmdd->name, cmdd->name);
		return (get_fcn_list(1));
	}

	/*Chain automatically the full sequence, that is to say */
	/* Load function */
	res = h_fcn_load(cmdd, atoms);
	if (res == -1) {
		printf("Load function failed\n");
		return -1;
	}

	res = h_ch_enable(cmdd, atoms);
	if (res == -1) {
		printf("Enable channel failed\n");
		return -1;
	}

	res = h_fcn_enable(cmdd, atoms);
	if (res == -1) {
		printf("Enable function failed\n");
		return -1;
	}

	res = h_fcn_select(cmdd, atoms);
	if (res == -1) {
		printf("Load function failed\n");
		return -1;
	}

	/* header function name*/
	strncpy(trigatoms[0].text, "sm_trig", strlen("sm_trig"));
	trigatoms[0].text[strlen("sm_trig")] = '\0';
	trigatoms[0].type = String;
	/* trigger name atom */
	strncpy(trigatoms[1].text, "start", strlen("start"));
	trigatoms[1].text[strlen("start")] = '\0';
	trigatoms[1].type = String;
	/* terminator */
	trigatoms[2].type = Terminator;
	trigatoms[2].text[0] = '\0';
	res = h_submodule_trigger(cmdd, trigatoms);
	return 1;
}

int main(int argc, char *argv[], char *envp[])
{
#if __CVORB_DEBUG__
	ts_calibrateCountPeriod(50, 10);
#endif
	ndevs = cvorbdev_get_nr_devices();
	if (!ndevs)
	{
		fprintf(stderr, "No cvorb installed.Exit.\n");
		exit(0);
	}
	indexes = calloc(ndevs, sizeof(*indexes));
	if (indexes == NULL)
	{
		fprintf(stderr, "calloc failed: No enough memory.Exit.\n");
		exit(0);
	}
	if (cvorbdev_get_device_list(indexes, ndevs) < 0) {
		mperr("cvorbdev_get_device_list failed. Exit");
		exit(0);
	}
	/* set current device to the first one*/
	__h_device(indexes[0]);
	return extest_init(argc, argv, DRIVER_NAME);
}

enum _tag_cmd_id {
	CmdCH = CmdUSR,
	CmdCH_NEXT,
	CmdCH_STATUS,
	CmdCH_ENABLE,
	CmdCH_DISABLE,
	CmdCH_SRC,
	CmdCH_REPEAT_COUNT,
	CmdDEVICE,
	CmdDEVICE_NEXT,
	CmdFCN_LOAD,
	CmdFCN_PLAY,
	CmdFCN_READ,
	CmdFCN_ENABLE,
	CmdFCN_DISABLE,
	CmdFCN_SELECT,
	CmdFCN_ENABLE_MASK,
	CmdFCN_LOAD_PLAY,
	CmdHW_VERSION,
	CmdHW_RESET,
	CmdPCB,
	CmdSUBMODULE_RESET,
	CmdSUBMODULE_TRIGGER,
	CmdSUBMODULE_STATUS,
	CmdSUBMODULE_INPOL,
	CmdSUBMODULE_EN_OPTOUT,
	CmdTEMP,
	CmdLAST
};

struct cmd_desc user_cmds[] = {
	{ 1, CmdCH,                     "ch", "Get/Set channel", "[ch_nr]", 0, h_ch },
	{ 1, CmdCH_NEXT,                "ch_next", "Next Channel", "", 0, h_ch_next },
	{ 1, CmdDEVICE,                 "dev", "Get/Set device", "[index]", 0, h_device },
	{ 1, CmdDEVICE_NEXT,            "dev_next", "Set next device", "", 0, h_device_next },
	{ 1, CmdTEMP,                   "temp", "Get temperature's board", "", 0, h_temp},
	{ 1, CmdHW_VERSION,             "hw_vs", "Get the HW version", "", 0, h_hw_version},
	{ 1, CmdPCB,                    "pcb", "Get the PCB Serial Number", "", 0, h_pcb},
	{ 1, CmdHW_RESET,               "rst", "Board software reset", "", 0, h_rst},
/* Not implemented
	{ 1, CmdSUBMODULE_EN_OPTOUT,    "sm_en_opt", "Enable submodule optical output", "sm_nr", 0, h_submodule_enopt},
*/
	{ 1, CmdSUBMODULE_INPOL,        "sm_inpol", "Submodule Input's polarity ", "[sm_nr] [polarity]", 0, h_submodule_inpol},
	{ 1, CmdSUBMODULE_RESET,        "sm_rst", "Submodule software reset", "[sm_nr]", 0, h_submodule_rst},
	{ 1, CmdSUBMODULE_STATUS,       "sm_st", "Get submodule status", "", 0, h_submodule_status},
	{ 1, CmdSUBMODULE_TRIGGER,      "sm_trig", "Submodule software pulse", "trigger", 0, h_submodule_trigger},
	{ 1, CmdCH_STATUS,              "ch_st", "Get current channel Status", "", 0, h_ch_status},
	{ 1, CmdCH_ENABLE,              "ch_en", "Enable current channel", "", 0, h_ch_enable},
	{ 1, CmdCH_DISABLE,             "ch_dis", "Disable current channel", "", 0, h_ch_disable},
	{ 1, CmdCH_SRC,                 "ch_src", "Connect the current channel to the given resource", "resource", 0, h_ch_src},
	{ 1, CmdCH_REPEAT_COUNT,        "ch_repeat", "Set number of times a function will be generated for the current channel", "[n]", 0, h_ch_repeat},
	{ 1, CmdFCN_LOAD,               "f_load", "Load the current channel's function into SRAM", "fcn_nr fcn_type", 0, h_fcn_load},
	{ 1, CmdFCN_PLAY,               "f_play", "Play the current channel's function from SRAM", "fcn_nr", 0, h_fcn_play},
	{ 1, CmdFCN_READ,               "f_read", "Read the current channel's function from SRAM", "fcn_nr display_type", 0, h_fcn_read},
	{ 1, CmdFCN_ENABLE,             "f_en", "Enable current channel's function", "fcn_nr", 0, h_fcn_enable},
	{ 1, CmdFCN_DISABLE,            "f_dis", "Disable current channel's function", "fcn_nr", 0, h_fcn_disable},
	{ 1, CmdFCN_ENABLE_MASK,        "f_en_mask", "Get current channel's function enable mask", "", 0, h_fcn_enable_mask},
	{ 1, CmdFCN_SELECT,             "f_sel", "Select current channel's function", "fcn_nr", 0, h_fcn_select},
	{ 1, CmdFCN_LOAD_PLAY,          "f_load_play", "Load, Enable and Play the current channel's function", "fcn_nr fcn_type", 0, h_load_and_play},
	{ 0, }
};

