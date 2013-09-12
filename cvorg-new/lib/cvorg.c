/**
 * @file lib/cvorg.c
 *
 * @brief CVORG library
 *
 * Copyright (c) 2012
 * @author Samuel Iglesias Gonsalvez <siglesia@cern.ch>
 * Based on the code from Emilio G. Cota for the SIS33 board.
 *
 * @section license_sec License
 * Released under the GPL v2.
 */

#include <sys/ioctl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include "libcvorg.h"
#include "cvorgdev.h"
#include "error.h"
#include "ad9516.h"
#include "libad9516.h"
#include "cvorg.h"
#include "libinternal.h"

#define LIBCVORG_VERSION	"1.0"

/**
 * @brief libcvorg version string
 */
const char *libcvorg_version = LIBCVORG_VERSION;

int __cvorg_init;
static struct cvorg_calib __cvorg_calib = {
	.devicenr_list = NULL,
	.dac_calib_list = NULL,
};
static int __cvorg_nr_dev;
static int __cvorg_fd_count = 0;

static void __cvorg_initialize(void)
{
	char *value;

	__cvorg_init = 1;
	value = getenv("LIBCVORG_LOGLEVEL");
	if (value) {
		__cvorg_loglevel = strtol(value, NULL, 0);
		LIBCVORG_DEBUG(3, "Setting loglevel to %d\n", __cvorg_loglevel);
	}
}

static void __cvorg_calib_free() {
	int idx;

	if (__cvorg_calib.devicenr_list != NULL)
		free(__cvorg_calib.devicenr_list);
	if (__cvorg_calib.dac_calib_list != NULL)
		for (idx=0; idx<__cvorg_nr_dev; ++idx)
			if (__cvorg_calib.dac_calib_list[idx] != NULL)
				free(__cvorg_calib.dac_calib_list[idx]);
		free(__cvorg_calib.dac_calib_list);
	__cvorg_calib.devicenr_list = NULL;
	__cvorg_calib.dac_calib_list = NULL;
}

static int __cvorg_calib_alloc() {

	__cvorg_calib.devicenr_list = malloc(sizeof(int)*__cvorg_nr_dev);
	if (__cvorg_calib.devicenr_list == NULL)
		goto out;

	__cvorg_calib.dac_calib_list = malloc(sizeof(uint32_t *)*__cvorg_nr_dev);
	if (__cvorg_calib.dac_calib_list == NULL)
		goto out;
	memset(__cvorg_calib.dac_calib_list, 0, sizeof(uint32_t *)*__cvorg_nr_dev);

	return 0;

out:
	if (__cvorg_calib.devicenr_list != NULL)
		free(__cvorg_calib.devicenr_list);
	if (__cvorg_calib.dac_calib_list != NULL)
		free(__cvorg_calib.dac_calib_list);
	return -1;
}

/**
 *
 */
static int __cvorg_get_pcb_id_list(uint64_t *cvorg_pcb_id_list)
{
	uint64_t val;
	int ret, dev_idx;

	for (dev_idx = 0; dev_idx < __cvorg_nr_dev; ++dev_idx) {
		ret = cvorgdev_get_attr_ulong(__cvorg_calib.devicenr_list[dev_idx], "pcb_id", &val);
		if (ret < 0)
			return -1;
		cvorg_pcb_id_list[dev_idx] = val;
	}
	return 0;
}

/**
 * This function initialize a table with all the dac offset/gain correction
 * for each devices. Those values are retrieved from a unique file:
 * "/usr/local/data/cvorg/cvorgCalibration.csv"
 */
