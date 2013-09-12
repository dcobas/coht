#ifndef _CVORBDEV_H_
#define _CVORBDEV_H_

#include <stdint.h>

/**
 * @brief defines the cvorb attribute's scope
 */
enum cvorb_attr_scope {
	CVORB_ATTR_BOARD,
	CVORB_ATTR_SUBMODULE,
	CVORB_ATTR_CHANNEL,
};

int cvorbdev_get_sysfs_path(int lun, char *path, int size);
int cvorbdev_get_nr_devices(void);
int cvorbdev_get_device_list(int *indexes, int elems);
int cvorbdev_device_exists(int index);
void cvorbdev_get_devname(int index, char *str, size_t len);
int cvorbdev_get_attr_int32(const char *attr_path, int32_t * valp);
int cvorbdev_get_attr_uint32(const char *attr_path, uint32_t * valp);
int cvorbdev_get_attr_char(const char *attr_path, char *valp, int size);
int cvorbdev_get_attr_uint64(const char *attr_path, uint64_t * valp);
int cvorbdev_get_attr_bin(const char *attr_path, void *value,
			  size_t count);
int cvorbdev_set_attr_int32(const char *attr_path, int32_t value);
int cvorbdev_set_attr_uint32(const char *attr_path, uint32_t value);
int cvorbdev_set_attr_bin(const char *attr_path, const void *value,
			  size_t count);

#endif				/* _CVORBDEV_H_ */
