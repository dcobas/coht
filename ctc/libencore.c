#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <netinet/in.h>

#include "vmeio.h"
#include "libencore.h"

#define VME_DMA_DEV	"/dev/vme_dma"
#define MAX_FILENAME	256
static char devtemplate[] = "/dev/%s.%d";

static int wltodw(char *wordlength)
{
	switch (wordlength[0]) {
	case 'c':
		return 1;
	case 's':
		return 2;
	case 'l':
		return 4;
	default:
		return 0;
	}
}

static void lowercase(char *s, int n)
{
	int i;

	for (i = 0; i < n; i++) {
		int c = s[i];

		if (c == 0)
			break;
		else if (isalpha(c))
			s[i] = tolower(c);
	}
}

encore_handle encore_open(char *devname, int lun)
{
	char	tmp[MAX_FILENAME];
	int	cc;
	int	i;
	encore_handle	ret;
	struct vmeio_get_mapping arg, *argp = &arg;

	if ((ret = malloc(sizeof(*ret))) == NULL) {
		errno = ENOMEM;
		return NULL;
	}
	cc = snprintf(tmp, MAX_FILENAME, devtemplate, devname, lun);
	if (cc < 0 || cc >= MAX_FILENAME) {
		errno = EINVAL;
		goto fail;
	}
	lowercase(tmp, MAX_FILENAME);
	if ((ret->fd = open(tmp, O_RDWR)) < 0) {
		errno = EINVAL;
		goto fail;
	}

	if (ioctl(ret->fd, VMEIO_GET_NREGS, &ret->nregs) < 0) {
		errno = ENODEV;
		goto fail2;
	}

	ret->reginfo = malloc(ret->nregs*sizeof(struct encore_reginfo));
	if (ret->reginfo == NULL) {
		errno = ENOMEM;
		goto fail2;
	}
	if (ioctl(ret->fd, VMEIO_GET_REGINFO, &ret->reginfo) < 0) {
		errno = ENODEV;
		goto fail3;
	}
	for (i = 0; i < 2; i++) {
		argp->mapnum = i + 1;
		if (ioctl(ret->fd, VMEIO_GET_MAPPING, argp) < 0) {
			errno = ENODEV;
			goto fail3;
		}
		memcpy(&ret->mapinfo[i], &argp->map, sizeof(argp->map));
	}
	if ((ret->dmafd = open(VME_DMA_DEV, O_RDWR)) < 0) {
		errno = ENODEV;
		goto fail3;
	}

	return ret;

fail3:	free(ret->reginfo);
fail2:	close(ret->fd);
fail:	free(ret);
	return NULL;
}

int encore_close(encore_handle h)
{
	int ret = 0;

	if (close(h->dmafd) < 0 || close(h->fd) < 0)
		ret = -1;
	free(h->reginfo);
	free(h);
	return ret;
}

int encore_set_timeout(encore_handle h, int timeout)
{
	return ioctl(h->fd, VMEIO_SET_TIMEOUT, &timeout);
}

int encore_get_timeout(encore_handle h, int *timeout)
{
	return ioctl(h->fd, VMEIO_GET_TIMEOUT, timeout);
}

int encore_wait(encore_handle h)
{
	struct vmeio_read_buf_s rb;
	return read(h->fd, &rb, sizeof(rb));
}

int encore_reg_id(encore_handle h, char *regname)
{
	int i;

	for (i = 0; i < h->nregs; i++) {
		if (strcmp(regname, h->reginfo[i].name) == 0)
			return i;
	}
	return -1;
}

int encore_raw_read(encore_handle h, int map,
	unsigned offset, int items, int data_width, void *dst)
{
	struct vmeio_riob riob;

	riob.mapnum = map;
	riob.offset = offset;
	riob.wsize  = items;
	riob.data_width = data_width;
	riob.buffer = dst;
	return ioctl(h->fd, VMEIO_RAW_READ, &riob);
}

int encore_raw_write(encore_handle h, int map,
	unsigned offset, int items, int data_width, void *src)
{
	struct vmeio_riob riob;

	riob.mapnum = map;
	riob.offset = offset;
	riob.wsize  = items;
	riob.data_width = data_width;
	riob.buffer = src;
	return ioctl(h->fd, VMEIO_RAW_WRITE, &riob);
}

int encore_get_window(encore_handle h, int reg_id, int from, int to,
					void *dst)
{
	struct encore_reginfo *reg;
	unsigned offset;
	int bytes;

	if (reg_id < 0 || reg_id >= h->nregs)
		return -1;
	reg = &h->reginfo[reg_id];
	if ((bytes = wltodw(reg->wordsize)) == 0)
		bytes = (reg->data_width/8);
	offset = reg->offset + from * bytes;
	return encore_raw_read(h, reg->block_address_space,
		offset, to-from, 8*bytes, dst);
}

int encore_set_window(encore_handle h, int reg_id, int from, int to,
					void *src)
{
	struct encore_reginfo *reg;
	unsigned offset;
	int bytes;

	if (reg_id < 0 || reg_id >= h->nregs)
		return -1;
	reg = &h->reginfo[reg_id];
	if ((bytes = wltodw(reg->wordsize)) == 0)
		bytes = (reg->data_width/8);
	offset = reg->offset + from * bytes;
	return encore_raw_write(h, reg->block_address_space,
		offset, to-from, 8*bytes, src);
}

