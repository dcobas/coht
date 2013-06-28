#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <asm/uaccess.h>        /* copy_*_user */
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <asm/atomic.h>
#include "vmodttl.h"
#include "cio8536.h"
#include "modulbus_register.h"
#include "lunargs.h"

#define DRIVER_NAME	"vmodttl"
#define PFX		DRIVER_NAME ": "

//#define MAX_IRQ		1

struct message_list {
	unsigned char dev;
	unsigned char val;
};

enum vmodttl_channel{
     VMOD_TTL_CHANNEL_A,
     VMOD_TTL_CHANNEL_B,
     VMOD_TTL_CHANNELS_AB
};

struct vmodttl_dev{
	int			created;        /* flag initialize      */
	struct vmod_dev         *config;
	int			dev;
	int			OpenCount;      /* open count */
	atomic_t		nr_readers;
	int			io_flag;
	int			us_pulse;	/* Time being up of the data strobe pulse */
	int			open_collector;
	spinlock_t		vmodttl_spinlock;
	spinlock_t		vmodttl_read;
	int			irq;
	wait_queue_head_t	wait;
	wait_queue_head_t	wait_readers;
	struct vmodttlconfig 	ioconfig;
	struct message_list	messages;	
};


/* The One And Only Device (OAOD) */
struct cdev                     cdev;

/* module config tables */
static struct vmod_devices    dt, *dev_table = &dt;

/* The One And Only Device (OAOD) */
static dev_t devno;

static struct vmodttl_dev       *pvmodttlDv[VMODTTL_MAX_BOARDS];

/* 
 * I/O operations
 *
 */


static uint16_t vmodttl_read_word(struct vmodttl_dev *pd, int offset)
{

	unsigned long ioaddr = 0;
	uint16_t val = 0;

	ioaddr = pd->config->address + offset;

	if (pd->config->is_big_endian)
		val = ioread16be((u16 *)(ioaddr));
	else
		val = ioread16((u16 *)(ioaddr));

	mb();
	//udelay(1);
	return val;
}

static void vmodttl_write_word(struct vmodttl_dev *pd, int offset, uint16_t value)
{
	unsigned long ioaddr = pd->config->address + offset;

	if(pd->config->is_big_endian)
		iowrite16be(value, (u16 *)ioaddr);
	else
		iowrite16(value, (u16 *)ioaddr);
	mb();
	//udelay(1);
}

static void vmodttl_write_reg(struct vmodttl_dev *pd, int offset, uint16_t value)
{
	vmodttl_write_word(pd, VMODTTL_CONTROL, offset);
	vmodttl_write_word(pd, VMODTTL_CONTROL, value);
}

static uint16_t vmodttl_read_reg(struct vmodttl_dev *pd, int offset)
{
	vmodttl_write_word(pd, VMODTTL_CONTROL, offset);
	return vmodttl_read_word(pd, VMODTTL_CONTROL);
}