static int __cvorg_get_dac_calib(void)
{
	FILE *fp = NULL;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	uint32_t calib_val[CVORG_NR_CALIB_VALUES];
	uint64_t *cvorg_pcb_id_list = NULL;
	uint64_t pcb_id;
	char *ptr_line;
	int ret=0, idx, i, nitems;

	if ((__cvorg_nr_dev = cvorgdev_get_nr_devices()) <  0)
		return -1;

	/* allocate cvorg calib struct */
	if (__cvorg_calib_alloc())
		return -1;

	if ((ret = cvorgdev_get_device_list(__cvorg_calib.devicenr_list, __cvorg_nr_dev)) < 0) {
		ret = -1;
		goto out;
	}

	/* allocate an array to store teh pcb-id of the devices */
	/* pcb_id is used as a primary keys to retrieve dac correction values */
	cvorg_pcb_id_list = malloc(sizeof(uint64_t)*__cvorg_nr_dev);
	if (cvorg_pcb_id_list == NULL) {
		ret = -1;
		goto out;
	}

	/* get the devices pcb_id */
	if ((ret = __cvorg_get_pcb_id_list(cvorg_pcb_id_list)) < 0)
		/* something went wrong */
		goto out;

	/* Open the dac correction values file */
	fp = fopen(CVORG_CALIB_FILE, "r");
	if (fp == NULL) {
		ret = -1; /* set returned error code */
		goto out;
	}

	while ((read = getline(&line, &len, fp)) != -1) {
		if (read == -1) {
			ret = -1;
			goto out;
		}
		if (line[0] != '0')
			continue; /* skip header lines */
		/* line starting by '0': 0xa500... */
		/* line format: column1(pcb_id) column[2..19](correction values) */
		ptr_line = line;
		nitems = sscanf(line, " %llx ", &pcb_id); /* get pcb_id */
		/* pcb_id is used as a key to retrieve the corresponding dac correction values */
		for (idx=0; idx<__cvorg_nr_dev; ++idx) {
			if (cvorg_pcb_id_list[idx] == pcb_id) { /* calib line of interest */
				for (i = 0; i < CVORG_NR_CALIB_VALUES; ++i) { /* get the calib values */
					ptr_line = strstr(ptr_line, " 0"); /* move the pointer to next item */
					if (!ptr_line) /* incomplete set of values */
						break;
					nitems += sscanf(ptr_line, " %x ", &calib_val[i]);
					++ptr_line; /* increment by 1 char to not get the same item in the next loop */
				}
				if (nitems != CVORG_NR_CALIB_VALUES+1) { /* number of items matched(pcb_id + nr calib) */
					ret = -1;
					goto out;
				}
				/* allocate the dac_calib buffer */
				__cvorg_calib.dac_calib_list[idx] = malloc(sizeof(uint32_t)*CVORG_NR_CALIB_VALUES);
				if (__cvorg_calib.dac_calib_list[idx] == NULL) {
					ret = -1;
					goto out;
				}
				/* set the cvorg calib corresponding to this device */
				memcpy(__cvorg_calib.dac_calib_list[idx], calib_val, sizeof(uint32_t)*CVORG_NR_CALIB_VALUES);
			}
		}
	}
	ret = 0; /* everything went fine */

out:
	if (ret)
		__cvorg_calib_free();
	if (cvorg_pcb_id_list != NULL)
		free(cvorg_pcb_id_list);
	if (line != NULL)
		free(line); /* buffer allocated by getline() */
	if (fp != NULL)
		fclose(fp);
	return ret;
}

/**
 * @brief Open an CVORG device handle
 * @param index		- Number of the device to open from 0 to n-1.
 * @param channelnr	- Channel number (enum)
 *
 * @return Pointer to the opened file handle on success, NULL on failure
 */
