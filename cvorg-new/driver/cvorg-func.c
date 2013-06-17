/**
 * cvorg-skel.c
 *
 * Copyright (c) 2012 Samuel Iglesias Gonsalvez <siglesia@cern.ch>
 * Based in the previous work of Emilio G. Cota <cota@braap.org> in 2010.
 *
 * Released under the GPL v2. (and only v2, not any later version)
 */

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/version.h>

#include <vmebus.h>
#include <cvorg.h>
#include <cvorg_priv.h>
#include <cvorg_hw.h>

#define DRIVER_NAME 	"cvorg"
#define PFX 		DRIVER_NAME ": "

static const int32_t cvorg_gains[] = {
	[0]	= -22,
	[1]	= -16,
	[2]	= -10,
	[3]	= -4,
	[4]	= 2,
	[5]	= 8,
	[6]	= 14,
	[7]	= 20,
};
#define CVORG_NR_GAINS	ARRAY_SIZE(cvorg_gains)

struct cvorg_limit {
	uint32_t	freq;
	uint32_t	len;
};

/* we use MHz here to avoid floating point arithmetic */
static const struct cvorg_limit cvorg_freqlimits[] = {
	{ 1,	16 },
	{ 10,	60 },
	{ 25,	1250 },
	{ 50,	3300 },
	{ 60,	4700 },
	{ 80,	9300 },
	{ 100,	23200 }
};
#define CVORG_NR_FREQLIMITS	ARRAY_SIZE(cvorg_freqlimits)

static inline int __cvorg_channel_busy(struct cvorg_channel *channel)
{
	return cvorg_rchan(channel, CVORG_STATUS) & CVORG_STATUS_BUSY;
}

static inline int __cvorg_channel_ready(struct cvorg_channel *channel)
{
	uint32_t status = cvorg_rchan(channel, CVORG_STATUS);
	int ret;

	/* ready to run? */
	ret = status & CVORG_STATUS_READY &&
		!(status & CVORG_STATUS_BUSY) &&
		!(status & CVORG_STATUS_ERR) &&
		  status & CVORG_STATUS_CLKOK;

	if(!ret)
		printk(KERN_WARNING PFX "Channel not ready. Status: 0x%x\n", status);

	return ret;
}

static inline int __cvorg_busy(struct cvorg *cvorg)
{
	return __cvorg_channel_busy(&cvorg->channels[0]) ||
	       __cvorg_channel_busy(&cvorg->channels[1]);
}

/* Note: ERR_CLK isn't (and can't be) cleared */
static inline void __cvorg_channel_clear_errors(struct cvorg_channel *channel)
{
	cvorg_wchan(channel, CVORG_CTL, CVORG_CTL_ERR_CLEAR);
}

/*
 * Module and driver cannot read at the same time from the module's SRAM.
 * Since the module has priority over the requests coming from the
 * driver, we must always check that the module is not accessing it
 * before going ahead. Otherwise we risk waiting for too long while
 * waiting for the module to finish, causing a bus error in the VME bus.
 */
static int __cvorg_sram_busy(struct cvorg_channel *channel)
{
	uint32_t regval;
	int i;

	for (i = 0; i < 10; i++) {
		regval = cvorg_rchan(channel, CVORG_STATUS);
		if (!(regval & CVORG_STATUS_SRAM_BUSY))
			return 0;
		udelay(10);
	}
	return 1;
}

static uint32_t cvorg_absdiff(int32_t s1, int32_t s2)
{
	if (s1 > s2)
		return s1 - s2;
	return s2 - s1;
}

static int32_t cvorg_nearest(const int32_t *array, int n, int32_t val)
{
	int i;
	uint32_t diff, mindiff;
	int result = 0;

	mindiff = ~0;

	for (i = 0; i < n; i++) {
		diff = cvorg_absdiff(array[i], val);
		if (diff < mindiff) {
			mindiff = diff;
			result = i;
		}
	}
	return array[result];
}

static int32_t cvorg_gain_approx(int32_t gain)
{
	if (gain < cvorg_gains[0])
		return cvorg_gains[0];

	if (gain > cvorg_gains[CVORG_NR_GAINS - 1])
		return cvorg_gains[CVORG_NR_GAINS - 1];

	return cvorg_nearest(&cvorg_gains[0], CVORG_NR_GAINS, gain);
}

static unsigned int cvorg_gain_to_hw(int32_t gain)
{
	switch (gain) {
	case -22:
		return CVORG_OUTGAIN_M22;
	case -16:
		return CVORG_OUTGAIN_M16;
	case -10:
		return CVORG_OUTGAIN_M10;
	case -4:
		return CVORG_OUTGAIN_M4;
	case 2:
		return CVORG_OUTGAIN_2;
	case 8:
		return CVORG_OUTGAIN_8;
	case 14:
		return CVORG_OUTGAIN_14;
	case 20:
	default:
		return CVORG_OUTGAIN_20;
	}
}

static inline int cvorg_invalid_gain_offset(int32_t gain, uint32_t offset)
{
	return offset == CVORG_OUT_OFFSET_2_5V && gain != 14;
}

static int
cvorg_check_gain_offset_wv(struct cvorg_channel *channel, struct cvorg_wv *wv)
{
	if (wv->dynamic_gain &&
		cvorg_invalid_gain_offset(wv->gain_val, channel->outoff))
		return -1;
	return 0;
}

/*
 * The way waveforms are played consecutively is as follows: There are
 * two alternating waveform players. Just after the first loads a
 * waveform, the second starts reading the following from the FPGA's SRAM.
 * For this to work, the time elapsed while playing the first waveform
 * has to be greater than the time it takes the second player to grab
 * the second waveform.
 */
static inline int cvorg_must_check_freqlimit(struct cvorg_seq *seq)
{
	return seq->nr == 0 || seq->n_waves > 2;
}

/*
 * Returns the i-th rank of the fixed set of lengths that we've got
 * given a sampling frequency.
 * Note that given n (freq, length) pairs, there are n - 1 ranges
 * in between them, i.e. i => [0,n-2].
 */
static int cvorg_freqlimit_range(uint32_t mhz)
{
	uint32_t x2, x1;
	int i;

	for (i = 0; i < CVORG_NR_FREQLIMITS - 1; i++) {
		x1 = cvorg_freqlimits[i].freq;
		x2 = cvorg_freqlimits[i + 1].freq;

		if ((x1 > mhz) && (mhz < x2))
			return i;
	}

	/* handle the out of range cases here */
	if (mhz <= cvorg_freqlimits[0].freq)
		return 0;
	else
		return CVORG_NR_FREQLIMITS - 2;
}

/*
 * Make a linear interpolation to find the length limit for the given
 * frequency and then check if we're over this limit or not.
 *
 * The interpolated curve is y = m*x + b, where m is the slope.
 * Note: we use MHz to avoid floating point arithmetic. For safety,
 * we round up the input in Hz to the nearest MHz unit.
 */
