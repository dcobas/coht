/**
 * cvorb-func.c
 *
 * Copyright (c) 2012 Samuel Iglesias Gonsalvez <siglesia@cern.ch>
 *		      Michel Arruat <michel.arruat@cern.ch>
 * Based in the previous work of Emilio G. Cota <cota@braap.org> in 2010.
 *
 * Released under the GPL v2. (and only v2, not any later version)
 */

#include <asm/io.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/module.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#include <linux/swab.h>
#else
#include <linux/byteorder/swab.h>
#endif
#include <vmebus.h>

#include <cvorb.h>
#include <cvorb_priv.h>
#include <cvorb_hw.h>

#define DRIVER_NAME 	"cvorb"
#define PFX 		DRIVER_NAME ": "
/* GCC Preprocessing option enabling dma code */
/*
#define __CVORB_DMA__
*/

/*
 * Module and driver cannot read/write at the same time from the module's SRAM.
 * Since the module has priority over the requests coming from the
 * driver, we must always check that the module is not accessing it
 * before going ahead. Otherwise we risk waiting for too long while
 * waiting for the module to finish, causing a bus error in the VME bus.
 */
static int __cvorb_sram_busy(struct cvorb_submodule* submodule)
{
        uint32_t status;
        int i;

        /* TODO: arbitrary delay inherited from the previous implementation */
        /* Check if this delay (10*10 = 100us) is correct */
        for (i = 0; i < 10; i++) {
                status = cvorb_readw(submodule->parent, submodule->reg_offset + CVORB_SUBMODULE_STATUS);
                if (!(status & CVORB_SUBMODULE_SRAM_BUSY))
                        /* sram is not busy */
                        return 0;
                udelay(10);
        }
        return 1;
}

static void __cvorb_submodule_reset(struct cvorb_submodule *submodule)
{
        cvorb_writew(submodule->parent, submodule->reg_offset + CVORB_SUBMODULE_CTL,
                        CVORB_SUBMODULE_RESET);
        udelay(5);
}

static void __cvorb_reset(struct cvorb_dev *cvorb)
{
        /* reset the FPGA */
        cvorb_writew(cvorb, CVORB_SUBMODULE_CTL, CVORB_FPGA_RESET);
        udelay(5);

        /* reset both submodules */
        __cvorb_submodule_reset(&cvorb->submodules[0]);
        __cvorb_submodule_reset(&cvorb->submodules[1]);
}

/* ====================================================================================
 * Board function
 * ====================================================================================
 */
/*
 * Full reset sysfs callback
 */
void cvorb_reset(struct cvorb_dev *cvorb)
{
        /* Enter Critical section */
        /**************************/
        mutex_lock(&cvorb->lock);

        __cvorb_reset(cvorb);
        /* Leave critical section */
        /**************************/
        mutex_unlock(&cvorb->lock);
}

/*
 * Get temperature sysfs callback
 */
void cvorb_get_temperature(struct cvorb_dev *cvorb, int32_t *temp)
{
        uint32_t regtemp = cvorb_readw(cvorb, CVORB_TEMP);

        *temp = (regtemp & 0x7ff) >> 4;
        if (regtemp & 0x800) /* sign bit */
                *temp *= -1;
}

/*
 * Get pcb id sysfs callback
 */
void cvorb_get_pcb_id(struct cvorb_dev *cvorb, uint64_t *value)
{
        *value = cvorb_readw(cvorb, CVORB_PCB_MSB);
        *value <<= 32;
        *value |= cvorb_readw(cvorb, CVORB_PCB_LSB);
}

/* ====================================================================================
 * Submodules function
 * ====================================================================================
 */
/*
 * Submodule reset sysfs callback
 */
void cvorb_submodule_reset(struct cvorb_submodule *submodule)
{
        /* Enter Critical section */
        /**************************/
        mutex_lock(&submodule->parent->lock);

        __cvorb_submodule_reset(submodule);
        /* Leave critical section */
        /**************************/
        mutex_unlock(&submodule->parent->lock);
}

/*
 * Submodule trigger sysfs callback
 */
