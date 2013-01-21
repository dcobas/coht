/*
 * cvorbdev.c
 * sysfs device management for cvorb devices
 *
 * Copyright (c) 2012	Samuel Iglesias Gonsalvez <siglesia@cern.ch> 
 * Based on the code from Emilio G. Cota for the CVORB driver.
 *
 * Released under the GPL v2.
 */
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#include <cvorb.h>

#include "cvorbdev.h"

static void remove_trailing_chars(char *path, char c)
{
	size_t len;

	if (path == NULL)
		return;
	len = strlen(path);
	while (len > 0 && path[len - 1] == c)
		path[--len] = '\0';
}

/* this assumes all devices are called cvorb.X */
void cvorbdev_get_devname(int lun, char *str, size_t len)
{
	snprintf(str, len, "cvorb.%d", lun);
	str[len - 1] = '\0';
}

static void path_append(char *path, const char *append, size_t len)
{
	snprintf(path + strlen(path), len - strlen(path), "/%s", append);
	path[len - 1] = '\0';
}

/*
 * As stated in linux-2.6/Documentation/sysfs-rules.txt, we first search
 * our devices in /sys/subsystem/cvorb. If it doesn't exist, we try with
 * the symlinks in /sys/class/cvorb.
 */
static int build_cvorbpath(char *path, size_t len)
{
	struct stat stats;
	int ret;

	snprintf(path, len, "/sys/subsystem/cvorb/devices");
	path[len - 1] = '\0';
	ret = stat(path, &stats);
	if (ret == 0)
		return 0;
	snprintf(path, len, "/sys/class/cvorb");
	path[len - 1] = '\0';
	ret = stat(path, &stats);
	if (ret == 0)
		return 0;
	fprintf(stderr, "warning: no cvorb devices found in sysfs\n");
	return -1;
}

static int build_devpath(int lun, char *path, size_t len)
{
	char name[32];
	int ret;

	ret = build_cvorbpath(path, len);
	if (ret < 0)
		return -1;
	cvorbdev_get_devname(lun, name, sizeof(name));
	if (ret < 0)
		return -1;
	path_append(path, name, len);
	return 0;
}

static int list_devices(const char *path, int *indexes, int elems)
{
	struct dirent *dent;
	DIR *dir;
	int i;

	dir = opendir(path);
	if (dir == NULL)
		return -1;

	i = 0;
	for (dent = readdir(dir); dent != NULL; dent = readdir(dir)) {
		if (dent->d_name[0] == '.')
			continue;
		/*
		 * Fill in the indexes array if provided.
		 * Assume all devices are called 'cvorb.X'.
		 */
		if (indexes) {
			if (i >= elems)
				break;
			indexes[i] =
			    strtol(dent->d_name + strlen("cvorb."), NULL,
				   0);
		}
		i++;
	}
	if (closedir(dir) < 0)
		return -1;
	return i;
}

int cvorbdev_get_nr_devices(void)
{
	char path[CVORB_PATH_SIZE];
	int ret;

	ret = build_cvorbpath(path, sizeof(path));
	if (ret < 0)
		return ret;
	return list_devices(path, NULL, 0);
}

static int cmp_ints(const void *a, const void *b)
{
	const int *n1 = a;
	const int *n2 = b;

	return *n1 - *n2;
}

int cvorbdev_get_device_list(int *indexes, int elems)
{
	char path[CVORB_PATH_SIZE];
	int ret;

	ret = build_cvorbpath(path, sizeof(path));
	if (ret < 0)
		return ret;
	ret = list_devices(path, indexes, elems);
	if (ret < 0)
		return ret;
	qsort(indexes, elems, sizeof(*indexes), cmp_ints);
	return 0;
}

/* Note: this function returns 1 on success */
int cvorbdev_device_exists(int index)
{
	struct stat statbuf;
	char path[CVORB_PATH_SIZE];

	if (build_devpath(index, path, sizeof(path)) < 0)
		return 0;
	if (stat(path, &statbuf))
		return 0;
	return 1;
}

