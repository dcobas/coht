/**
 * @file cmd_skel.c
 *
 * @brief extest's command handlers for skel-based drivers
 *
 * @author Copyright (C) 2012 CERN CO/HT Samuel Iglesias Gonsalvez
 *					<siglesia@cern.ch>
 *
 * Based on the code of cmd_skel.c done by:
 *
 * @author Copyright (C) 2009 CERN CO/HT Emilio G. Cota
 * 					<emilio.garcia.cota@cern.ch>
 * @author Copyright (C) 2008 CERN CO/HT Yury Georgievskiy
 *					<yury.georgievskiy@cern.ch>
 *
 * @section license_sec License
 * Released under the GPL v2. (and only v2, not any later version)
 */
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

/* local includes */
#include <err.h>
#include <general_usr.h>  /* for handy definitions (mperr etc..) */

#include <extest.h>
#include <cvorgdev.h>

extern int get_free_user_handle(int);
extern void print_modules();

/**
 * hndl_swdeb - handler for software debugging
 *
 * @param cmdd  - command description
 * @param atoms - command atoms list
 *
 * @return >= 0 - on success
 * @return tst_prg_err_t - on failure
 */
int hndl_swdeb(struct cmd_desc* cmdd, struct atom *atoms)
{
	return 1;
}

/**
 * hndl_getversion - handler for get driver/module version
 *
 * @param cmdd  - command description
 * @param atoms - command atoms list
 *
 * @return >= 0 - on success
 * @return tst_prg_err_t - on failure
 */
int hndl_getversion(struct cmd_desc* cmdd, struct atom *atoms)
{
	return 1;
}

/**
 * select_module - select a particular module
 *
 * @param modnr module number to select
 *
 * @return 0 - on success
 * @return (negative) error code - on failure
 */

int select_module(int modnr)
{
	cvorg_t *device;

	if (modnr == dev.index)
		return 0;

	device = cvorg_open(modnr, CVORG_CHANNEL_A);
	if (device == NULL) {
		mperr("cvorg open fails\n");
		return -TST_ERR_IOCTL;
	}
	close(dev.fd);
	dev.fd = device->fd;
	dev.index = modnr;
	dev.dac_calib = device->dac_calib;
	printf("Controlling module #%d\n", dev.index);
	return 0;
}

/**
 * hndl_module - handle for 'module' -- used to manage devices
 *
 * @param cmdd  - command description
 * @param atoms - command atoms list
 *
 * @return >= 0 - on success
 * @return tst_prg_err_t - on failure
 */
int hndl_module(struct cmd_desc *cmdd, struct atom *atoms)
{
	int sel; /* keep ret. code from select_module */
	int number;

	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("select or show modules\n"
			"%s - shows currently selected module\n"
			"%s ? - prints all managed modules by the driver\n"
			"%s x - select module x (where x ranges from 1 to n)\n"
			"%s ++ - select next module\n"
			"%s -- - select next module\n",
			cmdd->name, cmdd->name, cmdd->name, cmdd->name,
			cmdd->name);
		goto out;
	}
	number = cvorgdev_get_nr_devices();
	if(number < 0) {
		mperr("Can't get module count. %s ioctl fails\n",
			"GET_MODULE_COUNT");
		goto out;
	}

	if (!number) {
		printf("The driver does not control any modules\n");
		goto out;
	}
	++atoms;

	if (atoms->type != Numeric) {
		if (atoms->type == Operator)
			switch (atoms->oid) {
			case OprNOOP:
				//print_modules();
				break;
			case OprINC:
				sel = select_module(dev.index + 1);
				if (sel < 0)
					return sel;
				goto out;
			case OprDECR:
				sel = select_module(dev.index - 1);
				if (sel < 0)
					return sel;
				goto out;
			default:
				break;
			}
		print_modules();
		if (dev.index)
			printf("Controlling module #%d (out of %d)\n",
				dev.index, number);
		else 
		printf("The driver manages %d device%s\n",
			number, number > 1 ? "s" : "");
		goto out;
	}
	sel = select_module(atoms->val);
	if (sel < 0)
		return sel;
out:
	return 1;
}

/**
 * hndl_nextmodule - select next module handler
 *
 * @param cmdd  - command description
 * @param atoms - command atoms list
 *
 * @return >= 0 - on success
 * @return tst_prg_err_t - on failure
 */