static int cvorg_freqlimit_check(uint32_t length, uint32_t hz)
{
	uint32_t y1, y2, x1, x2;
	uint32_t minimum;
	uint32_t m;
	uint32_t mhz;
	int i;

	/* At 100 MHz, the board measures 100.002 MHz, we have 2 kHz of error.
	 * In other cases, we don't care because we round up to safer values.
	 */
	if (hz > 100002000) {
		printk(KERN_INFO PFX "Maximum frequency is 100 MHz. The actual frequency is %d Hz\n",
			hz);
		return -1;
	}

	mhz = hz / 1000000 + !!(hz % 1000000);

	/* If we have mhz = 101, it means that we round up when we shouldn't */
	if (mhz > 100)
		mhz = 100;

	i = cvorg_freqlimit_range(mhz);

	x1 = cvorg_freqlimits[i].freq;
	y1 = cvorg_freqlimits[i].len;

	x2 = cvorg_freqlimits[i + 1].freq;
	y2 = cvorg_freqlimits[i + 1].len;

	m = (y2 - y1) / (x2 - x1);
	minimum = m == 0 ? y2 : m * (mhz - x1) + y1;

	if (length < minimum) {
		printk(KERN_INFO PFX "Minimum number of points at %d MHz: %d. Requested: %d\n",
			mhz, minimum, length);
	}

	return length < minimum;
}

static int cvorg_invalid_freqlimit(struct cvorg_channel *channel,
				struct cvorg_seq *seq, struct cvorg_wv *wv)
{
	uint32_t length;
	uint32_t sampfreq;

	/*
	 * If recurr is not set, we depend on the external Event Stop
	 * to switch to another function, which means we can do nothing
	 * to prevent our users from causing an error on the module
	 * if the two triggers come very close in time.
	 */
	if (!cvorg_must_check_freqlimit(seq) || !wv->recurr)
		return 0;

	length = wv->recurr * wv->size / sizeof(uint16_t);
	sampfreq = cvorg_rchan(channel, CVORG_SAMPFREQ);

	return cvorg_freqlimit_check(length, sampfreq);
}

void cvorg_pll_cfg_init(struct ad9516_pll *pll)
{
	/*
	 * PLL defaults: fvco = 2.7GHz, f_output = 100MHz, for the given
	 * fref = 40MHz
	 */
	pll->a		= 0;
	pll->b		= 4375;
	pll->p		= 16;
	pll->r		= 1000;
	pll->dvco	= 2;
	pll->d1		= 14;
	pll->d2		= 1;
	pll->external	= 0;
	pll->input_freq = AD9516_OSCILLATOR_FREQ;
	pll->ext_clk_pll = 0;
}

#define CVORG_FREQ_DIV	10000
/*
 * Sleep until the configured frequency is available at the output.
 * The frequencies in this function are determined by the divider above.
 *
 * This divider is used to avoid floating point arithmetic when
 * calculating the resulting frequency given a particular PLL configuration.
 */
static int cvorg_poll_sampfreq(struct cvorg *cvorg)
{
	struct ad9516_pll *pll = &cvorg->pll;
	unsigned int n = pll->p * pll->b + pll->a;
	int32_t sampfreq;
	int32_t freq;
	unsigned int input_freq = AD9516_OSCILLATOR_FREQ;
	int i;

	/* When external, we cannot possibly know the desired frequency */
	if (pll->external)
		return 0;

	if (pll->ext_clk_pll)
		input_freq = pll->input_freq;

	/* Note: some dividers may be bypassed (ie set to zero) */
	freq = input_freq / CVORG_FREQ_DIV * n;
	if (pll->r)
		freq /= pll->r;
	if (pll->dvco)
		freq /= pll->dvco;
	if (pll->d1)
		freq /= pll->d1;
	if (pll->d2)
		freq /= pll->d2;

	sampfreq = cvorg_readw(cvorg, CVORG_SAMPFREQ) / CVORG_FREQ_DIV;
	i = 0;
	/*
	 * The loop below converges when the difference between the read
	 * frequency and the desired frequency is below CVORG_FREQ_DIV Hz.
	 */
	while (cvorg_absdiff(sampfreq, freq) > 1) {
		/* if it hasn't settled in 10 secs, return an error */
		if (i >= 100) {
			printk(KERN_INFO PFX "Timeout expired for changing the frequency\n");
			return -ETIME;
		}

		/* sleep for approximately a hundred msecs */
		mdelay(100);

		/* read again */
		sampfreq = cvorg_readw(cvorg, CVORG_SAMPFREQ) / CVORG_FREQ_DIV;
		i++;
	}

	return 0;
}

static void __cvorg_apply_channel_inpol(struct cvorg_channel *channel)
{
	if (channel->inpol == CVORG_NEGATIVE_PULSE)
		cvorg_ochan(channel, CVORG_CONFIG, CVORG_CONFIG_INPOL);
	else
		cvorg_achan(channel, CVORG_CONFIG, ~CVORG_CONFIG_INPOL);
}

static void __cvorg_apply_channel_outoff(struct cvorg_channel *channel)
{
	if (channel->outoff == CVORG_OUT_OFFSET_0V)
		cvorg_achan(channel, CVORG_CONFIG, ~CVORG_CONFIG_OUT_OFFSET);
	else
		cvorg_ochan(channel, CVORG_CONFIG, CVORG_CONFIG_OUT_OFFSET);
}

static void __cvorg_apply_channel_outgain(struct cvorg_channel *channel)
{
	uint32_t val = cvorg_gain_to_hw(channel->outgain);

	val <<= CVORG_CONFIG_OUT_GAIN_SHIFT;
	cvorg_uchan(channel, CVORG_CONFIG, val, CVORG_CONFIG_OUT_GAIN_MASK);
}

static void __cvorg_apply_channel_out_enable(struct cvorg_channel *channel)
{
	if (channel->out_enabled)
		cvorg_ochan(channel, CVORG_CONFIG, CVORG_CONFIG_OUT_ENABLE);
	else
		cvorg_achan(channel, CVORG_CONFIG, ~CVORG_CONFIG_OUT_ENABLE);
}

static void __cvorg_apply_channel_cfg(struct cvorg_channel *channel)
{
	__cvorg_apply_channel_inpol(channel);
	__cvorg_apply_channel_outoff(channel);
	__cvorg_apply_channel_outgain(channel);
	__cvorg_apply_channel_out_enable(channel);
}

static int __cvorg_apply_cfg(struct cvorg *cvorg)
{
	if (clkgen_apply_pll_conf(cvorg))
		printk(KERN_WARNING PFX "PLL config failed. Continuing, though..\n");

	__cvorg_apply_channel_cfg(&cvorg->channels[0]);
	__cvorg_apply_channel_cfg(&cvorg->channels[1]);

	return 0;
}

int cvorg_hw_version(struct cvorg *cvorg, char *mver)
{
	uint32_t ver;
	unsigned int vhdl_tens, vhdl_units, vhdl_tenths, vhdl_hundredths;

	/* read VHDL revision */
	ver = cvorg_readw(cvorg, CVORG_VERSION);
	vhdl_tens	= (ver >> CVORG_VERSION_TENS_SHIFT)		& 0xf;
	vhdl_units	= (ver >> CVORG_VERSION_UNITS_SHIFT)		& 0xf;
	vhdl_tenths	= (ver >> CVORG_VERSION_TENTHS_SHIFT)		& 0xf;
	vhdl_hundredths	= (ver >> CVORG_VERSION_HUNDREDTHS_SHIFT)	& 0xf;

	sprintf(mver, "v%d%d.%d%d",
		vhdl_tens, vhdl_units, vhdl_tenths, vhdl_hundredths);

	return 0;
}

static void channel_reset(struct cvorg_channel *channel)
{
	cvorg_wchan(channel, CVORG_CTL, CVORG_CTL_CHAN_RESET);
	udelay(5);
}

