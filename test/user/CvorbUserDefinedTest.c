#define _GNU_SOURCE /* asprintf rocks */
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include "CvorbTest.h"
#include "CvorbUserDefinedTest.h"
#include "CvorbUserDefinedDrvr.h"
#include "CvorbDefs.h"

static int load_vtf(FILE *, struct fv **);
static int open_vtf(FILE **);
static int get_channel(int *);
static int get_function(int *);
static int get_module(int *);
static int get_value(uint *, uint, uint);

/**
 * @brief
 *
 * @param handle - lib handle, ret. by @e DaEnableAccess() call
 * @param lun    - logical unit number.
 *
 * @return
 */
int UserDefinedMenu(HANDLE handle, int lun)
{
	int i;
	int choice;
	int do_usr_wait = 0;
	int fd, rc;
	static int cvorbh;
	int ch, fn, md;
	uint val;
	FILE *f;

	if (!cvorbh) /* init once */
		cvorbh = cvorb_init(lun);

	for (;;) {
		printf("%sCVORB %s <V. %d> Test Program - User Defined"
		       " Menu\n[%s]\n", ClearScreen,
		       (lun < 0)?"Simulator":"Driver", g_drvrVers,
		       g_drvrVersInfo);

		for (i = 0; i < screenWidth(); i++)
			printf("-");

		printf("\n\n");
		printf("01 -- Return to Main Menu\n");
		printf("02 -- Read VHDL version, Temperature and PCB"
		       " serial number\n");
		printf("03 -- Read Module/Channel Configuration && Status"
		       " registers\n"
		       "      Function selection && Function Enable "
		       "registers\n");
		printf("04 -- Load function into SRAM\n");
		printf("05 -- Read function from SRAM\n");
		printf("06 -- TODO Play already loaded function\n");
		printf("07 -- Load && play a function\n");
		printf("08 -- Select  Function\n");
		printf("09 -- Enable  Function\n");
		printf("10 -- Disable Function\n");
		printf("11 -- Set/Get Recurrent Cycles\n");
		printf("12 -- Set Module  Configuration register\n");
		printf("13 -- Set Channel Configuration register\n");
		printf("14 -- Test functionality\n");
		printf("\n> ");

		scanf("%d", &choice);
		getchar();

		switch (choice) {
		case 14:
			{
				do_usr_wait = 1;
				break;
			}
		case 1:
			return(OK);
			break;
		case 2:
			{
				int t;
				char str[32] = { 0 };
				uint vhdlv = 0;

				do_usr_wait = 1;

				/* as an example of HOWTO use DAL */
				fd = DaGetNodeFd(handle);
				if (fd < 0)
					printf("Can't get driver node"
					       " (cc = %d)\n", fd);
				if (ioctl(fd, CVORB_VHDL, &vhdlv))
					printf("CVORB_VHDL ioctl failed\n");
				else
					printf("VHDL v --> 0x%x ", vhdlv);
				cvorb_rd_vhdl_vers(cvorbh, str);
				cvorb_rd_temp(cvorbh, &t);
				printf(" (%s)\n", str); /* VHDL string */
				printf("Temp   --> %d C\n", t);
				cvorb_rd_pcb_sn(cvorbh, str);
				printf("PCB SN --> %s\n", str);
				break;
			}
		case 3:
			{
				uint data;
				uint mask[2];
				int i;
				do_usr_wait = 1;

				printf("Select Channel to query"
				       " (0 -- all of them)\n");

				ch = 0;
				if (get_channel(&ch))
					break;

				if (ch)
					i = ch;
				else {
					i = 1;
					ch = MAX_CHAN_AM;
				}

				for (; i <=ch; i++) {
					cvorb_rd_mconfig(cvorbh, i, &data);
					printf("+---------[ ch#%d ]--------\n",
					       i);
					printf("| Module Configuration reg  "
					       "--> 0x%x\n", data);

					cvorb_rd_mstat(cvorbh, i, &data);
					printf("| Module Status reg         "
					       "--> 0x%x\n", data);

					cvorb_rd_cconfig(cvorbh, i, &data);
					printf("| Channel Configuration reg "
					       "--> 0x%x\n", data);

					cvorb_rd_cstat(cvorbh, i, &data);
					printf("| Channel Status reg        "
					       "--> 0x%x\n", data);

					printf("| Function Selection reg    "
					       "--> %d\n",
					       cvorb_func_get(cvorbh, i));

					cvorb_rd_rcyc(cvorbh, i, &data);
					printf("| Recurrent Cycles reg      "
					       "--> 0x%x\n", data);

					cvorb_rd_fem(cvorbh, i, mask);
					printf("| Function Enable Mask"
					       " registers:\n"
					       "| [63-32] --> 0x%x\n"
					       "| [31-0]  --> 0x%x\n",
					       mask[0], mask[1]);
					printf("+--------------------------\n");
				}
				break;
			}
		case 4: /* load function into SRAM */
			{
				struct fv *fv; /* function vector table */
				int noe; /* number of entries */

				do_usr_wait = 1;
				/* get channel, function &&
				   open vector table file */
				fn = ch = 1;
				if (get_channel(&ch) || get_function(&fn) ||
				    open_vtf(&f))
					break;

				noe = load_vtf(f, &fv);
				if (noe < 0) {
					printf("ERROR! Can't load VTF\n");
					break;
				}

				/* load function vector table */
				rc = cvorb_func_load(cvorbh, ch, fn, fv, noe);
				if (rc < 0) {
					printf("ERROR! Load Function#%d for"
					       " Channel#%d in device SRAM"
					       " failed [%s]\n",
					       fn, ch, cvorb_perr(rc));
					break;
				} else {
					printf("Function#%d for Channel#%d"
					       " loaded\n", fn, ch);
					break;
				}
			}
		case 5:
			{
				int i;
				struct fv fv[2048];

				do_usr_wait = 1;
				/* get channel && function */
				fn = ch = 1;
				if (get_channel(&ch) || get_function(&fn))
					break;

				/* read function */
				rc = cvorb_func_read(cvorbh, ch, fn, fv,
						     sizeof(fv));
				if (rc < 0) {
					printf("ERROR! Read Channel Function"
					       " from device SRAM failed"
					       " [%s]\n",
					       cvorb_perr(rc));
					break;
				} else {
					printf("Channel function read "
					       "Number of vectors is %d\n", rc);
					for (i = 0; i < rc; i++)
						printf("%u %hu\n",
						       fv[i].t, fv[i].v);
					break;
				}
			}
		case 6: /* play already loaded function */
			break;
		case 7: /* load && play function */
			{
				struct fv *fv; /* function vector table */
				int noe; /* number of entries */

				do_usr_wait = 1;
				/* get channel, function &&
				   open vector table file */
				fn = ch = 1;
				if (get_channel(&ch) || get_function(&fn) ||
				    open_vtf(&f))
					break;

				/* get Vector Table Functions from the file */
				noe = load_vtf(f, &fv);
				if (noe < 0) {
					printf("ERROR! Can't load VTF\n");
					break;
				}

				/* load function vector table */
				rc = cvorb_func_load(cvorbh, ch, fn, fv, noe);
				if (rc < 0) {
					printf("ERROR! Load Function#%d for"
					       " Channel#%d in device SRAM"
					       " failed [%s]\n",
					       fn, ch, cvorb_perr(rc));
					break;
				}

				/* enable function in Functioin Enable Mask */
				if (cvorb_func_en(cvorbh, ch, fn)) {
					printf("Can't Enable Function#%d in"
					       " channel#%d\n", fn, ch);
					break;
				}

				/* select function if Function Selection */
				if (cvorb_func_sel(cvorbh, ch, fn)) {
					printf("Can't Select Function#%d in"
					       " channel#%d\n", fn, ch);
					break;
				}

				/* do module software start,
				   i.e. play the function */
				if (cvorb_swp(cvorbh, (ch>CHAM)?2:1, SPR_MSS))
					printf("Software Start FAILED!\n");
				else
					printf("Function#%d @channel#%d"
					       " has been played\n", fn, ch);

				break;
			}
		case 8:
			do_usr_wait = 1;
			/* get channel */
			ch = 1;
			if (get_channel(&ch))
				break;

			/* get current value */
			printf("Current selected Function is --> %d\n",
			       cvorb_func_get(cvorbh, ch));
			/* get function */
			fn = 1;
			if (get_function(&fn))
				break;

			rc = cvorb_func_sel(cvorbh, ch, fn);
			if (!rc)
				printf("New function set\n");
			else
				printf("Can't set new function! [%s]\n",
				       cvorb_perr(rc));
			break;
		case 9:	 /* enable function */
			{
				uint mask[2];
				do_usr_wait = 1;
				ch = 1;
				if (get_channel(&ch))
					break;

				cvorb_rd_fem(cvorbh, ch, mask);
				printf("Current Function Enable"
				       " Mask registers are:\n"
				       "[63-32] --> 0x%x [31-0] --> 0x%x\n\n",
				       mask[0], mask[1]);

				printf("Select Function to Enable"
				       " (0 -- all of them)\n");
				fn = 0;
				if (get_function(&fn))
					break;

				/* enable function in Functioin Enable Mask */
				if (cvorb_func_en(cvorbh, ch, fn))
					printf("Can't Enable Function#%d in"
					       " channel#%d\n", fn, ch);
				else
					printf("Function enabled\n");
				break;
			}
		case 10: /* disable function */
			{
				uint mask[2];
				do_usr_wait = 1;
				ch = 1;
				if (get_channel(&ch))
					break;

				cvorb_rd_fem(cvorbh, ch, mask);
				printf("Current Function Enable"
				       " Mask registers are:\n"
				       "[63-32] --> 0x%x [31-0] --> 0x%x\n\n",
				       mask[0], mask[1]);

				printf("Select Function to Disable"
				       " (0 -- all of them)\n");
				fn = 0;
				if (get_function(&fn))
					break;

				/* enable function in Functioin Enable Mask */
				if (cvorb_func_dis(cvorbh, ch, fn))
					printf("Can't Disable Function#%d in"
					       " channel#%d\n", fn, ch);
				else
					printf("Function disabled\n");

				break;
			}
		case 11: /* set/get Recurrent Cycles */
			{
				uint res;
				do_usr_wait = 1;

				ch = 1;
				if (get_channel(&ch))
					break;

				cvorb_rd_rcyc(cvorbh, ch, &res);
				printf("Recurrent cycles is %d\n\n", res);
				printf("Choose new recurrent cycle\n");
				if (get_value(&val, 0, (uint)-1))
					break;
				if (cvorb_wr_rcyc(cvorbh, ch, val))
					printf("Can't set new Recurrent"
					       " Cycles value\n");
				else
					printf("New Recurrent Cycles value"
					       " set\n");
				break;
			}
		case 12: /* set module config register */
			do_usr_wait = 1;
			if (get_module(&md))
				break;
			if (cvorb_rd_mconfig(cvorbh, (md==1)?md:9, &val)) {
				printf("Can't get current Module"
				       " Configuration register\n");
				break;
			}
			printf("Current Module Configuration reg --> 0x%x\n",
			       val);
			if (get_value(&val, 0, (uint)-1))
				break;
			if (cvorb_wr_mconfig(cvorbh, (md==1)?md:9, val)) {
				printf("Can't set new value in Module"
				       " Configuration register\n");
				break;
			}
			printf("Module Configuration register written\n");
			break;
		case 13: /* set chan config register */
			do_usr_wait = 1;
			ch = 1;
			if (get_channel(&ch))
				break;
			if (cvorb_rd_cconfig(cvorbh, ch, &val)) {
				printf("Can't get current Channel"
				       " Configuration register\n");
				break;
			}
			printf("Current Channel Configuration reg --> 0x%x\n",
			       val);
			if (get_value(&val, 0, (uint)-1))
				break;
			if (cvorb_wr_cconfig(cvorbh, ch, val)) {
				printf("Can't set new value in Channel"
				       " Configuration register\n");
				break;
			}
			printf("Channel Configuration register written\n");
			break;
		default:
			printf("Please enter a valid menu item.\n\n<enter>"
			       " to continue");
			getchar();
			break;
		}

		if (do_usr_wait) {
			printf("\n<enter> to continue");
			getchar();
		}
	}
}

