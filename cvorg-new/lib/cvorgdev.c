/*
 * cvorgdev.c
 * sysfs device management for cvorg devices
 *
 * Copyright (c) 2012	Samuel Iglesias Gonsalvez <siglesia@cern.ch> 
 * Based on the code from Emilio G. Cota for the CVORG driver.
 *
 * Released under the GPL v2.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include "cvorg.h"

#include "cvorgdev.h"

static void remove_trailing_chars(char *path, char c)
{
	size_t len;

	if (path == NULL)
		return;
	len = strlen(path);
	while (len > 0 && path[len - 1] == c)
		path[--len] = '\0';
}

/* this assumes all devices are called cvorg.X */
int cvorgdev_get_devname(int index, char *str, size_t len)
{
	snprintf(str, len, "cvorg.%d", index);
	str[len - 1] = '\0';
	return 0;
}

/* this assumes all channels are called channel.X */
int cvorgdev_get_channame(int index, char *str, size_t len)
{
	snprintf(str, len, "channel.%d", index);
	str[len - 1] = '\0';
	return 0;
}


static void path_append(char *path, const char *append, size_t len)
{
	snprintf(path + strlen(path), len - strlen(path), "/%s", append);
	path[len - 1] = '\0';
}

/*
 * As stated in linux-2.6/Documentation/sysfs-rules.txt, we first search
 * our devices in /sys/subsystem/cvorg. If it doesn't exist, we try with
 * the symlinks in /sys/class/cvorg.
 */
static int build_cvorgpath(char *path, size_t len)
{
	struct stat stats;
	int ret;

	snprintf(path, len, "/sys/subsystem/cvorg/devices");
	path[len - 1] = '\0';
	ret = stat(path, &stats);
	if (ret == 0)
		return 0;
	snprintf(path, len, "/sys/class/cvorg");
	path[len - 1] = '\0';
	ret = stat(path, &stats);
	if (ret == 0)
		return 0;
	fprintf(stderr, "warning: no cvorg devices found in sysfs\n");
	return -1;
}

static int build_devpath(int index, char *path, size_t len)
{
	char name[32];
	int ret = 0;

	ret = build_cvorgpath(path, len);
	if (ret < 0)
		return -1;
	ret = cvorgdev_get_devname(index, name, sizeof(name));
	if (ret < 0)
		return -1;
	path_append(path, name, len);
	return 0;
}

/* Access to channel subfolder */
static int build_chanpath(int index, char *path, size_t len)
{
	char name[32];
	int ret = 0;

	ret = cvorgdev_get_channame(index, name, sizeof(name));
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
		 * Assume all devices are called 'cvorg.X'.
		 */
		if (indexes) {
			if (i >= elems)
				break;
			indexes[i] = strtol(dent->d_name + strlen("cvorg."), NULL, 0);
		}
		i++;
	}
	if (closedir(dir) < 0)
		return -1;
	return i;
}

int cvorgdev_get_nr_devices(void)
{
	char path[CVORG_PATH_MAX];
	int ret;

	ret = build_cvorgpath(path, sizeof(path));
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

int cvorgdev_get_device_list(int *indexes, int elems)
{
	char path[CVORG_PATH_MAX];
	int ret;

	ret = build_cvorgpath(path, sizeof(path));
	if (ret < 0)
		return ret;
	ret = list_devices(path, indexes, elems);
	if (ret < 0)
		return ret;
	qsort(indexes, elems, sizeof(*indexes), cmp_ints);
	return 0;
}

/* Note: this function returns 1 on success */
int cvorgdev_device_exists(int index)
{
	struct stat statbuf;
	char path[CVORG_PATH_MAX];

	if (build_devpath(index, path, sizeof(path)) < 0)
		return 0;
	if (stat(path, &statbuf))
		return 0;
	return 1;
}

static int cvorgdev_get_attr_dev(char *path, size_t pathlen, const char *attr, char *value, size_t len)
{
	struct stat statbuf;
	size_t size;
	int fd;

	/* make sure it's NUL-terminated even if we fail before filling it in */
	value[len - 1] = '\0';
	path_append(path, attr, pathlen);

	if (lstat(path, &statbuf) != 0) {
		fprintf(stderr, "warning: Attribute %s not found in %s\n", attr, path);
		return -1;
	}

	if (S_ISLNK(statbuf.st_mode))
		return -1;
	if (S_ISDIR(statbuf.st_mode))
		return -1;
	if (!(statbuf.st_mode & S_IRUSR))
		return -1;

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "warning: cannot open '%s'\n", path);
		return -1;
	}
	size = read(fd, value, len);
	value[len - 1] = '\0';
	if (close(fd) < 0)
		return -1;
	if (size < 0)
		return -1;
	if (size == len)
		return -1;

	value[size] = '\0';
	remove_trailing_chars(value, '\n');
	return 0;
}