static void disable_interrupts(struct cvorg *cvorg)
{
	/* disable interrupts */
	cvorg_writew(cvorg, CVORG_INTEN, 0);

	/* clear interrupts */
	cvorg_readw(cvorg, CVORG_INTSR);
}

static void config_interrupts(struct cvorg *cvorg)
{

	if (cvorg->irq == 0) {
		printk(KERN_INFO PFX "cvorg%d No interrupts configured", cvorg->lun);
		disable_interrupts(cvorg);
		return;
	}

	cvorg_writew(cvorg, CVORG_INTVECT, cvorg->irq);
	cvorg_writew(cvorg, CVORG_INTLEVEL, cvorg->irq_level);
}

static void enable_interrupts(struct cvorg *cvorg)
{
	uint32_t mask = CVORG_INTEN_ENDFUNC;

	cvorg_writew(cvorg, CVORG_INTEN, mask & CVORG_INTEN_VALID);
}

static void cvorg_channel_cfg_init(struct cvorg_channel *channel)
{
	channel->inpol	= CVORG_POSITIVE_PULSE;
	channel->outoff = CVORG_OUT_OFFSET_0V;
	channel->outgain = 20;
	channel->out_enabled = 0;
}

static void cvorg_cfg_init(struct cvorg *cvorg)
{
	cvorg_pll_cfg_init(&cvorg->pll);
	cvorg_channel_cfg_init(&cvorg->channels[0]);
	cvorg_channel_cfg_init(&cvorg->channels[1]);
}

static int __cvorg_reset(struct cvorg *cvorg)
{
	int ret;

	/* reset the FPGA */
	cvorg_writew(cvorg, CVORG_CTL, CVORG_CTL_FPGA_RESET);
	udelay(5);

	/* reset both channels */
	channel_reset(&cvorg->channels[0]);
	channel_reset(&cvorg->channels[1]);

	/* load the default config */
	cvorg_cfg_init(cvorg);

	/* start-up clkgen */
	clkgen_startup(cvorg);

	/* apply the config */
	ret = __cvorg_apply_cfg(cvorg);

	/* initialise interrupts */
	config_interrupts(cvorg);
	enable_interrupts(cvorg);

	return ret;
}

int cvorg_reset(struct cvorg *cvorg)
{
	int ret;

	mutex_lock(&cvorg->lock);
	ret = __cvorg_reset(cvorg);
	mutex_unlock(&cvorg->lock);

	return ret;
}

static void cvorg_init_channel(struct cvorg *cvorg, int index)
{
	struct cvorg_channel *channel = &cvorg->channels[index];

	channel->parent = cvorg;

	if (index == 0)
		channel->reg_offset = CVORG_CHAN1_OFFSET;
	else
		channel->reg_offset = CVORG_CHAN2_OFFSET;
}

static void cvorg_init_channels(struct cvorg *cvorg)
{
	cvorg_init_channel(cvorg, 0);
	cvorg_init_channel(cvorg, 1);
}

static int cvorg_version_check(struct cvorg *cvorg)
{
	uint32_t val;

	val = cvorg_readw(cvorg, CVORG_VERSION);

	/* store the hardware revision number */
	cvorg->hw_rev = val & CVORG_VERSION_FW_MASK;

	val >>= CVORG_VERSION_FW_SHIFT;
	val &= 0xff;

	return val != 'G';
}

/* map cvorg VME address space */

static struct pdparam_master param = {
	.iack   = 1,			/* no iack */
	.rdpref = 0,			/* no VME read prefetch option */
	.wrpost = 0,			/* no VME write posting option */
	.swap   = 1,			/* VME auto swap option */
	.dum    = { VME_PG_SHARED,	/* window is sharable */
			0,		/* XPC ADP-type */
			0, },		/* window is sharable */
};


static unsigned long cvorg_map(unsigned long base_address)
{
	return find_controller(base_address, CVORG_WINDOW_LENGTH,
		CVORG_ADDRESS_MODIFIER, 0, CVORG_DATA_SIZE, &param);
}

static inline uint32_t get_first_set_bit(uint32_t mask)
{
	int i;
	int retval;

	for (i = 0; i < CVORG_NR_INTERRUPTS; i++) {
		retval = 1 << i;
		if (retval & mask)
			return retval; /* only one bit per interrupt */
	}
	return 0;
}

/* Note: An interrupt is triggered from a single channel at a time. */
int cvorg_isr(void *arg)
{
	struct cvorg *cvorg = (struct cvorg *)arg; 
	struct cvorg_channel *channel;
	uint32_t mask, source;

	mask = cvorg_readw(cvorg, CVORG_INTSR);

	if (!(mask & CVORG_INTSR_BITMASK))
		return IRQ_NONE; /* no ISR coming from this module */

	/* check which channel it comes from */
	channel = &cvorg->channels[mask & CVORG_INTSR_CHANNEL];
	source = get_first_set_bit(mask);

	switch (source) {
	case CVORG_INTSR_ENDFUNC:
	default:
		/* do nothing for the time being */
		break;
	}

	return IRQ_HANDLED;
}

static int cvorg_open(struct inode *inode, struct file *file)
{
        struct cvorg *card;

        card = container_of(inode->i_cdev, struct cvorg, cdev);
        file->private_data = card;
        return 0;
}

static int cvorg_release(struct inode *inode, struct file *file)
{
        return 0;
}

static long cvorg_ioctl(struct file *fp, unsigned op, unsigned long arg)
{
        struct cvorg *cvorg = container_of(fp->f_dentry->d_inode->i_cdev,
					   struct cvorg, cdev);
	int ret;

	switch(op) {
	case CVORG_IOCTL_ADC:
	{
		struct cvorg_adc adc;;	

		if(copy_from_user((char *)&adc, (char *)arg, sizeof(struct cvorg_adc)))
			return -EIO;

		ret = cvorg_chan_adc_read(&cvorg->channels[adc.channel], &adc);
		if (ret)
			return ret;

		if(copy_to_user((char *)arg, (char *)&adc, sizeof(struct cvorg_adc)))
			return -EFAULT;
		break;
	}
	
	case CVORG_IOCTL_READ_SRAM:
	{
		struct cvorg_sram_entry sram;;	

		if(copy_from_user((char *)&sram, (char *)arg, sizeof(struct cvorg_sram_entry)))
			return -EIO;

		ret = cvorg_chan_sram(&cvorg->channels[sram.channel], &sram, 0);
		if (ret)
			return ret;

		if(copy_to_user((char *)arg, (char *)&sram, sizeof(struct cvorg_sram_entry)))
			return -EFAULT;
		break;
	}

	case CVORG_IOCTL_WRITE_SRAM:
	{
		struct cvorg_sram_entry sram;;	

		if(copy_from_user((char *)&sram, (char *)arg, sizeof(struct cvorg_sram_entry)))
			return -EIO;

		ret = cvorg_chan_sram(&cvorg->channels[sram.channel], &sram, 1);
		if (ret)
			return ret;

		if(copy_to_user((char *)arg, (char *)&sram, sizeof(struct cvorg_sram_entry)))
			return -EFAULT;
		break;
	}

	default:
		printk(KERN_INFO PFX "Invalid IOCTL\n");
		return -EINVAL;
	}
 
	return 0;

}

static const struct file_operations cvorg_fops = { 
        .owner          = THIS_MODULE,
        .open           = cvorg_open,
        .release        = cvorg_release,
	.unlocked_ioctl = cvorg_ioctl,
};



