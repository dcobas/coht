#ifndef _CVORGDEV_H_
#define _CVORGDEV_H_

#include "cvorg.h"

#define CVORG_PATH_MAX	256

int cvorgdev_device_exists(int index);
int cvorgdev_get_nr_devices(void);
int cvorgdev_get_device_list(int *indexes, int elems);
int cvorgdev_get_devname(int index, char *str, size_t len);
int cvorgdev_get_attr(int index, const char *name, char *value, size_t count);
int cvorgdev_get_attr_int(int index, const char *name, int *valp);
int cvorgdev_get_attr_uint(int index, const char *name, unsigned int *valp);
int cvorgdev_get_attr_char(int index, const char *name, char *valp);
int cvorgdev_get_attr_ulong(int index, const char *name, uint64_t *valp);
int cvorgdev_get_chan_attr_int(int index, int channel, const char *name, int *valp);
int cvorgdev_get_chan_attr_uint(int index, int channel, const char *name, unsigned int *valp);
int cvorgdev_set_attr(int index, const char *name, const char *value);
int cvorgdev_set_attr_int(int index, const char *name, int value);
int cvorgdev_set_attr_uint(int index, const char *name, unsigned int value);
int cvorgdev_set_chan_attr_int(int index, int channel, const char *name, int value);
int cvorgdev_set_chan_attr_uint(int index, int channel, const char *name, unsigned int value);
int cvorgdev_set_chan_attr_bin(int index, int channel, const char *name, void *value, ssize_t count);
int cvorgdev_get_chan_attr_bin(int index, int channel, const char *name, void *value, ssize_t count);

#endif /* _CVORGDEV_H_ */