int cvorb_submodule_trigger(struct cvorb_submodule *submodule, enum cvorb_trigger event)
{
        uint32_t val;
        switch (event) {
        case CVORB_START:
                val = CVORB_SUBMODULE_START;
                break;
        case CVORB_STOP:
                val = CVORB_SUBMODULE_STOP;
                break;
        case CVORB_EVT_START:
                val = CVORB_SUBMODULE_EVSTART;
                break;
        case CVORB_EVT_STOP:
                val = CVORB_SUBMODULE_EVSTOP;
                break;
        default:
                return -1;
        }
        /* Enter Critical section */
        /**************************/
        mutex_lock(&submodule->parent->lock);

        cvorb_writew(submodule->parent, submodule->reg_offset + CVORB_SUBMODULE_CTL, val);
        /* Leave critical section */
        /**************************/
        mutex_unlock(&submodule->parent->lock);

        return 0;
}

static inline int cvorb_submodule_busy(struct cvorb_submodule *submodule)
{
        return cvorb_readw(submodule->parent, submodule->reg_offset + CVORB_SUBMODULE_STATUS) & CVORB_SUBMODULE_BUSY;
}

/*
 * Submodule set input pulse polarity sysfs callback
 */
int cvorb_submodule_inpol(struct cvorb_submodule *submodule, uint32_t value)
{
        uint32_t hw_val=value;
        if (cvorb_submodule_busy(submodule)) {
                return -EBUSY;
        }
        /* Enter Critical section */
        /**************************/
        mutex_lock(&submodule->parent->lock);

        submodule->inpol = value;
        /* apply the required shift corresponding to the bits definition*/
        hw_val <<= CVORB_SUBMODULE_INPOL_SHIFT;
        cvorb_writebitfields(submodule->parent, submodule->reg_offset + CVORB_SUBMODULE_CONFIG,
                        CVORB_SUBMODULE_INPOL_MASK, hw_val);
        /* Leave critical section */
        /**************************/
        mutex_unlock(&submodule->parent->lock);

        return 0;
}

void cvorb_submodule_status(struct cvorb_submodule *submodule, uint32_t *status)
{
        *status = cvorb_readw(submodule->parent, submodule->reg_offset + CVORB_SUBMODULE_STATUS);
}

void cvorb_submodule_dacsource(struct cvorb_submodule *submodule, uint32_t value)
{
        uint32_t hw_val=value;
        /* Enter Critical section */
        /**************************/
        mutex_lock(&submodule->parent->lock);

        submodule->dac_source = value;
        /* apply the required shift corresponding to the bits definition*/
        hw_val <<= CVORB_SUBMODULE_DAC_SEL_SHIFT;
        cvorb_writebitfields(submodule->parent, submodule->reg_offset + CVORB_SUBMODULE_CONFIG,
                        CVORB_SUBMODULE_DAC_SEL_MASK, hw_val);
        /* Leave critical section */
        /**************************/
        mutex_unlock(&submodule->parent->lock);
}

void cvorb_submodule_ledsource(struct cvorb_submodule *submodule, uint32_t value)
{
        uint32_t hw_val=value;
        /* Enter Critical section */
        /**************************/
        mutex_lock(&submodule->parent->lock);

        submodule->led_source = value;
        /* apply the required shift corresponding to the bits definition*/
        hw_val <<= CVORB_SUBMODULE_LEDS_SEL_SHIFT;
        cvorb_writebitfields(submodule->parent, submodule->reg_offset + CVORB_SUBMODULE_CONFIG,
                        CVORB_SUBMODULE_LEDS_SEL_MASK, hw_val);
        /* Leave critical section */
        /**************************/
        mutex_unlock(&submodule->parent->lock);
}

void cvorb_submodule_opticalsource(struct cvorb_submodule *submodule, uint32_t value)
{
        uint32_t hw_val=value;
        /* Enter Critical section */
        /**************************/
        mutex_lock(&submodule->parent->lock);

        submodule->optical_source = value;
        /* apply the required shift corresponding to the bits definition*/
        hw_val <<= CVORB_SUBMODULE_OPTICAL_SEL_SHIFT;
        cvorb_writebitfields(submodule->parent, submodule->reg_offset + CVORB_SUBMODULE_CONFIG,
                        CVORB_SUBMODULE_OPTICAL_SEL_MASK, hw_val);
        /* Leave critical section */
        /**************************/
        mutex_unlock(&submodule->parent->lock);
}