/**
 * @brief Load Function Vector table from the *.vtf file
 *
 * @param fn -- filename.
 *              If no extention given -- .vtf will be added
 *              Exact file name will be searched otherwise
 *
 * Files are expected to be in ./vtf directory
 *
 * @return   number of elements in Function Vector Table
 */
static int load_vtf(FILE *fd, struct fv **fv)
{
	int rc, i = 0;
#define MAX_VECT 2048
	static struct fv data[MAX_VECT] = { { 0 } };

	/* get data from *.vtf file into local structure */
        while ( (rc = fscanf(fd, "%u %hu\n", &data[i].t, &data[i].v)) != EOF) {
                if (!rc) { /* skip matching failure */
                        rc = fscanf(fd, "%*[^\n]");
                        continue;
                }
                ++i;
		if (i == MAX_VECT)
			break;
        }

	fclose(fd); /* we should close it */

	*fv = data;
	return i;
}

/**
 * @brief Try to open *.vtf Vector Table file
 *
 * @param fd -- opened FILE descriptor goes here
 *
 * Returned file descriptor should be closed afterwards
 *
 * @return < 0 -- FAILED
 * @return   0 -- SUCCESS
 */
static int open_vtf(FILE **fd)
{
	char vtfn[32]; /* vector table file namea */
	char *fname;
	int rc = 0;

	printf("Vector table file (*.vtf) -> ");
	scanf("%s", vtfn);
	getchar();

	if (!index(vtfn, '.'))
		/* we should build *.vtf name */
		asprintf(&fname, "./vtf/%s.vtf", vtfn);
	else
		/* should search for exact name */
		asprintf(&fname, "./vtf/%s", vtfn);

	*fd = fopen(fname, "r");
	if (!*fd) {
		printf("ERROR! Can't open %s Vector Table File\n", fname);
		rc = -1;
	}

	free(fname);
	return rc;
}