int cvorg_module_install(struct device *pdev, uint32_t lun, unsigned long base_address, uint32_t irq, uint32_t irq_level, unsigned int index)
{
	struct cvorg *cvorg;
	int ret;
	dev_t devno = MKDEV(MAJOR(cvorg_devno), lun);

	cvorg = (void *)kmalloc(sizeof(*cvorg), GFP_KERNEL);

	if (cvorg == NULL) {
		printk(KERN_ERR PFX "Not enough memory for the module's data\n");
		return -ENOMEM;
	}
	memset((void *)cvorg, 0, sizeof(*cvorg));

	cvorg->lun = lun;
	cvorg->irq = irq;
	cvorg->irq_level = irq_level;
	cvorg->channels[0].chan_nr = 0;
	cvorg->channels[1].chan_nr = 1;
	cvorg->n_channels = CVORG_CHANNELS;

        dev_set_drvdata(pdev, cvorg);

	cvorg->iomap = (void *)cvorg_map(base_address);
	if (cvorg->iomap == NULL) {
		printk(KERN_ERR PFX "Can't find virtual address of the module's registers\n");
		ret = -EINVAL;
		goto out_free;
	}

	if (cvorg_version_check(cvorg)) {
		printk(KERN_ERR PFX "cvorg.%d: device not present\n", cvorg->lun);
		ret = -ENODEV;
		goto out_free;
	}

	ret = vme_request_irq(irq, cvorg_isr, cvorg, "cvorg");
	if (ret) {
		printk(KERN_ERR PFX "cvorg.%d cannot register ISR\n", lun);
		goto out_free;
	}

	cdev_init(&cvorg->cdev, &cvorg_fops);
	cvorg->cdev.owner = THIS_MODULE;

	ret = cdev_add(&cvorg->cdev, devno, 1);
	if (ret) {
		printk(KERN_ERR PFX "Error adding cdev %d\n", lun);
		goto out_irq;
	}
	
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
	cvorg->dev = device_create(cvorg_class, pdev, devno, NULL, "cvorg.%d", lun);
#else
	cvorg->dev = device_create(cvorg_class, pdev, devno, "cvorg.%d", lun);
#endif

	if(IS_ERR(cvorg->dev)) {
		printk(KERN_ERR PFX "error %ld creating cvorg.%d device\n", PTR_ERR(cvorg->dev), lun);
		ret = PTR_ERR(cvorg->dev);
		goto out_device_create;
	}

	dev_set_drvdata(cvorg->dev, cvorg);

	ret = cvorg_create_sysfs_files(cvorg);
	if (ret)
		goto out_sysfs;

	cvorg_table[index] = cvorg;

	mutex_init(&cvorg->lock);
	cvorg_init_channels(cvorg);

	ret = cvorg_reset(cvorg);
	if (ret)
		goto out_reset;


	return 0;

 out_reset:
	cvorg_remove_sysfs_files(cvorg);

 out_sysfs:
	cvorg->dev = NULL;

 out_device_create:
	cdev_del(&cvorg->cdev);

 out_irq:
	vme_free_irq(irq);

 out_free:
	if (cvorg) {
		kfree((void *)cvorg);
	}
	return ret;
}

static void cvorg_quiesce_channel(struct cvorg_channel *channel)
{
	/* disable waveform playing */
	cvorg_achan(channel, CVORG_CONFIG, ~CVORG_CONFIG_SEQREADY);

	/* make sure there's nothing being played right now */
	cvorg_wchan(channel, CVORG_CTL, CVORG_CTL_CHAN_STOP);
}

void cvorg_module_uninstall(struct cvorg *cvorg)
{
	int i;
	dev_t devno = MKDEV(MAJOR(cvorg_devno), cvorg->lun);

	if (cvorg == NULL)
		return;

	/* Delete sysfs files */
	cvorg_remove_sysfs_files(cvorg);

	/* disable channels */
	for (i = 0; i < CVORG_CHANNELS; i++)
		cvorg_quiesce_channel(&cvorg->channels[i]);

	/* disable interrupts */
	disable_interrupts(cvorg);
	vme_free_irq(cvorg->irq);

	/* Unmap the memory */
	return_controller((unsigned long)cvorg->iomap, CVORG_WINDOW_LENGTH);

	device_destroy(cvorg_class, devno);
	cdev_del(&cvorg->cdev);

	/* free userdata */
	kfree((void *)cvorg);
}

static void free_wv(struct cvorg_wv *wv)
{
	if (!wv)
		return;
	if (!wv->form || !wv->size)
		return;
	kfree((void *)wv->form);
}

static void free_seq(struct cvorg_seq *seq)
{
	int i;

	if (!seq)
		return;
	if (!seq->n_waves || !seq->waves)
		goto out;
	for (i = 0; i < seq->n_waves; i++)
		free_wv(&seq->waves[i]);

 out:
	kfree((void *)seq->waves);
}

static int copy_wv_from_user(struct cvorg_wv *to, struct cvorg_wv *from)
{
	/* copy the waveform descriptor */
	if (copy_from_user(to, from, sizeof(struct cvorg_wv))) {
		printk(KERN_ERR PFX "cannot copy the waveform descriptor\n");
		goto out_perm_err;
	}

	/* allocate space for the waveform */
	to->form = (void *)kmalloc(to->size, GFP_KERNEL);
	if (to->form == NULL) {
		printk(KERN_ERR PFX "Not enough memory to allocate the waveform\n");
		goto out_err;
	}

	/* copy the waveform */
	if (copy_from_user(to->form, from->form, to->size)) {
		printk(KERN_ERR PFX "cannot copy the waveform from user-space\n");
		goto out_perm_err;
	}

	return 0;
 out_err:
	return -ENOMEM;
 out_perm_err:
	return -EFAULT;
}

/**
 * seq_init - initialise a sequence from a user's descriptor
 *
 * @seq:	sequence descriptor to be initialised
 * @user_seq:	user's sequence descriptor
 *
 * Note that @seq is in kernel space. However the contents of
 * any of the pointers it may have are pointing to user-space addresses.
 * This function sets errno upon failure.
 *
 * @return 0 - on success
 * @return -1 - on failure
 */
static int seq_init(struct cvorg_seq *seq, struct cvorg_seq *user_seq)
{
	int i;

	memcpy(seq, user_seq, sizeof(struct cvorg_seq));

	/* allocate space for the waveform descriptors */
	seq->waves = (void *)kmalloc(seq->n_waves * sizeof(struct cvorg_wv), GFP_KERNEL);
	if (seq->waves == NULL) {
		printk(KERN_ERR PFX "ENOMEM for the waveform descriptors (%d waves)\n",
			seq->n_waves);
		goto out_mem_err;
	}

	/* copy the descriptors into kernel space */
	for (i = 0; i < seq->n_waves; i++)
		if (copy_wv_from_user(&seq->waves[i], &user_seq->waves[i])) {
			printk(KERN_ERR PFX "Copying the waveform from user-space failed\n");
			goto out_err;
		}
	return 0;

 out_mem_err:
	free_seq(seq);
	return -ENOMEM;
 out_err:
	free_seq(seq);
	return -1;
}

static ssize_t seq_size(struct cvorg_seq *seq)
{
	ssize_t size = 0;
	int i;

	for (i = 0; i < seq->n_waves; i++)
		size += seq->waves[i].size;
	return size;
}

static ssize_t avail_sram(struct cvorg_channel *channel)
{
	return CVORG_SRAM_SIZE;
}