/* Init the ports with the desired setup */
static void vmodttl_def_io(struct vmodttl_dev	*pd)
{
	unsigned char	tmp;

	/* It has been needed reset MCCR with 0 to avoid a missing configuration of the register DDR_A */
	/* Do not delete this two lines! */
	vmodttl_write_reg(pd, MCCR, 0);

	/* port A		*/
	/* -------------------- */
	/* disable interrupt	*/
	vmodttl_write_reg(pd, PCSR_A, CLEAR_IE);
	/* clear ip/ius		*/
	vmodttl_write_reg(pd, PCSR_A, CLEAR_IP_IUS);

	vmodttl_write_reg(pd, PMSR_A, 0x14); /* /LPM	Latch on Pattern Match	*/

	if((pd->open_collector & A_CHAN_OPEN_COLLECTOR) == A_CHAN_OPEN_COLLECTOR) {
		/* open collector connection mode */
		if(pd->io_flag & A_CHAN_OUT) {
			vmodttl_write_reg(pd, DDR_A, 0x00); 	/* output	*/
			vmodttl_write_reg(pd, DPPR_A, 0x00);
		} else {
			vmodttl_write_reg(pd, DDR_A, 0xff);	/* input 	*/

			if(pd->io_flag & VMODTTL_O)
				vmodttl_write_reg(pd, DPPR_A, 0xff);	/* invert! 	*/
			else
				vmodttl_write_reg(pd, DPPR_A, 0x00);
		}
	} else {
		/* normal TTL connection mode */
		if(pd->io_flag & A_CHAN_OUT) {
			vmodttl_write_reg(pd, DDR_A, 0x00);	/* output	*/ 
		} else {
			vmodttl_write_reg(pd, DDR_A, 0xff); /* input */
		}

		if(pd->io_flag & VMODTTL_O)
	 		vmodttl_write_reg(pd, DPPR_A, 0xff);		/* invert! 	*/
		else
			vmodttl_write_reg(pd, DPPR_A, 0x00);
	}

	vmodttl_write_reg(pd, SIOCR_A, 0x00);

	/* port B		*/
	/* -------------------- */
	/* disable interrupt	*/
	vmodttl_write_reg(pd, PCSR_B, CLEAR_IE);
	/* clear ip/ius		*/
	vmodttl_write_reg(pd, PCSR_B, CLEAR_IP_IUS);

	vmodttl_write_reg(pd ,PMSR_B, 0x14);     /* /LPM	 Latch on Pattern Match */ 

	if((pd->open_collector & B_CHAN_OPEN_COLLECTOR) == B_CHAN_OPEN_COLLECTOR) {
		/* open collector connection mode */
		if(pd->io_flag & B_CHAN_OUT) {
			vmodttl_write_reg(pd, DDR_B, 0x00);	/* output	 	*/
			vmodttl_write_reg(pd, DPPR_B, 0x00);
		} else {
			vmodttl_write_reg(pd, DDR_B, 0xff);	/* input 	 	*/
			if(pd->io_flag & VMODTTL_O)
				vmodttl_write_reg(pd, DPPR_B, 0xff);	/* invert! 	*/
			else
				vmodttl_write_reg(pd, DPPR_B, 0x00);
		}
	} else {
		/* normal TTL connection mode */
		if(pd->io_flag & B_CHAN_OUT) {
			vmodttl_write_reg(pd, DDR_B, 0x00);		/* output 	*/
		} else {
			vmodttl_write_reg(pd, DDR_B, 0xff);		/* input  	*/
		}
		if(pd->io_flag & VMODTTL_O)
			vmodttl_write_reg(pd, DPPR_B, 0xff);		/* invert! 	*/
		else
			vmodttl_write_reg(pd, DPPR_B, 0x00);
	}

	vmodttl_write_reg(pd, SIOCR_B, 0x00);

	/* port C	generates data strobe (Output) */
	if((pd->open_collector & C_CHAN_OPEN_COLLECTOR) == C_CHAN_OPEN_COLLECTOR) {
		/* open collector connection mode */
		vmodttl_write_reg(pd, DDR_C, 0x00);		/* output 	*/
		vmodttl_write_reg(pd, DPPR_C, 0x00);
	} else {
		/* normal TTL connection mode */
		vmodttl_write_reg(pd, DDR_C, 0x00);		/* output 	*/

		if(pd->io_flag & VMODTTL_O)
			vmodttl_write_reg(pd, DPPR_C, 0x0f);		/* invert!	*/
		else
			vmodttl_write_reg(pd, DPPR_C, 0x00);
	}

	vmodttl_write_reg(pd, SIOCR_C, 0x00);

	tmp  = vmodttl_read_reg(pd, MCCR) | PAE | PBE | PCE; 
	vmodttl_write_reg(pd, MCCR, tmp);
}

