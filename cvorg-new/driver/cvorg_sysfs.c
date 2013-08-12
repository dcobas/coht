/*
 * cvorg_sysfs.c
 * sysfs management for cvorg
 *
 * Copyright (c) 2012 Samuel Iglesias Gonsalvez <siglesia@cern.ch>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */
#include <linux/version.h>
#include <linux/device.h>
#include <linux/ctype.h>
#include <linux/sysfs.h>
#include <linux/stat.h>
#include <asm/uaccess.h>

#include "cvorg.h"
#include "cvorg_priv.h"

#ifdef CONFIG_SYSFS

#define to_cvorg_channel(k) container_of(k, struct cvorg_channel, kobj)
#define to_cvorg_channel_attr(a) container_of(a, struct cvorg_channel_attribute, attr)

/*
 * Note that the read-only parameters' handlers do not take the card's lock.
 * This is because the information they export is not supposed to change
 * once the device has been loaded.
 */

static ssize_t
cvorg_show_hw_version(struct device *pdev, struct device_attribute *attr, char *buf)
{
	struct cvorg *card = dev_get_drvdata(pdev);

	if (card->hw_rev)
		return snprintf(buf, PAGE_SIZE, "0x%x\n", card->hw_rev);
	else
		return snprintf(buf, PAGE_SIZE, "none\n");
}

static ssize_t
cvorg_show_temperature(struct device *pdev, struct device_attribute *attr, char *buf)
{
	struct cvorg *card = dev_get_drvdata(pdev);
	uint32_t temperature;

	cvorg_get_temperature(card, &temperature);
	return snprintf(buf, PAGE_SIZE, "%d\n", temperature);
}

static ssize_t
cvorg_show_pcb_id(struct device *pdev, struct device_attribute *attr, char *buf)
{
	struct cvorg *card = dev_get_drvdata(pdev);
	uint64_t pcb_id;

	cvorg_get_pcb_id(card, &pcb_id);
	return snprintf(buf, PAGE_SIZE, "0x%llx\n", pcb_id);
}


static ssize_t
cvorg_store_reset(struct device *pdev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct cvorg *card = dev_get_drvdata(pdev);
	cvorg_reset(card);
	return count;
}
static DEVICE_ATTR(hw_version, S_IRUGO, cvorg_show_hw_version, NULL);
static DEVICE_ATTR(temperature, S_IRUGO, cvorg_show_temperature, NULL);
static DEVICE_ATTR(pcb_id, S_IRUGO, cvorg_show_pcb_id, NULL);
static DEVICE_ATTR(reset, S_IWUSR, NULL, cvorg_store_reset);

static struct attribute *cvorg_attrs[] = {
	&dev_attr_hw_version.attr,
	&dev_attr_temperature.attr,
	&dev_attr_reset.attr,
	&dev_attr_pcb_id.attr,
	NULL,
};

static struct attribute_group cvorg_attr_group = {
	.attrs = cvorg_attrs,
};

static int cvorg_getval(const char *buf, size_t count, int32_t *val)
{
	unsigned long value;
	char *endp;

	if (!isdigit(buf[0]))
		return -EINVAL;
	value = simple_strtoul(buf, &endp, 0);
	/* add one to endp to allow a trailing '\n' */
	if (endp + 1 < buf + count)
		return -EINVAL;
	*val = value;
	return 0;
}

static ssize_t
cvorg_show_int(struct cvorg *card, char *buf, int *valp)
{
	ssize_t ret;

	if (mutex_lock_interruptible(&card->lock))
		return -EINTR;
	ret = snprintf(buf, PAGE_SIZE, "%d\n", *valp);
	mutex_unlock(&card->lock);
	return ret;
}

static ssize_t
cvorg_show_uint(struct cvorg *card, char *buf, unsigned int *valp)
{
	ssize_t ret;

	if (mutex_lock_interruptible(&card->lock))
		return -EINTR;
	ret = snprintf(buf, PAGE_SIZE, "%u\n", *valp);
	mutex_unlock(&card->lock);
	return ret;
}