/*
 * Get the offset on mapped memory of a descriptor block
 * Note that the blocks are written contiguously
 */
static inline unsigned int block_offset(int blocknr)
{
	return CVORG_BLKOFFSET + CVORG_BLKSIZE * blocknr;
}

/* get the offset of the previous descriptor block on mapped memory */
static inline unsigned int prev_block_offset(int blocknr)
{
	return CVORG_BLKOFFSET + CVORG_BLKSIZE * (blocknr - 1);
}

/*
 * Given two 16-bit long values, convert to a 32-bit SRAM entry
 * Note: the module plays first the rightmost u16 of a 32-bit SRAM entry
 */
static inline uint32_t cvorg_cpu_to_hw(uint16_t *data)
{
	uint32_t first = cpu_to_be16(data[0]);
	uint32_t second = cpu_to_be16(data[1]);

	return (first << 16) | second;
}

/*
 * Given a 32-bit long SRAM entry, convert to two 16-bit long values
 */
static inline void cvorg_hw_to_cpu(uint32_t hwval, uint16_t *data)
{
	uint32_t first, second;

	first = hwval >> 16;
	second = hwval & 0xffff;

	data[0] = be16_to_cpu(first);
	data[1] = be16_to_cpu(second);
}

static inline uint32_t cvorg_cpu_to_hw_single(uint16_t *data)
{
	uint32_t pointval = cpu_to_be16(data[0]);

	return pointval << 16;
}

/*
 * The two LSBs are discarded by the module.
 * Since they might be of some use in the future, we set them
 * to zero here, rounding up 3 to 4.
 */
static void normalize_wv(struct cvorg_wv *wave)
{
	uint16_t *data = (uint16_t *)wave->form;
	int len = wave->size / sizeof(uint16_t);
	int i;

	for (i = 0; i < len; i++) {
		/* round up making sure there's no overflow */
		if (data[i] & 0x3 && data[i] != 0xffff)
			data[i] += 1;

		data[i] &= ~0x3;
	}
}

/*
 * Note. the caller has to make sure that he's going to write to a valid
 * location
 */
static void
__sram(struct cvorg_channel *chan, struct cvorg_wv *wave, unsigned dest)
{
	uint16_t *data = (uint16_t *)wave->form;
	int len = wave->size / 2;
	int i;

	/* the address auto-increments on every write */
	cvorg_wchan(chan, CVORG_SRAMADDR, dest);

	/* each write stores two points */
	for (i = 0; i < len - 1; i += 2)
		cvorg_wchan_noswap(chan, CVORG_SRAMDATA, cvorg_cpu_to_hw(&data[i]));

	if (len % 2) {
		uint32_t lastpoint = cvorg_cpu_to_hw_single(&data[i]);

		cvorg_wchan_noswap(chan, CVORG_SRAMDATA, lastpoint);
	}
}

static inline void
cvorg_wv_mark_prev(struct cvorg_channel *channel, unsigned int prevblock,
		unsigned int currblock_index)
{
	unsigned int offset = prevblock + CVORG_WFNEXTBLK;

	cvorg_uchan(channel, offset, currblock_index, CVORG_WFNEXTBLK_NEXT_MASK);
}

static inline void
cvorg_wv_mark_last(struct cvorg_channel *channel, unsigned int block)
{
	unsigned int offset = block + CVORG_WFNEXTBLK;

	/* write the magic number which marks the end of the sequence */
	cvorg_uchan(channel, offset, CVORG_WFNEXTBLK_LAST, CVORG_WFNEXTBLK_NEXT_MASK);
}

static inline void
cvorg_wv_gain_store(struct cvorg_channel *channel, unsigned int block,
		struct cvorg_wv *wv)
{
	unsigned int offset = block + CVORG_WFNEXTBLK;
	unsigned int val;

	/* set to the default gain (unset the CONF_GAIN bit) */
	cvorg_achan(channel, offset, ~CVORG_WFNEXTBLK_CONF_GAIN);

	/* exit if the user didn't explicitly set the gain */
	if (!wv->dynamic_gain)
		return;

	wv->gain_val = 	cvorg_gain_approx(wv->gain_val);
	val = cvorg_gain_to_hw(wv->gain_val);

	/* set the CONF_GAIN bit */
	cvorg_ochan(channel, offset, CVORG_WFNEXTBLK_CONF_GAIN);

	/* and write the specified gain to the module */
	val <<= CVORG_WFNEXTBLK_GAIN_SHIFT;
	cvorg_uchan(channel, offset, val, CVORG_WFNEXTBLK_GAIN_MASK);
}

static inline void
cvorg_wv_store(struct cvorg_channel *channel, unsigned int block,
	struct cvorg_wv *wv)
{
	cvorg_wchan(channel, block + CVORG_WFLEN, wv->size / 2);
	cvorg_wchan(channel, block + CVORG_WFRECURR, wv->recurr);
	cvorg_wv_gain_store(channel, block, wv);
}

/*
 * Note: The caller has to make sure that there's enough memory in the channel
 * and that the SRAM can be safely accessed.
 */
static void __cvorg_storeseq(struct cvorg_channel *channel,
			struct cvorg_seq *seq)
{
	unsigned int currblock;
	unsigned int ramaddr = 0;
	struct cvorg_wv *wv;
	int i;

	/*
	 * If there's only one waveform in the sequence, the hardware
	 * only accepts 1 as the number of sequence cycles.
	 * To loop we have then to put the number of iterations in
	 * the number of waveform cycles--hence the multiplication below.
	 */
	if (seq->n_waves == 1) {
		wv = &seq->waves[0];

		wv->recurr *= seq->nr;
		seq->nr = 1;
	}

	cvorg_wchan(channel, CVORG_SEQNR, seq->nr);

	for (i = 0; i < seq->n_waves; i++) {
		wv = &seq->waves[i];

		normalize_wv(wv);

		/* write to sram */
		__sram(channel, wv, ramaddr);

		/* get the offset of the current block */
		currblock = block_offset(i);

		/* point the previous block to the current one */
		if (i > 0)
			cvorg_wv_mark_prev(channel, prev_block_offset(i), i);

		/* point the block to the address in SRAM */
		cvorg_wchan(channel, currblock + CVORG_WFSTART, ramaddr);

		/* fill the rest of the block descriptor */
		cvorg_wv_store(channel, currblock, wv);

		/* mark the last item of the sequence as such */
		if (i == seq->n_waves - 1)
			cvorg_wv_mark_last(channel, currblock);

		ramaddr += wv->size;
	}
}

/**
 * cvorg_storeseq - store a sequence
 *
 * @channel:	channel to write it to
 * @seq:	sequence to be written
 *
 * @seq is a descriptor containing the set of waveforms to be written. This
 * function checks if there's enough room in the module to host the waveforms
 * and if that's the case, the set of waveforms are written to the module.
 *
 * return 0	- on success
 * return -1	- on failure
 */
static int cvorg_storeseq(struct cvorg_channel *channel, struct cvorg_seq *seq)
{
	/* check there's enough space on SRAM */
	if (seq_size(seq) > avail_sram(channel)) {
		printk(KERN_WARNING PFX "Not enough memory on the channel for the desired "
			"waveform. Available/Requested: %zd/%zd\n",
			avail_sram(channel), seq_size(seq));
		goto mem_err;
	}

	if (seq->n_waves <= 0 || seq->n_waves > CVORG_MAX_SEQUENCES) {
		printk(KERN_WARNING PFX "Invalid number of desired blocks. Maximum=%d\n",
			CVORG_MAX_SEQUENCES);
		goto val_err;
	}

	if (__cvorg_sram_busy(channel)) {
		printk(KERN_WARNING PFX "Access to the module's SRAM timed out\n");
		goto sram_err;
	}

	__cvorg_storeseq(channel, seq);

	return 0;

 sram_err:
	return -EAGAIN;
 mem_err:
	return -ENOMEM;
 val_err:
	return -EINVAL;
}

