#ifndef __VMODTTL_H__
#define __VMODTTL_H__

#ifdef __cplusplus
extern "C" {
#endif

	/************************************************************************/
	/* DEFINES								*/
	/************************************************************************/
#define CHAN_A		0
#define CHAN_B		1
#define CHAN_C		2

#define A_CHAN_IN	0x00
#define A_CHAN_OUT	0x01
#define B_CHAN_IN	0x00
#define B_CHAN_OUT  	0x02
#define C_CHAN_IN	0x00
#define C_CHAN_OUT  	0x04
#define VMODTTL_O   	0x08

#define A_CHAN_OPEN_COLLECTOR	0x01
#define B_CHAN_OPEN_COLLECTOR	0x02
#define C_CHAN_OPEN_COLLECTOR	0x04

#define DEFAULT_DELAY 1 /* Used to wait for a while to allow the Z8536 to perform the read/write operations */

	/************************************************************************/
	/* IOCTL commands							*/
	/************************************************************************/
#define VMODTTL_READ_CHAN	100	/* function read (8/4 bit) */
#define VMODTTL_WRITE_CHAN	101	/* function write */
#define VMODTTL_CONFIG		112

struct vmodttlarg {
	int 	dev;
	int	val;
};

struct vmodttlconfig {
	int		io_flag;
	int 		us_pulse;
	int		open_collector;
	short int	invert;
};


#define VMODTTL_MAX_BOARDS 16		/* max. # of boards supported   */
#define MAXNDEV	 	  4	   	/* device count A, B, C, TIMER	*/
#define CARRIER_NAME	  20

#define VMODTTL_PORTC	0
#define VMODTTL_PORTB 	2
#define VMODTTL_PORTA 	4
#define VMODTTL_CONTROL  6

struct vmodttl {
	int             lun;            /** logical unit number */
	char            *cname;         /** carrier name */
	int             carrier;        /** supporting carrier */
	int             slot;           /** slot we're plugged in */
	unsigned long   address;        /** address space */
	int             is_big_endian;  /** endianness */
};

#ifdef __cplusplus
}
#endif

#endif /* __VMODTTL_H__ */