int encore_get_register(encore_handle h, int reg_id, unsigned int *value)
{
	return encore_get_window(h, reg_id, 0, 1, value);
}

int encore_set_register(encore_handle h, int reg_id, unsigned int value)
{
	return encore_set_window(h, reg_id, 0, 1, &value);
}

int encore_dma_read(encore_handle h, unsigned long address,
	unsigned am, unsigned data_width, unsigned long size,
	void *dst)
{
	struct vme_dma dma_desc;
	int ret;

	memset(&dma_desc, 0, sizeof(dma_desc));

	dma_desc.dir = VME_DMA_FROM_DEVICE;

	dma_desc.src.data_width = data_width;
	dma_desc.src.am = am;
	dma_desc.src.addru = 0;
	dma_desc.src.addrl = address;
	dma_desc.dst.addru = 0;
	dma_desc.dst.addrl = (unsigned long) dst;
	dma_desc.length = size;

	dma_desc.ctrl.pci_block_size = VME_DMA_BSIZE_4096;
	dma_desc.ctrl.pci_backoff_time = VME_DMA_BACKOFF_0;
	dma_desc.ctrl.vme_block_size = VME_DMA_BSIZE_4096;
	dma_desc.ctrl.vme_backoff_time = VME_DMA_BACKOFF_0;

	ret = ioctl(h->dmafd, VME_IOCTL_START_DMA, &dma_desc);

	if (data_width == 16) {
		int i;
		uint16_t *ptr = dst;

		for (i = 0; i < size/2; i++) {
			*ptr = htons(*ptr);
			ptr++;
		}
	} else if (data_width == 32) {
		int i;
		uint32_t *ptr = dst;

		for (i = 0; i < size/4; i++) {
			*ptr = htonl(*ptr);
			ptr++;
		}
	}

	return ret;
}

int encore_dma_write(encore_handle h, unsigned long address,
	unsigned am, unsigned data_width, unsigned long size,
	void *src)
{
	struct vme_dma dma_desc;
	int ret;

	memset(&dma_desc, 0, sizeof(dma_desc));

	dma_desc.dir = VME_DMA_TO_DEVICE;

	dma_desc.dst.data_width = data_width;
	dma_desc.dst.am = am;
	dma_desc.dst.addru = 0;
	dma_desc.dst.addrl = address;
	dma_desc.src.addru = 0;
	dma_desc.src.addrl = (unsigned long) src;
	dma_desc.length = size;

	dma_desc.ctrl.pci_block_size = VME_DMA_BSIZE_4096;
	dma_desc.ctrl.pci_backoff_time = VME_DMA_BACKOFF_0;
	dma_desc.ctrl.vme_block_size = VME_DMA_BSIZE_4096;
	dma_desc.ctrl.vme_backoff_time = VME_DMA_BACKOFF_0;

	if (data_width == 16) {
		int i;
		uint16_t *ptr = src;

		for (i = 0; i < size/2; i++) {
			*ptr = htons(*ptr);
			ptr++;
		}
	} else if (data_width == 32) {
		int i;
		uint32_t *ptr = src;

		for (i = 0; i < size/4; i++) {
			*ptr = htonl(*ptr);
			ptr++;
		}
	}

	ret = ioctl(h->dmafd, VME_IOCTL_START_DMA, &dma_desc);
	return ret;
}

int encore_dma_get_register(encore_handle h, int reg_id, unsigned int *value)
{
	return encore_dma_get_window(h, reg_id, 0, 1, value);
}

int encore_dma_set_register(encore_handle h,
			int reg_id, unsigned int value)
{
	return encore_dma_set_window(h, reg_id, 0, 1, &value);
}

int encore_dma_get_window(encore_handle h, int reg_id, int from, int to,
					void *dst)
{
	struct encore_reginfo *reg;
	int mapno;
	unsigned am;
	unsigned address;
	int bytespw;

	if (reg_id < 0 || reg_id >= h->nregs)
		return -1;
	reg     = &h->reginfo[reg_id];
	mapno   = reg->block_address_space - 1;
	am      = h->mapinfo[mapno].am;
	address = h->mapinfo[mapno].vme_addrl + reg->offset;
	bytespw = wltodw(reg->wordsize);
	if (bytespw == 0)
		bytespw = (reg->data_width/8);
	address += from * bytespw;
	return encore_dma_read(h, address, am, 8*bytespw, bytespw * (to-from), dst);
}

int encore_dma_set_window(encore_handle h, int reg_id, int from, int to,
					void *src)
{
	struct encore_reginfo *reg;
	int mapno;
	unsigned am;
	unsigned address;
	int bytespw;

	if (reg_id < 0 || reg_id >= h->nregs)
		return -1;
	reg     = &h->reginfo[reg_id];
	mapno   = reg->block_address_space - 1;
	am      = h->mapinfo[mapno].am;
	address = h->mapinfo[mapno].vme_addrl + reg->offset;
	bytespw = wltodw(reg->wordsize);
	if (bytespw == 0)
		bytespw = (reg->data_width/8);
	address += from * bytespw;
	return encore_dma_write(h, address, am, 8*bytespw, bytespw * (to-from), src);
}