cvorg_t *cvorg_open(int index, enum cvorg_channelnr channelnr)
{
	cvorg_t *dev;
	char filename[CVORG_PATH_MAX];
	char devname[32];
	int idx;

	if (!__cvorg_init)
		__cvorg_initialize();
	LIBCVORG_DEBUG(4, "opening device %d\n", index);

	if (!cvorgdev_device_exists(index)) {
		__cvorg_libc_error(__func__);
		__cvorg_lib_error(LIBCVORG_ENODEV);
		return NULL;
	}

	dev = malloc(sizeof(*dev));
	if (dev == NULL) {
		__cvorg_libc_error(__func__);
		return NULL;
	}
	memset(dev, 0, sizeof(*dev));

	// Index and channelnr are needed to access the proper sysfs files
	dev->index = index;
	dev->channelnr = channelnr - 1;

	if (channelnr < 1 || channelnr > 2)
		goto out_free;

        if (cvorgdev_get_devname(index, devname, sizeof(devname)) < 0)
                goto out_free;

        snprintf(filename, sizeof(filename), "/dev/%s", devname);
        filename[sizeof(filename) - 1] = '\0';

	dev->fd = open(filename, O_RDWR, 0);
	if (dev->fd < 0)
		goto out_free;

	LIBCVORG_DEBUG(4, "opened device %d ch %d on handle %p, fd %d\n", index, channelnr, dev, dev->fd);

	/* Retrieve dac offset/gain correction values if isn't done */
	if ( __cvorg_calib.dac_calib_list == NULL )
		__cvorg_get_dac_calib();
	if (__cvorg_calib.dac_calib_list) { /* dac calib exist */
		for (idx=0; idx < __cvorg_nr_dev; ++idx) {
			if (index == __cvorg_calib.devicenr_list[idx]) {
				/* calib for the concerned device */
				dev->dac_calib = __cvorg_calib.dac_calib_list[idx];
				break;
			}
		}
	}
	else
		dev->dac_calib = NULL;
	++__cvorg_fd_count; /* increment the number of device file descriptor */
	return dev;

out_free:
	if (dev)
		free(dev);
	__cvorg_libc_error(__func__);
	return NULL;
}

/**
 * @brief Close an CVORG handle
 * @param device	- CVORG device handle
 *
 * @return 0 on success, -1 on failure
 */
int cvorg_close(cvorg_t *device)
{
	int ret;

	LIBCVORG_DEBUG(4, "handle %p\n", device);
	ret = close(device->fd);
	if (ret < 0)
		__cvorg_libc_error(__func__);

	free(device);
	--__cvorg_fd_count; /* decrement the number of file descriptor */
	if (__cvorg_fd_count == 0)  /* time to free global buffer */
		__cvorg_calib_free();
	return ret;
}


int cvorg_lock(cvorg_t *device)
{
	return 0;
}

int cvorg_unlock(cvorg_t *device)
{
	return 0;
}


/**
 * @brief Set the clock source of the device
 * @param device	- CVORG device handle
 * @param freq		- desired frequency
 *
 * @return 0 on success, -1 on failure
 */
int cvorg_set_sampfreq(cvorg_t *device, unsigned int freq)
{
        struct ad9516_pll pll;
        int ret;

        LIBCVORG_DEBUG(4, "fd %d freq %d\n", device->fd, freq);
        if (freq > CVORG_MAX_FREQ) {
                LIBCVORG_DEBUG(2, "Invalid frequency (%u Hz). Max: %d Hz\n",
                        freq, CVORG_MAX_FREQ);
                __cvorg_errno = LIBCVORG_EINVAL;
                return -1;
        }

        if (ad9516_fill_pll_conf(freq, &pll)) {
                __cvorg_internal_error(LIBCVORG_EINVAL);
                return -1;
        }

       	ret = cvorgdev_set_chan_attr_bin(device->index, device->channelnr, "pll", &pll, sizeof(pll));
	if (ret < 0) {
		__cvorg_libc_error(__func__);
		return -1;
	}

	return 0;
}

int cvorg_set_sampfreq_ext_clk(cvorg_t *device, unsigned int freq, unsigned int ext_clk_freq)
{
        struct ad9516_pll pll;
        int ret;

        LIBCVORG_DEBUG(4, "fd %d freq %d\n", device->fd, freq);
        if (freq > CVORG_MAX_FREQ) {
                LIBCVORG_DEBUG(2, "Invalid frequency (%d Hz). Max: %d Hz\n",
                        freq, CVORG_MAX_FREQ);
                __cvorg_errno = LIBCVORG_EINVAL;
                return -1;
        }

        if (ad9516_fill_pll_conf_ext_clk(freq, &pll, ext_clk_freq)) {
                __cvorg_internal_error(LIBCVORG_EINVAL);
                return -1;
        }

       	ret = cvorgdev_set_chan_attr_bin(device->index, device->channelnr, "pll", &pll, sizeof(pll));
	if (ret < 0) {
		__cvorg_libc_error(__func__);
		return -1;
	}

	return 0;
}