static ssize_t
cvorg_show_offset(struct cvorg_channel *channel, char *buf)
{
	struct cvorg *card = channel->parent;
	uint32_t value;
	int ret;

	ret = cvorg_chan_offset(channel, &value, 0);
	if(ret < 0)
		return ret;
	return cvorg_show_int(card, buf, (int *)&value);
}

static ssize_t
cvorg_store_offset(struct cvorg_channel *channel, const char *buf, size_t count)
{
	uint32_t value;
	int ret;

	ret = cvorg_getval(buf, count, &value);
	if (ret)
		return ret;

	ret = cvorg_chan_offset(channel, &value, 1);
	if (ret)
		return ret;

	return count;
}

static ssize_t
cvorg_show_gain(struct cvorg_channel *channel, char *buf)
{

	struct cvorg *card = channel->parent;
	uint32_t value;
	int ret;

	ret = cvorg_chan_output_gain(channel, &value, 0);
	if (ret < 0)
		return ret;
	return cvorg_show_int(card, buf, &value);
}

static ssize_t
cvorg_store_gain(struct cvorg_channel *channel, const char *buf, size_t count)
{
	uint32_t value;
	int ret;

	ret = cvorg_getval(buf, count, &value);
	if (ret)
		return ret;

	ret = cvorg_chan_output_gain(channel, &value, 1);
	if (ret)
		return ret;

	return count;
}

static ssize_t
cvorg_show_enable_output(struct cvorg_channel *channel, char *buf)
{
	struct cvorg *card = channel->parent;
	uint32_t value;

	cvorg_chan_out_enable(channel, &value, 0);
	return cvorg_show_int(card, buf, &value);
}

static ssize_t
cvorg_store_enable_output(struct cvorg_channel *channel, const char *buf, size_t count)
{
	uint32_t value;
	int ret;

	ret = cvorg_getval(buf, count, &value);
	if (ret)
		return ret;

	ret = cvorg_chan_out_enable(channel, &value, 1);
	if (ret)
		return ret;

	return count;
}

/*
 * Purposedly we don't take the device's lock here, to make this as close to an
 * external trigger as possible.
 */
static ssize_t
cvorg_store_trigger(struct cvorg_channel *channel, const char *buf, size_t count)
{
	uint32_t value;
	int ret;

	ret = cvorg_getval(buf, count, &value);
	if (ret)
		return ret;

	ret = cvorg_chan_sw_trigger(channel, &value);
	if (ret)
		return ret;

	return count;
}

static ssize_t
cvorg_show_input_polarity(struct cvorg_channel *channel, char *buf)
{
	struct cvorg *card = channel->parent;
	int value;	

	value = cvorg_chan_inpol(channel, buf, 0);
	return cvorg_show_int(card, buf, &value);
}

static ssize_t
cvorg_store_input_polarity(struct cvorg_channel *channel, const char *buf, size_t count)
{
	uint32_t value;
	int ret;

	ret = cvorg_getval(buf, count, &value);
	if (ret)
		return ret;

	ret = cvorg_chan_inpol(channel, &value, 1);
	if (ret)
		return ret;

	return count;
}

static ssize_t
cvorg_show_status(struct cvorg_channel *channel, char *buf)
{
	struct cvorg *card = channel->parent;
	int val;

	cvorg_chan_status(channel, &val);
	return cvorg_show_int(card, buf, &val);
}

static ssize_t
cvorg_show_sampling_freq(struct cvorg_channel *channel, char *buf)
{
	uint32_t val;
	struct cvorg *card = channel->parent;
	
	cvorg_read_sampfreq(card, &val);
	return cvorg_show_uint(card, buf, &val);
}

static ssize_t
cvorg_show_pll(struct file *filp,
		struct kobject *kobj,
		struct bin_attribute *bin_attr,
		char *buf, loff_t off, size_t count)
{
        struct cvorg_channel *channel = to_cvorg_channel(kobj);
	int ret;
	
	ret = cvorg_pll(channel->parent, buf, 0);
	if(ret) {
		printk(KERN_ERR "Error: cvorg %d channel %d, read PLL\n", 
			channel->parent->lun, channel->chan_nr);
		return -EINVAL;
	}
	
	return sizeof(struct ad9516_pll);
}