/* Configure the Zilog Z8536 CIO */
static void vmodttl_default(struct vmodttl_dev  *pd)
{
	unsigned int	dummy;
	int		i = 0;
	unsigned long	flags;
	
	spin_lock_irqsave(&pd->vmodttl_spinlock, flags);

	/* Prepare the Zilog Z8536 CIO to be configured */
	dummy = vmodttl_read_word(pd, VMODTTL_CONTROL);	
	vmodttl_write_reg(pd, MCCR, 0);
	dummy = vmodttl_read_reg(pd, MICR);	
	vmodttl_write_reg(pd, MICR, 0);		

	vmodttl_write_reg(pd, MICR, RESET);
	vmodttl_write_word(pd, VMODTTL_CONTROL,0);

	vmodttl_def_io(pd);

	/* enable master interrupts	*/
	i = vmodttl_read_reg(pd, MICR) | MIE | NV;
	vmodttl_write_reg(pd, MICR, i);

	spin_unlock_irqrestore(&pd->vmodttl_spinlock, flags);
	printk(KERN_INFO PFX "board %d initialized\n", pd->dev);
}

int vmodttl_open(struct inode *inode, struct file *filp)
{
	unsigned int		minor = iminor(inode);
    	struct vmodttl_dev	*pd =  (struct vmodttl_dev *)pvmodttlDv[minor];

	pd->OpenCount++;
	return 0; 
}

int vmodttl_release(struct inode *inode, struct file *filp)
{
	unsigned int		minor = iminor(inode);
    	struct vmodttl_dev	*pd =  (struct vmodttl_dev *)pvmodttlDv[minor];

	pd->OpenCount--;
	return 0;
}

int vmodttl_interrupt(void *device, void *extra)
{
	int			dev = CHAN_A;
	unsigned char		val = -1;
	struct vmodttl_dev	*pd = (struct vmodttl_dev *) device;
	unsigned long		flags;
	int num = 0;
	int ret = 0;


	/* It's needed to do this loop, in order to catch the source of the IRQ
	 * Sometimes the registers are not ready at the time we check them.
	 */
	do {
		if(vmodttl_read_reg(pd, PCSR_A) & IP) {
			dev = CHAN_A;
			goto device_found;
		} 

		if(vmodttl_read_reg(pd, PCSR_B) & IP) {
			dev = CHAN_B;
			goto device_found;
		}
		num ++;
	} while(num < 10);
	printk(KERN_WARNING PFX "vmodttl_interrupt: device not found.\n");
	vmodttl_write_reg(pd, PCSR_A, CLEAR_IP_IUS);
	vmodttl_write_reg(pd, PCSR_B, CLEAR_IP_IUS);
	return -1;

device_found:

	spin_lock_irqsave(&pd->vmodttl_read, flags);
	/* interrupt from channel A, B or timer */
	switch(dev) {
	case CHAN_A:	/* channel A */
		val = vmodttl_read_word(pd, VMODTTL_PORTA);
		vmodttl_write_reg(pd, PCSR_A, CLEAR_IP_IUS);
		break;

	case CHAN_B: 	/* channel B */
		val = vmodttl_read_word(pd, VMODTTL_PORTB);
		vmodttl_write_reg(pd, PCSR_B, CLEAR_IP_IUS);
		break;
	default:
		printk(KERN_ERR PFX "Interrupt unknown. This cannot happen.");
		ret = -1;
		goto out;
	}

	pd->messages.dev = dev;
	pd->messages.val = val;
	pd->irq = 1;
	wake_up_interruptible(&pd->wait);

out:
	spin_unlock_irqrestore(&pd->vmodttl_read, flags);
	return ret;
}

static void vmodttl_change_bit(int *reg, unsigned bit,  unsigned char pos)
{
	unsigned char mask;
	unsigned char value;

	*reg = *reg & 0xff;
	value = (bit << pos);

	/* Change the corresponding bit and keep the value of the rest */
	mask = ~(1 << pos);
	*reg &= mask; 
	*reg |= value;

}