/**
 * @brief Get the clock source of the device
 * @param device	- CVORG device handle
 * @param clksrc	- retrieved clock source
 *
 * @return 0 on success, -1 on failure
 */
int cvorg_get_sampfreq(cvorg_t *device, unsigned int *freq)
{
	unsigned int val;
	int ret;

	LIBCVORG_DEBUG(4, "handle %p\n", device);
	ret = cvorgdev_get_chan_attr_uint(device->index, device->channelnr, "sampling_frequency", &val);
	if (ret < 0) {
		__cvorg_libc_error(__func__);
		return -1;
	}
	*freq = val;
	return 0;
}

int cvorg_channel_set_inpol(cvorg_t *device, enum cvorg_inpol inpol)
{
	int ret;

	LIBCVORG_DEBUG(4, "handle %p\n", device);
	ret = cvorgdev_set_chan_attr_uint(device->index, device->channelnr, "input_polarity", inpol);
	if (ret < 0) {
		__cvorg_libc_error(__func__);
		return -1;
	}
	return 0;
}

int cvorg_channel_get_inpol(cvorg_t *device, enum cvorg_inpol *inpol)
{
	unsigned int val;
	int ret;

	LIBCVORG_DEBUG(4, "handle %p\n", device);
	ret = cvorgdev_get_chan_attr_uint(device->index, device->channelnr, "input_polarity", &val);
	if (ret < 0) {
		__cvorg_libc_error(__func__);
		return -1;
	}
	*inpol = val;
	return 0;
}

int cvorg_channel_get_status(cvorg_t *device, unsigned int *status)
{
	unsigned int val;
	int ret;

	LIBCVORG_DEBUG(4, "handle %p\n", device);

	ret = cvorgdev_get_chan_attr_uint(device->index, device->channelnr, "status", &val);
	if (ret < 0) {
		__cvorg_libc_error(__func__);
		return -1;
	}
	*status = val;
	return 0;
}

int cvorg_channel_set_trigger(cvorg_t *device, enum cvorg_trigger trigger)
{
	int ret;

	LIBCVORG_DEBUG(4, "handle %p\n", device);
	ret = cvorgdev_set_chan_attr_uint(device->index, device->channelnr, "trigger", trigger);
	if (ret < 0) {
		__cvorg_libc_error(__func__);
		return -1;
	}
	return 0;
}

int cvorg_channel_set_sequence(cvorg_t *device, struct cvorg_seq *sequence)
{
	int ret;

	LIBCVORG_DEBUG(4, "handle %p\n", device);

       	ret = cvorgdev_set_chan_attr_bin(device->index, device->channelnr, "set_sequence", sequence, sizeof(struct cvorg_seq));
	if (ret < 0) {
		__cvorg_libc_error(__func__);
		return -1;
	}
	return 0;
}

int cvorg_channel_set_waveform(cvorg_t *device, struct cvorg_wv *waveform)
{
        struct cvorg_seq seq;

        LIBCVORG_DEBUG(4, "fd %d recurr %d size %d dynamic_gain %d gain_val %d "
                "form: %p\n", device->fd, waveform->recurr, waveform->size,
                waveform->dynamic_gain, waveform->gain_val, waveform->form);
        memset(&seq, 0, sizeof(seq));

        seq.nr          = 1;
        seq.n_waves     = 1;
        seq.waves       = waveform;

        return cvorg_channel_set_sequence(device, &seq);
}


char *cvorg_get_hw_version(cvorg_t *device)
{
	char *hw_version;
	int ret;

	LIBCVORG_DEBUG(4, "handle %p\n", device);

	hw_version = calloc(sizeof(char), CVORG_HW_VER_MAX_CHAR);
	ret = cvorgdev_get_attr_char(device->index, "hw_version", hw_version);
	if (ret < 0) {
		__cvorg_libc_error(__func__);
		free(hw_version);
		return NULL;
	}

	return hw_version;
}