static ssize_t
cvorg_store_pll(struct file *filp,
		struct kobject *kobj,
		struct bin_attribute *bin_attr,
		char *buf, loff_t off, size_t count)
{
        struct cvorg_channel *channel = to_cvorg_channel(kobj);
	int ret;
	
	ret = cvorg_pll(channel->parent, buf, 1);
	if(ret) {
		printk(KERN_ERR "Error: cvorg %d channel %d, write PLL\n", 
			channel->parent->lun, channel->chan_nr);
		return -EINVAL;
	}
	

	return count;
}

static inline ssize_t
cvorg_show_pll_compat(struct kobject *kobj,
		struct bin_attribute *bin_attr,
		char *buf, loff_t off, size_t count)
{
	return cvorg_show_pll(NULL, kobj, bin_attr, buf, off, count);
}

static inline ssize_t
cvorg_store_pll_compat(struct kobject *kobj,
		struct bin_attribute *bin_attr,
		char *buf, loff_t off, size_t count)
{
	return cvorg_store_pll(NULL, kobj, bin_attr, buf, off, count);
}

static ssize_t
cvorg_store_set_sequence(struct file *filp,
		struct kobject *kobj,
		struct bin_attribute *bin_attr,
		char *buf, loff_t off, size_t count)
{
        struct cvorg_channel *channel = to_cvorg_channel(kobj);
	int ret;

	ret = cvorg_chan_loadseq(channel, buf);
	if (ret) {
		printk(KERN_ERR "Error: cvorg %d channel %d, set_sequence\n", 
			channel->parent->lun, channel->chan_nr);
		return -EINVAL;
	}

	return count;
}

static inline ssize_t
cvorg_store_set_sequence_compat(struct kobject *kobj,
		struct bin_attribute *bin_attr,
		char *buf, loff_t off, size_t count)
{
	return cvorg_store_set_sequence(NULL, kobj, bin_attr, buf, off, count);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
#define	cvorg_show_pll			cvorg_show_pll_compat
#define	cvorg_store_pll			cvorg_store_pll_compat
#define	cvorg_store_set_sequence	cvorg_store_set_sequence_compat
#endif

static ssize_t
cvorg_show_dac_gain(struct cvorg_channel *channel, char *buf)
{
	uint16_t value;
	cvorg_chan_dac_gain(channel, &value, 0);
	return cvorg_show_int(channel->parent, buf, (int *)&value);
}


static ssize_t
cvorg_store_dac_gain(struct cvorg_channel *channel, const char *buf, size_t count)
{
	int ret;
	uint32_t value;

	ret = cvorg_getval(buf, count, &value);
	if (ret)
		return ret;

	ret = cvorg_chan_dac_gain(channel, &value, 1);
	if (ret)
		return ret;

	return count;
}

static ssize_t
cvorg_show_dac_offset(struct cvorg_channel *channel, char *buf)
{
	uint16_t value;
	cvorg_chan_dac_offset(channel, &value, 0);
	return cvorg_show_int(channel->parent, buf, (int *)&value);
}


static ssize_t
cvorg_store_dac_offset(struct cvorg_channel *channel, const char *buf, size_t count)
{
	int ret;
	uint32_t value;

	ret = cvorg_getval(buf, count, &value);
	if (ret)
		return ret;

	ret = cvorg_chan_dac_offset(channel, &value, 1);
	if (ret)
		return ret;

	return count;
}

static ssize_t
cvorg_show_dac_value(struct cvorg_channel *channel, char *buf)
{
	uint32_t value;
	cvorg_chan_dac_val(channel, &value, 0);
	return cvorg_show_int(channel->parent, buf, &value);
}

static ssize_t
cvorg_store_dac_value(struct cvorg_channel *channel, const char *buf, size_t count)
{
	int ret;
	uint32_t value;

	ret = cvorg_getval(buf, count, &value);
	if (ret)
		return ret;

	ret = cvorg_chan_dac_val(channel, &value, 1);
	if (ret)
		return ret;

	return count;
}

static ssize_t
cvorg_show_test_mode(struct cvorg_channel *channel, char *buf)
{
	uint32_t value;
	cvorg_chan_test_mode(channel, &value, 0);
	return cvorg_show_int(channel->parent, buf, &value);
}

static ssize_t
cvorg_store_test_mode(struct cvorg_channel *channel, const char *buf, size_t count)
{
	int ret;
	uint32_t value;

	ret = cvorg_getval(buf, count, &value);
	if (ret)
		return ret;

	ret = cvorg_chan_test_mode(channel, &value, 1);
	if (ret)
		return ret;

	return count;
}

/* cvorg specific attribute structure */
struct cvorg_channel_attribute {
        struct attribute attr;
         ssize_t(*show) (struct cvorg_channel *, char *);
         ssize_t(*store) (struct cvorg_channel *, const char *, size_t);
};


/* Set of show/store higher level functions for default cvorg attributes */
static ssize_t cvorg_channel_show(struct kobject *kobj,
                        struct attribute *attr, char *buffer)
{
        struct cvorg_channel *channel = to_cvorg_channel(kobj);
        struct cvorg_channel_attribute *cvorg_channel_attr = to_cvorg_channel_attr(attr);

        if (cvorg_channel_attr->show)
                return cvorg_channel_attr->show(channel,
                                        buffer);
        return -EIO;
}

static ssize_t cvorg_channel_store(struct kobject *kobj, struct attribute *attr,
                        const char *buffer, size_t count)
{
        struct cvorg_channel *channel = to_cvorg_channel(kobj);
        struct cvorg_channel_attribute *cvorg_channel_attr = to_cvorg_channel_attr(attr);

        if (cvorg_channel_attr->store)
                return cvorg_channel_attr->store(channel,
                                        buffer,
                                        count);
        return -EIO;
}

static struct sysfs_ops cvorg_chan_fs_ops = {
        .show = cvorg_channel_show,
        .store = cvorg_channel_store
};

#define CVORG_CHAN_ATTR(_name,_mode,_show,_store)        \
struct cvorg_channel_attribute attr_##_name = {                       \
        .attr = {.name = __stringify(_name), .mode = _mode },   \
        .show   = _show,                                        \
        .store  = _store,                                       \
};