static int vmodttl_pattern(struct vmodttl_pattern buf, struct vmodttl_dev *pd)
{ 

	int dev = -1;
	int byte;
	unsigned long flags;
	int tmp;

	dev = buf.port;
	/* Check invalid channel */ 
	if(dev < CHAN_A || dev > CHAN_B){
		printk(KERN_ERR PFX "Invalid channel: 0x%x.\n", dev);
		return -EINVAL;
	}

	spin_lock_irqsave(&pd->vmodttl_spinlock, flags);
	/* Disable interrupts */
	tmp = vmodttl_read_reg(pd, MICR) & ~MIE;
	vmodttl_write_reg(pd, MICR, tmp);

	switch(dev) {
	case CHAN_A:
	{
		if(pd->io_flag & A_CHAN_OUT) {
			spin_unlock_irqrestore(&pd->vmodttl_spinlock, flags);
			return -EIO;
		}
		/* Copy the configuration to ioconfig */
		pd->ioconfig.bit_pattern_a[buf.pos] = buf;

		/* port A		*/
		/* -------------------- */
		/* disable interrupt	*/
		vmodttl_write_reg(pd, PCSR_A, CLEAR_IE);
		/* clear ip/ius	*/
		vmodttl_write_reg(pd, PCSR_A, CLEAR_IP_IUS);
		/* disable port	*/
		vmodttl_write_word(pd, VMODTTL_CONTROL, MCCR);
		byte = vmodttl_read_word(pd, VMODTTL_CONTROL) & ~(PAE);
		vmodttl_write_reg(pd, MCCR, byte);
		/* bit port */
		if(pd->ioconfig.pattern_mode_a)
			vmodttl_write_reg(pd, PMSR_A, 0x12);	/* Pattern AND */
		else
			vmodttl_write_reg(pd, PMSR_A, 0x14);	/* Pattern OR */


		if(pd->io_flag & VMODTTL_O)
			vmodttl_write_reg(pd, DPPR_A, 0xff);	/* invert! */
		else
			vmodttl_write_reg(pd, DPPR_A, 0x00);

		vmodttl_write_reg(pd, DDR_A, 0xff);
		vmodttl_write_reg(pd, SIOCR_A, 0x00);

		/* PPR */
		byte = vmodttl_read_reg(pd, PPR_A);
		vmodttl_change_bit(&byte, buf.ppr, buf.pos);
		vmodttl_write_reg(pd, PPR_A, byte);
		/* PTR */
		byte = vmodttl_read_reg(pd, PTR_A);
		vmodttl_change_bit(&byte, buf.ptr, buf.pos);
		vmodttl_write_reg(pd, PTR_A, byte);
		/* PMR */
		byte = vmodttl_read_reg(pd, PMR_A);
		vmodttl_change_bit(&byte, buf.pmr, buf.pos);
		vmodttl_write_reg(pd, PMR_A, byte);

		/* enable port	 	*/
		byte = vmodttl_read_reg(pd, MCCR) | PAE;
		vmodttl_write_reg(pd, MCCR, byte);
		/* enable interrupt	*/
		vmodttl_write_reg(pd, PCSR_A, SET_IE);
	}
	break;
	case CHAN_B:
	{

		if(pd->io_flag & B_CHAN_OUT){
			spin_unlock_irqrestore(&pd->vmodttl_spinlock, flags);
			return -EIO;
		}
	
		/* Copy the configuration to ioconfig */
		pd->ioconfig.bit_pattern_a[buf.pos] = buf;

		/* port B		*/
		/* -------------------- */
		/* disable interrupt	*/
		vmodttl_write_reg(pd, PCSR_B, CLEAR_IE);
		/* clear ip/ius		*/
		vmodttl_write_reg(pd, PCSR_B, CLEAR_IP_IUS);
		/* disable port 	*/
		byte = vmodttl_read_reg(pd, MCCR) & ~PBE;
		vmodttl_write_reg(pd, MCCR, byte);
		/* bit port		*/
		if(pd->ioconfig.pattern_mode_b)
			vmodttl_write_reg(pd, PMSR_B, 0x12);	/* Pattern AND */
		else
			vmodttl_write_reg(pd, PMSR_B, 0x14);	/* Pattern OR */
	    
		if(pd->io_flag & VMODTTL_O)
			vmodttl_write_reg(pd, DPPR_B, 0xff);	/* invert! */
		else
			vmodttl_write_reg(pd, DPPR_B, 0x00);

		vmodttl_write_reg(pd, DDR_B, 0xff);
		vmodttl_write_reg(pd, SIOCR_B, 0x00);

		/* PPR */
		byte = vmodttl_read_reg(pd, PPR_B);
		vmodttl_change_bit(&byte, buf.ppr, buf.pos);
		vmodttl_write_reg(pd, PPR_B, byte);
		/* PTR */
		byte = vmodttl_read_reg(pd, PTR_B);
		vmodttl_change_bit(&byte, buf.ptr, buf.pos);
		vmodttl_write_reg(pd, PTR_B, byte);
		/* PMR */
		byte = vmodttl_read_reg(pd, PMR_B);
		vmodttl_change_bit(&byte, buf.pmr, buf.pos);
		vmodttl_write_reg(pd, PMR_B, byte);

		/* enable port	 	*/
		byte = vmodttl_read_reg(pd, MCCR) | PBE;
		vmodttl_write_reg(pd, MCCR, byte);
		/* enable interrupt	*/
		vmodttl_write_reg(pd, PCSR_B, SET_IE);

	}
	break;
	
	default:
		printk(KERN_ERR PFX "vmodttl_pattern: Invalid port to configure.\n");
		spin_unlock_irqrestore(&pd->vmodttl_spinlock, flags);
		return -EINVAL;
	}
	/* enable master interrupts	*/
	tmp = vmodttl_read_reg(pd, MICR) | MIE | NV;
	vmodttl_write_reg(pd, MICR, tmp);
	spin_unlock_irqrestore(&pd->vmodttl_spinlock, flags);
	return 0;
}

