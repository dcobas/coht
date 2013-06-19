/* ==================================================================== */
/* VD80 Library                                                         */
/* Julian Lewis Fri 26th April 2013 - total rewrite                     */
/* ==================================================================== */

#ifndef VD80_LIB_LIB
#define VD80_LIB_LIB

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <vd80.h>     /* Driver API */

#define VD80_LIB_CHANNELS 16
#define VD80_IO_DIR_READ  0x1
#define VD80_IO_DIR_WRITE 0x2

/* ==================================================================== */
/* Some standard error codes                                            */

typedef enum {
	VD80_LIB_ERR_SUCCESS,  /* All went OK, No error                 */
	VD80_LIB_ERR_TIMEOUT,  /* Timeout in wait                       */
	VD80_LIB_ERR_IO,       /* IO or BUS error                       */
	VD80_LIB_ERR_ERRORS    /* Total errors */
} vd80_err_t;

/* ============================================================= */
/* A signal INPUT                                                */

struct vd80_input_s {
	vd80_edge_t        edge;
	vd80_termination_t termination;
	vd80_source_t      source;
};

/* ==================================================================== */
/* Sample buffers are read back from a sampler and contain the acquired */
/* data for a channel. The buffer pointer should be cast according to   */
/* the buffer data type.                                                */

struct vd80_buffer_s {
	int16_t  *addr;  /* Address of alloocated sample memory */
	uint32_t  bsze;  /* Buffer size in shorts               */
	uint32_t  post;  /* Requested number of post samples    */
	uint32_t  tpos;  /* Actual position of trigger          */
	uint32_t  asze;  /* Actual number of samples in buffer  */
	uint32_t  ptsr;  /* Number of 100K ticks since start    */
};

/* ==================================================================== */
/* Interrupt source connection mask                                     */

typedef enum {
	VD80_LIB_INTR_MASK_TRIGGER      = 0x1, /* Trigger interrupt */
	VD80_LIB_INTR_MASK_ACQUISITION  = 0x2, /* Data ready interrupt */
	VD80_LIB_INTR_MASK_ERR          = 0x4, /* Hardware error interrupt */
} vd80_intr_mask_t;

#define VD80_LIB_INTR_MASKS 3
#define VD80_LIB_INTR_MASK  0x7

/* ==================================================================== */
/* Open and close driver handle                                         */

int vd80OpenHandle(int dev);
void vd80CloseHandle(int fd);

vd80_err_t vd80GetModuleAddress(int fd, struct vd80_mod_addr_s *moad);

/* ==================================================================== */
/* Set/Get the users debug mask. Writing zero turns debug off.          */

vd80_err_t vd80SetDebug(int fd, vd80_debug_level_t  debug);
vd80_err_t vd80GetDebug(int fd, vd80_debug_level_t *debug);

/* ==================================================================== */
/* Reset is the equivalent to a VME bus reset. All settings and data on */
/* the module will be lost.                                             */

vd80_err_t vd80ResetMod(int fd);

/* ==================================================================== */
/* Calibrate requires that zero and Max volts are applied to an input   */
/* and the Adc value then read back. Once done the values obtained can  */
/* be used to calibrate the module.                                     */

vd80_err_t vd80GetAdcValue(int fd, uint32_t channel, int *adc);

/* ==================================================================== */
/* A channel has a state, the state must be IDLE to read back data.     */

vd80_err_t vd80GetState(int fd, vd80_state_t *state);

/* ==================================================================== */
/* Discover how many modules are installed.                             */

vd80_err_t vd80GetModuleCount(int fd, uint32_t *count);

/* ==================================================================== */
/* This returns the hardware, driver and DLL library versions.          */

vd80_err_t vd80GetVersion(int fd, struct vd80_version_s *version);

/* ==================================================================== */

vd80_err_t vd80SetTrigger(int fd, struct vd80_input_s *trigger);
vd80_err_t vd80GetTrigger(int fd, struct vd80_input_s *trigger);

/* ==================================================================== */

vd80_err_t vd80SetClock(int fd, struct vd80_input_s *clock);
vd80_err_t vd80GetClock(int fd, struct vd80_input_s *clock);

/* ==================================================================== */
/* Divide the clock frequency by an uint32_teger.                            */
/* One is added to the divisor, so 0 means divide by 1.                 */

vd80_err_t vd80SetClockDivide(int fd, uint32_t  divisor);
vd80_err_t vd80GetClockDivide(int fd, uint32_t *divisor);

/* ==================================================================== */
/* Buffers contain samples that occur before and after the trigger.     */
/* The buffer is allocated by the library in an optimum way for DMA.    */
/* Any byte ordering operations are performed by the library.           */
/* The user specifies the number of post trigger samples and the size   */
/* of the buffer. In some hardware such as the VD80 the DMA transfers   */
/* are carried out in chunks. Thus the exact triigger position within   */
/* the buffer is adjusted to the nearset size/post boundary. Hence the  */
/* actual number of pre/post trigger samples may vary.                  */
/* One buffer is allocated for each module/channel specified in the     */
/* module/channel masks.                                                */

vd80_err_t vd80SetPostSamples(int fd, uint32_t post);
vd80_err_t vd80GetPostSamples(int fd, uint32_t *post);

/* ==================================================================== */
/* Basic commands start, trigger and stop. Ane error may be returned if */
/* you try to control an external source.                               */

vd80_err_t vd80StrtAcquisition(int fd);
vd80_err_t vd80TrigAcquisition(int fd);
vd80_err_t vd80StopAcquisition(int fd);

/* ==================================================================== */
/* Connecting to zero disconnects from all previous connections         */

vd80_err_t vd80Connect(int fd, vd80_intr_mask_t imask);
vd80_err_t vd80GetConnect(int fd, vd80_intr_mask_t *imask);

vd80_err_t vd80SetTimeout(int fd, uint32_t tmout);
vd80_err_t vd80GetTimeout(int fd, uint32_t *tmout);

vd80_err_t vd80Wait(int fd, struct vd80_int_buf_s *intr);

/* ==================================================================== */
/* Get information about a buffer so that the data, if any, can be used */

vd80_err_t vd80GetBuffer(int fd, uint32_t chn, struct vd80_buffer_s *buffer);

/* ==================================================================== */
/* Set/Get the trigger analogue levels                                  */

vd80_err_t vd80SetTriggerLevel(int fd, struct vd80_analogue_trig_s *atrg);
vd80_err_t vd80GetTriggerLevel(int fd, struct vd80_analogue_trig_s *atrg);

/* ==================================================================== */
/* Get set trigger configuration params, delay and min pretrig samples  */

vd80_err_t vd80SetTriggerConfig(int fd, struct vd80_trig_config_s *ctrg);
vd80_err_t vd80GetTriggerConfig(int fd, struct vd80_trig_config_s *ctrg);

/* ==================================================================== */
/* Print error codes as a string                                        */

char *vd80ErrToStr(vd80_err_t error);

/* ==================================================================== */

vd80_err_t vd80RawIo(int fd, struct vd80_riob_s *rio, int io_dir);

#ifdef __cplusplus
}
#endif

#endif