CVORG_CHAN_ATTR(offset, S_IWUSR | S_IRUGO, cvorg_show_offset, cvorg_store_offset);
CVORG_CHAN_ATTR(gain, S_IWUSR | S_IRUGO, cvorg_show_gain, cvorg_store_gain);
CVORG_CHAN_ATTR(enable_output, S_IWUSR | S_IRUGO, cvorg_show_enable_output, cvorg_store_enable_output);
CVORG_CHAN_ATTR(trigger, S_IWUSR, NULL, cvorg_store_trigger);
CVORG_CHAN_ATTR(input_polarity, S_IWUSR | S_IRUGO, cvorg_show_input_polarity, cvorg_store_input_polarity);
CVORG_CHAN_ATTR(status, S_IRUGO, cvorg_show_status, NULL);
CVORG_CHAN_ATTR(sampling_frequency, S_IRUGO, cvorg_show_sampling_freq, NULL);
CVORG_CHAN_ATTR(dac_gain, S_IWUSR | S_IRUGO, cvorg_show_dac_gain, cvorg_store_dac_gain);
CVORG_CHAN_ATTR(dac_offset, S_IWUSR | S_IRUGO, cvorg_show_dac_offset, cvorg_store_dac_offset);
CVORG_CHAN_ATTR(dac_value, S_IWUSR | S_IRUGO, cvorg_show_dac_value, cvorg_store_dac_value);
CVORG_CHAN_ATTR(test_mode, S_IWUSR | S_IRUGO, cvorg_show_test_mode, cvorg_store_test_mode);

/* default attributes of the CSROW<id> object */
static struct cvorg_channel_attribute *default_cvorg_chan_attr[] = {
       	&attr_offset,
	&attr_gain,
	&attr_enable_output,
	&attr_trigger,
	&attr_input_polarity,
	&attr_status,
	&attr_sampling_frequency,
	&attr_dac_gain,
	&attr_dac_offset,
	&attr_dac_value,
	&attr_test_mode,
        NULL,
};

static void cvorg_channel_instance_release(struct kobject *kobj)
{
}