static void cvorg_poll_startable(struct cvorg_channel *channel)
{
	uint32_t reg;
	int i;

	reg = cvorg_rchan(channel, CVORG_STATUS);
	i = 0;
	while (reg & CVORG_STATUS_SRAM_BUSY ||
		!(reg & CVORG_STATUS_STARTABLE)) {
		if (i > 20) {
			printk(KERN_INFO PFX "Timeout expired: module not startable or RAM busy\n");
			return;
		}
		udelay(10);
		reg = cvorg_rchan(channel, CVORG_STATUS);
		i++;
	}
}

static void play_wv(struct cvorg_channel *channel)
{
	uint32_t cmd;

	/* Check the status. If it is not ready, we still continue */
	__cvorg_channel_ready(channel);

	/* Mark the sequence in SRAM as valid and set to normal mode */
	cmd = CVORG_CONFIG_MODE_NORMAL | CVORG_CONFIG_SEQREADY;
	cvorg_uchan(channel, CVORG_CONFIG, cmd, CVORG_CONFIG_MODE);

	/* Go read the sequence from SRAM */
	cvorg_wchan(channel, CVORG_CTL, CVORG_CTL_CHAN_LOADSEQ);

	/* sleep until the module can accept START triggers */
	cvorg_poll_startable(channel);
}

static int __cvorg_loadseq(struct cvorg_channel *channel, struct cvorg_seq *seq)
{
	/* not even an external trigger will disturb us from now on */
	cvorg_quiesce_channel(channel);

	/* write sequence to SRAM */
	if (cvorg_storeseq(channel, seq))
		return -1;

	/* issue the write */
	play_wv(channel);

	return 0;
}

static int
cvorg_invalid_seq(struct cvorg_channel *channel, struct cvorg_seq *seq)
{
	int i;
	struct cvorg_wv *wv;

	if (!seq->n_waves) {
		printk(KERN_INFO PFX "seq->n_waves invalid\n");
		return -EINVAL;
	}

	if (!seq->waves) {
		printk(KERN_INFO PFX "empty seq->waves\n");
		return -EINVAL;
	}

	for (i = 0; i < seq->n_waves; i++) {
		wv = &seq->waves[i];

		if (cvorg_check_gain_offset_wv(channel, wv)) {
			printk(KERN_INFO PFX "wave %d: Invalid gain\n", i);
			return -EINVAL;
		}

		if (cvorg_invalid_freqlimit(channel, seq, wv)) {
			printk(KERN_INFO PFX "wave %d: not enough points or frequency > 100 MHz\n", i);
			return -EINVAL;
		}

		if (cvorg_rchan(channel, CVORG_SAMPFREQ) > 50000000 &&
			wv->size / sizeof(uint16_t) < 2) {
			printk(KERN_INFO PFX "wave %d: at least 2 points are required "
				"at frequencies higher than 50MHz\n", i);
			return -EINVAL;
		}
	}

	return 0;
}

/*
 * The waveforms in the sequence might have been updated by the driver.
 * Think, for instance, of fields that can be rounded, such as the
 * value of the dynamic gain.
 * This function writes back the updated values of these fields into user-space.
 */
static int seq_user_writeback(struct cvorg_seq *useq, struct cvorg_seq *kseq)
{
	int i;
	struct cvorg_wv *uwv;
	struct cvorg_wv *kwv;

	for (i = 0; i < kseq->n_waves; i++) {
		kwv = &kseq->waves[i];
		uwv = &useq->waves[i];

		if (!kwv->dynamic_gain)
			continue;

		if (copy_to_user(&uwv->gain_val, &kwv->gain_val,
					sizeof(&kwv->gain_val))) {
			printk(KERN_ERR PFX "Copying the gain value to user-space failed\n");
			return -EFAULT;
		}
	}
	return 0;
}

/* write a sequence to a channel and play it */
int cvorg_chan_loadseq(struct cvorg_channel *channel, void *arg)
{
	struct cvorg_seq *user_seq = arg;
	struct cvorg_seq seq;
	int ret = 0;

	/*
	 * We're going to load a new configuration. We don't care if a
	 * previous faulty config caused an error, so we wipe it out.
	 */
	__cvorg_channel_clear_errors(channel);

	if (!__cvorg_channel_ready(channel)) {
		return -EBUSY;
	}

	ret = cvorg_invalid_seq(channel, user_seq);
	if (ret)
		return ret;

	if (seq_init(&seq, user_seq)) {
		printk(KERN_INFO PFX "Initialisation of the sequence failed\n");
		return -1;
	}

	if (__cvorg_loadseq(channel, &seq)) {
		printk(KERN_INFO PFX "__cvorg_loadseq failed\n");
		ret = -1;
		goto out;
	}

	if (seq_user_writeback(user_seq, &seq)) {
		printk(KERN_INFO PFX "seq_copy_to_user failed\n");
		ret = -1;
		goto out;
	}

 out:
	free_seq(&seq);
	return ret;
}

int cvorg_chan_offset(struct cvorg_channel *channel, void *arg, int set)
{
	uint32_t *offset = arg;

	if (set) {
		if (cvorg_invalid_gain_offset(channel->outgain, *offset)) {
			printk(KERN_INFO PFX "With an offset of 2.5V, a gain of 14dB must "
				"be applied.\n");
			return -EINVAL;
		}

		switch (*offset) {
		case CVORG_OUT_OFFSET_2_5V:
			channel->outoff = CVORG_OUT_OFFSET_2_5V;
			break;
		case CVORG_OUT_OFFSET_0V:
		default:
			channel->outoff = CVORG_OUT_OFFSET_0V;
			break;
		}

		__cvorg_apply_channel_outoff(channel);
		return 0;
	}

	*offset = channel->outoff;
	return 0;
}

int cvorg_chan_inpol(struct cvorg_channel *channel, void *arg, int set)
{
	enum cvorg_inpol *polarity = arg;

	if (set) {
		if (__cvorg_channel_busy(channel)) {
			return -EBUSY;
		}

		if (*polarity)
			channel->inpol = CVORG_NEGATIVE_PULSE;
		else
			channel->inpol = CVORG_POSITIVE_PULSE;
		__cvorg_apply_channel_inpol(channel);

		return 0;
	}

	*polarity = channel->inpol;
	return 0;
}

/* Software triggers: start, stop, event stop. */
int cvorg_chan_sw_trigger(struct cvorg_channel *channel, void *arg)
{
	uint32_t *trig = arg;

	switch (*trig) {
	case CVORG_TRIGGER_START:
		cvorg_wchan(channel, CVORG_CTL, CVORG_CTL_CHAN_START);
		break;
	case CVORG_TRIGGER_STOP:
		cvorg_wchan(channel, CVORG_CTL, CVORG_CTL_CHAN_STOP);
		break;
	case CVORG_TRIGGER_EVSTOP:
		cvorg_wchan(channel, CVORG_CTL, CVORG_CTL_CHAN_EVSTOP);
		break;
	default:
		printk(KERN_INFO PFX "Unknown trigger event (0x%x)\n", *trig);
		return -EINVAL;
	}
	return 0;
}