/* Note: this function returns 1 on success */
int cvorbdev_get_sysfs_path(int lun, char *path, int size)
{
	struct stat statbuf;

	if (build_devpath(lun, path, size) < 0)
		return 0;
	if (stat(path, &statbuf))
		return 0;
	return 1;
}

/*Upon success returns the number of characters read. On failure -1 is returned*/
static int cvorbdev_get_attr(const char *attr_path, void *value,
			     size_t len)
{
	struct stat statbuf;
	size_t size;
	int fd;

	if (lstat(attr_path, &statbuf) != 0) {
		fprintf(stderr, "warning: Attribute is not found %s\n",
			attr_path);
		return -1;
	}

	if (S_ISLNK(statbuf.st_mode))
		return -1;
	if (S_ISDIR(statbuf.st_mode))
		return -1;
	if (!(statbuf.st_mode & S_IRUSR))
		return -1;

	fd = open(attr_path, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "warning: cannot open '%s'\n", attr_path);
		return -1;
	}
	size = read(fd, value, len);
	if (close(fd) < 0)
		return -1;
	if (size < 0)
		return -1;
	if (size == len)
		return -1;
	return size;
}

int cvorbdev_get_attr_int32(const char *attr_path, int *valp)
{
	char value[32];
	int ret;

	ret = cvorbdev_get_attr(attr_path, value, sizeof(value));
	if (ret == -1)
		return ret;
	*valp = strtol(value, NULL, 0);
	return 0;
}

int cvorbdev_get_attr_char(const char *attr_path, char *valp, int size)
{
	int ret;
	ret = cvorbdev_get_attr(attr_path, valp, size);
	if (ret == -1)
		return ret;
	valp[ret] = '\0';
	remove_trailing_chars(valp, '\n');
	return 0;
}

int cvorbdev_get_attr_uint32(const char *attr_path, uint32_t * valp)
{
	char value[32];
	int ret;

	ret = cvorbdev_get_attr(attr_path, value, sizeof(value));
	if (ret == -1)
		return ret;
	*valp = strtoul(value, NULL, 0);
	return 0;
}

int cvorbdev_get_attr_uint64(const char *attr_path, uint64_t * valp)
{
	char value[64];
	int ret;

	ret = cvorbdev_get_attr(attr_path, value, sizeof(value));
	if (ret == -1)
		return ret;
	*valp = strtoull(value, NULL, 0);
	return 0;
}

int cvorbdev_get_attr_bin(const char *attr_path, void *value, size_t count)
{
	return cvorbdev_get_attr(attr_path, value, count);
}

/*Upon success returns the number of characters read. On failure -1 is returned*/
static int cvorbdev_set_attr(const char *attr_path, const void *value,
			     size_t size)
{
	struct stat statbuf;
	size_t len;
	int fd;

	if (lstat(attr_path, &statbuf) != 0) {
		fprintf(stderr, "warning: Attribute not found %s\n",
			attr_path);
		return -1;
	}

	if (S_ISLNK(statbuf.st_mode))
		return -1;
	if (S_ISDIR(statbuf.st_mode))
		return -1;
	if (!(statbuf.st_mode & S_IWUSR))
		return -1;

	fd = open(attr_path, O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "warning: cannot open '%s'\n", attr_path);
		return -1;
	}
	len = write(fd, value, size);
	if (len == -1)
		fprintf(stderr, "cvorbdev_set_attr failed: %s (%d)",
			strerror(errno), errno);
	if (close(fd) < 0)
		return -1;
	return len;
}

int cvorbdev_set_attr_int32(const char *attr_path, int32_t value)
{
	char buf[32];
	int len;

	len = snprintf(buf, sizeof(buf) - 1, "%d", value);
	return cvorbdev_set_attr(attr_path, buf, len);
}

int cvorbdev_set_attr_uint32(const char *attr_path, uint32_t value)
{
	char buf[32];
	int len;

	len = snprintf(buf, sizeof(buf) - 1, "%u", value);
	return cvorbdev_set_attr(attr_path, buf, len);
}


int cvorbdev_set_attr_bin(const char *attr_path, const void *value,
			  size_t count)
{
	return cvorbdev_set_attr(attr_path, value, count);
}
