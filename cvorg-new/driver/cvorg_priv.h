/*
 * cvorg_priv.h
 *
 * private CVORG definitions
 */

#ifndef _CVORG_PRIV_H_
#define _CVORG_PRIV_H

#include <linux/device.h>
#include <linux/cdev.h>
#include <asm-generic/iomap.h>

#include <cvorg.h>
#include <cvorg_hw.h>
#include <ad9516.h>
#include <ad9516_priv.h>

/* define this to debug I/O accesses */
#define CVORG_MAX_BOARDS 	16

/* Table to save the pointers to all cvorg devices */
extern struct cvorg* cvorg_table[CVORG_MAX_BOARDS];

/* Needed to create the sysfs files */
extern struct class *cvorg_class;
extern dev_t cvorg_devno;


/**
 * struct cvorg_channel - internal channel structure
 * @inpol:	input polarity
 * @outoff:	analog output offset
 * @outgain:	analog output gain
 * @reg_offset:	offset of the channel's registers within the @parent module.
 * @out_enabled:Set to 1 when the output is enabled. 0 otherwise.
 * @parent:	parent CVORG module
 */
struct cvorg_channel {
	struct kobject  kobj;
	int		inpol;
	int		outoff;
	int		outgain;
	unsigned int	reg_offset;
	int		out_enabled;
	int		chan_nr;
	struct cvorg	*parent;
};

/**
 * struct cvorg - description of a CVORG module
 * @hw_rev:	hardware revision number
 * @lock:	module's lock
 * @owner:	struct file that owns the module
 * @iomap:	kernel virtual address of the module's mapped I/O region
 * @pll:	PLL configuration
 * @channels:	array containing the contexts of the two channels
 */
struct cvorg {
	struct device 		*dev;
	struct cdev             cdev;
	int			n_channels;
	uint32_t		pcb_id;
	uint32_t		hw_rev;
	struct mutex		lock;
	void			*iomap;
	struct ad9516_pll	pll;
	struct cvorg_channel	channels[CVORG_CHANNELS];
	uint32_t		irq;
	uint32_t		irq_level;
	uint32_t		lun;
};

static inline uint32_t __cvorg_readw(struct cvorg *cvorg, unsigned int offset)
{
	return ioread32be(cvorg->iomap + offset);
}

static inline uint32_t cvorg_readw(struct cvorg *cvorg, unsigned int offset)
{
	uint32_t value;

	value = __cvorg_readw(cvorg, offset);
	return value;
}

static inline void
__cvorg_writew(struct cvorg *cvorg, unsigned int offset, uint32_t value)
{
	iowrite32be(value, cvorg->iomap + offset);
}

static inline void
cvorg_writew(struct cvorg *cvorg, unsigned int offset, uint32_t value)
{
	__cvorg_writew(cvorg, offset, value);
}

/* Read from a channel's register */
static inline uint32_t
__cvorg_rchan(struct cvorg_channel *channel, unsigned int offset)
{
	return __cvorg_readw(channel->parent, channel->reg_offset + offset);
}

static inline uint32_t
cvorg_rchan(struct cvorg_channel *channel, unsigned int offset)
{
	return cvorg_readw(channel->parent, channel->reg_offset + offset);
}

/* Write to a channel's register */
static inline void
__cvorg_wchan(struct cvorg_channel *channel, unsigned int offset, uint32_t value)
{
	return __cvorg_writew(channel->parent, channel->reg_offset + offset, value);
}

static inline void
cvorg_wchan(struct cvorg_channel *channel, unsigned int offset, uint32_t value)
{
	return cvorg_writew(channel->parent, channel->reg_offset + offset, value);
}

/* OR a channel's register */
static inline void
cvorg_ochan(struct cvorg_channel *channel, unsigned int offset, uint32_t value)
{
	uint32_t regval;

	regval = cvorg_rchan(channel, offset);
	cvorg_wchan(channel, offset, regval | value);
}

/* AND a channel's register */
static inline void
cvorg_achan(struct cvorg_channel *channel, unsigned int offset, uint32_t value)
{
	uint32_t regval;

	regval = cvorg_rchan(channel, offset);
	cvorg_wchan(channel, offset, regval & value);
}

/* Update only certain bits of a channel's register */
static inline void cvorg_uchan(struct cvorg_channel *channel,
			unsigned int offset, uint32_t value, uint32_t mask)
{
	cvorg_achan(channel, offset, ~mask);
	cvorg_ochan(channel, offset, value);
}

/* write to a channel's register, without swapping */
static inline void
cvorg_wchan_noswap(struct cvorg_channel *channel, unsigned int offset, uint32_t value)
{
	iowrite32(value, channel->parent->iomap + channel->reg_offset + offset);
}

/* read from a channel's register, without swapping */
static inline uint32_t
cvorg_rchan_noswap(struct cvorg_channel *channel, unsigned int offset)
{
	return ioread32(channel->parent->iomap + channel->reg_offset + offset);
}

/* Note: the clkgen functions that may fail shall set errno appropriately */
int clkgen_check_pll(struct ad9516_pll *pll);
void clkgen_startup(struct cvorg *cvorg);
int clkgen_apply_pll_conf(struct cvorg *cvorg);
void clkgen_get_pll_conf(struct cvorg *cvorg, struct ad9516_pll *pll);

/* Functions used on sysfs callbacks */
int cvorg_reset(struct cvorg *cvorg);
int cvorg_read_sampfreq(struct cvorg *cvorg, void *arg);
int cvorg_get_pcb_id(struct cvorg *cvorg, void *arg);
int cvorg_get_temperature(struct cvorg *cvorg, void *arg);
int cvorg_chan_offset(struct cvorg_channel *channel, void *arg, int set);
int cvorg_chan_inpol(struct cvorg_channel *channel, void *arg, int set);
int cvorg_chan_status(struct cvorg_channel *channel, void *arg);
int cvorg_chan_output_gain(struct cvorg_channel *channel, void *arg, int set);
int cvorg_chan_sw_trigger(struct cvorg_channel *channel, void *arg);
int cvorg_chan_out_enable(struct cvorg_channel *channel, void *arg, int set);
int cvorg_chan_dac_gain(struct cvorg_channel *channel, void *arg, int set);
int cvorg_chan_dac_offset(struct cvorg_channel *channel, void *arg, int set);
int cvorg_chan_dac_val(struct cvorg_channel *channel, void *arg, int set);
int cvorg_chan_adc_read(struct cvorg_channel *channel, void *arg);
int cvorg_chan_loadseq(struct cvorg_channel *channel, void *arg);
int cvorg_chan_test_mode(struct cvorg_channel *channel, void *arg, int set);
int cvorg_chan_sram(struct cvorg_channel *channel, void *arg, int set);
int cvorg_pll(struct cvorg *cvorg, void *arg, int set);

/* Functios used by cvorgdrv.c */

int cvorg_module_install(struct device *pdev, uint32_t lun, unsigned long base_address, 
				uint32_t irq, uint32_t irq_level, unsigned int index);
void cvorg_module_uninstall(struct cvorg *cvorg);
int cvorg_create_sysfs_files(struct cvorg *cvorg);
void cvorg_remove_sysfs_files(struct cvorg *cvorg);

#endif /* _CVORG_PRIV_H_ */