/* Note: the PLL is common to both channels */
static int __cvorg_apply_pll(struct cvorg *cvorg)
{
	int ret = 0;

	if (clkgen_apply_pll_conf(cvorg)) {
		ret = -1;
		goto out;
	}

	if (cvorg_poll_sampfreq(cvorg)) {
		ret = -1;
		goto out;
	}

 out:
	return ret;
}

/*
 * each client has its own's PLL configuration. When the client issues a write,
 * his particular configuration is set in the hardware.
 */
int cvorg_pll(struct cvorg *cvorg, void *arg, int set)
{
	struct ad9516_pll *pll = arg;

	if (!set) {
		memcpy(pll, &cvorg->pll, sizeof(*pll));
		return 0;
	}

	if (__cvorg_busy(cvorg)) {
		return -EBUSY;
	}

	/* sanity check */
	if (clkgen_check_pll(pll))
		return -1;

	memcpy(&cvorg->pll, pll, sizeof(*pll));
	return __cvorg_apply_pll(cvorg);
}

int cvorg_chan_sram(struct cvorg_channel *channel, void *arg, int set)
{
	struct cvorg_sram_entry *entry = arg;

	if (set && __cvorg_channel_busy(channel)) {
		return -EBUSY;
	}

	/* sanity check */
	if (entry->address >= CVORG_SRAM_SIZE) {
		return -EINVAL;
	}

	if (__cvorg_sram_busy(channel)) {
		printk(KERN_INFO PFX "Access to SRAM timed out\n");
		return -EAGAIN;
	}

	/* avoid unaligned accesses */
	cvorg_wchan(channel, CVORG_SRAMADDR, entry->address & ~0x3);

	if (!set) {
		uint32_t rval = cvorg_rchan_noswap(channel, CVORG_SRAMDATA);

		cvorg_hw_to_cpu(rval, entry->data);
		return 0;
	}

	cvorg_wchan_noswap(channel, CVORG_SRAMDATA, cvorg_cpu_to_hw(entry->data));

	return 0;
}

int cvorg_chan_status(struct cvorg_channel *channel, void *arg)
{
	uint32_t *status = arg;
	uint32_t val;

	*status = 0;

	val = cvorg_rchan(channel, CVORG_STATUS);

	if (val & CVORG_STATUS_READY)
		*status |= CVORG_CHANSTAT_READY;

	if (val & CVORG_STATUS_BUSY)
		*status |= CVORG_CHANSTAT_BUSY;

	if (val & CVORG_STATUS_SRAM_BUSY)
		*status |= CVORG_CHANSTAT_SRAM_BUSY;

	if (val & CVORG_STATUS_ERR)
		*status |= CVORG_CHANSTAT_ERR;

	if (val & CVORG_STATUS_ERR_CLK)
		*status |= CVORG_CHANSTAT_ERR_CLK;

	if (val & CVORG_STATUS_ERR_TRIG)
		*status |= CVORG_CHANSTAT_ERR_TRIG;

	if (val & CVORG_STATUS_ERR_SYNC)
		*status |= CVORG_CHANSTAT_ERR_SYNC;

	if (val & CVORG_STATUS_CLKOK)
		*status |= CVORG_CHANSTAT_CLKOK;

	if (val & CVORG_STATUS_STARTABLE)
		*status |= CVORG_CHANSTAT_STARTABLE;

	val = cvorg_rchan(channel, CVORG_CONFIG);
	if (val & CVORG_CONFIG_OUT_ENABLE)
		*status |= CVORG_CHANSTAT_OUT_ENABLED;

	return 0;
}

int cvorg_read_sampfreq(struct cvorg *cvorg, void *arg)
{
	uint32_t *freq = arg;

	*freq = cvorg_readw(cvorg, CVORG_SAMPFREQ);
	return 0;
}

static enum cvorg_mode cvorg_hw_to_mode(uint32_t regval)
{
	switch (regval) {
	case CVORG_CONFIG_MODE_OFF:
		return CVORG_MODE_OFF;

	case CVORG_CONFIG_MODE_NORMAL:
		return CVORG_MODE_NORMAL;

	case CVORG_CONFIG_MODE_DAC:
		return CVORG_MODE_DAC;

	case CVORG_CONFIG_MODE_TEST1:
		return CVORG_MODE_TEST1;

	case CVORG_CONFIG_MODE_TEST2:
		return CVORG_MODE_TEST2;

	case CVORG_CONFIG_MODE_TEST3:
		return CVORG_MODE_TEST3;

	default:
		printk(KERN_WARNING PFX "Unknown operation mode. Returning OFF\n");
		return CVORG_MODE_OFF;
	}
}

static int cvorg_mode_is_valid(uint32_t mode)
{
	switch (mode) {
	case CVORG_MODE_OFF:
	case CVORG_MODE_NORMAL:
	case CVORG_MODE_DAC:
	case CVORG_MODE_TEST1:
	case CVORG_MODE_TEST2:
	case CVORG_MODE_TEST3:
		return 1;
	default:
		return 0;
	}
}

static uint32_t cvorg_mode_to_hw(uint32_t mode)
{
	switch (mode) {
	case CVORG_MODE_OFF:
		return CVORG_CONFIG_MODE_OFF;

	case CVORG_MODE_NORMAL:
		return CVORG_CONFIG_MODE_NORMAL;

	case CVORG_MODE_DAC:
		return CVORG_CONFIG_MODE_DAC;

	case CVORG_MODE_TEST1:
		return CVORG_CONFIG_MODE_TEST1;

	case CVORG_MODE_TEST2:
		return CVORG_CONFIG_MODE_TEST2;

	case CVORG_MODE_TEST3:
		return CVORG_CONFIG_MODE_TEST3;

	default:
		printk(KERN_WARNING PFX "Unknown mode (0x%x). Setting to NORMAL\n", mode);
		return CVORG_CONFIG_MODE_NORMAL;
	}
}

static void cvorg_mode_set(struct cvorg_channel *channel, enum cvorg_mode mode)
{
	uint32_t regval;

	regval = cvorg_rchan(channel, CVORG_CONFIG);
	regval &= ~CVORG_CONFIG_MODE;
	regval |= cvorg_mode_to_hw(mode);

	cvorg_wchan(channel, CVORG_CONFIG, regval);
}

static enum cvorg_mode cvorg_mode_get(struct cvorg_channel *channel)
{
	uint32_t reg = cvorg_rchan(channel, CVORG_CONFIG);

	return cvorg_hw_to_mode(reg & CVORG_CONFIG_MODE);
}

int cvorg_chan_test_mode(struct cvorg_channel *channel, void *arg, int set)
{
	uint32_t *val = arg;

	if (!set) {
		*val = cvorg_mode_get(channel);
		return 0;
	}

	if (!cvorg_mode_is_valid(*val)) {
		return -EINVAL;
	}
	/*
	 * we don't check here if the channel is busy because this is only used
	 * by experts when testing the module.
	 */
	cvorg_mode_set(channel, *val);

	return 0;
}

