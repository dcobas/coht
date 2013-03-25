/**
 * cvorb_dma.c
 *
 * derived from sis33 dma module
 * Copyright (c) 2012 Michel Arruat <michel.arruat@cern.ch>
 * Copyright (c) 2009 Emilio G. Cota <cota@braap.org>
 * Released under the GPL v2. (and only v2, not any later version)
 */
#include <linux/kernel.h>

#include <vmebus.h>

/* GCC Preprocessing option enabling dma code */
/*
#define __CVORB_DMA__
*/

#ifdef __CVORB_DMA__
#define VME_SOURCE 1
#define PCI_SOURCE 2

static int
__cvorb_dma(struct device *dev, unsigned int vme_addr,
	    enum vme_address_modifier am, void *addr, ssize_t size,
	    int src)
{
	struct vme_dma desc;
	struct vme_dma_attr *vme;
	struct vme_dma_attr *pci;
	int ret;

	memset(&desc, 0, sizeof(desc));

	vme = (src == VME_SOURCE) ? &desc.src : &desc.dst;
	pci = (src == PCI_SOURCE) ? &desc.src : &desc.dst;
	desc.dir =
	    (src == PCI_SOURCE) ? VME_DMA_TO_DEVICE : VME_DMA_FROM_DEVICE;
	desc.length = size;

	desc.ctrl.pci_block_size = VME_DMA_BSIZE_2048;
	desc.ctrl.pci_backoff_time = VME_DMA_BACKOFF_0;
	desc.ctrl.vme_block_size = VME_DMA_BSIZE_2048;
	desc.ctrl.vme_backoff_time = VME_DMA_BACKOFF_0;

	pci->addru = upper_32_bits((unsigned long) addr);
	pci->addrl = lower_32_bits((unsigned long) addr);

	vme->addru = 0;
	vme->addrl = vme_addr;
	vme->am = am;
	vme->data_width = VME_D32;

/*        printk("DMA 0x%8x size: 0x%x\n", vme_addr, desc.length); */
	dev_dbg(dev, "DMA 0x%8x size: 0x%x\n", vme_addr, desc.length);
/*
	if (to_user)
		ret = vme_do_dma(&desc);
	else
*/
	ret = vme_do_dma_kernel(&desc);
	return ret;
}

/**
 * cvorb_dma_read_mblt - read a block of data to a kernel bufferfrom VME using MBLT-DMA
 * @dev:	device to read from
 * @vme_addr:	VME address to start reading from
 * @addr:	address to write to
 * @size:	length of the block (in bytes) to be read
 *
 * returns 0 on success, negative error code on failure
 */
int cvorb_dma_read_mblt(struct device *dev, unsigned int vme_addr,
			void __kernel * addr, ssize_t size)
{
	return __cvorb_dma(dev, vme_addr, VME_A16_USER,
			   (void __force *) addr, size, VME_SOURCE);
}

EXPORT_SYMBOL_GPL(cvorb_dma_read_mblt);

/**
 * cvorb_dma_read_mblt - read a block of data to a kernel bufferfrom VME using MBLT-DMA
 * @dev:        device to read from
 * @vme_addr:   VME address to start reading from
 * @addr:       address to write to
 * @size:       length of the block (in bytes) to be read
 *
 * returns 0 on success, negative error code on failure
 */
int cvorb_dma_write_mblt(struct device *dev, unsigned int vme_addr,
			 void __kernel * addr, ssize_t size)
{
	return __cvorb_dma(dev, vme_addr, VME_A16_USER,
			   (void __force *) addr, size, PCI_SOURCE);
}

EXPORT_SYMBOL_GPL(cvorb_dma_write_mblt);
#endif
