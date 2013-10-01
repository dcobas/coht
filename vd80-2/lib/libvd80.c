/* ==================================================================== */
/* Implement Vd80 sampler library - total rewrite of old version        */
/* Fri 26 Apr 2013 Julian Lewis BE/CO/Ht                                */
/* ==================================================================== */

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
#include <dlfcn.h>
#include <unistd.h>
#include <errno.h>

#include <stdint.h>
#include <vd80.h>
#include <vd80hard.h>
#include "libvd80.h"

static __attribute__((unused)) char version[] = "version: " GIT_VERSION;

/* ==================================================================== */

static int do_command(int fd, int chn, int cmd)
{

	int val;

	val = cmd;
	if (cmd == VD80_COMMAND_READ)
		val |= (VD80_OPERANT_MASK & ((chn -1) << VD80_OPERANT_SHIFT));

	if (ioctl(fd,VD80_SET_COMMAND,&val) < 0)
		return VD80_LIB_ERR_IO;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Open driver handle                                                   */

int vd80OpenHandle(int dev)
{

	char fnm[32];
	int fd;

	sprintf(fnm,"/dev/vd80.%1d",dev);
	if ((fd = open(fnm,O_RDWR,0)) > 0)
		return fd;

	return 0;
}

/* ==================================================================== */
/* Close driver handle                                                  */

void vd80CloseHandle(int fd)
{
	if (fd > 0)
		close(fd);
}

/* ==================================================================== */
/* Get the module address                                               */

vd80_err_t vd80GetModuleAddress(int fd, struct vd80_mod_addr_s *moad)
{
	if (ioctl(fd,VD80_GET_MODULE_ADDRESS,moad) < 0)
		return VD80_LIB_ERR_IO;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Reset module                                                         */

vd80_err_t vd80ResetMod(int fd)
{

	int reset = 1;

	if (ioctl(fd,VD80_RESET,&reset) < 0)
		return VD80_LIB_ERR_IO;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Set debug mask options                                               */

vd80_err_t vd80SetDebug(int fd, vd80_debug_level_t deb)
{

	if (ioctl(fd,VD80_SET_SW_DEBUG,&deb) < 0)
		return VD80_LIB_ERR_IO;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Get debug mask options                                               */

vd80_err_t vd80GetDebug(int fd, vd80_debug_level_t *deb)
{

	if (ioctl(fd,VD80_GET_SW_DEBUG,deb) < 0)
		return VD80_LIB_ERR_IO;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Get instantaneous ADC value for a given channel and module           */

vd80_err_t vd80GetAdcValue(int fd, uint32_t chn, int *adc)
{

	int val;
	short sval;

	val = chn;
	if (ioctl(fd,VD80_READ_ADC,&val) < 0)
		return VD80_LIB_ERR_IO;
	sval = val & 0xFFFF;
	*adc = sval;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Get State of the module                                              */

vd80_err_t vd80GetState(int fd, vd80_state_t *ste)
{

	if (ioctl(fd,VD80_GET_STATE,ste) < 0)
		return VD80_LIB_ERR_IO;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Get installed module count                                           */

vd80_err_t vd80GetModuleCount(int fd, uint32_t *cnt)
{

	if (ioctl(fd, VD80_GET_MODULE_COUNT,cnt) < 0)
		return VD80_LIB_ERR_IO;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Get driver and hardware version                                      */

vd80_err_t vd80GetVersion(int fd, struct vd80_version_s *ver)
{
	if (ioctl(fd,VD80_GET_VERSION,ver) < 0)
		return VD80_LIB_ERR_IO;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Set Trigger                                                          */

vd80_err_t vd80SetTrigger(int fd, struct vd80_input_s *inp)
{

	uint32_t val;

	val = inp->edge;
	if (ioctl(fd,VD80_SET_TRIGGER_EDGE,&val) < 0)
		return VD80_LIB_ERR_IO;

	val = inp->termination;
	if (ioctl(fd,VD80_SET_TRIGGER_TERMINATION,&val) < 0)
		return VD80_LIB_ERR_IO;

	val = inp->source;
	if (ioctl(fd,VD80_SET_TRIGGER,&val) < 0)
		return VD80_LIB_ERR_IO;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Get trigger input options.                                           */

vd80_err_t vd80GetTrigger(int fd, struct vd80_input_s *inp)
{

	int val;

	if (ioctl(fd,VD80_GET_TRIGGER_EDGE,&val) < 0)
		return VD80_LIB_ERR_IO;
	inp->edge = val;

	if (ioctl(fd,VD80_GET_TRIGGER_TERMINATION,&val) < 0)
		return VD80_LIB_ERR_IO;
	inp->termination = val;

	if (ioctl(fd,VD80_GET_TRIGGER,&val) < 0)
		return VD80_LIB_ERR_IO;
	inp->source = val;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Set clock input settings.                                            */

vd80_err_t vd80SetClock(int fd, struct vd80_input_s *inp)
{

	int val;

	val = inp->edge;
	if (ioctl(fd,VD80_SET_CLOCK_EDGE,&val) < 0)
		return VD80_LIB_ERR_IO;

	val = inp->termination;
	if (ioctl(fd,VD80_SET_CLOCK_TERMINATION,&val) < 0)
		return VD80_LIB_ERR_IO;

	val = inp->source;
	if (ioctl(fd,VD80_SET_CLOCK,&val) < 0)
		return VD80_LIB_ERR_IO;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Get clock input settings                                             */

vd80_err_t vd80GetClock(int fd, struct vd80_input_s *inp)
{

	int val;

	if (ioctl(fd,VD80_GET_CLOCK_EDGE,&val) < 0)
		return VD80_LIB_ERR_IO;
	inp->edge = val;

	if (ioctl(fd,VD80_GET_CLOCK_TERMINATION,&val) < 0)
		return VD80_LIB_ERR_IO;
	inp->termination = val;

	if (ioctl(fd,VD80_GET_CLOCK,&val) < 0)
		return VD80_LIB_ERR_IO;
	inp->source = val;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Set clock sample rate                                                */

vd80_err_t vd80SetClockDivide(int fd, uint32_t dvd)
{

	int val;

	val = VD80_CLKDIVMODE_DIVIDE;
	if (ioctl(fd,VD80_SET_CLOCK_DIVIDE_MODE,&val) < 0)
		return VD80_LIB_ERR_IO;

	val = dvd;
	if (ioctl(fd,VD80_SET_CLOCK_DIVISOR,&val) < 0)
		return VD80_LIB_ERR_IO;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Get clock sample rate                                                */

vd80_err_t vd80GetClockDivide(int fd, uint32_t *dvd)
{

	if (ioctl(fd,VD80_GET_CLOCK_DIVISOR,dvd) < 0)
		return VD80_LIB_ERR_IO;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Start acquisition, state will be pretrigger                          */

vd80_err_t vd80StrtAcquisition(int fd)
{

	return do_command(fd,0,VD80_COMMAND_START);
}

/* ==================================================================== */
/* Trigger acquisition, state will be posttrigger                       */

vd80_err_t vd80TrigAcquisition(int fd)
{

	return do_command(fd,0,VD80_COMMAND_TRIGGER);
}

/* ==================================================================== */
/* Stop acquisition, state will be idle                                 */

vd80_err_t vd80StopAcquisition(int fd)
{

	return do_command(fd,0,VD80_COMMAND_STOP);
}

/* ==================================================================== */
/* Connect to module interrupt                                          */

vd80_err_t vd80Connect(int fd, vd80_intr_mask_t imsk)
{

	if (ioctl(fd,VD80_CONNECT, &imsk) < 0)
		return VD80_LIB_ERR_IO;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Get connections                                                      */

vd80_err_t vd80GetConnect(int fd, vd80_intr_mask_t *imsk)
{

	if (ioctl(fd,VD80_GET_CONNECT, imsk) < 0)
		return VD80_LIB_ERR_IO;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Set wait timeout, 0 means wait for ever                              */

vd80_err_t vd80SetTimeout(int fd, uint32_t tmo)
{
	if (ioctl(fd,VD80_SET_TIMEOUT,&tmo) < 0)
		return VD80_LIB_ERR_IO;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Get wait timeout, 0 means wait for ever                              */

vd80_err_t vd80GetTimeout(int fd, uint32_t *tmo)
{

	if (ioctl(fd,VD80_GET_TIMEOUT,tmo) < 0)
	       return VD80_LIB_ERR_IO;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Wait for an interrupt                                                */

vd80_err_t vd80Wait(int fd, struct vd80_int_buf_s *intr)
{

	int ret;

	ret = read(fd,intr,sizeof(struct vd80_int_buf_s));
	if (ret <= 0)
		return VD80_LIB_ERR_TIMEOUT;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Set module post sample count                                         */

vd80_err_t vd80SetPostSamples(int fd, uint32_t post)
{

	if (ioctl(fd,VD80_SET_POSTSAMPLES,&post) < 0)
		return VD80_LIB_ERR_IO;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Get module post sample count                                         */

vd80_err_t vd80GetPostSamples(int fd, uint32_t *post)
{

	if (ioctl(fd,VD80_GET_POSTSAMPLES,post) < 0)
		return VD80_LIB_ERR_IO;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Transfer module data by DMA to users buffer                          */

vd80_err_t vd80GetBuffer(int fd, uint32_t chn, struct vd80_buffer_s *buf)
{

	struct vd80_sample_buf_s sbuf;
	vd80_err_t err;
	int tps;

	err = do_command(fd,chn,VD80_COMMAND_READ);
	if (err != VD80_LIB_ERR_SUCCESS)
		return err;

	tps = buf->bsze - buf->post -1;
	if (tps < 0)
		tps = 0;

	sbuf.sample_buf       = buf->addr;
	sbuf.buf_size_samples = buf->bsze;
	sbuf.channel          = chn;
	sbuf.trig_position    = tps;
	sbuf.samples_read     = 0;

	if (ioctl(fd,VD80_READ_SAMPLE,&sbuf) < 0)
		return VD80_LIB_ERR_IO;

	buf->tpos = sbuf.trig_position;
	buf->asze = sbuf.samples_read;
	buf->ptsr = sbuf.pre_trig_stat;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Set the trigger analogue levels                                      */

vd80_err_t vd80SetTriggerLevel(int fd, struct vd80_analogue_trig_s *atrg)
{

	if (ioctl(fd,VD80_SET_ANALOGUE_TRIGGER,atrg) < 0)
		return VD80_LIB_ERR_IO;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Get the trigger analogue level                                       */

vd80_err_t vd80GetTriggerLevel(int fd, struct vd80_analogue_trig_s *atrg)
{

	if (ioctl(fd,VD80_GET_ANALOGUE_TRIGGER,atrg) < 0)
		return VD80_LIB_ERR_IO;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Set trigger configuration params, delay and min pretrig samples      */

vd80_err_t vd80SetTriggerConfig(int fd, struct vd80_trig_config_s *ctrg)
{

	if (ioctl(fd,VD80_SET_TRIGGER_CONFIG,ctrg) < 0)
		return VD80_LIB_ERR_IO;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Get trigger configuration params, delay and min pretrig samples      */

vd80_err_t vd80GetTriggerConfig(int fd, struct vd80_trig_config_s *ctrg)
{

	if (ioctl(fd,VD80_GET_TRIGGER_CONFIG,ctrg) < 0)
		return VD80_LIB_ERR_IO;

	return VD80_LIB_ERR_SUCCESS;
}

/* ==================================================================== */
/* Convert an vd80_err_tor to a string suitable for printing.               */
/* This routine can not return a NULL pointer. If the error is out of   */
/* range it returns a string saying just that. The pointer returned is  */
/* thread safe and there is no need to free it, the allocated memory is */
/* kept in the callers address space.                                   */
/* If the handle pointer is NULL, the error text is kept in static      */
/* memory, and is not thread safe. In this case copy it yourself.       */

#define VD80_LIB_ERR_STRING_SIZE 128

static char *ErrTexts[VD80_LIB_ERR_ERRORS] = {
	"All went OK, No error                      ",  // VD80_LIB_ERR_SUCCESS,
	"Timeout in wait                            ",  // VD80_LIB_ERR_TIMEOUT,
	"IOCTL call to driver failed                ",  // VD80_LIB_ERR_IO,
 };

char *vd80ErrToStr(vd80_err_t error)
{
	char         *ep;    /* Error text string pointer */
	char         *cp;    /* Char pointer */
	unsigned int i;
	static char err[VD80_LIB_ERR_STRING_SIZE]; /* Static text area when null handle */

	i = (unsigned int) error;
	if (i >= VD80_LIB_ERR_ERRORS) return "Invalid error code";
	ep = err;

	strncpy(ep,ErrTexts[i],(size_t) VD80_LIB_ERR_STRING_SIZE);

	if (error != VD80_LIB_ERR_SUCCESS) {

		/* Extra info available from errno */

		cp = strerror(errno);
		if (cp) {
			strcat(ep,": ");
			i = VD80_LIB_ERR_STRING_SIZE - strlen(ep) -1;  /* -1 is 0 terminator byte */
			strncat(ep,cp,i);
		}
	}
	return ep;
}

/* ==================================================================== */
/* Write first read second raw io                                       */

vd80_err_t vd80RawIo(int fd, struct vd80_riob_s *rio, int io_dir)
{
	if (io_dir & VD80_IO_DIR_WRITE)
		if (ioctl(fd,VD80_RAW_WRITE,rio) < 0)
			return VD80_LIB_ERR_IO;

	if (io_dir & VD80_IO_DIR_READ)
		if (ioctl(fd,VD80_RAW_READ,rio) < 0)
			return VD80_LIB_ERR_IO;

	return VD80_LIB_ERR_SUCCESS;
}