int cvorgdev_get_attr(int index, const char *name, char *value, size_t count)
{
	char path[CVORG_PATH_MAX];
	int ret;

	ret = build_devpath(index, path, sizeof(path));
	if (ret < 0)
		return ret;
	return cvorgdev_get_attr_dev(path, sizeof(path), name, value, count);
}

int cvorgdev_get_chan_attr(int index, int channel, const char *name, char *value, size_t count)
{
	char path[CVORG_PATH_MAX];
	int ret;

	ret = build_devpath(index, path, sizeof(path));
	if (ret < 0)
		return ret;
	ret = build_chanpath(channel, path, sizeof(path));
	if (ret < 0)
		return ret;

	return cvorgdev_get_attr_dev(path, sizeof(path), name, value, count);
}


int cvorgdev_get_attr_int(int index, const char *name, int *valp)
{
	char value[32];
	int ret;

	ret = cvorgdev_get_attr(index, name, value, sizeof(value));
	if (ret)
		return ret;
	*valp = strtol(value, NULL, 0);
	return 0;
}

int cvorgdev_get_attr_ulong(int index, const char *name, uint64_t *valp)
{
	char value[32];
	int ret;

	ret = cvorgdev_get_attr(index, name, value, sizeof(value));
	if (ret)
		return ret;

	*valp = strtoull(value, NULL, 0);
	return 0;
}


int cvorgdev_get_attr_char(int index, const char *name, char *valp)
{
	char value[CVORG_HW_VER_MAX_CHAR];
	int ret;

	ret = cvorgdev_get_attr(index, name, value, sizeof(value));
	if (ret)
		return ret;
	snprintf(valp, CVORG_HW_VER_MAX_CHAR, "%s", value);
	return 0;
}


int cvorgdev_get_chan_attr_int(int index, int channel, const char *name, int *valp)
{
	char value[32];
	int ret;

	ret = cvorgdev_get_chan_attr(index, channel, name, value, sizeof(value));
	if (ret)
		return ret;
	*valp = strtol(value, NULL, 0);
	return 0;
}


int cvorgdev_get_attr_uint(int index, const char *name, unsigned int *valp)
{
	char value[32];
	int ret;

	ret = cvorgdev_get_attr(index, name, value, sizeof(value));
	if (ret)
		return ret;
	*valp = strtoul(value, NULL, 0);
	return 0;
}

int cvorgdev_get_chan_attr_uint(int index, int channel, const char *name, unsigned int *valp)
{
	char value[32];
	int ret;

	ret = cvorgdev_get_chan_attr(index, channel, name, value, sizeof(value));
	if (ret)
		return ret;
	*valp = strtoul(value, NULL, 0);
	return 0;
}


static int cvorgdev_set_attr_dev(char *path, size_t pathlen, const char *attr, const char *value)
{
	struct stat statbuf;
	size_t size;
	int fd;

	path_append(path, attr, pathlen);

	if (lstat(path, &statbuf) != 0) {
		fprintf(stderr, "warning: Attribute %s not found in %s\n", attr, path);
		return -1;
	}

	if (S_ISLNK(statbuf.st_mode))
		return -1;
	if (S_ISDIR(statbuf.st_mode))
		return -1;
	if (!(statbuf.st_mode & S_IWUSR))
		return -1;

	fd = open(path, O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "warning: cannot open '%s'\n", path);
		return -1;
	}
	size = write(fd, value, strlen(value));
	if (close(fd) < 0)
		return -1;
	return size;
}

static int cvorgdev_get_attr_bin(char *path, size_t pathlen, const char *attr, void *value, size_t count)
{
	struct stat statbuf;
	size_t size;
	int fd;

	path_append(path, attr, pathlen);

	if (lstat(path, &statbuf) != 0) {
		fprintf(stderr, "warning: Attribute %s not found in %s\n", attr, path);
		return -1;
	}

	if (S_ISLNK(statbuf.st_mode))
		return -1;
	if (S_ISDIR(statbuf.st_mode))
		return -1;
	if (!(statbuf.st_mode & S_IWUSR))
		return -1;

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "warning: cannot open '%s'\n", path);
		return -1;
	}
	size = read(fd, value, count);
	if (close(fd) < 0)
		return -1;
	return size;
}