void cvorb_submodule_enable_opticalout(struct cvorb_submodule *submodule, uint32_t value)
{
        uint32_t hw_val=value;
        /* Enter Critical section */
        /**************************/
        mutex_lock(&submodule->parent->lock);

        submodule->optical_out_enabled = value;
        /* apply the required shift corresponding to the bits definition*/
        hw_val <<= CVORB_SUBMODULE_OPT_EN_SHIFT;
        cvorb_writebitfields(submodule->parent, submodule->reg_offset + CVORB_SUBMODULE_CONFIG,
                        CVORB_SUBMODULE_OPT_EN_MASK, hw_val);
        /* Leave critical section */
        /**************************/
        mutex_unlock(&submodule->parent->lock);
}

int cvorb_ch_status(struct cvorb_channel *channel, uint32_t *status)
{
        *status = cvorb_readw(channel->parent->parent, channel->reg_offset + CVORB_CH_STATUS);
        return 0;
}

int cvorb_ch_enable(struct cvorb_channel *ch, uint32_t value)
{
        uint32_t hw_val;

        /* Enter Critical section */
        /**************************/
        mutex_lock(&ch->parent->parent->lock);

        ch->enable = value;
        /* apply the required shift corresponding to the bits definition*/
        hw_val = (value) ? CVORB_CH_ENABLE : CVORB_CH_DISABLE;
        cvorb_writebitfields(ch->parent->parent, ch->reg_offset + CVORB_CH_CONFIG,
                        CVORB_CH_ENABLE_MASK, value);
        /* Leave critical section */
        /**************************/
        mutex_unlock(&ch->parent->parent->lock);

        return 0;
}

int cvorb_select_fcn(struct cvorb_channel *ch, uint32_t value)
{
        /* Enter Critical section */
        /**************************/
        mutex_lock(&ch->parent->parent->lock);

        ch->selected_fcn = value;
        cvorb_writew(ch->parent->parent, ch->reg_offset + CVORB_CH_SELECT_FCN, value);
        /* Leave critical section */
        /**************************/
        mutex_unlock(&ch->parent->parent->lock);

        return 0;
}

int cvorb_enable_fcn(struct cvorb_channel *ch, uint32_t value)
{
        /* value contains the function nr in the range [0,63]. This value gives the bit nr to set */
        uint64_t hw_val = ((uint64_t)1)<<value;
        /* Enter Critical section */
        /**************************/
        mutex_lock(&ch->parent->parent->lock);

        ch->enable_fcn_mask |= hw_val;
        /* the HW stores this enable function mask using two 32 bits register */
        /* CVORB_CH_ENABLE_FCN_MASK_L: contains mask for function 0 to 31     */
        /* CVORB_CH_ENABLE_FCN_MASK_H: contains mask for function 32 to 63    */
        if (value < 32) /* Low register */
        {
                cvorb_writew(ch->parent->parent, ch->reg_offset + CVORB_CH_ENABLE_FCN_MASK_L, (uint32_t)ch->enable_fcn_mask);
        }
        else /* High register */
        {
                cvorb_writew(ch->parent->parent, ch->reg_offset + CVORB_CH_ENABLE_FCN_MASK_H, (uint32_t)(ch->enable_fcn_mask>>32));
        }
        /* Leave critical section */
        /**************************/
        mutex_unlock(&ch->parent->parent->lock);

        return 0;
}

