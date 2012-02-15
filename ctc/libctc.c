/*-
 * Copyright (c) 2012 Samuel I. Gonsalvez <siglesia@cern.ch>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of copyright holders nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "vmeio.h"
#include "ctc_regs.h"
#include "libctc.h"

#define MAX_DEVNAME	256

int ctc_open(int lun)
{
	char devname[MAX_DEVNAME];
	static char devnamefmt[] = "/dev/ctc.%d";
	int cc;

	cc = snprintf(devname, MAX_DEVNAME, devnamefmt, lun);
	if (cc < 0 || cc >= MAX_DEVNAME)
		return -EINVAL;
	return open(devname, O_RDWR);
}

int ctc_close(int fd)
{
	return close(fd);
}

enum encore_direction {
	ENCORE_READ = 0,
	ENCORE_WRITE = 1,
};

static int ctc_raw(int fd, int mapping,
	unsigned offset, unsigned items, unsigned data_width,
	void *buffer, enum encore_direction write)
{
	struct vmeio_riob riob, *riobp = &riob;

	riobp->mapnum = mapping;
	riobp->offset = offset;
	riobp->wsize  = items;
	riobp->buffer = buffer;
	riobp->data_width = data_width;
	
	if (write)
		return ioctl(fd, VMEIO_RAW_WRITE, riobp);
	else
		return ioctl(fd, VMEIO_RAW_READ, riobp);
}

/*********************************************************************
 *********************************************************************
 *
 * 	Exported User-space CTC library 
 *	
 *	Author: Samuel Iglesias Gonsalvez <siglesia@cern.ch>
 *	Date: 15/02/2012
 *
 *********************************************************************
 ********************************************************************/

/* 
 * Use of the ctc_open and ctc_close functions already defined at the beginning.
 */

int ctc_get_hw_version(int fd, int *hw_version)
{
	long val;
	int ret;
	int offset = 0;

	ret = ctc_raw(fd, 1, offset, 1, 32, &val, ENCORE_READ); 
	if(ret)
		return ret;

	*hw_version = val;
	return 0;
}

int ctc_chan_get_status(int fd, int chan, int *status)
{
	long val;
	int ret;
	int offset = 0x4;

	ret = ctc_raw(fd, 1, offset, 1, 32, &val, ENCORE_READ); 
	if (ret) {
		return ret;
	}

	*status = (val >> (8 - chan + 1)) & 0x1;
	return 0;
}

int ctc_chan_enable(int fd, int chan)
{
	long val;
	int ret;
	int offset = 0x4;

	if(chan < 1 || chan > 8) {
		errno = EINVAL;
		return -1;
	}

	ret = ctc_raw(fd, 1, offset, 1, 32, &val, ENCORE_READ); 
	if (ret) {
		return ret;
	}

	val |= (1 << (8 - chan + 1));
	ret = ctc_raw(fd, 1, offset, 1, 32, &val, ENCORE_WRITE); 
	if (ret)
		return ret;

	return 0;
}

int ctc_chan_disable(int fd, int chan)
{
	long val;
	int ret;
	int offset = 0x4;

	if(chan < 1 || chan > 8) {
		errno = EINVAL;
		return -1;
	}

	ret = ctc_raw(fd, 1, offset, 1, 32, &val, ENCORE_READ); 
	if (ret) {
		errno = EFAULT;
		return -1;
	}

	val &= ~(1 << (8 - chan + 1));

	ret = ctc_raw(fd, 1, offset, 1, 32, &val, ENCORE_WRITE); 
	if (ret)
		return ret;

	return 0;
}

int ctc_chan_set_ext_start(int fd, int chan, int ext_start)
{

	unsigned int offset = 0x08 + 0x18*(chan - 1);
	long val;
	int ret;

	if(chan < 1 || chan > 8 || ext_start < 1 || ext_start > 40) {
		errno = EINVAL;
		return -1;
	}
	
	ret = ctc_raw(fd, 1, offset, 1, 32, &val, ENCORE_READ); 
	if (ret)
		return ret;

	/* Convert the value to [0-39] needed to write into the register */
	ext_start--;
	/* Clear the external start */
	val &= 0x00ffffff;
	/* Setup the external start */
	val |= ext_start << 24;

	return ctc_raw(fd, 1, offset, 1, 32, &val, ENCORE_WRITE); 
}