static int cvorgdev_set_attr_bin(char *path, size_t pathlen, const char *attr, const void *value, size_t count)
{
	struct stat statbuf;
	size_t size;
	int fd;

	path_append(path, attr, pathlen);

	if (lstat(path, &statbuf) != 0) {
		fprintf(stderr, "warning: Attribute %s not found in %s\n", attr, path);
		return -1;
	}

	if (S_ISLNK(statbuf.st_mode))
		return -1;
	if (S_ISDIR(statbuf.st_mode))
		return -1;
	if (!(statbuf.st_mode & S_IWUSR))
		return -1;

	fd = open(path, O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "warning: cannot open '%s'\n", path);
		return -1;
	}
	size = write(fd, value, count);
	if (close(fd) < 0)
		return -1;
	return size;
}

int cvorgdev_set_attr(int index, const char *name, const char *value)
{
	char path[CVORG_PATH_MAX];
	int ret;

	ret = build_devpath(index, path, sizeof(path));
	if (ret < 0)
		return ret;
	return cvorgdev_set_attr_dev(path, sizeof(path), name, value);
}

int cvorgdev_set_chan_attr(int index, int channel, const char *name, const char *value)
{
	char path[CVORG_PATH_MAX];
	int ret;

	ret = build_devpath(index, path, sizeof(path));
	if (ret < 0)
		return ret;

	ret = build_chanpath(channel, path, sizeof(path));
	if (ret < 0)
		return ret;

	return cvorgdev_set_attr_dev(path, sizeof(path), name, value);
}

int cvorgdev_set_attr_int(int index, const char *name, int value)
{
	char buf[32];

	snprintf(buf, sizeof(buf) - 1, "%d", value);
	buf[sizeof(buf) - 1] = '\0';
	return cvorgdev_set_attr(index, name, buf);
}

int cvorgdev_set_chan_attr_int(int index, int channel, const char *name, int value)
{
	char buf[32];

	snprintf(buf, sizeof(buf) - 1, "%d", value);
	buf[sizeof(buf) - 1] = '\0';
	return cvorgdev_set_chan_attr(index, channel, name, buf);
}


int cvorgdev_set_attr_uint(int index, const char *name, unsigned int value)
{
	char buf[32];

	snprintf(buf, sizeof(buf) - 1, "%u", value);
	buf[sizeof(buf) - 1] = '\0';
	return cvorgdev_set_attr(index, name, buf);
}

int cvorgdev_set_chan_attr_uint(int index, int channel, const char *name, unsigned int value)
{
	char buf[32];

	snprintf(buf, sizeof(buf) - 1, "%u", value);
	buf[sizeof(buf) - 1] = '\0';
	return cvorgdev_set_chan_attr(index, channel, name, buf);
}

static int __cvorgdev_set_chan_attr_bin(int index, int channel, const char *name, const void *value, size_t count)
{
	char path[CVORG_PATH_MAX];
	int ret;

	ret = build_devpath(index, path, sizeof(path));
	if (ret < 0)
		return ret;

	ret = build_chanpath(channel, path, sizeof(path));
	if (ret < 0)
		return ret;
	return cvorgdev_set_attr_bin(path, sizeof(path), name, value, count);
}


int cvorgdev_set_chan_attr_bin(int index, int channel, const char *name, void *value, ssize_t count)
{
	return __cvorgdev_set_chan_attr_bin(index, channel, name, value, count);
}

static int __cvorgdev_get_chan_attr_bin(int index, int channel, const char *name, void *value, size_t count)
{
	char path[CVORG_PATH_MAX];
	int ret;

	ret = build_devpath(index, path, sizeof(path));
	if (ret < 0)
		return ret;

	ret = build_chanpath(channel, path, sizeof(path));
	if (ret < 0)
		return ret;
	return cvorgdev_get_attr_bin(path, sizeof(path), name, value, count);
}


int cvorgdev_get_chan_attr_bin(int index, int channel, const char *name, void *value, ssize_t count)
{
	return __cvorgdev_get_chan_attr_bin(index, channel, name, value, count);
}