/*
 * vmodttl_read : wait for an IRQ and give the value of the channel to user space.
 *
 * We have a queue of readers for each device. They should read the same IRQ event.
 * For this case, it is implemented a new wait queue (wait_readers) that forces the
 * readers to wait until the last one picks the data. This wait has a timeout, just
 * in case
 */

ssize_t vmodttl_read (struct file *file, char *buf, size_t count, loff_t *f_pos)
{
	int minor = iminor(file->f_dentry->d_inode);
    	struct vmodttl_dev *pd =  (struct vmodttl_dev *)pvmodttlDv[minor];
	int count_read = 0;
	unsigned long flags;

	// Counting the number of readers waiting for a IRQ on such device.
	atomic_inc(&pd->nr_readers);
    
	if(wait_event_interruptible(pd->wait, (pd->irq == 1))) {
		atomic_dec(&pd->nr_readers);
		count_read = -ERESTARTSYS;
		goto out;
	}

	spin_lock_irqsave(&pd->vmodttl_read, flags);

	if(copy_to_user(buf, (void *)&pd->messages, 2*sizeof(unsigned char)) != 0){
		atomic_dec(&pd->nr_readers);
		spin_unlock_irqrestore(&pd->vmodttl_read, flags);
		count_read = -EFAULT;
		goto out;
	}

	count_read += 2*sizeof(unsigned char);

	if (atomic_dec_and_test(&pd->nr_readers)) {
		atomic_set(&pd->nr_readers, 0);
		pd->irq = 0;
		spin_unlock_irqrestore(&pd->vmodttl_read, flags);
		// As this is the last reader, switch off the lights when it leaves.
		wake_up_interruptible_all(&pd->wait_readers);
	} else {

		spin_unlock_irqrestore(&pd->vmodttl_read, flags);

		/*
		 * Wait until the rest of readers have got the same IRQ. If not, timeout of 1 ms.
		 * Don't care if a signal arrives or it is timeout, the reader has the IRQ value, 
		 * so continue.
		 */
		wait_event_interruptible_timeout(pd->wait_readers, (atomic_read(&pd->nr_readers) == 0), 0.001*HZ);
	}

out:
	return count_read;	


}

