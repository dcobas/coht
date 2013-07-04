#ifndef _CVORB_H_
#define _CVORB_H_

/* this file is shared by the driver and the library */
#if !defined(__KERNEL__)
#include <stdint.h>
#else
#include <linux/types.h>
#endif

/* CVORB is a 16 channels module, identified with numbers in the range [1,16] */
#define CVORB_MAX_CH_NR 16

/* CVORB is a 64 functions per channel, identified with numbers in the range [1,64] */
#define CVORB_MAX_FCT_NR 64

/* MAX function vectors amount */
#define CVORB_MAX_VECTOR 680

/* Size of the char * returned by cvorg_get_hw_version: '0x' + uint32_t + '\n' + '\0' = 12 bytes */
#define CVORB_HW_VER_SIZE 12

/**
 * @brief Software pulses
 */
enum cvorb_trigger {
        CVORB_START,
        CVORB_STOP,
        CVORB_EVT_START,
        CVORB_EVT_STOP
};

/**
 * @brief Input (triggers) polarity
 */
enum cvorb_input_polarity {
        CVORB_POS_PULSE,
        CVORB_NEG_PULSE
};

/**
 * @brief Sub Module status bit mask
 */
#define CVORB_SUBMOD_READY             0x1
#define CVORB_SUBMOD_SRAM_BUSY         0x2
#define CVORB_SUBMOD_BUSY              0x4
#define CVORB_SUBMOD_PAUSE             0x8

/**
 * @brief Channel status bit mask
 */
#define CVORB_CH_BUSY                   0x1
#define CVORB_CH_FCN_PAUSED             0x2
#define CVORB_CH_SERIAL_LINK_ERR        0x10

/**
 * @brief Function Vectors
 *
 * * This structure is used to load or read current function to/from the module
 *   by cvorb_function_set() and cvorb_function_get()\n
 * * An array of "cvorb_vector" is passed between the user and kernel
 *   space.\n
 * * When writing a new vector (using cvorb_function_set()) -- Element[0]
 *   should @b ALWAYS have t == 0 \n
 * * Array should not be bigger then @ref CVORB_MAX_VECTOR (for Element[0])
 *   elements long \n
 */
struct cvorb_vector_fcn {
        double t; /**< values of time from function start in ms */
        unsigned short v; /**< amplitudes in corresponding physical unit */
};

/** @cond INTERNAL */
/** All the following declarations between the tags cond and endcond will be ignored by doxygen */

#define CVORB_FCT_HEADER_SZ 3                   /* Hw function header size in 16 bits word */
#define CVORB_VCT_SZ 3                          /* Hw function vector size in 16 bits word */
#define CVORB_FOOTER_SZ 6                       /* HW function footer size in 16 bits word */
#define CVORB_MIN_STEP_SIZE 5                   /* HW function minimum step size is 5 us */
#define CVORB_MAX_STEPS ((1<<16)-1)             /* HW function MAX number of steps per vector*/
#define CVORB_MAX_STEP_SIZE ((1<<15)-1)         /* HW function MAX step size */
/* HW function MAX possible time between 2 vectors with min step size (327675 us)  */
#define CVORB_MAX_TIME_MIN_STEP (CVORB_MAX_STEPS*CVORB_MIN_STEP_SIZE)

/** this structure is exchanged between the library and the driver to set/get a function */
struct cvorb_hw_fcn {
        uint16_t submodulenr;   /** sub module [0, 1] */
        uint16_t channr;        /** channel [0 - 7] */
        uint16_t fcnnr;         /** function [0 - 63] */
        uint16_t n_vector;      /** number of vectors */
        uint16_t hw_fcn_size32; /** hw_fcn buffer size in 32 bits word */
        uint32_t *hw_fcn;       /** HW representation of the vector table given by the client */
};

/* IOCTL commands */
#define CVORB_IOCTL_SET_FCT    _IOW('b', 0, struct cvorb_hw_fcn)
#define CVORB_IOCTL_GET_FCT    _IOW('b', 1, struct cvorb_hw_fcn)

/** @endcond */

#endif /* _CVORB_H_ */