int cvorg_get_pcb_id(cvorg_t *device, uint64_t *pcb_id)
{
	uint64_t val;
	int ret;

	LIBCVORG_DEBUG(4, "handle %p\n", device);
	ret = cvorgdev_get_attr_ulong(device->index, "pcb_id", &val);
	if (ret < 0) {
		__cvorg_libc_error(__func__);
		return -1;
	}
	*pcb_id = val;
	return 0;
}

int cvorg_channel_disable_output(cvorg_t *device)
{
	int ret;

	LIBCVORG_DEBUG(4, "handle %p\n", device);
	ret = cvorgdev_set_chan_attr_uint(device->index, device->channelnr, "enable_output", 0);
	if (ret < 0) {
		__cvorg_libc_error(__func__);
		return -1;
	}
	return 0;
}

int cvorg_channel_enable_output(cvorg_t *device)
{
	int ret;

	LIBCVORG_DEBUG(4, "handle %p\n", device);
	ret = cvorgdev_set_chan_attr_uint(device->index, device->channelnr, "enable_output", 1);
	if (ret < 0) {
		__cvorg_libc_error(__func__);
		return -1;
	}
	return 0;
}


int cvorg_dac_get_conf(cvorg_t *device, struct cvorg_dac *conf)
{
	uint32_t val;
	int ret;

	LIBCVORG_DEBUG(4, "handle %p\n", device);

	ret = cvorgdev_set_chan_attr_uint(device->index, device->channelnr, "test_mode", CVORG_MODE_DAC);
	if (ret < 0) {
		__cvorg_libc_error(__func__);
		return -1;
	}

	ret = cvorgdev_get_chan_attr_uint(device->index, device->channelnr, "dac_offset", &val);
	if (ret < 0) {
		__cvorg_libc_error(__func__);
		return -1;
	}
	conf->offset = val;

	ret = cvorgdev_get_chan_attr_uint(device->index, device->channelnr, "dac_gain", &val);
	if (ret < 0) {
		__cvorg_libc_error(__func__);
		return -1;
	}
	conf->gain = val;

	ret = cvorgdev_set_chan_attr_uint(device->index, device->channelnr, "test_mode", CVORG_MODE_OFF);
	if (ret < 0) {
		__cvorg_libc_error(__func__);
		return -1;
	}

	return 0;
}

int cvorg_dac_set_conf(cvorg_t *device, struct cvorg_dac conf)
{
	int ret;
	unsigned int status;

	LIBCVORG_DEBUG(4, "fd %d\n", device->fd);

        /* Check if the current channel is busy or not */
        ret = cvorg_channel_get_status(device, &status);
        if(ret < 0)
                return ret;

        if (status & CVORG_CHANSTAT_BUSY ||
                status & CVORG_CHANSTAT_SRAM_BUSY) {
                fprintf(stderr, "current channel is busy. Cannot set DAC configuration.\n");
                return -1;
        }
#if 0
	ret = cvorgdev_set_chan_attr_uint(device->index, device->channelnr, "test_mode", CVORG_MODE_DAC);
	if (ret < 0) {
		__cvorg_libc_error(__func__);
		return -1;
	}
#endif
	ret = cvorgdev_set_chan_attr_uint(device->index, device->channelnr, "dac_gain", conf.gain);
	if (ret < 0) {
		__cvorg_libc_error(__func__);
		return -1;
	}

	ret = cvorgdev_set_chan_attr_uint(device->index, device->channelnr, "dac_offset", conf.offset);
	if (ret < 0) {
		__cvorg_libc_error(__func__);
		return -1;
	}
#if 0
	ret = cvorgdev_set_chan_attr_uint(device->index, device->channelnr, "test_mode", CVORG_MODE_OFF);
	if (ret < 0) {
		__cvorg_libc_error(__func__);
		return -1;
	}
#endif
	return 0;
}

