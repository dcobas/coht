
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include "cvora.h"
#include "libcvora.h"

static char __attribute__((unused)) lib_version[] =
				__FILE__ " version: " GIT_VERSION;

int cvora_init(int lun)
{
	char fname[256];
	int fnum;

	sprintf(fname, "/dev/cvora.%d", lun);
	if ((fnum = open(fname, O_RDWR, 0)) < 0)
		fprintf(stderr, "Error:cvora_open:"
			"Can't open:%s for read/write\n", fname);
	return fnum;
}

int cvora_close(int fd)
{
	return close(fd);
}

static int read_reg(int fd, unsigned offset, unsigned *value)
{
	struct vmeio_riob_s cb;

	cb.winum = 1;
	cb.offset = offset;
	cb.bsize = sizeof(unsigned);
	cb.buffer = value;

	return ioctl(fd, VMEIO_RAW_READ, &cb);
}


static int write_reg(int fd, unsigned offset, unsigned value)
{
	struct vmeio_riob_s cb;

	cb.winum = 1;
	cb.offset = offset;
	cb.bsize = sizeof(unsigned);
	cb.buffer = &value;

	return ioctl(fd, VMEIO_RAW_WRITE, &cb);
}

static int set_reg_bit(int fd, unsigned offset, unsigned bit,
		      int value)
{
	unsigned int contents;
	int cc;

	if ((cc = read_reg(fd, offset, &contents)) != 0)
		return cc;
	contents &= ~(1 << bit);
	contents |= (value & 1) << bit;
	return write_reg(fd, offset, contents);
}

static int get_reg_bit(int fd, unsigned offset, unsigned bit,
		      int *value)
{
	unsigned int contents;
	int cc;

	if ((cc = read_reg(fd, offset, &contents)) != 0)
		return cc;
	contents &= ~(1 << bit);
	contents >>= bit;
	*value = contents;
	return 0;
}

int cvora_get_version(int fd, int *version)
{
	unsigned int ver;
	int cc;

	if ((cc = read_reg(fd, CVORA_CONTROL, &ver)) != 0)
		return cc;
	ver >>= CVORA_VERSION_BIT;
	*version = ver;
	return 0;
}

int cvora_set_pulse_polarity(int fd, int polarity)
{
	return set_reg_bit(fd, CVORA_CONTROL, CVORA_POLARITY_BIT, polarity);
}

int cvora_get_pulse_polarity(int fd, int *polarity)
{
	return get_reg_bit(fd, CVORA_CONTROL, CVORA_POLARITY_BIT, polarity);
}

int cvora_enable_module(int fd)
{
	return set_reg_bit(fd, CVORA_CONTROL, CVORA_MODULE_ENABLE_BIT, 1);
}

int cvora_disable_module(int fd)
{
	return set_reg_bit(fd, CVORA_CONTROL, CVORA_MODULE_ENABLE_BIT, 0);
}

int cvora_enable_interrupts(int fd)
{
	return set_reg_bit(fd, CVORA_CONTROL, CVORA_INT_ENABLE_BIT, 1);
}

int cvora_disable_interrupts(int fd)
{
	return set_reg_bit(fd, CVORA_CONTROL, CVORA_INT_ENABLE_BIT, 0);
}

int cvora_get_mode(int fd, enum cvora_mode *mode)
{
	unsigned modereg;
	int cc;

	if ((cc = read_reg(fd, CVORA_MODE, &modereg)) != 0)
		return cc;
	*mode = modereg & CVORA_MODE_MASK;
	return 0;
}

int cvora_set_mode(int fd, enum cvora_mode mode)
{
	if (mode & ~CVORA_MODE_MASK)
		return -EINVAL;
	return write_reg(fd, CVORA_MODE, mode);
}

int cvora_get_hardware_status(int fd, unsigned int *status)
{
	return read_reg(fd, CVORA_CONTROL, status);
}

int cvora_set_timeout(int fd, int timeout)
{
	return ioctl(fd, VMEIO_SET_TIMEOUT, &timeout);
}

int cvora_get_timeout(int fd, int *timeout)
{
	return ioctl(fd, VMEIO_SET_TIMEOUT, timeout);
}

int cvora_wait(int fd)
{
	struct vmeio_read_buf_s event;

	return read(fd, &event, sizeof(event));
}

int cvora_get_sample_size(int fd, int *memsz)
{
	int cc;
	unsigned memp;		/* Memory pointer */

	if ((cc = read_reg(fd, CVORA_MEMORY_POINTER, &memp)) != 0)
		return cc;
	if (memp < CVORA_MEM_MIN || memp > CVORA_MEM_MAX)
		return -EINVAL;
	*memsz = memp - CVORA_MEM_MIN;
	return 0;
}

static inline swab32(uint32_t x)
{
	return (((x & 0x000000ff) << 24) |
		((x & 0x0000ff00) <<  8) |
		((x & 0x00ff0000) >>  8) |
		((x & 0xff000000) >> 24));
}

int cvora_read_samples(int fd, int maxsz, int *actsz, unsigned int *buf)
{
	int cc;
	int i;
	struct vmeio_riob_s riob;
	uint32_t *buffer = (uint32_t *)buf;

	if ((cc = cvora_get_sample_size(fd, actsz)) != 0)
		return cc;

	if (*actsz > maxsz)
		*actsz = maxsz;

	riob.winum = 1;
	riob.offset = CVORA_MEMORY;
	riob.bsize = *actsz;
	riob.buffer = buf;

	if ((cc = ioctl(fd, VMEIO_RAW_READ_DMA, &riob)) != 0)
		return cc;

	for (i = 0; i < (*actsz >> 2); i++)
		buffer[i] = swab32(buffer[i]);
	return 0;
}

int cvora_soft_start(int fd)
{
	return set_reg_bit(fd, CVORA_CONTROL, CVORA_SOFT_START_BIT, 1);
}

int cvora_soft_rearm(int fd)
{
	return set_reg_bit(fd, CVORA_CONTROL, CVORA_SOFT_REARM_BIT, 1);
}

int cvora_soft_stop(int fd)
{
	return set_reg_bit(fd, CVORA_CONTROL, CVORA_SOFT_STOP_BIT, 1);
}

int cvora_get_dac(int fd, unsigned int *dacv)
{
	return read_reg(fd, CVORA_DAC, dacv);
}

int cvora_get_clock_frequency(int fd, unsigned int *freq)
{
	return read_reg(fd, CVORA_FREQUENCY, freq);
}

int cvora_set_plot_input(int fd, unsigned int plti)
{
	return write_reg(fd, CVORA_FREQUENCY, plti);
}

int cvora_get_channels_mask(int fd, unsigned int *chans)
{
	return read_reg(fd, CVORA_CHANNEL, chans);
}

int cvora_set_channels_mask(int fd, unsigned int chans)
{
	int status;

	cvora_get_hardware_status(fd, &status);
	if (status & (1<<CVORA_MODULE_ENABLE_BIT))
		return -1;
	return write_reg(fd, CVORA_CHANNEL, chans);
}