int hndl_nextmodule(struct cmd_desc *cmdd, struct atom *atoms)
{
	int sel; /* keep ret. code from select_module */

	if (atoms == (struct atom *)VERBOSE_HELP) {
		printf("%s - select next module\n"
			"This command takes no arguments.\n", cmdd->name);
		goto out;
	}
	sel = select_module(dev.index + 1);
	if (sel < 0)
		return sel;
out:
	return 1;
}

/**
 * hndl_rawio - 'RawIO' handler
 *
 * @param cmdd  - command description
 * @param atoms - command atoms list
 *
 * @return >= 0 - on success
 * @return tst_prg_err_t - on failure
 */
int hndl_rawio(struct cmd_desc* cmdd, struct atom *atoms)
{
	return 1;
}

/**
 * hndl_maps - handler for showing the address mappings of the current device
 *
 * @param cmdd  - command description
 * @param atoms - command atoms list
 *
 * @return >= 0 - on success
 * @return tst_prg_err_t - on failure
 */
int hndl_maps(struct cmd_desc *cmdd, struct atom *atoms)
{
	return 1;
}

/**
 * hndl_timeout - 'timeout' handler
 *
 * @param cmdd  - command description
 * @param atoms - command atoms list
 *
 * @return >= 0 - on success
 * @return tst_prg_err_t - on failure
 */
int hndl_timeout(struct cmd_desc *cmdd, struct atom *atoms)
{
	return 1;
}

/**
 * hndl_queue - 'queue' handler
 *
 * @param cmdd  - command description
 * @param atoms - command atoms list
 *
 * @return >= 0 - on success
 * @return tst_prg_err_t - on failure
 */
int hndl_queue(struct cmd_desc *cmdd, struct atom *atoms)
{
	return 1;

}

/**
 * hndl_clients - 'list of clients' handler
 *
 * @param cmdd  - command description
 * @param atoms - command atoms list
 *
 * @return >= 0 - on success
 * @return tst_prg_err_t - on failure
 */
int hndl_clients(struct cmd_desc *cmdd, struct atom *atoms)
{
	return 1;
}

/**
 * hndl_connect - 'connect' handler
 *
 * @param cmdd  - command description
 * @param atoms - command atoms list
 *
 * @return >= 0 - on success
 * @return tst_prg_err_t - on failure
 */

int hndl_connect(struct cmd_desc *cmdd, struct atom *atoms)
{
	return 1;
}

/**
 * hndl_enable - 'enable' handler
 *
 * @param cmdd  - command description
 * @param atoms - command atoms list
 *
 * @return >= 0 - on success
 * @return tst_prg_err_t - on failure
 */
int hndl_enable(struct cmd_desc *cmdd, struct atom *atoms)
{
	return 1;
}

/**
 * hndl_reset - 'reset' handler
 *
 * @param cmdd  - command description
 * @param atoms - command atoms list
 *
 * @return >= 0 - on success
 * @return tst_prg_err_t - on failure
 */
int hndl_reset(struct cmd_desc* cmdd, struct atom *atoms)
{
	if (atoms == (struct atom*)VERBOSE_HELP) {
		printf("%s - reset current module\n", cmdd->name);
		goto out;
	}
	if (!do_yes_no("Reset the current module. Are you sure", NULL))
		goto out;

	if(cvorgdev_set_attr_int(dev.index, "reset", 1) < 0) {
		mperr("%s reset fails\n");
		return -TST_ERR_IOCTL;
	}

	printf("Module %d reset correctly\n", dev.index);
out:
	return 1;
}

/**
 * hndl_getstatus - 'status' handler
 *
 * @param cmdd  - command description
 * @param atoms - command atoms list
 *
 * @return >= 0 - on success
 * @return tst_prg_err_t - on failure
 */
int hndl_getstatus(struct cmd_desc* cmdd, struct atom *atoms)
{
	return 1;
}

/**
 * hndl_waitintr - connect to an interrupt via read()
 *
 * @param cmdd  - command description
 * @param atoms - command atoms list
 *
 * @return index of the next atom to parse in the current command
 * @return -TST_ERR_IOCTL - IOCTL fails
 */
int hndl_waitintr(struct cmd_desc* cmdd, struct atom *atoms)
{
	return 1;
}

/**
 * hndl_simintr - simulate an interrupt via write()
 *
 * @param cmdd  - command description
 * @param atoms - command atoms list
 *
 * @return index of the next atom to parse in the current command
 * @return -TST_ERR_IOCTL - IOCTL fails
 */
int hndl_simintr(struct cmd_desc* cmdd, struct atom *atoms)
{
	return 1;
}