int cvorb_disable_fcn(struct cvorb_channel *ch, uint32_t value)
{
        /* value contains the function nr in the range [0,63]. This value gives the bit nr to unset */
        uint64_t hw_val = ~(((uint64_t)1)<<value);
        /* Enter Critical section */
        /**************************/
        mutex_lock(&ch->parent->parent->lock);

        ch->enable_fcn_mask &= hw_val;
        /* the HW stores this enable function mask using two 32 bits registers */
        /* CVORB_CH_ENABLE_FCN_MASK_L: contains mask for function 0 to 31     */
        /* CVORB_CH_ENABLE_FCN_MASK_H: contains mask for function 32 to 63    */
        if (value < 32) /* Low register */
        {
                cvorb_writew(ch->parent->parent, ch->reg_offset + CVORB_CH_ENABLE_FCN_MASK_L, (uint32_t)ch->enable_fcn_mask);
        }
        else /* High register */
        {
                cvorb_writew(ch->parent->parent, ch->reg_offset + CVORB_CH_ENABLE_FCN_MASK_H, (uint32_t)(ch->enable_fcn_mask>>32));
        }

        /* Leave critical section */
        /**************************/
        mutex_unlock(&ch->parent->parent->lock);

        return 0;
}

int cvorb_ch_repeat_count(struct cvorb_channel *ch, uint32_t value)
{
        /* Enter Critical section */
        /**************************/
        mutex_lock(&ch->parent->parent->lock);

        ch->repeat_count = value;
        cvorb_writew(ch->parent->parent, ch->reg_offset + CVORB_CH_REPEAT_FCN, value);
        /* Leave critical section */
        /**************************/
        mutex_unlock(&ch->parent->parent->lock);

        return 0;
}

#ifdef __CVORB_DMA__
static int get_endian(void)
{
        int i = 1;
        char *p = (char *)&i;

        if (p[0] == 1)
                return LITTLE_ENDIAN;
        else
                return BIG_ENDIAN;
}
#endif

static int __cvorb_set_fcn(struct cvorb_dev *cvorb, struct cvorb_hw_fcn *fcn, int is_ioctl)
{
    uint32_t* hw_fcn;
    int ret = 0, i;
    unsigned int ramaddr = 0;
    uint32_t reg_offset;
#ifdef __CVORB_DMA__
    uint32_t *swapp; /* used to swap le to be */
#endif
    if (is_ioctl) {
        /* allocate the hw_fcn for which the size in 32 bits word is given by hw_fcn_size32 */
        hw_fcn = kzalloc((ssize_t)fcn->hw_fcn_size32*sizeof(uint32_t), GFP_KERNEL);
        if (hw_fcn == NULL)
            return -ENOMEM;

        /* copy the hw function itself*/
        if (copy_from_user(hw_fcn, fcn->hw_fcn, fcn->hw_fcn_size32*sizeof(uint32_t))) {
            ret = -EFAULT;
            goto out_free_hw_fcn;
        }
    }
    else {
        /* sysfs has already alocated a buffer in the kernel space */
        hw_fcn = fcn->hw_fcn;
    }

#ifdef __CVORB_DMA__
    /* endianess should be treated only in case of DMA. */
    /* In case of read/write 32 bits register */
    /* endianess is done by ioread32be/iowrite32be routines */
    /* only if necessary, that is to say the CPU is little endian */
    /* (knowing VME bus is big endian)  */
    if (get_endian() == LITTLE_ENDIAN) {
        /* CPU is little endian but VME is big endian                   */
        /* therefore we have to swap all the bytes in the 32 bits words */
        for (i = 0, swapp = hw_fcn; i < fcn->hw_fcn_size32; ++i, ++swapp)
            *swapp = swab32(*swapp);
    }
#endif

    ramaddr = ((fcn->channr)<<CVORB_SUBMODULE_SRAM_CHAN_SHIFT) |
                 (fcn->fcnnr<<CVORB_SUBMODULE_SRAM_FUNC_SHIFT);

    /* Enter Critical section */
    /**************************/
    mutex_lock(&(cvorb->lock));

    /* Check first if the module is accessing the SRAM: exclusive access*/
    if (__cvorb_sram_busy(&(cvorb->submodules[fcn->submodulenr]))) {
        ret = -EAGAIN;
        goto out_free_lock;
    }
    /* Write SRAM start address where the function is located */
    /* Then, the address auto-increments on every write       */
    cvorb_writew(cvorb,
                cvorb->submodules[fcn->submodulenr].reg_offset + CVORB_SUBMODULE_SRAM_ADDR,
                ramaddr);

    /* write the function itself */
    reg_offset = cvorb->submodules[fcn->submodulenr].reg_offset + CVORB_SUBMODULE_SRAM_DATA;
#ifdef __CVORB_DMA__
    ret = cvorb_dma_write_mblt(cvorb->dev, (int)cvorb->iomap+reg_offset,
                                hw_fcn, fcn->hw_fcn_size32*sizeof(uint32_t));
#else
/* TODO: replace iowrite32be by memcpy_toio */
    for (i=0; i<fcn->hw_fcn_size32; ++i) {
        cvorb_writew(cvorb, reg_offset, hw_fcn[i]);
    }
#endif

out_free_lock:
    /* Leave critical section */
    /**************************/
    mutex_unlock(&cvorb->lock);
out_free_hw_fcn:
    if (is_ioctl) /* allocated just in case of ioctl */
        kfree(hw_fcn);
    return ret;
}

