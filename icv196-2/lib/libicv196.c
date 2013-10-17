/* ==================================================================== */
/* Implement Icv196 digital IO library                                  */
/* Wed 3rd July 2013 Julian Lewis BE/CO/HT                              */
/* ==================================================================== */

#define _XOPEN_SOURCE
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
#include <strings.h>
#include <stdint.h>
#include <libicv196.h>

/* ==================================================================== */
/* Open driver handle                                                   */

int icv196Open(int minor)
{

	char fnm[32];
	int fd;

	sprintf(fnm,"/dev/icv196.%1d",minor);
	if ((fd = open(fnm,O_RDWR,0)) > 0)
		return fd;

	return 0;
}

/* ==================================================================== */
/* Close driver handle                                                  */

void icv196Close(int fd)
{
	if (fd > 0)
		close(fd);
}

/* ==================================================================== */
/* Get the device address                                               */

icv196_err_t icv196GetModuleAddress(int fd, struct icv196_mod_addr_s *moad)
{
	if (ioctl(fd,ICV196_GET_DEVICE_ADDRESS,moad) < 0)
		return ICV196_LIB_ERR_IO;

	return ICV196_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Reset device                                                         */

icv196_err_t icv196Reset(int fd)
{

	uint32_t reset = 1;

	if (ioctl(fd,ICV196_RESET,&reset) < 0)
		return ICV196_LIB_ERR_IO;

	return ICV196_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Set debug mask options                                               */

icv196_err_t icv196SetDebug(int fd, uint32_t deb)
{

	if (ioctl(fd,ICV196_SET_SW_DEBUG,&deb) < 0)
		return ICV196_LIB_ERR_IO;

	return ICV196_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Get debug mask options                                               */

icv196_err_t icv196GetDebug(int fd, uint32_t *deb)
{

	if (ioctl(fd,ICV196_GET_SW_DEBUG,deb) < 0)
		return ICV196_LIB_ERR_IO;

	return ICV196_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Get installed device count                                           */

icv196_err_t icv196GetDeviceCount(int fd, uint32_t *cnt)
{

	if (ioctl(fd, ICV196_GET_DEVICE_COUNT,cnt) < 0)
		return ICV196_LIB_ERR_IO;

	return ICV196_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Get driver and hardware version                                      */

icv196_err_t icv196GetVersion(int fd, struct icv196_version_s *ver)
{
	if (ioctl(fd,ICV196_GET_VERSION,ver) < 0)
		return ICV196_LIB_ERR_IO;

	return ICV196_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Connect to device interrupt                                          */

icv196_err_t icv196Connect(int fd, uint32_t imsk)
{

	if (ioctl(fd,ICV196_CONNECT, &imsk) < 0)
		return ICV196_LIB_ERR_IO;

	return ICV196_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Get connections                                                      */

icv196_err_t icv196GetConnect(int fd, uint32_t *imsk)
{

	if (ioctl(fd,ICV196_GET_CONNECT, imsk) < 0)
		return ICV196_LIB_ERR_IO;

	return ICV196_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Set wait timeout, 0 means wait for ever                              */

icv196_err_t icv196SetTimeout(int fd, uint32_t tmo)
{
	if (ioctl(fd,ICV196_SET_TIMEOUT,&tmo) < 0)
		return ICV196_LIB_ERR_IO;

	return ICV196_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Get wait timeout, 0 means wait for ever                              */

icv196_err_t icv196GetTimeout(int fd, uint32_t *tmo)
{

	if (ioctl(fd,ICV196_GET_TIMEOUT,tmo) < 0)
	       return ICV196_LIB_ERR_IO;

	return ICV196_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Wait for an interrupt                                                */

icv196_err_t icv196Wait(int fd, struct icv196_int_buf_s *intr)
{

	int ret;

	ret = read(fd,intr,sizeof(struct icv196_int_buf_s));

	if ((ret < 0) || (intr->src == 0)) {
		bzero(intr,sizeof(struct icv196_int_buf_s));
		return ICV196_LIB_ERR_TIMEOUT;
	}

	return ICV196_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Get set output groups                                                */

static uint16_t swap_bits(uint16_t val)
{
	uint16_t a, b, c;

	a = 0x55555 & val;
	b = 0xAAAAA & val;
	c = (a << 1) | (b >> 1);
	return c;
}

icv196_err_t icv196SetGroupsAsOutput(int fd, uint32_t omsk)
{
	uint32_t msk = swap_bits(omsk);

	if (ioctl(fd,ICV196_BYTE_SET_OUTPUT,&msk) < 0)
		return ICV196_LIB_ERR_IO;

	return ICV196_LIB_ERR_SUCCESS;
}

icv196_err_t icv196GetGroupsAsOutput(int fd, uint32_t *omsk)
{
	uint32_t msk = 0;

	if (ioctl(fd,ICV196_BYTE_GET_OUTPUT,&msk) < 0)
	       return ICV196_LIB_ERR_IO;

	*omsk = swap_bits(msk);

	return ICV196_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Read/Write groups of bits (bytes)                                    */

static void swap_digiob(struct icv196_digiob_s *giob)
{

	swab(giob->val,giob->val,MAX_BYTES);
	giob->msk = swap_bits(giob->msk);
}

icv196_err_t icv196SetGroups(int fd, struct icv196_digiob_s giob)
{
	swap_digiob(&giob);

	if (ioctl(fd,ICV196_BYTE_WRITE,&giob) < 0)
		return ICV196_LIB_ERR_IO;

	return ICV196_LIB_ERR_SUCCESS;
}

icv196_err_t icv196GetGroups(int fd, struct icv196_digiob_s *giob)
{
	uint16_t msk = giob->msk;

	swap_digiob(giob);

	if (ioctl(fd,ICV196_BYTE_READ,giob) < 0) {
		giob->msk = msk;
		return ICV196_LIB_ERR_IO;
	}
	swab(giob->val,giob->val,MAX_BYTES);
	giob->msk = msk;
	return ICV196_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Convert an icv196_err_tor to a string suitable for printing.               */
/* This routine can not return a NULL pointer. If the error is out of   */
/* range it returns a string saying just that. The pointer returned is  */
/* thread safe and there is no need to free it, the allocated memory is */
/* kept in the callers address space.                                   */
/* If the handle pointer is NULL, the error text is kept in static      */
/* memory, and is not thread safe. In this case copy it yourself.       */

#define ICV196_LIB_ERR_STRING_SIZE 128

static char *ErrTexts[ICV196_LIB_ERR_ERRORS] = {
	"All went OK, No error                      ",  // ICV196_LIB_ERR_SUCCESS,
	"Timeout in wait                            ",  // ICV196_LIB_ERR_TIMEOUT,
	"IOCTL call to driver failed                ",  // ICV196_LIB_ERR_IO,
 };

char *icv196ErrToStr(icv196_err_t error)
{
	char         *ep;    /* Error text string pointer */
	char         *cp;    /* Char pointer */
	unsigned int i;
	static char err[ICV196_LIB_ERR_STRING_SIZE]; /* Static text area when null handle */

	i = (unsigned int) error;
	if (i >= ICV196_LIB_ERR_ERRORS) return "Invalid error code";
	ep = err;

	strncpy(ep,ErrTexts[i],(size_t) ICV196_LIB_ERR_STRING_SIZE);

	if (error != ICV196_LIB_ERR_SUCCESS) {

		/* Extra info available from errno */

		cp = strerror(errno);
		if (cp) {
			strcat(ep,": ");
			i = ICV196_LIB_ERR_STRING_SIZE - strlen(ep) -1;  /* -1 is 0 terminator byte */
			strncat(ep,cp,i);
		}
	}
	return ep;
}

/* ==================================================================== */
/* Write first read second raw io                                       */

icv196_err_t icv196RawIo(int fd, struct icv196_riob_s *rio, int io_dir)
{
	if (io_dir & ICV196_IO_DIR_WRITE)
		if (ioctl(fd,ICV196_RAW_WRITE,rio) < 0)
			return ICV196_LIB_ERR_IO;

	if (io_dir & ICV196_IO_DIR_READ)
		if (ioctl(fd,ICV196_RAW_READ,rio) < 0)
			return ICV196_LIB_ERR_IO;

	return ICV196_LIB_ERR_SUCCESS;
}
