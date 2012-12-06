/*
 * cvorb_priv.h
 *
 * private CVORB definitions
 */

#ifndef _CVORB_PRIV_H_
#define _CVORB_PRIV_H

#include <linux/device.h>
#include <linux/cdev.h>
#include <asm-generic/iomap.h>

#include <cvorb_hw.h>

#define DRIVER_NAME     "cvorb"
#define PFX             DRIVER_NAME ": "

/* define this to debug I/O accesses */
#undef CVORB_DEBUG_IO

#define CVORB_MAX_BOARDS 	16

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 0
#endif

#ifndef BIG_ENDIAN
#define BIG_ENDIAN 1
#endif
/* Table to save the pointers to all cvorb devices */
/*extern struct cvorb* cvorb_table[CVORB_MAX_BOARDS];*/

/* Needed to create the sysfs files */
/*extern struct class *cvorb_class;*/
/*extern dev_t cvorb_devno; */

/**
 * struct cvorb_channel - internal channel structure
 * @enable:             1:ch is enabled 0:ch is disabled
 * @enable_fcn_mask:    function enable mask
 * @selected_fcn:       function currently selected
 * @repeat_count:       repeat count defining the  number of times a function will be generated
 * @channels_dir:       kobject corresponding to the channel dir in sysfs
 * @reg_offset:         offset of the channel's registers within the @parent module.
 * @chan_nr:            channel number in the range of [0,7]
 * @parent:	        parent CVORB submodule
 */
struct cvorb_channel
{
        uint32_t                enable;
        uint64_t                enable_fcn_mask;
        uint32_t                selected_fcn;
        uint32_t                repeat_count;
        struct kobject 		channels_dir;
        uint32_t                reg_offset;
	uint32_t 	        chan_nr;
	struct cvorb_submodule 	*parent;
};

/**
 * struct cvorb_submodule - CVORB module is made of two identical submodule.
 * @inpol:	                input polarity
 * @optical_out_enabled:	enable optical output
 * @dac_source:	                channel connected to the dac resource
 * @led_source:                 channel connected to the led resource
 * @optical_source:             channel connected to the optical resource
 * @submodules_dir:             kobject corresponding to the submodule dir in sysfs
 * @reg_offset:	                offset of the submodule's registers within the @parent module.
 * @submod_nr:                  submodule number in the range [0,1]
 * @channels:                   the 8 channels of the submodule
 * @parent:	                parent CVORB board
 */
struct cvorb_submodule
{
	uint32_t 		inpol;
	uint32_t                optical_out_enabled;
	uint32_t                dac_source;
	uint32_t                led_source;
	uint32_t                optical_source;
        struct kobject          submodules_dir;
	uint32_t 		reg_offset;
	uint32_t                submod_nr;
	struct cvorb_channel 	channels[CVORB_CHANNELS];
	struct cvorb_dev 	*parent;
};

/**
 * struct cvorb - description of a CVORB board
 * @cdev:       associated character device
 * @dev:        associated device created in sysfs under the related class
 * @pcb_id      pcd id (read once from the HW)
 * @hw_rev:	hardware revision number (read once from the HW)
 * @vme_base    VME base address of the module
 * @lock:	module's lock
 * @iomap:      kernel virtual address of the module's mapped I/O region
 * @submodules  the two submodules of the cvorb board
 * @lun         Logical Unit number of the board [0,n]
 */
struct cvorb_dev
{
	struct cdev 		cdev;
	struct device*          dev;
	uint64_t 		pcb_id;
	char 		        hw_rev[CVORB_VERSION_AS_STRING_SZ];
	uint32_t                vme_base;
	struct mutex 		lock;
	void 			*iomap;
	struct cvorb_submodule	submodules[CVORB_SUBMODULES];
	uint32_t 		lun;
};

enum cvorb_reset {
        CVORB_FULL,
        CVORB_SUBMODULE_1,
        CVORB_SUBMODULE_2
};

static inline uint32_t cvorb_readw(struct cvorb_dev *cvorb, unsigned int offset)
{
        return ioread32be(cvorb->iomap + offset);
}

static inline void cvorb_writew(struct cvorb_dev *cvorb, unsigned int offset, uint32_t value)
{
        iowrite32be(value, cvorb->iomap + offset);
}

/*
 * helper function allowing to set bit field.
 * @param mask: identifies the bit field
 * @param value: desired value to set in the bit field.
 */
static inline void cvorb_writebitfields(struct cvorb_dev* cvorb, unsigned int offset,
        uint32_t mask, uint32_t value)
{
        uint32_t regval;

        /* reads the current value from the HW */
        regval = cvorb_readw(cvorb, offset);
        /* clear the bits corresponding to the mask by doing a bitwise AND with the mask complement*/
        regval &= ~mask;
        /* set properly the bit fields */
        regval |= value;
        /* writes the new value to the HW */
        cvorb_writew(cvorb, offset, regval);
}

/* Functions used on sysfs callbacks */
void cvorb_reset(struct cvorb_dev *cvorb);
void cvorb_get_pcb_id(struct cvorb_dev *cvorb, uint64_t *value);
void cvorb_get_temperature(struct cvorb_dev *cvorb, int32_t *value);

void cvorb_submodule_reset(struct cvorb_submodule *submodule);
int cvorb_submodule_trigger(struct cvorb_submodule *submodule, enum cvorb_trigger event);
int cvorb_submodule_inpol(struct cvorb_submodule *submodule, uint32_t value);
void cvorb_submodule_status(struct cvorb_submodule *submodule, uint32_t *value);
void cvorb_submodule_dacsource(struct cvorb_submodule *submodule, uint32_t value);
void cvorb_submodule_ledsource(struct cvorb_submodule *submodule, uint32_t value);
void cvorb_submodule_opticalsource(struct cvorb_submodule *submodule, uint32_t value);
void cvorb_submodule_enable_opticalout(struct cvorb_submodule *submodule, uint32_t value);
int cvorb_ch_status(struct cvorb_channel *channel, uint32_t *status);
int cvorb_ch_enable(struct cvorb_channel *ch, uint32_t value);
int cvorb_select_fcn(struct cvorb_channel *ch, uint32_t value);
int cvorb_enable_fcn(struct cvorb_channel *ch, uint32_t value);
int cvorb_disable_fcn(struct cvorb_channel *ch, uint32_t value);
int cvorb_ch_repeat_count(struct cvorb_channel *ch, uint32_t value);
int cvorb_sysfs_set_fcn(struct cvorb_channel *channel, void *arg);

/* Functions used by cvorbdrv.c */
int cvorb_install(struct class *class, struct device *parentDev, dev_t cvorb_devno, struct cvorb_dev *cvorb);
void cvorb_uninstall(struct class* class, struct cvorb_dev *cvorb, dev_t cvorb_devno);
int cvorb_create_sysfs_files(struct cvorb_dev *cvorb);
void cvorb_remove_sysfs_files(struct cvorb_dev *cvorb);

#endif /* _CVORB_PRIV_H_ */