int cvorb_sysfs_set_fcn(struct cvorb_channel *channel, uint32_t fcn_nr, void *buffer)
{
    struct cvorb_hw_fcn fcn;
    fcn.hw_fcn = (uint32_t *)buffer;
    fcn.submodulenr = channel->parent->submod_nr;
    fcn.channr = channel->chan_nr;
    fcn.fcnnr = fcn_nr;
    return __cvorb_set_fcn(channel->parent->parent, &fcn, 0);
}

static int cvorb_ioctl_set_fcn(struct cvorb_dev *cvorb, void __user *arg)
{
    struct cvorb_hw_fcn fcn;
    if (copy_from_user(&fcn, arg, sizeof(fcn)))
        return -EFAULT;
    return (__cvorb_set_fcn(cvorb, &fcn, 1));
}

static int __cvorb_get_fcn(struct cvorb_dev *cvorb, struct cvorb_hw_fcn *fcn, ssize_t *count, int is_ioctl)
{
    uint32_t* hw_fcn = NULL;
    int ret = 0;
    unsigned int ramaddr = 0;
    uint32_t reg_offset, hw_n_vector;
    int hw_fcn_sz32=0, sz16, i;

    ramaddr = ((fcn->channr)<<CVORB_SUBMODULE_SRAM_CHAN_SHIFT) |
                (fcn->fcnnr<<CVORB_SUBMODULE_SRAM_FUNC_SHIFT);

    /* Enter Critical section */
    /**************************/
    mutex_lock(&(cvorb->lock));

    /* Check first if the module is accessing the SRAM: exclusive access*/
    if (__cvorb_sram_busy(&(cvorb->submodules[fcn->submodulenr]))) {
        ret = -EAGAIN;
        goto out_free_lock;
    }

    /* Write SRAM start address where the function is located */
    /* Then, the address auto-increments on every read        */
    cvorb_writew(cvorb, 
                cvorb->submodules[fcn->submodulenr].reg_offset + CVORB_SUBMODULE_SRAM_ADDR,
                ramaddr);
    /* retrieve the number of vector of the function */
    reg_offset = cvorb->submodules[fcn->submodulenr].reg_offset + CVORB_SUBMODULE_SRAM_DATA;
    /* Get number of vector and check against the client buffer size */
    hw_n_vector = cvorb_readw(cvorb, reg_offset);
    /* check if it's a realistic function or garbage */
    if (hw_n_vector <= 0 || hw_n_vector >= CVORB_MAX_VECTOR) {
        /* Probably this slot has no function  resident in the SRAM. */
        ret = -EBADSLT; /* Invalid slot */
        goto out_free_lock;
    }

    /* Checking client buffer size and fcn size cannot be done anymore in the driver */
    /* due to the sysfs implementation (no way to pass input parameter with a sysfs read */
    /* Don't worry the library takes care of this check */
    //        if (hw_n_vector > fcn.n_vector) {
    //                /* User space is not big enough*/
    //                ret = -ENOBUFS; /* No buffer space available */
    //                goto out_free_lock;
    //        }

    /* A function is encoded in the HW as a table of 2 bytes word and is organized this way:  */
    /* Header: 3 words                                                                        */
    /* Vectors: 3 words * nb_of_vectors                                                       */
    /*           (vector is encoded using 3 words:amplitude, nr of steps, step size)          */
    /* Footer: 6 words                                                                        */
    /* compute HW fcn size in 32 bits word */
    sz16 = CVORB_FCT_HEADER_SZ + (hw_n_vector*CVORB_VCT_SZ);
    if (sz16 & 1)
        ++sz16; /*should be even number of 16 bits words (in order to be 32 bits word aligned)*/
    sz16 += CVORB_FOOTER_SZ;
    /* convert in 32 bits words */
    hw_fcn_sz32= sz16 >> 1;

    /* allocate the hw_fcn for which the size in 32 bits word is given by hw_fcn_size32 */
    if (is_ioctl) {
        hw_fcn = kzalloc((ssize_t)hw_fcn_sz32*sizeof(uint32_t), GFP_KERNEL);
        if (hw_fcn == NULL) {
            ret = -ENOMEM; /* Out of memory */
            goto out_free_lock;
        }
    }
    else
        hw_fcn = fcn->hw_fcn; // sysfs has already allocated a buffer

    fcn->hw_fcn_size32 = hw_fcn_sz32;
    fcn->n_vector = hw_n_vector;
    hw_fcn[0] = hw_n_vector;
    /* TODO: replace ioread32be by memcpy_fromio */
    for (i=1; i<=hw_fcn_sz32; ++i) {
        hw_fcn[i] = cvorb_readw(cvorb, reg_offset);
    }

    out_free_lock:
    /* Leave critical section */
    /**************************/
    mutex_unlock(&cvorb->lock);

    if (is_ioctl && hw_fcn != NULL) { /* buffer has been allocated */
        if(copy_to_user(fcn->hw_fcn, (char *)hw_fcn, hw_fcn_sz32*sizeof(uint32_t)))
            return -EFAULT; /* Bad address */
        kfree(hw_fcn);
    }
    else
        *count = hw_fcn_sz32*sizeof(uint32_t);
    return ret;
}