int cvorg_chan_output_gain(struct cvorg_channel *channel, void *arg, int set)
{
	int32_t *outgain = arg;

	if (set) {
		int32_t gain = cvorg_gain_approx(*outgain);

		if (__cvorg_channel_busy(channel)) {
			return -EBUSY;
		}

		if (cvorg_invalid_gain_offset(gain, channel->outoff)) {
			printk(KERN_INFO PFX "With an offset of 2.5V, only a gain of 14dB "
				"can be applied.\n");
			return -EINVAL;
		}

		channel->outgain = gain;
		__cvorg_apply_channel_outgain(channel);
		return 0;
	}

	*outgain = channel->outgain;
	return 0;
}

static int cvorg_adc_channel_to_hw(enum cvorg_adc_channel channel, uint32_t *reg)
{
	uint32_t val;

	switch (channel) {
	case CVORG_ADC_CHANNEL_MONITOR:
		val = CVORG_CONFIG_ADCSELECT_MONITOR;
		break;
	case CVORG_ADC_CHANNEL_REF:
		val = CVORG_CONFIG_ADCSELECT_REF;
		break;
	case CVORG_ADC_CHANNEL_5V:
		val = CVORG_CONFIG_ADCSELECT_5V;
		break;
	case CVORG_ADC_CHANNEL_GND:
		val = CVORG_CONFIG_ADCSELECT_GND;
		break;
	default:
		return -1;
	}

	*reg &= ~CVORG_CONFIG_ADCSELECT_MASK;
	*reg |= val << CVORG_CONFIG_ADCSELECT_SHIFT;
	return 0;
}

static int cvorg_adc_range_to_hw(enum cvorg_adc_range range, uint32_t *reg)
{
	uint32_t val;

	switch (range) {
	case CVORG_ADC_RANGE_5V_BIPOLAR:
		val = CVORG_CONFIG_ADCRANGE_5V_BIPOLAR;
		break;
	case CVORG_ADC_RANGE_5V_UNIPOLAR:
		val = CVORG_CONFIG_ADCRANGE_5V_UNIPOLAR;
		break;
	case CVORG_ADC_RANGE_10V_BIPOLAR:
		val = CVORG_CONFIG_ADCRANGE_10V_BIPOLAR;
		break;
	case CVORG_ADC_RANGE_10V_UNIPOLAR:
		val = CVORG_CONFIG_ADCRANGE_10V_UNIPOLAR;
		break;
	default:
		return -1;
	}

	*reg &= ~CVORG_CONFIG_ADCRANGE_MASK;
	*reg |= val << CVORG_CONFIG_ADCRANGE_SHIFT;
	return 0;
}

int cvorg_chan_adc_read(struct cvorg_channel *channel, void *arg)
{
	struct cvorg_adc *adc = arg;
	uint32_t reg;
	int i;

	if (__cvorg_channel_busy(channel)) {
		return -EBUSY;
	}

	/* select the ADC channel and range */
	reg = 0;

	if (cvorg_adc_channel_to_hw(adc->channel, &reg)) {
		printk(KERN_INFO PFX "Invalid ADC Channel\n");
		return -EINVAL;
	}

	if (cvorg_adc_range_to_hw(adc->range, &reg)) {
		printk(KERN_INFO PFX "Invalid ADC Range\n");
		return -EINVAL;
	}
	cvorg_uchan(channel, CVORG_CONFIG, reg,
		CVORG_CONFIG_ADCSELECT_MASK | CVORG_CONFIG_ADCRANGE_MASK);

	/* trigger an ADC read on the configured ADC channel */
	cvorg_wchan(channel, CVORG_CTL, CVORG_CTL_READADC);

	/* wait for the ADC value to be up to date */
	reg = cvorg_rchan(channel, CVORG_ADCVAL);
	i = 0;
	while (!(reg & CVORG_ADCVAL_UP2DATE)) {
		if (i > 10) {
			printk(KERN_INFO PFX "Timeout expired for reading the ADC Value\n");
			return -EAGAIN;
		}
		udelay(10);
		reg = cvorg_rchan(channel, CVORG_ADCVAL);
		i++;
	}

	/* fetch the valid ADC value */
	adc->value = reg & CVORG_ADCVAL_READVAL;

	return 0;
}

/* experts-only function: no need to check if the module's busy */
int cvorg_chan_dac_val(struct cvorg_channel *channel, void *arg, int set)
{
	uint32_t *val = arg;

	if (set)
		cvorg_wchan(channel, CVORG_DACVAL, *val & CVORG_DACVAL_MASK);
	else
		*val = cvorg_rchan(channel, CVORG_DACVAL);

	return 0;
}

/* experts-only function: no need to check if the module's busy */
int cvorg_chan_dac_gain(struct cvorg_channel *channel, void *arg, int set)
{
	uint16_t *val = arg;

	if (set) {
		cvorg_uchan(channel, CVORG_DACCORR, *val & CVORG_DACCORR_GAIN_MASK,
			CVORG_DACCORR_GAIN_MASK);
	} else {
		uint32_t regval = cvorg_rchan(channel, CVORG_DACCORR);

		regval &= CVORG_DACCORR_GAIN_MASK;
		*val = (uint16_t)regval;
	}

	return 0;
}

/* experts-only function: no need to check if the module's busy */
int cvorg_chan_dac_offset(struct cvorg_channel *channel, void *arg, int set)
{
	int16_t *offset = arg;
	uint32_t regval;

	if (set) {
		/* the offset is represented by 15 bits in 2's complement */
		if (((-(1 << 14)) > *offset) || (*offset > ((1 << 14) - 1))) {
			printk(KERN_INFO PFX "Offset correction out of range: 0x%x\n", *offset);
			return -EINVAL;
		}
		/*
		 * Once we know it cannot overflow, converting to 15 bits
		 * in 2's complement is just a matter of trimming it down.
		 */
		regval = (*offset & 0x7fff) << CVORG_DACCORR_OFFSET_SHIFT;

		cvorg_uchan(channel, CVORG_DACCORR, regval,
			CVORG_DACCORR_OFFSET_MASK);
	} else {
		uint16_t val;

		regval = cvorg_rchan(channel, CVORG_DACCORR);

		regval &= CVORG_DACCORR_OFFSET_MASK;
		regval >>= CVORG_DACCORR_OFFSET_SHIFT;

		val = (int16_t)regval;
		/* sign extension because of two's complement in 15 bits */
		if (val & 0x4000)
			regval |= 0x8000;

		*offset = val;
	}

	return 0;
}

int cvorg_get_temperature(struct cvorg *cvorg, void *arg)
{
	int32_t *temperature = arg;
	uint32_t regtemp = cvorg_readw(cvorg, CVORG_TEMP);
	int temp;

	temp = (regtemp & 0x7ff) >> 4;
	if (regtemp & 0x800) /* sign bit */
		temp *= -1;

	*temperature = temp;
	return 0;
}

/*
 * Calling this function while playing a waveform only affects subsequent
 * start pulses. This means the current sequence will go on normally;
 * however, if the output has been disabled, start pulses arriving
 * after the sequence has finished will be ignored.
 */
int cvorg_chan_out_enable(struct cvorg_channel *channel, void *arg, int set)
{
	int32_t *enable = arg;
	
	if(set) {

		channel->out_enabled = !!*enable;
		__cvorg_apply_channel_out_enable(channel);
	} else {
		*enable = channel->out_enabled;
	}

	return 0;
}

int cvorg_get_pcb_id(struct cvorg *cvorg, void *arg)
{
	uint64_t *id = arg;

	*id = cvorg_readw(cvorg, CVORG_PCB_MSB);
	*id <<= 32;
	*id |= cvorg_readw(cvorg, CVORG_PCB_LSB);
	return 0;
}