static int vmodttl_config(struct vmodttlconfig conf, struct vmodttl_dev *pd)
{
	unsigned long flags;

	spin_lock_irqsave(&pd->vmodttl_spinlock, flags);
	if((conf.io_flag > (A_CHAN_OUT + B_CHAN_OUT + C_CHAN_OUT)) ||
			conf.io_flag < 0){
		spin_unlock_irqrestore(&pd->vmodttl_spinlock, flags);
		return -EINVAL;
	}
	pd->io_flag = conf.io_flag;
	
	/* Used to keep up the strobe pulse during us_pulse time */
	if (conf.us_pulse < DEFAULT_DELAY)
		pd->us_pulse = DEFAULT_DELAY; 
	else
		pd->us_pulse = conf.us_pulse;

	if(conf.invert)
		pd->io_flag |= VMODTTL_O;

	pd->open_collector = conf.open_collector;
	vmodttl_def_io(pd); 
	spin_unlock_irqrestore(&pd->vmodttl_spinlock, flags);
	return 0;
}

static int vmodttl_read_chan(struct vmodttlarg buf, struct vmodttl_dev *pd)
{
	int val = 0;
	int strobe_value = 0;

	spin_lock(&pd->vmodttl_spinlock);

	switch(buf.dev){
		case VMOD_TTL_CHANNEL_A:
			val = (int) vmodttl_read_word(pd, VMODTTL_PORTA);
			strobe_value = 0x04;
			break;

		case VMOD_TTL_CHANNEL_B:
			val = (int) vmodttl_read_word(pd, VMODTTL_PORTB);		
			strobe_value = 0x08;
			break;

		case VMOD_TTL_CHANNELS_AB:
			val = (int) vmodttl_read_word(pd, VMODTTL_PORTA);
			val += ((int) vmodttl_read_word(pd, VMODTTL_PORTB)) << 8;

		default:
			break;
	}

	/* produce the strobe on C channel (pulse) */
	vmodttl_write_word(pd, VMODTTL_PORTC, strobe_value);
	udelay(pd->us_pulse);
	vmodttl_write_word(pd, VMODTTL_PORTC, 0);
	spin_unlock(&pd->vmodttl_spinlock);

	return val;

}

static int vmodttl_write_chan(struct vmodttlarg buf, struct vmodttl_dev *pd)
{
	int dev;
	uint16_t data;
	uint16_t tmp;
	int strobe_value = 0;

	dev = buf.dev;
	data = buf.val;

	if(dev < VMOD_TTL_CHANNEL_A || dev > VMOD_TTL_CHANNELS_AB)
		return -EINVAL; /* invalid channel */

	spin_lock(&pd->vmodttl_spinlock);
	switch(dev){
	case VMOD_TTL_CHANNEL_A:

		if((pd->io_flag & A_CHAN_OUT) == A_CHAN_IN){
			spin_unlock(&pd->vmodttl_spinlock);
			return -EIO;
		}

		vmodttl_write_word(pd, VMODTTL_PORTA, data);
		tmp = vmodttl_read_reg(pd, MCCR);

		vmodttl_write_reg(pd, MCCR, (tmp | PAE));
		strobe_value = 0x01;
		break;

	case VMOD_TTL_CHANNEL_B:

		if((pd->io_flag & B_CHAN_OUT) == B_CHAN_IN){
			spin_unlock(&pd->vmodttl_spinlock);
			return -EIO;
		}

		vmodttl_write_word(pd, VMODTTL_PORTB, data);
		tmp = vmodttl_read_reg(pd, MCCR);

		vmodttl_write_reg(pd, MCCR, (tmp | PBE));
		strobe_value = 0x02;
		break;

	case VMOD_TTL_CHANNELS_AB:

		if(((pd->io_flag & B_CHAN_OUT) == B_CHAN_IN) ||
				(pd->io_flag & A_CHAN_OUT) == A_CHAN_IN) {
			spin_unlock(&pd->vmodttl_spinlock);
			return -EIO;
		}

		vmodttl_write_word(pd, VMODTTL_PORTA, data & 0xff);
		tmp = vmodttl_read_reg(pd, MCCR);

		vmodttl_write_word(pd, VMODTTL_PORTB, (data >> 8) & 0xff); 	
		tmp = vmodttl_read_reg(pd, MCCR);

		vmodttl_write_reg(pd, MCCR, (tmp | PAE | PBE));
		strobe_value = 0x03;
		break;
	default:
		break;
	}

	/* produce the strobe on C channel (pulse) */
	vmodttl_write_word(pd, VMODTTL_PORTC, strobe_value);
	udelay(pd->us_pulse);
	vmodttl_write_word(pd, VMODTTL_PORTC, 0);
	spin_unlock(&pd->vmodttl_spinlock);
	return 0;
}

