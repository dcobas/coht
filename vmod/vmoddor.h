

#ifndef __VMODDOR_H__
#define __VMODDOR_H__


#define VMODDOR_MAX_DEVICES 16
#define VMODDOR_SIZE_CARRIER_NAME 20

/* 
 *	IOctl Operations
 *
 */
#define VMODDOR_WRITE	0
#define VMODDOR_READ	1

struct vmoddor_dev
{
	int		created;        /* flag initialize      */
	int		dev;
	int		lun;
	int		carrier;
	unsigned char	*cname;		/* carrier name */
	int		slot;
	int		is_big_endian;
	int		io_flag;
	unsigned long	handle;
};

#endif /* __VMODDOR_H__ */