int cvorb_sysfs_get_fcn(struct cvorb_channel *channel, uint32_t fcn_nr,
                        void *buffer, ssize_t *count)
{
        struct cvorb_hw_fcn fcn;
        fcn.hw_fcn = (uint32_t *)buffer;
        fcn.submodulenr = channel->parent->submod_nr;
        fcn.channr = channel->chan_nr;
        fcn.fcnnr = fcn_nr;
        return __cvorb_get_fcn(channel->parent->parent, &fcn, count, 0);
}

static int cvorb_ioctl_get_fcn(struct cvorb_dev *cvorb, void __user *arg)
{
        struct cvorb_hw_fcn fcn;
        ssize_t count;
        int ret;
        if (copy_from_user(&fcn, arg, sizeof(fcn)))
            return -EFAULT;
        ret = __cvorb_get_fcn(cvorb, &fcn, &count, 1);
        if(copy_to_user((char *)arg, (char *)&fcn, sizeof(struct cvorb_hw_fcn)))
            return -EFAULT; /* Bad address */
        return ret;
}

static long cvorb_ioctl(struct file *fp, unsigned op, unsigned long arg)
{
        struct cvorb_dev *cvorb = fp->private_data;

    switch(op) {
    case CVORB_IOCTL_SET_FCT:
            return cvorb_ioctl_set_fcn(cvorb, (void __user *)arg);
    case CVORB_IOCTL_GET_FCT:
            return cvorb_ioctl_get_fcn(cvorb, (void __user *)arg);
    default:
            return -ENOTTY;
    }
}

/*
 * Cvorb channel struct keeps simple settings.
 * Therefore at the start-up, it is required to sync it with the HW.
 */
