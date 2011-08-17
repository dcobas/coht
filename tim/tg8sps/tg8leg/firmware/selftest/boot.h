/**************************************************************************/
/* Constants and structures needed for downloading procedure              */
/**************************************************************************/

#ifndef BOOT_H
#define BOOT_H

#define _MVME_167_ 0  /* 1=MVME; 0=PPC */
#define _PPC_      1  /* 1=PPC platform */
#define _PS_       0  /* 1=the PS version; 0=the SL version */
#define _SL_       1 /* 1=the SL version; 0=the PS version */

#ifdef BE_UNFAIR
#define DEBUG      1  /* 1=debugg mode ON; 0=debugg mode OFF */
#endif /* BE_UNFAIR */

/*************************************************************************/
/* General type definitions                                              */
/*************************************************************************/

#ifndef TG8_TYPES
#define TG8_TYPES
typedef unsigned char  Uchar;
typedef unsigned char  Byte;

typedef unsigned short Ushort;
typedef unsigned short Word;

typedef unsigned int   Uint;
typedef unsigned int   DWord;
#endif

/***************************************************************/
/* Tg8's accelerators (machines) with their own timing systems */
/***************************************************************/

typedef enum { Tg8NO_MACHINE=0,
	       Tg8LEP,	/* 1 */
	       Tg8SPS,	/* 2 */
	       Tg8CPS,	/* 3 */
	       Tg8PSB,	/* 4 */
	       Tg8LPI,	/* 5 */
	       Tg8MACHINES	/* Total number of machines +1 */
	     } Tg8Machine;


/***** The Tg8's Events and headers (as used by the SL timing system) ****/

typedef enum {
	Tg8MILLISECOND_HEADER = 0x01,
	Tg8SECOND_HEADER = 0x02,
	Tg8MINUTE_HEADER = 0x03,
	Tg8HOUR_HEADER   = 0x04,
	Tg8DAY_HEADER    = 0x05,
	Tg8MONTH_HEADER  = 0x06,
	Tg8YEAR_HEADER   = 0x07,
	Tg8TIME_HEADER = 0x08,
	Tg8DATE_HEADER = 0x09
  } Tg8EventHeader;

typedef enum {
	Tg8SSC_HEADER	       = 0, /* Start Super Cycle for machine */
	Tg8SIMPLE_EVENT_HEADER = 1, /* Programmable event for the SPS (Simple event) */
	Tg8SUPER_CYCLE_HEADER  = 2, /* Programmable event for the LEP or */
				    /* External Event for the SPS ?      */
	Tg8TELEGRAM_HEADER      = 3, /* Machine telegram event */
	Tg8psSIMPLE_EVENT_HEADER= 4, /* The PS acc. simple event */
	Tg8psTCU_EVENT_HEADER   = 5, /* The PS acc. TCU (Timing Control Unit) event */
	Tg8OLD_TG1_HEADER = 0xF      /* Events for the Tg1 modukes */
  } Tg8MachineEventHeader; /* Real event header looks like <Machine><Header> byte value */

/*******************/
/* Firmware status */
/*******************/

#define Tg8RUNNING          0x7777
#define Tg8BOOT             0x3842
#define Tg8332Bug           0x3844
#define Tg8DOWNLOAD         0x3856
#define Tg8DOWNLOAD_PENDING 0x4678

#define Tg8DWNLD_FLAG_READY 0x3456
#define Tg8DWNLD_FLAG_DONE  0x4657
#define Tg8LBUF_SIZE        (0x7fc-20)/2

typedef struct {
  short IoDirection; /* not used by the down loader */
  short DoneFlag;    /* not used too */
  short Firmware_status;
  short Exception_vector;
  short Exception_address[2];
  short Loader_done_flag;
  short Address[2]; /* Bad alignment for 'int' is used ! */
  short Length;
  short LdBuffer[Tg8LBUF_SIZE];
} Tg8LoadDpm;

/***************************************************************************/
/* The firmware object - executable program for the MPC                    */
/***************************************************************************/

#define Tg8FIRMWARE_OBJECT_SIZE 0x3000 /* Max. length of executable file */

typedef struct {
  Uint	Size;
  Uint	StartAddress;
  short Object[Tg8FIRMWARE_OBJECT_SIZE];
} Tg8FirmwareObject;

#endif

/* eof */
