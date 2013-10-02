/**
 * =========================================================================================
 *
 * ICV196 (Digital IO) Device driver.
 * Definitions file
 * Julian Lewis BE/CO/HT Thu 14th Mar 2013
 *
 * =========================================================================================
 */

#ifndef _ICV196_H
#define _ICV196_H

#define DEBUG 0
#define TIMEOUT 1000

/*
 * =========================================================================================
 * Debug level
 */

typedef enum {
	ICV196_DEBUG_OFF,
	ICV196_DEBUG_IOCTL,
	ICV196_DEBUG_BUF
} icv196_debug_level_t;

/**
 * =========================================================================================
 * Parameter for module address supplied as insmod parameters
 */

struct icv196_mod_addr_s {
	uint32_t lun;     /* Logical unit number */
	uint32_t vec;     /* Interrupt vector */
	uint32_t vmeb;    /* ICV196 VME base address */
};

/**
 * =========================================================================================
 * Get compliation dates of the driver and library
 */

struct icv196_version_s {
	uint32_t drvrver; /* Driver  version (UTC compilation time) */
	uint32_t vhdlver; /* ICV196 firmware revision ID */
};

/*
 * =========================================================================================
 * Parameter for raw IO
 */

#define ICV196_IO_DIR_READ 1
#define ICV196_IO_DIR_WRITE 2

struct icv196_riob_s {
	uint8_t boffs;   /** Byte offset in map */
	uint8_t bsize;   /** The number of bytes to read/write */
	uint8_t *buffer; /** Pointer to user data area */
};

/*
 * =========================================================================================
 * Parameter for byte digital IO
 */

#define MAX_BYTES 12

struct icv196_digiob_s {
	uint16_t msk;             /** Mask of bytes to be written [0..0xFFF] */
	uint8_t val[MAX_BYTES];   /** Data area for read/write */
};

/*
 * =========================================================================================
 * Whats read back from waiting on interrupts
 */

struct icv196_int_buf_s {
	uint32_t src; /** 16-bit (0xFFFF) interrupt mask */
	uint32_t cnt; /** Interrupt total count */
	uint32_t dev; /** Interrupt device minor number */
};

/**
 * =========================================================================================
 * IOCTL definitions
 */

typedef enum {

	icv196SET_SW_DEBUG,            /** Set driver debug mode */
	icv196GET_SW_DEBUG,            /** Get driver debug mode */
	icv196GET_VERSION,             /** Get version date */
	icv196SET_TIMEOUT,             /** Set the read timeout value */
	icv196GET_TIMEOUT,             /** Get the read timeout value */
	icv196RESET,                   /** Reset the module re-establish connections */
	icv196CONNECT,                 /** Connect to interrupt */
	icv196GET_CONNECT,             /** Get 16-bit interrupt connection mask */

	icv196GET_DEVICE_COUNT,        /** Get the number of installed modules */
	icv196GET_DEVICE_ADDRESS,      /** Get the VME module base address */

	icv196RAW_READ,                /** Raw direct read from module address space */
	icv196RAW_WRITE,               /** Ditto for direct write */

	icv196BYTE_GET_OUTPUT,         /** Get 12-bit (0xFFF) byte output mask */
	icv196BYTE_READ,               /** Bytes read */
	icv196BYTE_SET_OUTPUT,         /** Set 12-bit (0xFFF) byte output mask */
	icv196BYTE_WRITE,              /** Bytes write */

	icv196LAST                     /** Internal use */

} icv196_ioctl_function_t;

/*
 * =========================================================================================
 * Set up the IOCTL numbers
 */

#define MAGIC 'V'

#define VIO(nr)      _IO(MAGIC,nr)
#define VIOR(nr,sz)  _IOR(MAGIC,nr,sz)
#define VIOW(nr,sz)  _IOW(MAGIC,nr,sz)
#define VIOWR(nr,sz) _IOWR(MAGIC,nr,sz)

/** TODO: Put in the correct structures when its not uint23_t */

#define ICV196_SET_SW_DEBUG       VIOWR(icv196SET_SW_DEBUG       ,uint32_t)                 /** Set driver debug mode */
#define ICV196_GET_SW_DEBUG       VIOWR(icv196GET_SW_DEBUG       ,uint32_t)                 /** Get driver debug mode */
#define ICV196_GET_VERSION        VIOWR(icv196GET_VERSION        ,struct icv196_version_s)  /** Get version date */
#define ICV196_SET_TIMEOUT        VIOWR(icv196SET_TIMEOUT        ,uint32_t)                 /** Set the read timeout value */
#define ICV196_GET_TIMEOUT        VIOWR(icv196GET_TIMEOUT        ,uint32_t)                 /** Get the read timeout value */
#define ICV196_RESET              VIOWR(icv196RESET              ,uint32_t)                 /** Reset the module re-establish connections */
#define ICV196_CONNECT            VIOWR(icv196CONNECT            ,uint32_t)                 /** 16-Bit interrupt mask */
#define ICV196_GET_CONNECT        VIOWR(icv196GET_CONNECT        ,uint32_t)                 /** Get 16-Bit interrupt mask */

#define ICV196_GET_DEVICE_COUNT   VIOWR(icv196GET_DEVICE_COUNT   ,uint32_t)                 /** Get the number of installed modules */
#define ICV196_GET_DEVICE_ADDRESS VIOWR(icv196GET_DEVICE_ADDRESS ,struct icv196_mod_addr_s) /** Get the VME module base address */

#define ICV196_RAW_READ           VIOWR(icv196RAW_READ           ,struct icv196_riob_s)     /** Raw direct read from module address space */
#define ICV196_RAW_WRITE          VIOWR(icv196RAW_WRITE          ,struct icv196_riob_s)     /** Ditto for direct write */

#define ICV196_BYTE_GET_OUTPUT    VIOWR(icv196BYTE_GET_OUTPUT    ,uint32_t)                 /** Get 12 bit (0xFFF) output bytes mask */
#define ICV196_BYTE_READ          VIOWR(icv196BYTE_READ          ,struct icv196_digiob_s)   /** Byte read from module address space */
#define ICV196_BYTE_SET_OUTPUT    VIOWR(icv196BYTE_SET_OUTPUT    ,uint32_t)                 /** Set 12 bit (0xFFF) output bytes mask */
#define ICV196_BYTE_WRITE         VIOWR(icv196BYTE_WRITE         ,struct icv196_digiob_s)   /** Byte write to module  */

#endif /* _ICV196_H */