int cvorg_channel_get_outgain(cvorg_t *device, int32_t *outgain)
{
	unsigned int val;
	int ret;

	LIBCVORG_DEBUG(4, "handle %p\n", device);
	ret = cvorgdev_get_chan_attr_uint(device->index, device->channelnr, "gain", &val);
	if (ret < 0) {
		__cvorg_libc_error(__func__);
		return -1;
	}
	*outgain = val;
	return 0;
}

int cvorg_channel_get_outoff(cvorg_t *device, enum cvorg_outoff *outoff)
{
	unsigned int val;
	int ret;

	LIBCVORG_DEBUG(4, "handle %p\n", device);
	ret = cvorgdev_get_chan_attr_uint(device->index, device->channelnr, "offset", &val);
	if (ret < 0) {
		__cvorg_libc_error(__func__);
		return -1;
	}
	*outoff = val;
	return 0;
}
/* calib table is 2D array: [channel][gain] */
/* 2 channels */
/* 9 valid offset/gain combination: 0/-22db 0/-16db 0/-10db 0/-4db 0/-2db 0/8db 0/14db 0/20db 2.5/14db */
/* for offset 2.5V only the gain 14db is allowd and therefore there is a single dac calib value */
typedef uint32_t (* dac_calib_array)[2][9];
static int __cvorg_dac_set_calib(cvorg_t *device)
{
	int32_t outgain;
	enum cvorg_outoff outoff;
	int i, gain_idx=0;
	dac_calib_array ptr_calib;
	int32_t calib;
	struct cvorg_dac conf;

	if (cvorg_channel_get_outoff(device, &outoff) < -1)
		return -1;
	if (cvorg_channel_get_outgain(device, &outgain) < -1)
		return -1;
	/* compute offset */
	if (outoff == CVORG_OUT_OFFSET_2_5V)
		/* there is only one calib value located at the index 8 */
		gain_idx = 8;
	else {
		for (i=0; i<CVORG_NR_GAINS; ++i) {
			if (cvorg_gains[i] == outgain) {
				gain_idx = i;
				break;
			}
		}
	}
	ptr_calib = (dac_calib_array)device->dac_calib;
	calib = (*ptr_calib)[device->channelnr][gain_idx];
	conf.gain = calib & 0xFFFF;
	conf.offset= calib >> 16;
	return cvorg_dac_set_conf(device, conf);
}

int cvorg_channel_set_outoff(cvorg_t *device, enum cvorg_outoff outoff)
{
	int ret;

	LIBCVORG_DEBUG(4, "handle %p\n", device);
	ret = cvorgdev_set_chan_attr_uint(device->index, device->channelnr, "offset", outoff);
	if (ret < 0) {
		__cvorg_libc_error(__func__);
		return -1;
	}
	if (device->dac_calib == NULL) {
		/* for some reason dac calib values doesn't exist for this device: set cvorg_errno properly */
                __cvorg_errno = LIBCVORG_NOCALIB;
		return -1; /* just to warn the caller that the dac is not calibrate */
	}
	return __cvorg_dac_set_calib(device);
}


int cvorg_channel_set_outgain(cvorg_t *device, int32_t *outgain)
{
	int ret;

	LIBCVORG_DEBUG(4, "handle %p\n", device);
	ret = cvorgdev_set_chan_attr_uint(device->index, device->channelnr, "gain", (unsigned int)*outgain);
	if (ret < 0) {
		__cvorg_libc_error(__func__);
		return -1;
	}

	ret = cvorg_channel_get_outgain(device, outgain);
	if (ret < 0) {
		__cvorg_libc_error(__func__);
		return -1;
	}
	if (device->dac_calib == NULL) {
		/* for some reason dac calib values doesn't exist for this device: set cvorg_errno properly */
                __cvorg_errno = LIBCVORG_NOCALIB;
		return -1; /* just to warn the caller that the dac is not calibrate */
	}
	return __cvorg_dac_set_calib(device);
}