static void cvorb_init_channel(struct cvorb_channel *ch)
{
    cvorb_writew(ch->parent->parent, ch->reg_offset + CVORB_CH_CONFIG, 0);
    ch->enable = 0;
    cvorb_writew(ch->parent->parent, ch->reg_offset + CVORB_CH_ENABLE_FCN_MASK_L, 0);
    cvorb_writew(ch->parent->parent, ch->reg_offset + CVORB_CH_ENABLE_FCN_MASK_H, 0);
    ch->enable_fcn_mask = 0;
    /* For the repeat-count the default value is 1- 0 means infinite repeat count */
    cvorb_writew(ch->parent->parent, ch->reg_offset + CVORB_CH_REPEAT_FCN, 1);
    ch->repeat_count = 1;
    cvorb_writew(ch->parent->parent, ch->reg_offset + CVORB_CH_SELECT_FCN, 0);
    ch->selected_fcn = 0;
}

/*
 * Cvorb submodule struct keeps simple settings.
 * Therefore at the start-up, it is required to sync it with the HW.
 * Write initial value
 */
static void cvorb_init_submodule(struct cvorb_submodule *sm)
{
    int i;

    cvorb_writew(sm->parent, sm->reg_offset + CVORB_SUBMODULE_CONFIG, 0);
    sm->inpol = 0;
    sm->dac_source = 0;
    sm->led_source = 0;
    sm->optical_source = 0;
    sm->optical_out_enabled = 0;

    for (i=0; i<CVORB_CHANNELS; i++)
    {
        cvorb_init_channel(&sm->channels[i]);
    }
}

static void cvorb_init(struct cvorb_dev *cvorb)
{
    cvorb_get_pcb_id(cvorb, &(cvorb->pcb_id));
    cvorb_init_submodule(&cvorb->submodules[0]);
    cvorb_init_submodule(&cvorb->submodules[1]);
}

static int cvorb_version_check(struct cvorb_dev *cvorb)
{
    uint32_t ver;
    unsigned int vhdl_tens, vhdl_units, vhdl_tenths, vhdl_hundredths;

    /* read VHDL revision */
    ver = cvorb_readw(cvorb, CVORB_VERSION);

    vhdl_tens       = (ver >> CVORB_VERSION_TENS_SHIFT)             & 0xf;
    vhdl_units      = (ver >> CVORB_VERSION_UNITS_SHIFT)            & 0xf;
    vhdl_tenths     = (ver >> CVORB_VERSION_TENTHS_SHIFT)           & 0xf;
    vhdl_hundredths = (ver >> CVORB_VERSION_HUNDREDTHS_SHIFT)       & 0xf;

    sprintf(cvorb->hw_rev, "v%d%d.%d%d",
            vhdl_tens, vhdl_units, vhdl_tenths, vhdl_hundredths);

    ver >>= CVORB_VERSION_FW_SHIFT;
    ver &= 0xff;

    return ver != 'B';
}

/* map cvorb_dev VME address space */
static struct pdparam_master param = {
    .iack   = 1,                    /* no iack */
    .rdpref = 0,                    /* no VME read prefetch option */
    .wrpost = 0,                    /* no VME write posting option */
    .swap   = 1,                    /* VME auto swap option */
    .dum    = { VME_PG_SHARED,      /* window is sharable */
                    0,              /* XPC ADP-type */
                    0, },           /* window is sharable */
};

static unsigned long cvorb_map(unsigned long base_address)
{
    return find_controller(base_address, CVORB_WINDOW_LENGTH,
            CVORB_ADDRESS_MODIFIER, 0, CVORB_DATA_SIZE, &param);
}

/* The driver is notified each time the device is opened */
static int cvorb_open(struct inode *inode, struct file *file)
{
    struct cvorb_dev *cvorb = container_of(inode->i_cdev, struct cvorb_dev, cdev);
    file->private_data = cvorb;
    return 0;
}

static int cvorb_release(struct inode *inode, struct file *file)
{
    return 0;
}

static const struct file_operations cvorb_fops = {
    .owner          = THIS_MODULE,
    .open           = cvorb_open,
    .release        = cvorb_release,
    .unlocked_ioctl          = cvorb_ioctl
};

/*
 * Installs a CVORB board
 */
