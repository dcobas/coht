/**
 * User library for cvora
 * Julian Lewis BE/CO/HT Tue 19th October 2010
 * David Cobas BE/CO/HT May 9th 2011
 */

#ifndef _LIBCVORA_H
#define _LIBCVORA_H

#ifdef __cplusplus
extern "C"
{
#endif

/** @cond */
/** register offsets */
#define CVORA_CONTROL		0x0
#define CVORA_MEMORY_POINTER	0x4
#define CVORA_MODE		0x8
#define CVORA_CHANNEL		0xc
#define CVORA_FREQUENCY		0x10
#define	CVORA_DAC		0x14
#define	CVORA_MEMORY		0x20

/** positions and masks within control register */
#define CVORA_POLARITY_BIT	0
#define CVORA_POLARITY_MASK	1
#define CVORA_MODULE_ENABLE_BIT	1
#define CVORA_INT_ENABLE_BIT	2
#define CVORA_SOFT_START_BIT	3
#define CVORA_SOFT_STOP_BIT	4
#define CVORA_SOFT_REARM_BIT	5
#define CVORA_COUNTER_OVERFLOW	6
#define CVORA_RAM_OVERFLOW	7
#define CVORA_VECTOR_BIT	8
#define CVORA_VECTOR_MASK	(0xff << CVORA_VECTOR_BIT)
#define CVORA_VERSION_BIT	16
#define CVORA_VERSION_MASK	(0xffff << CVORA_VERSION_BIT)

/** positions and masks withing mode register */
#define CVORA_MODE_BIT		0
#define CVORA_MODE_MASK		0x7

/** memory boundaries */
#define CVORA_MEM_MIN  0x20
#define CVORA_MEM_MAX  0x7FFFC
#define CVORA_MEM_SIZE (CVORA_MEM_MAX-CVORA_MEM_MIN)

/** polarity bit */
#define	POSITIVE	1
#define	NEGATIVE	0
/** @endcond */

/** cvora modes of operation */
enum cvora_mode {
	cvora_reserved,		/**< reserved but parallel input for the moment */
	cvora_optical_16,       /**< one optical input 16 bits - Input 2 is ignored */
	cvora_copper_16,        /**< one copper Input 16 bits */
	cvora_btrain_counter,   /**< Btrain counters */
	cvora_parallel_input,   /**< parallel input */
	cvora_optical_2_16,     /**< two optical inputs 16 bits */
	cvora_copper_2_16,      /**< two copper Inputs 16 bits */
	cvora_serial_32,        /**< 32 Serial Inputs on rear panel (P2 connector). */
};

/**
 * @brief Initialize cvora user library
 * @param lun logical unit number
 * @return file descriptor, or < 0 if error
 */

int cvora_init(int lun);

/**
 * @brief close cvora file handle
 * @param fd file descriptor
 * @return 0 if success, < 0 if error
 */

int cvora_close(int fd);

/**
 * @brief get version of the module
 * @param fd  file descriptor returned from cvora_init
 * @param version	module version
 * @return 0 if OK, < 0 if error
 */
int cvora_get_version(int fd, int *version);

/**
 * @brief get mode
 * @param fd  file descriptor returned from cvora_init
 * @param mode one of the CVORA modes of operation
 * @return 0 if OK, < 0 if error
 */
int cvora_get_mode(int fd, enum cvora_mode *mode);

/**
 * @brief set mode
 * @param fd  file descriptor returned from cvora_init
 * @param mode one of the CVORA modes of operation
 * @return 0 if OK, < 0 if error
 */
int cvora_set_mode(int fd, enum cvora_mode mode);

/**
 * @brief read status register (DEPRECATED)
 * @param fd  file descriptor returned from cvora_init
 * @param status a bitmask of status bits
 * @return 0 if OK, < 0 if error
 */
int cvora_get_hardware_status(int fd, unsigned int *status);

/**
 * @brief set interrupt wait timeout
 * @param fd  file descriptor returned from cvora_init
 * @param timeout timeout value in milliseconds
 * @return 0 if OK, < 0 if error
 */
int cvora_set_timeout(int fd, int timeout);

/**
 * @brief get interrupt wait timeout
 * @param fd  file descriptor returned from cvora_init
 * @param timeout timeout value in milliseconds
 * @return 0 if OK, < 0 if error
 */
int cvora_get_timeout(int fd, int *timeout);

/**
 * @brief set pulse polarity
 * @param fd  file descriptor returned from cvora_init
 * @param polarity polarity
		(POSITIVE = 1 like in CTRx,
		 NEGATIVE = 0 * like in TG8)
 * @return 0 if OK, < 0 if error
 */
int cvora_set_pulse_polarity(int fd, int polarity);

/**
 * @brief get pulse polarity
 * @param fd  file descriptor returned from cvora_init
 * @param polarity returned polarity
		(POSITIVE = 1 like in CTRx,
		 NEGATIVE = 0 * like in TG8)
 * @return 0 if OK, < 0 if error
 */
int cvora_get_pulse_polarity(int fd, int *polarity);

/**
 * @brief enable module
 * @param fd  file descriptor returned from cvora_init
 * @return 0 if OK, < 0 if error
 */
int cvora_enable_module(int fd);

/**
 * @brief disable module
 * @param fd  file descriptor returned from cvora_init
 * @return 0 if OK, < 0 if error
 */
int cvora_disable_module(int fd);

/**
 * @brief enable interrupts
 * @param fd  file descriptor returned from cvora_init
 * @return 0 if OK, < 0 if error
 */
int cvora_enable_interrupts(int fd);

/**
 * @brief disable interrupts
 * @param fd  file descriptor returned from cvora_init
 * @return 0 if OK, < 0 if error
 */
int cvora_disable_interrupts(int fd);

/**
 * @brief wait for end of sample interrupt.
 * This function blocks until an interrupt occurs, which signals a stop
 * acquisition event
 * @param fd  file descriptor returned from cvora_init
 * @return 0 if OK, < 0 if error
 */
int cvora_wait(int fd);

/**
 * @brief read memory buffer samples size
 * @param fd  file descriptor returned from cvora_init
 * @param memsz available memory size in bytes
 * @return 0 if OK, < 0 if error
 */
int cvora_get_sample_size(int fd, int *memsz);

/**
 * @brief Read memory sample buffer
 * @param fd  file descriptor returned from cvora_init
 * @param maxsz max byte size to read
 * @param actsz actual byte size read
 * @param buf pointer to data area
 * @return 0 if OK, < 0 if error
 */
int cvora_read_samples(int fd, int maxsz, int *actsz, unsigned int *buf);

/**
 * @brief Issue a software start
 * @param fd  file descriptor returned from cvora_init
 * @return 0 if OK, < 0 if error
 */
int cvora_soft_start(int fd);

/**
 * @brief Issue a software stop
 * @param fd  file descriptor returned from cvora_init
 * @return 0 if OK, < 0 if error
 */
int cvora_soft_stop(int fd);

/**
 * @brief Issue a software soft rearm
 * @param fd  file descriptor returned from cvora_init
 * @return 0 if OK, < 0 if error
 */
int cvora_soft_rearm(int fd);

/**
 * @brief read DAC
 * @param fd  file descriptor returned from cvora_init
 * @param dacv dac value
 * @return 0 if OK, < 0 if error
 */
int cvora_get_dac(int fd, unsigned int *dacv);

/**
 * @brief read clock frequency
 * @param fd  file descriptor returned from cvora_init
 * @param freq frequency value
 * @return 0 if OK, < 0 if error
 */
int cvora_get_clock_frequency(int fd, unsigned int *freq);

/**
 * @brief set plot input
 * @param fd  file descriptor returned from cvora_init
 * @param plti plot input P2 channel routed to output DAC 1..32
 * @return 0 if OK, < 0 if error
 */
int cvora_set_plot_input(int fd, unsigned int plti);

/**
 * @brief read parallel channels mask
 * @param fd  file descriptor returned from cvora_init
 * @param chans value
 * @return 0 if OK, < 0 if error
 */
int cvora_get_channels_mask(int fd, unsigned int *chans);

/**
 * @brief set parallel channels mask
 * @param fd  file descriptor returned from cvora_init
 * @param chans value
 * @return 0 if OK, < 0 if error
 */
int cvora_set_channels_mask(int fd, unsigned int chans);


#ifdef __cplusplus
}
#endif
#endif	/* _LIBCVORA_H */