int ctc_chan_get_ext_start(int fd, int chan, int *ext_start)
{

	unsigned int offset = 0x08 + 0x18*(chan - 1);
	long val;
	int ret;

	if(chan < 1 || chan > 8) {
		errno = EINVAL;
		return -1;
	}
	
	/* Read the value from the register */
	ret = ctc_raw(fd, 1, offset, 1, 32, &val, ENCORE_READ); 
	if (ret)
		return ret;

	/* Clear the external start */
	*ext_start = (val >> 24) & 0x00ff;
	*ext_start = *ext_start + 1;
	return 0;
}

int ctc_chan_set_clk_counter1(int fd, int chan, int clk)
{

	unsigned int offset = 0x08 + 0x18*(chan - 1);
	unsigned long val;
	int ret;

	if(chan < 1 || chan > 8 || clk < 1 || clk > 6) {
		errno = EINVAL;
		return -1;
	}
	
	ret = ctc_raw(fd, 1, offset, 1, 32, &val, ENCORE_READ); 
	if (ret)
		return ret;

	/* Convert the value to [0-5] */
	clk--;
	clk = clk & 0x0f;

	/* Clear the clk for counter1 */
	val &= 0xff0fffff;
	/* Setup the clk for counter1 */
	val |= clk << 20;

	return ctc_raw(fd, 1, offset, 1, 32, &val, ENCORE_WRITE); 
}

int ctc_chan_get_clk_counter1(int fd, int chan, int *clk)
{

	unsigned int offset = 0x08 + 0x18*(chan - 1);
	long val;
	int ret;

	if(chan < 1 || chan > 8) {
		errno = EINVAL;
		return -1;
	}
	
	/* Read the value from the register */
	ret = ctc_raw(fd, 1, offset, 1, 32, &val, ENCORE_READ); 
	if (ret)
		return ret;

	*clk = (val >> 20) & 0x0f;
	*clk = *clk + 1;
	return 0;
}

int ctc_chan_set_clk_counter2(int fd, int chan, int clk)
{

	unsigned int offset = 0x08 + 0x18*(chan - 1);
	long val;
	int ret;

	if(chan < 1 || chan > 8 || clk < 1 || clk > 6) {
		errno = EINVAL;
		return -1;
	}
	
	ret = ctc_raw(fd, 1, offset, 1, 32, &val, ENCORE_READ); 
	if (ret)
		return ret;

	clk--;
	clk = clk & 0x0f;
	/* Clear the clk for counter1 */
	val &= 0xfff0ffff;
	/* Setup the clk for counter1 */
	val |= clk << 16;

	return ctc_raw(fd, 1, offset, 1, 32, &val, ENCORE_WRITE); 
}

int ctc_chan_get_clk_counter2(int fd, int chan, int *clk)
{

	unsigned int offset = 0x08 + 0x18*(chan - 1);
	long val;
	int ret;

	if(chan < 1 || chan > 8) {
		errno = EINVAL;
		return -1;
	}
	
	/* Read the value from the register */
	ret = ctc_raw(fd, 1, offset, 1, 32, &val, ENCORE_READ); 
	if (ret)
		return ret;

	*clk = (val >> 16) & 0x0f;
	*clk = *clk + 1;
	return 0;
}

int ctc_chan_set_mode(int fd, int chan, int mode, int direction)
{

	unsigned int offset = 0x08 + 0x18*(chan - 1);
	long val;
	int ret;

	if(chan < 1 || chan > 8 || mode < NORMAL_MODE || mode > UPDOWN_MODE 
				|| direction < FALLING_EDGE_COUNTER2 || direction > FALLING_EDGE_COUNTER1) {
		errno = EINVAL;
		return -1;
	}
	
	ret = ctc_raw(fd, 1, offset, 1, 32, &val, ENCORE_READ); 
	if (ret)
		return ret;

	/* Clear the clk for counter1 */
	val &= 0xfffffff0;
	/* Setup the clk for counter1 */
	val |= (direction << 1) | mode;

	return ctc_raw(fd, 1, offset, 1, 32, &val, ENCORE_WRITE); 
}