int cvorb_install(struct class *cvorb_class, struct device *parentDev,
                    dev_t cvorb_devno, struct cvorb_dev *cvorb)
{
    int submoduleIdx, chanIdx;
    struct cvorb_submodule *submodule = NULL;
    struct cvorb_channel *channel = NULL;
    int ret =0;

    dev_t devno = MKDEV(MAJOR(cvorb_devno), cvorb->lun);

	// init CVORB struct and its descendants : submodules and channels
	cvorb->iomap = (void *)cvorb_map(cvorb->vme_base);
	// set properly the channel and the submodule number
	for (submoduleIdx=0; submoduleIdx < CVORB_SUBMODULES; ++submoduleIdx) {
        submodule = &(cvorb->submodules[submoduleIdx]);
        submodule->submod_nr = submoduleIdx;
        submodule->reg_offset = CVORB_SUBMODULE_OFFSET*submoduleIdx;
        submodule->parent = cvorb;
		for (chanIdx=0; chanIdx < CVORB_CHANNELS; ++chanIdx) {
            channel = &(submodule->channels[chanIdx]);
            channel->chan_nr = chanIdx;
            channel->parent = submodule;
            channel->reg_offset = submodule->reg_offset + CVORB_CH_SIZE*chanIdx;
		}
	}

	if (cvorb->iomap == NULL) {
		printk(KERN_ERR PFX "Can't find virtual address of the module's registers\n");
		ret = -EINVAL;
		goto out_free;
	}

	if (cvorb_version_check(cvorb)) {
		printk(KERN_ERR PFX "cvorb.%d: device not present\n", cvorb->lun);
		ret = -ENODEV;
		goto out_free;
	}

	/* Set a pointer to specific driver data: cvorb.*/
	dev_set_drvdata(parentDev, cvorb);

	/*
	 * Creates a struct device in sysfs, registered to the specified class
	 * The newly created struct device will be a child of the parentDev (vme) in sysfs
	 * Any further sysfs files that might be required can be created using this pointer.
	 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
	cvorb->dev = device_create(cvorb_class, parentDev, devno, NULL, "cvorb.%d", cvorb->lun);
#else
	cvorb->dev = device_create(cvorb_class, parentDev, devno, "cvorb.%d", cvorb->lun);
#endif

	if(IS_ERR(cvorb->dev)) {
		printk(KERN_ERR PFX "error %ld creating cvorb.%d device\n",
                PTR_ERR(cvorb->dev), cvorb->lun);
		ret = PTR_ERR(cvorb->dev);
		goto out_free;
	}

	dev_set_drvdata(cvorb->dev, cvorb);
	ret = cvorb_create_sysfs_files(cvorb);
	if (ret)
		goto out_sysfs;

	mutex_init(&cvorb->lock);
        cvorb_reset(cvorb);
        /* Sync HW and cvorb structs by writing default initial values*/
        cvorb_init(cvorb);

	/* Last but not least: the device is now fully initialized, register it */
	cvorb->cdev.owner = THIS_MODULE;
	cdev_init(&cvorb->cdev, &cvorb_fops);
	ret = cdev_add(&cvorb->cdev, devno, 1);
	if (ret) {
		printk(KERN_ERR PFX "Error adding cdev %d\n", cvorb->lun);
		goto out_cdev_add;
	}

	return 0;
out_cdev_add:	
out_sysfs:
    cvorb->dev = NULL;
out_free:
	if (cvorb) {
		kfree((void *)cvorb);
	}
	return ret;
}

/*
 * Uninstalls a CVORB board.
 */
void cvorb_uninstall(struct class *class, struct cvorb_dev *cvorb, dev_t cvorb_devno)
{
    dev_t devno = MKDEV(MAJOR(cvorb_devno), cvorb->lun);

    if (cvorb == NULL)
            return;
    printk(KERN_INFO PFX "Uninstall cvorb lun %d VME address 0x%8x\n",
            cvorb->lun, cvorb->vme_base);

    /* Delete sysfs files */
    cvorb_remove_sysfs_files(cvorb);

    /* Unmap the memory */
    return_controller((unsigned long)cvorb->iomap, CVORB_WINDOW_LENGTH);

    device_destroy(class, devno);
    cdev_del(&cvorb->cdev);

    /* free userdata */
    kfree((void *)cvorb);
}
