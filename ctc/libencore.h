#ifndef _LIBENCORE_H
#define _LIBENCORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmeio.h"

typedef struct {
	int			fd;
	int			dmafd;
	int			nregs;
	struct encore_reginfo	*reginfo;
	struct vme_mapping	mapinfo[2];
	int			unused1;
	int			unused2;
	int			unused3;
	int			unused4;
} *encore_handle;

encore_handle encore_open(char *devname, int lun);
int encore_close(encore_handle h);
int encore_set_timeout(encore_handle h, int timeout);
int encore_get_timeout(encore_handle h, int *timeout);
int encore_wait(encore_handle h);

int encore_reg_id(encore_handle h, char *regname);
int encore_get_register(encore_handle h, int reg_id, unsigned int *value);
int encore_set_register(encore_handle h, int reg_id, unsigned int value);
int encore_get_window(encore_handle h, int reg_id, int from, int to,
					void *buffer);
int encore_set_window(encore_handle h, int reg_id, int from, int to,
					void *buffer);
int encore_raw_read(encore_handle h, int map,
	unsigned offset, int items, int data_width, void *dst);
int encore_raw_write(encore_handle h, int map,
	unsigned offset, int items, int data_width, void *src);

int encore_dma_read(encore_handle h, unsigned long address,
	unsigned am, unsigned data_width, unsigned long size,
	void *dst);
int encore_dma_write(encore_handle h, unsigned long address,
	unsigned am, unsigned data_width, unsigned long size,
	void *src);
int encore_dma_get_register(encore_handle h, int reg_id, unsigned int *value);
int encore_dma_set_register(encore_handle h, int reg_id, unsigned int value);
int encore_dma_get_window(encore_handle h, int reg_id, int from, int to,
					void *dst);
int encore_dma_set_window(encore_handle h, int reg_id, int from, int to,
					void *src);

#ifdef __cplusplus
}
#endif
#endif /* _LIBENCORE_H */