static long vmodttl_ioctl(struct file *fp, unsigned op, unsigned long arg)
{
	unsigned int minor = iminor(fp->f_dentry->d_inode);
	struct vmodttl_dev *pd =  (struct vmodttl_dev *)pvmodttlDv[minor];
	int ret;

	switch(op){

	case VMODTTL_CONFIG:
	{
		struct vmodttlconfig conf;

		if(copy_from_user((char *)&conf, (char *)arg, sizeof(struct vmodttlconfig))) 
			return -EFAULT;

		pd->ioconfig = conf;
		ret = vmodttl_config(conf, pd);
		if (ret < 0)
			return ret;
		break;
	}
	case VMODTTL_READ_CONFIG:
		if(copy_to_user((char *)arg, (char *)&pd->ioconfig, sizeof(struct vmodttlconfig)))
			return -EFAULT;
	
		break;

	case VMODTTL_SIMPIO:
		/* It's the default behaviour: Bit port simple operation without interrupts */
		/* This ioctl operation resets the channels to this mode */
		vmodttl_def_io(pd);
		break;

	case VMODTTL_PATTERN:
	{ 
		struct vmodttl_pattern buf;

		if(copy_from_user((char *)&buf, (char *)arg, sizeof(struct vmodttl_pattern))) 
			return -EFAULT;

		ret = vmodttl_pattern(buf, pd);
		if (ret < 0)
			return ret;
		break;
	}
	case VMODTTL_READ_CHAN:
	{
		struct vmodttlarg buf;
		short int dev;

		if(copy_from_user((char *)&buf, (char *)arg,sizeof(struct vmodttlarg)))
			return -EIO;

		dev = buf.dev & 0x00ff; /* Get channel */	
		if(dev < VMOD_TTL_CHANNEL_A || dev > VMOD_TTL_CHANNELS_AB)
			return -EINVAL; /* invalid channel */

		buf.val = vmodttl_read_chan(buf, pd);
		if(copy_to_user((char *)arg, (char *)&buf, sizeof(struct vmodttlarg)))
			return -EFAULT;
				
		break;
	}
	case VMODTTL_WRITE_CHAN:
	{
		struct vmodttlarg buf;

		if(copy_from_user((char *)&buf, (char *)arg, sizeof(struct vmodttlarg)))
			return -EIO; 

		ret = vmodttl_write_chan(buf, pd);
		if (ret < 0)
			return ret;
		break;
	}
	default:
		printk(KERN_INFO PFX "There is no default option");
		return -EINVAL;
	}

	return 0;

}
struct file_operations vmodttl_fops = {
        .owner 		= THIS_MODULE,
	.read 		= vmodttl_read,
        .unlocked_ioctl	= vmodttl_ioctl,
        .open 		= vmodttl_open,
        .release 	= vmodttl_release
};

