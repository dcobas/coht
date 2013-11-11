/* ==================================================================== */
/* ICV196 Library                                                       */
/* Julian Lewis Tue 2nd July 2013                 e                     */
/* ==================================================================== */

#ifndef ICV196_LIB_LIB
#define ICV196_LIB_LIB

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "icv196.h"  /* Driver API */

/* ==================================================================== */
/* Some standard error codes                                            */

typedef enum {
	ICV196_LIB_ERR_SUCCESS,  /* All went OK, No error                 */
	ICV196_LIB_ERR_TIMEOUT,  /* Timeout in wait                       */
	ICV196_LIB_ERR_IO,       /* IO or BUS error                       */
	ICV196_LIB_ERR_ERRORS    /* Total errors */
} icv196_err_t;

/* ==================================================================== */
/* Open and close driver handle                                         */

/**
 * @brief Open the icv196 driver
 * @param The given minor device number (module)
 * @return The posative file descriptor, or zero on error
 */

int icv196Open(int minor);

/**
 * @brief Close the icv196 driver file handle
 * @param The file descriptor you got back from calling icv196Open
 */

void icv196Close(int fd);

/* ==================================================================== */
/* Get module address for given file descriptor                         */

/**
 * @brief Get the VME base address and interrupt vector
 * @param The file descriptor you got back from calling icv196Open
 * @param The structure address where the information will be stored
 * @return The library error code, 0=OK else icv196ErrToStr to get string
 */

icv196_err_t icv196GetModuleAddress(int fd, struct icv196_mod_addr_s *moad);

/* ==================================================================== */
/* Set/Get the users debug mask. Writing zero turns debug off.          */
/* There are 3 driver debug levels: See dmesg                           */
/*    0=Off, 1=ioctl calls, 2=caller parameters                         */

/**
 * @brief Set the driver debug level
 * @param The file descriptor you got back from calling icv196Open
 * @param The level to set [0..2]
 * @return The library error code, 0=OK else icv196ErrToStr to get string
 */

icv196_err_t icv196SetDebug(int fd, uint32_t debug);

/**
 * @brief Get the current driver debug level
 * @param The file descriptor you got back from calling icv196Open
 * @param The address where the level will be stored
 * @return The library error code, 0=OK else icv196ErrToStr to get string
 */

icv196_err_t icv196GetDebug(int fd, uint32_t *debug);

/* ==================================================================== */
/* Reset is the equivalent to a VME bus reset. All settings and data on */
/* the module will be lost.                                             */

/**
 * @brief Reset the icv196 hardware to the basic state
 * @param The file descriptor you got back from calling icv196Open
 * @return The library error code, 0=OK else icv196ErrToStr to get string
 */

icv196_err_t icv196Reset(int fd);

/* ==================================================================== */
/* Discover how many modules are installed.                             */

/**
 * @brief Get the total number of icv196 modules installed with driver
 * @param The file descriptor you got back from calling icv196Open
 * @param The address where the device count will be stored
 * @return The library error code, 0=OK else icv196ErrToStr to get string
 */

icv196_err_t icv196GetDeviceCount(int fd, uint32_t *count);

/* ==================================================================== */
/* This returns the hardware and driver versions.                       */

/**
 * @brief Get the driver compilation date and hardware version code
 * @param The file descriptor you got back from calling icv196Open
 * @param The address of the structure where version data will be stored
 * @return The library error code, 0=OK else icv196ErrToStr to get string
 */

icv196_err_t icv196GetVersion(int fd, struct icv196_version_s *version);

/* ==================================================================== */
/* Connecting to zero disconnects from all previous connections         */

/**
 * @brief It is possible to asociate interrupts with the first 16 lines
 * @param The file descriptor you got back from calling icv196Open
 * @param The 16-bit interrupt mask [0..0xFFFF]
 * @return The library error code, 0=OK else icv196ErrToStr to get string
 */

icv196_err_t icv196Connect(int fd, uint32_t imask);

