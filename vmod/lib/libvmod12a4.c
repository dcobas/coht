/**
   @file   libvmod12a4.c
   @brief  Library file for the vmod12a4 driver
   @author Juan David Gonzalez Cobas
   @date   October 17 2009
 */

#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "vmod12a4.h"
#include "libvmod12a4.h"

int vmod12a4_get_handle(unsigned int lun)
{
	const int filename_sz = 256;
	char devname[filename_sz];
	const char *driver_name = "vmod12a4";

	int fd;

	/* open the device file */
	snprintf(devname, filename_sz, "/dev/%s.%d", driver_name, lun);
	fd = open(devname, O_RDONLY);
	return fd;
}

int vmod12a4_convert(int fd, int channel, int datum)
{
	struct vmod12a4_output	arg, *argp = &arg;

	argp->channel = channel;
	argp->value = datum;

        return ioctl(fd, VMOD12A4_IOCPUT, &arg);
}

int vmod12a4_close(int fd)
{
	return close(fd);
}