static int register_module_isr(struct vmodttl_dev *dev, isrcb_t handler)
{
        int err;
        risr_t register_isr;
        char *carrier = dev->config->carrier_name;
        int lun = dev->config->carrier_lun;
        int slot = dev->config->slot;

        register_isr = modulbus_carrier_isr_entry(carrier);
        if (register_isr == NULL)
		goto error;
           
        err = register_isr(handler, (void *)dev, lun, slot);
        if (err < 0) 
		goto error;
           
        return 0;
error:
	printk(KERN_ERR PFX 
			"could not register irq handler for lun %d\n",
			dev->config->lun);
	return -1; 

}

static int unregister_module_isr(struct vmodttl_dev *dev)
{
        return register_module_isr(dev, NULL);
}

static int __init vmodttl_init(void)
{
	int err;
	int i;
	struct vmodttl_dev *pd;

        printk(KERN_INFO PFX "reading parameters\n");

        err = read_params(DRIVER_NAME, dev_table);
        if (err != 0)
                return -1;
        printk(KERN_INFO PFX "initialized driver for %d (max %d) cards\n",
		dev_table->num_modules, VMODTTL_MAX_BOARDS);

        err = alloc_chrdev_region(&devno, 0, VMODTTL_MAX_BOARDS, DRIVER_NAME);
        if (err != 0) 
		goto fail_chrdev;

        printk(KERN_INFO PFX "allocated device %d\n", MAJOR(devno));

        cdev_init(&cdev, &vmodttl_fops);
        cdev.owner = THIS_MODULE;
        err = cdev_add(&cdev, devno, VMODTTL_MAX_BOARDS);
        if (err){
        	printk(KERN_ERR PFX "Added cdev with err = %d\n", err);
		goto fail_cdev;
	}

	for(i = 0; i < dev_table->num_modules; i++){ 
		struct vmod_dev	*mod = &dev_table->module[i];

		pd = kzalloc(sizeof(struct vmodttl_dev), GFP_KERNEL);

		if (pd == 0)
			goto fail_pd;

		pvmodttlDv[mod->lun] = (void *)pd;
		pd->config = mod;

		pd->dev = lun_to_index(dev_table, mod->lun);
		pd->OpenCount = 0;
		atomic_set(&pd->nr_readers, 0);
		pd->io_flag = 0; /* All channels are inputs by default */
		pd->open_collector = 0; /* All channels are TTL by default */
		pd->created = 1;
		spin_lock_init(&pd->vmodttl_spinlock);
		spin_lock_init(&pd->vmodttl_read);
		init_waitqueue_head(&pd->wait);
		init_waitqueue_head(&pd->wait_readers);
		pd->irq = 0;
		if (register_module_isr(pd, vmodttl_interrupt) < 0)
                	goto fail_messages;

		vmodttl_default(pd);
	}

	return 0;

fail_messages: 
	{
		int k;
		for(k =0; k < k; k++) {
			if(pvmodttlDv[k] != 0) {
				unregister_module_isr(pvmodttlDv[k]);
				kfree(pvmodttlDv[k]);
			}
		}
        }
fail_pd: 
	cdev_del(&cdev);
fail_cdev:      
	unregister_chrdev_region(devno, VMODTTL_MAX_BOARDS);
fail_chrdev:    return -1;

}

static void __exit vmodttl_exit(void)
{
	int i;

	for(i =0; i < dev_table->num_modules; i++){
		if(pvmodttlDv[i] != 0){
			unregister_module_isr(pvmodttlDv[i]);
			kfree(pvmodttlDv[i]);
		}
	}
	
	cdev_del(&cdev);
        unregister_chrdev_region(devno, VMODTTL_MAX_BOARDS);
	printk(KERN_INFO "VMODTTL: exit module.\n");
}

module_init(vmodttl_init);
module_exit(vmodttl_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Samuel Iglesias Gonsalvez");
MODULE_DESCRIPTION("VMOD-TTL device driver");
MODULE_VERSION("1.1");
MODULE_VERSION(GIT_VERSION);