static struct bin_attribute cvorg_chan_pll_attr = {
	.attr = {
		.name = "pll",
		.mode = S_IRUGO | S_IWUSR,
	},
	.size = sizeof(struct ad9516_pll),
	.read = cvorg_show_pll,
	.write = cvorg_store_pll,
};

static struct bin_attribute cvorg_chan_set_sequence_attr = {
	.attr = {
		.name = "set_sequence",
		.mode = S_IRUGO | S_IWUSR,
	},
	.size = sizeof(struct cvorg_seq),
	.read = NULL,
	.write = cvorg_store_set_sequence,
};


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25)
/*
 * kobject_init_and_add (and its helper kobject_set_name_vargs) are
 * lifted here from 2.6.25, for the case of 2.6.24 compilation
 */
static int kobject_set_name_vargs(struct kobject *kobj, const char *fmt,
                                  va_list vargs)
{
        va_list aq;
        char *name;

        va_copy(aq, vargs);
        name = kvasprintf(GFP_KERNEL, fmt, vargs);
        va_end(aq);

        if (!name)
                return -ENOMEM;

        /* Free the old name, if necessary. */
        kfree(kobj->k_name);

        /* Now, set the new name */
        kobj->k_name = name;

        return 0;
}

int kobject_init_and_add(struct kobject *kobj, struct kobj_type *ktype,
		         struct kobject *parent, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	kobject_set_name_vargs(kobj, fmt, args);
	va_end(args);
	kobj->parent = parent;
	kobj->ktype = ktype;
	return kobject_register(kobj);
}
#endif

/* the kobj_type instance for a CSROW */
static struct kobj_type ktype_cvorg_chan = {
        .release = cvorg_channel_instance_release,
        .sysfs_ops = &cvorg_chan_fs_ops,
        .default_attrs = (struct attribute **)default_cvorg_chan_attr,
};

static int cvorg_dev_add_attributes(struct cvorg *card)
{
	struct cvorg_channel *channel;
	int ret;
	int i;

	for (i = 0; i < card->n_channels; i++) {
		channel = &card->channels[i];

		memset(&channel->kobj, 0, sizeof(struct kobject));
		ret = kobject_init_and_add(&channel->kobj, &ktype_cvorg_chan,
			&card->dev->kobj, "channel.%d", i);
		if (ret)
			goto error_channel;

		ret = sysfs_create_bin_file(&channel->kobj, &cvorg_chan_pll_attr);
		if(ret)
			goto error_pll;
		ret = sysfs_create_bin_file(&channel->kobj, &cvorg_chan_set_sequence_attr);
		if(ret)
			goto error_set_sequence;
	}
	return 0;

 error_set_sequence:
	sysfs_remove_bin_file(&channel->kobj, &cvorg_chan_pll_attr);

 error_pll:
	kobject_put(&channel->kobj);

 error_channel:
	for (i--; i >= 0; i--) {
		channel = &card->channels[i];
		kobject_put(&channel->kobj);
	}
	return ret;
}

static void cvorg_dev_del_attributes(struct cvorg *card)
{
	struct cvorg_channel *channel;
	int i;

	for (i = 0; i < card->n_channels; i++) {
		channel = &card->channels[i];
		kobject_put(&channel->kobj);
	}
}

int cvorg_create_sysfs_files(struct cvorg *card)
{
	int error;

	error = sysfs_create_group(&card->dev->kobj, &cvorg_attr_group);
	if (error)
		return error;
	error = cvorg_dev_add_attributes(card);
	if (error)
		goto add_attributes_failed;
	return 0;

 add_attributes_failed:
	sysfs_remove_group(&card->dev->kobj, &cvorg_attr_group);
	return error;
}

void cvorg_remove_sysfs_files(struct cvorg *card)
{
	cvorg_dev_del_attributes(card);
	sysfs_remove_group(&card->dev->kobj, &cvorg_attr_group);
}

#else

int cvorg_create_sysfs_files(struct cvorg *card)
{
	return 0;
}

void cvorg_remove_sysfs_files(struct cvorg *card)
{
}

#endif /* CONFIG_SYSFS */