int ctc_chan_get_mode(int fd, int chan, int *mode, int *direction)
{

	unsigned int offset = 0x08 + 0x18*(chan - 1);
	long val;
	int ret;

	if(chan < 1 || chan > 8) {
		errno = EINVAL;
		return -1;
	}
	
	/* Read the value from the register */
	ret = ctc_raw(fd, 1, offset, 1, 32, &val, ENCORE_READ); 
	if (ret)
		return ret;

	*mode = val & 0x01;
	*direction = (val & 0x02) >> 1;
	return 0;
}

int ctc_chan_set_delay_counter1(int fd, int chan, int delay)
{

	unsigned int offset = 0x0C + 0x18*(chan - 1);

	if(chan < 1 || chan > 8) {
		errno = EINVAL;
		return -1;
	}
	
	return ctc_raw(fd, 1, offset, 1, 32, &delay, ENCORE_WRITE); 
}

int ctc_chan_get_delay_counter1(int fd, int chan, int *delay)
{

	unsigned int offset = 0x0C + 0x18*(chan - 1);
	long val;
	int ret;

	if(chan < 1 || chan > 8) {
		errno = EINVAL;
		return -1;
	}
	
	/* Read the value from the register */
	ret = ctc_raw(fd, 1, offset, 1, 32, &val, ENCORE_READ); 
	if (ret)
		return ret;

	*delay = val;
	return 0;
}

int ctc_chan_set_delay_counter2(int fd, int chan, int delay)
{

	unsigned int offset = 0x10 + 0x18*(chan - 1);

	if(chan < 1 || chan > 8) {
		errno = EINVAL;
		return -1;
	}
	
	return ctc_raw(fd, 1, offset, 1, 32, &delay, ENCORE_WRITE); 
}

int ctc_chan_get_delay_counter2(int fd, int chan, int *delay)
{

	unsigned int offset = 0x10 + 0x18*(chan - 1);
	long val;
	int ret;

	if(chan < 1 || chan > 8) {
		errno = EINVAL;
		return -1;
	}
	
	/* Read the value from the register */
	ret = ctc_raw(fd, 1, offset, 1, 32, &val, ENCORE_READ); 
	if (ret)
		return ret;

	*delay = val;
	return 0;
}

int ctc_chan_get_output_counter(int fd, int chan, int *value)
{

	unsigned int offset = 0x14 + 0x18*(chan - 1);
	long val;
	int ret;

	if(chan < 1 || chan > 8) {
		errno = EINVAL;
		return -1;
	}
	
	/* Read the value from the register */
	ret = ctc_raw(fd, 1, offset, 1, 32, &val, ENCORE_READ); 
	if (ret)
		return ret;

	*value = val;
	return 0;
}

int ctc_chan_get_cur_val_counter1(int fd, int chan, int *value)
{

	unsigned int offset = 0x18 + 0x18*(chan - 1);
	long val;
	int ret;

	if(chan < 1 || chan > 8) {
		errno = EINVAL;
		return -1;
	}
	
	/* Read the value from the register */
	ret = ctc_raw(fd, 1, offset, 1, 32, &val, ENCORE_READ); 
	if (ret)
		return ret;

	*value = val;
	return 0;
}

int ctc_chan_get_cur_val_counter2(int fd, int chan, int *value)
{

	unsigned int offset = 0x1C + 0x18*(chan - 1);
	long val;
	int ret;

	if(chan < 1 || chan > 8) {
		errno = EINVAL;
		return -1;
	}
	
	/* Read the value from the register */
	ret = ctc_raw(fd, 1, offset, 1, 32, &val, ENCORE_READ); 
	if (ret)
		return ret;

	*value = val;
	return 0;
}

void ctc_reset(int fd)
{
	int chan;

	// Disable all channels and all setups.
	for(chan = 1; chan <= 8; chan++) {
		ctc_chan_disable(fd, chan);
		ctc_chan_set_ext_start(fd, chan, 0);
		ctc_chan_set_clk_counter1(fd, chan, 0);
		ctc_chan_set_clk_counter2(fd, chan, 0);
		ctc_chan_set_delay_counter1(fd, chan, 0);
		ctc_chan_set_delay_counter2(fd, chan, 0);
		ctc_chan_set_mode(fd, chan, 0, 0);
	}
	
}