/**
 * @brief Get channel number from user
 *
 * @param ch -- results goes here
 *
 * Test for valid channel range
 *
 * @return < 0 -- channel is out-of-range
 * @return   0 -- OK
 */
static int get_channel(int *ch)
{
	int res;
	printf("Channel[%d - 16] -> ", *ch);
	scanf("%d", &res);
	getchar();
	if (!WITHIN_RANGE(*ch, res, MAX_CHAN_AM)) {
		printf("ERROR! Channel provided is out-of-range\n");
		return -1;
	}
	*ch = res;
	return 0;
}

/**
 * @brief Get functioin number from user
 *
 * @param fn -- First allowed function number to choose. \n
 *              User-Chosen function goes here.
 *
 * Test for valid functioin range.
 *
 * @return < 0 -- function is out-of-range
 * @return   0 -- OK
 */
static int get_function(int *fn)
{
	int res;
	printf("Function[%d-64] -> ", *fn);
	scanf("%d", &res);
	getchar();
	if (!WITHIN_RANGE(*fn, res, FAM)) {
		printf("ERROR! Function provided is out-of-range\n");
		return -1;
	}
	*fn = res;
	return 0;
}

/**
 * @brief Get Module number from user
 *
 * @param m -- results goes here
 *
 * Test for valid channel range
 *
 * @return < 0 -- Module is out-of-range
 * @return   0 -- OK
 */
static int get_module(int *m)
{
	printf("Module[1, 2] -> ");
	scanf("%d", m);
	getchar();
	if (!WITHIN_RANGE(1, *m, SMAM)) {
		printf("ERROR! Module provided is out-of-range\n");
		return -1;
	}
	return 0;
}

/**
 * @brief Get value from user
 *
 * @param val -- results goes here
 * @param min -- MIN possible value
 * @param max -- MAX possible value
 *
 * Test for valid functioin range.
 *
 * @return < 0 -- function is out-of-range
 * @return   0 -- OK
 */
static int get_value(uint *val, uint min, uint max)
{
	printf("Enter new value[0x%x-0x%x] -> ", min, max);
	scanf("%u", val);
	getchar();
	if (!WITHIN_RANGE(min, *val, max)) {
		printf("ERROR! Value provided is out-of-range\n");
		return -1;
	}
	return 0;
}
