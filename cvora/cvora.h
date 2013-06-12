/**
 * =========================================================================================
 *
 * Implement driver to do basic VME IO
 * Definitions file
 * Julian Lewis BE/CO/HT Tue 19th October 2010
 *
 * =========================================================================================
 * Basic driver API for raw VME IO
 */

#ifndef VMEIO
#define VMEIO

#define DEBUG 0
#define TIMEOUT 1000

/*
 * Determin if we are on a 64 bit kernel
 * BITS_PER_LONG is defined by types.h
 */

#include <asm/types.h>

#if BITS_PER_LONG == 64
#ifndef __64BIT
#define __64BIT
#endif
#endif

#define	DRV_MAX_DEVICES	32

/**
 * Connect and read buffer
 * The interrupt counter is the total number of interrupts
 * so far on the lun, so you can know how many you missed.
 */

struct vmeio_read_buf_s {
   int logical_unit;    /** Logical unit number for interrupt */
   int interrupt_mask;  /** Interrupt enable/source mask */
   int interrupt_count; /** Current interrupt counter value */
};

/**
 * Parameter for get window
 */

struct vmeio_get_window_s {

   int lun;     /* Logical unit number */
   int lvl;     /* Interrupt level */
   int vec;     /* Interrupt vector */
   int vme1;    /* First VME base address */
   int vme2;    /* Second VME base address or zero */

   int amd1;    /* First address modifier */
   int amd2;    /* Second address modifier or zero */
   int dwd1;    /* First data width */
   int dwd2;    /* Second data width or zero */
   int win1;    /* First window size */
   int win2;    /* Second window size or zero */
   int nmap;    /* No map window flag, 1=DMA only */
   int isrc;    /* Offset of isrc in vme1 to be read in the isr */
};

/**
 * Get compliation dates of the driver and library
 */

struct vmeio_version_s {
   int driver;  /* Driver  version (UTC compilation time) */
   int library; /* Library version (UTC compilation time) */
};

/*
 * Parameter for raw IO
 */

#define vmeioMAX_BUF 8192

#ifdef __64BIT
struct vmeio_riob_s {
   int winum;    /** Window number 1..2 */
   int offset;   /** Byte offset in map */
   int bsize;    /** The number of bytes to read */
   void *buffer; /** Pointer to data area */
};
#else
struct vmeio_riob_s {
   int winum;    /** Window number 1..2 */
   int offset;   /** Byte offset in map */
   int bsize;    /** The number of bytes to read */
   void *buffer; /** Pointer to data area */
   int  compat;  /** Pack out size to at least 64 bits */
};
#endif

/*
 * Enumerate IOCTL functions
 */

#define PRANDOM 0

typedef enum {

   vmeioFIRST = PRANDOM,

   vmeioSET_DEBUG,     /** Debug level 0..4 */
   vmeioGET_DEBUG,

   vmeioGET_VERSION,   /** Get driver and library dates */

   vmeioSET_TIMEOUT,   /** Timeout in milliseconds */
   vmeioGET_TIMEOUT,

   vmeioGET_DEVICE,    /** Get the window described in struct vmeio_get_window_s */
   vmeioRAW_READ,      /** Raw read VME registers */
   vmeioRAW_WRITE,     /** Raw write VME registers */
   vmeioRAW_READ_DMA,  /** Raw read VME registers */
   vmeioRAW_WRITE_DMA, /** Raw write VME registers */

   vmeioSET_DEVICE,    /** Very dangerous IOCTL, not for users */

   vmeioLAST           /** For range checking (LAST - FIRST) */

} vmeio_ioctl_function_t;

#define vmeioIOCTL_FUNCTIONS (vmeioLAST - vmeioFIRST - 1)

/*
 * Set up the IOCTL numbers
 */

#define MAGIC 'V'

#define VIO(nr)      _IO(MAGIC,nr)
#define VIOR(nr,sz)  _IOR(MAGIC,nr,sz)
#define VIOW(nr,sz)  _IOW(MAGIC,nr,sz)
#define VIOWR(nr,sz) _IOWR(MAGIC,nr,sz)

#define VMEIO_GET_DEBUG     VIOR(vmeioGET_DEBUG,      int)
#define VMEIO_SET_DEBUG     VIOW(vmeioSET_DEBUG,      int)
#define VMEIO_GET_VERSION   VIOR(vmeioGET_VERSION,    int)
#define VMEIO_GET_TIMEOUT   VIOR(vmeioGET_TIMEOUT,    int)
#define VMEIO_SET_TIMEOUT   VIOW(vmeioSET_TIMEOUT,    int)
#define VMEIO_GET_DEVICE    VIOR(vmeioGET_DEVICE,     struct vmeio_get_window_s)
#define VMEIO_RAW_READ      VIOWR(vmeioRAW_READ,      struct vmeio_riob_s)
#define VMEIO_RAW_WRITE     VIOWR(vmeioRAW_WRITE,     struct vmeio_riob_s)
#define VMEIO_RAW_READ_DMA  VIOWR(vmeioRAW_READ_DMA,  struct vmeio_riob_s)
#define VMEIO_RAW_WRITE_DMA VIOWR(vmeioRAW_WRITE_DMA, struct vmeio_riob_s)
#define VMEIO_SET_DEVICE    VIOW(vmeioGET_DEVICE,     struct vmeio_get_window_s)

#endif