/**
 * @brief Get the connected interrupts lines mask
 * @param The file descriptor you got back from calling icv196Open
 * @param The address of the 16-bit interrupt mask
 * @return The library error code, 0=OK else icv196ErrToStr to get string
 */

icv196_err_t icv196GetConnect(int fd, uint32_t *imask);

/* ==================================================================== */
/* Get/Set timeouts in milliseconds, timeout of zero means no tiomeout  */

/**
 * @brief Set the timeout value in milliseconds
 * @param The file descriptor you got back from calling icv196Open
 * @param The timeout 0=No timeout, else milliseconds
 * @return The library error code, 0=OK else icv196ErrToStr to get string
 */

icv196_err_t icv196SetTimeout(int fd, uint32_t tmout);

/**
 * @brief Get the timeout value in milliseconds
 * @param The file descriptor you got back from calling icv196Open
 * @param Address of the timeout 0=No timeout, else milliseconds
 * @return The library error code, 0=OK else icv196ErrToStr to get string
 */

icv196_err_t icv196GetTimeout(int fd, uint32_t *tmout);

/* ==================================================================== */
/* Wait for an interrupt or timeout                                     */
/* N.B All connections for all devices are seen on all file descriptors */

/**
 * @brief Wait for interrupt, posative edge is seen on a connected line
 * @param The file descriptor you got back from calling icv196Open
 * @param Address of the interrupt structure
 * @return The library error code, 0=OK else icv196ErrToStr to get string
 */

icv196_err_t icv196Wait(int fd, struct icv196_int_buf_s *intr);

/* ==================================================================== */
/* Set the 12 byte io directions via a 12-bit mask                      */
/* N.B Bytes 0-1 can not be outputs if the are connected to interrupts  */

/**
 * @brief Set the digital io byte directions
 * @param The file descriptor you got back from calling icv196Open
 * @param The 12-bit [0..0xFFF] output mask, bit set=output else input
 * @return The library error code, 0=OK else icv196ErrToStr to get string
 */

icv196_err_t icv196SetGroupsAsOutput(int fd, uint32_t omsk);

/**
 * @brief Get the digital io byte directions
 * @param The file descriptor you got back from calling icv196Open
 * @param Address og he 12-bit output mask, bit set=output else input
 * @return The library error code, 0=OK else icv196ErrToStr to get string
 */

icv196_err_t icv196GetGroupsAsOutput(int fd, uint32_t *omsk);

/* ==================================================================== */
/* Do the input/output of the 12 digital io bytes                       */

/**
 * @brief Set output byte values according to the given giob
 * @param The file descriptor you got back from calling icv196Open
 * @param The output array to be set
 * @return The library error code, 0=OK else icv196ErrToStr to get string
 */

icv196_err_t icv196SetGroups(int fd, struct icv196_digiob_s giob);

/**
 * @brief Get input/output byte values according to the given giob
 * @param The file descriptor you got back from calling icv196Open
 * @param The input array to be read, including outputs
 * @return The library error code, 0=OK else icv196ErrToStr to get string
 */

icv196_err_t icv196GetGroups(int fd, struct icv196_digiob_s *giob);

/* ==================================================================== */
/* Do raw hardware IO, WARNING using this will upset the icv196 state   */
/* machine, so only use this if you want to do something exotic !       */

/**
 * @brief Do raw input output to the icv196 address map
 * @param The file descriptor you got back from calling icv196Open
 * @param The rwa io data structure used in read/write transfers
 * @param The io direction 1=Read, 2=Write
 * @return The library error code, 0=OK else icv196ErrToStr to get string
 */

icv196_err_t icv196RawIo(int fd, struct icv196_riob_s *riob, int io_dir);

/* ==================================================================== */
/* Print error codes as a string                                        */

/**
 * @brief Convert error code to string using last errno code
 * @param The error code you want to convert
 * @return The string, its never NULL
 */

char *icv196ErrToStr(icv196_err_t error);

#ifdef __cplusplus
}
#endif

#endif
