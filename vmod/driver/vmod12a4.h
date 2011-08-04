#ifndef _VMOD12A4_H_
#define _VMOD12A4_H_



/* VMOD 12A2 channels */
#define VMOD_12A2_CHANNELS 	2
#define VMOD_12A2_CHANNEL0 	0x0
#define VMOD_12A2_CHANNEL1 	0x2

/* VMOD 12A4 channels */
#define VMOD_12A4_CHANNELS 	4
#define VMOD_12A4_CHANNEL0 	VMOD_12A2_CHANNEL0 
#define VMOD_12A4_CHANNEL1 	VMOD_12A2_CHANNEL1 
#define VMOD_12A4_CHANNEL2 	0x4
#define VMOD_12A4_CHANNEL3 	0x6

/* driver internals, device table size and ioctl commands */

#define	VMOD12A4_MAX_MODULES	64

#define	VMOD12A4_IOC_MAGIC	'Z'
#define VMOD12A4_IOCSELECT 	_IOW(VMOD12A4_IOC_MAGIC, 1, struct vmod12a4_select)
#define	VMOD12A4_IOCPUT		_IOW(VMOD12A4_IOC_MAGIC, 2, int)

struct vmod12a4_output {
	int	channel;
	int	value;
};
#endif /* _VMOD12A4_H_ */
