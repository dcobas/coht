/**
 * Julian Lewis Fri-30th March 2012 BE/CO/HT
 * julian.lewis@cern.ch
 *
 * CTR timing library stub.
 * This library is to be used exclusivlely by the new open CBCM timing library.
 * It is up to the implementers of the open CBCM timing library to provide any
 * abstraction layers and to hide this completely from user code.
 */

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sched.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <dlfcn.h>

#include <libctr.h>

/**
 * @brief This routine gets called when a function is not implemented
 * @return Always returns -1
 *
 * Notice that () isnt the same as (void) !!, it means any number of args are ignored.
 */

int ctr_not_implemented()
{
	errno = ENOTIMPL;
	return -1;
}

/**
 * @brief Get a handle to be used in subsequent library calls
 * @param Version string or NULL for the latest
 * @return The handle to be used in subsequent calls or -1
 *
 * The ctr_open call returns a pointer to an opeaque structure
 * defined within the library internal implementation. Clients
 * never see what is behind the void pointer.
 *
 * If a version string is specified and shared objects are in use,
 * then the specified version will be loaded, else a NULL or empty
 * string points to the installed version. Version strings consits
 * of two integers seperated by a point eg "3.1" or "1.0" these
 * numbers are the major and minor version numbers.
 *
 * Implementation hint:
 * NEVER hard code the version number into the source!! Its part
 * of the environment, suggest CTR_LIB_VERSION environment variable.
 * If it's not defined, use the default NULL string.
 *
 * The returned handle is -1 on error otherwise its a valid handle.
 * On error use the standard Linux error functions for details.
 *
 * Each time ctr_open is called a new handle is allocated, due to the
 * current ctr driver implementation there can never be more than
 * 16 open handles at any one time (this limitation should be removed).
 *
 *  void *my_handle;
 *  my_handle = ctr_open(NULL);
 *  if ((int) my_handle == CTR_ERROR)
 *          perror("ctr_open error");
 *
 */
void *ctr_open(char *version)
{

	struct ctr_handle_s *h;
	char *cp = NULL, path[CTR_PATH_SIZE];
	int  i, fd = -1, errsv;
	CtrDrvrVersion version;
	void *ptr;

	h = (struct ctr_handle_s*) malloc(sizeof(struct ctr_handle_s));
	if (!h)
		return (void *) -1;

	for (i=1; i<=CtrDrvrCLIENT_CONTEXTS; i++) {
		sprintf(path,"/dev/ctr.%1d",i);
		if ((fd = open(path,O_RDWR,0)) > 0)
			break;
	}
	if (fd <= 0) {
		errsv = errno;
		free(h);
		errno = errsv;
		return (void *) -1;
	}
	h->fd = fd;

	version.HardwareType = CtrDrvrHardwareTypeNONE;
	if (ioctl(fd,CtrIoctlGET_VERSION,&version) < 0) {
		errsv = errno;
		free(h);
		errno = errsv;
		return (void *) -1;
	}

	if ((version.HardwareType == CtrDrvrHardwareTypeCTRP)
	||  (version.HardwareType == CtrDrvrHardwareTypeCTRI))
		cp = "ctrp";

	if ((version.HardwareType == CtrDrvrHardwareTypeCTRV)
	||  (version.HardwareType == CtrDrvrHardwareTypeCTRE))
		cp = "ctrv";

	if (!cp) {
		errno = ENODEV;
		return (void *) -1;
	}

	sprintf(path,"/usr/local/drivers/ctr/lib%s.so.%s",cp,version);
	h->dll_handle = dlopen(sopath, RTLD_LOCAL | RTLD_LAZY);
	if (!h->dll_handle) {
		errsv = errno;
		fprintf(stderr,"ctr_open:%s\n",dlerror());
		free(h);
		errno = errsv;
		return (void *) -1;
	}

	ptr = &h->api;
	for (i=0; i<CTR_INDEX_LAST; i++) {
		ptr[i] = dlsym(h->dll_handle, api_names[i]);
		if (!ptr[i]) {
			fprintf(stderr,"ctr_open:%s\n",dlerror());
			ptr[i] = ctr_not_implemented;
		}
	}
	return h;
}

/**
 * @brief Close a handle and free up resources
 * @param A handle that was allocated in open
 * @return Zero means success else -1 is returned on error, see errno
 *
 * This routine disconnects from all interrupts, frees up memory and
 * closes the ctr driver. It should be called once for each ctr_open.
 */
int ctr_close(void *handle)
{
	struct ctr_handle_s *h;
	int errsv;

	h = handle;
	if (h) {
		if (!dlclose(h->dll_handle)) {
			errsv = errno;
			fprintf(stderr,"ctr_close:%s\n",dlerror());
			errno = errsv;
			return (void *) -1;
		}

		if (close(h->fd))
			return (void *) -1;

		free(h);
		return 0;
	}
	errno = EBADFD;
	return (void *) -1;
}
