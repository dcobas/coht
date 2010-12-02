/**
 * carrier_common.h
 *
 * driver for the carrier TEWS TPCI-200 
 * Copyright (c) 2009 Nicolas Serafini, EIC2 SA
 * Copyright (c) 2010,2011 Samuel Iglesias Gonsalvez <siglesia@cern.ch>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#ifndef _CARRIERCOMMON_H_
#define _CARRIERCOMMON_H_

#include <linux/version.h>
#include <linux/limits.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include "carrier.h"

#define VME_IOID_SPACE  "IOID"
#define VME_MEM_SPACE  "MEM"

/**
 * struct params_pci - informations specific for installation of PCI carrier boards.
 * @bus_number		PCI bus number
 * @slot_number		PCI slot number
 * @vendor_id		PCI board vendor ID
 * @device_id		PCI board device ID
 * @subvendor_id	PCI board subvendor ID
 * @subdevice_id	PCI board subdevice ID
 *
 */
struct params_pci {
    unsigned int bus_number;
    unsigned int slot_number;
    unsigned int vendor_id;
    unsigned int device_id;
    unsigned int subvendor_id;
    unsigned int subdevice_id;
};

/**
 * struct vme_addr_space - VME address space definition.
 * @address_modifier
 * @data_witdth
 * @base_address
 * @window_size
 *
 */
struct vme_addr_space {
    unsigned int address_modifier;
    unsigned int data_width;
    unsigned int base_address;
    unsigned int window_size;
};

/**
 * struct params_vme - informations specific for installation of VME carrier boards.
 * @ioid_space	IO and ID address space informations
 * @mem_space	Memory space informations
 *
 */
struct params_vme {
    struct vme_addr_space ioid_space;
    struct vme_addr_space mem_space;
};

/**
 * struct slot_irq - slot IRQ definition.
 * @vector	Vector number
 * @handler	Handler called when IRQ arrives
 * @arg		Handler argument
 * @name	IRQ name
 *
 */
struct slot_irq{
    int          vector;
    int         (*handler)(void*);
    void         *arg;
    char         name[SLOT_IRQ_NAME_SIZE];
};

/**
 * struct carrier_slot - data specific to the carrier slot.
 * @slot_id	Slot identification gived to external interface
 * @irq		Slot IRQ infos
 * @io_phys	IO physical base address register of the slot
 * @id_phys	ID physical base address register of the slot
 * @mem_phys	MEM physical base address register of the slot
 *
 */
struct carrier_slot {
    struct slot_id*       slot_id;
    struct slot_irq*      irq;
    struct slot_addr_space io_phys;
    struct slot_addr_space id_phys;
    struct slot_addr_space mem_phys;
};

/**
 * struct carrier_board - all configuration information about a board.
 * @carrier_number	Logical board position
 * @carrier_board_API	Carrier board API
 * @carrier_mutex	Carrier mutex to prevent concurrent access
 * @slots 		Array os slots
 *
 */
struct str_carrier_api;

struct carrier_board {
    unsigned int          carrier_number;
    struct str_carrier_api *carrier_api;
    void                  *carrier_specific;
    struct mutex          carrier_mutex;
    struct carrier_slot         *slots;
};

/**
 * struct str_carrier_api_t - API for each board
 * @carrier_register	Carrier board specific initialisation
 * @carrier_unregister	Carrier board specific initialisation
 * @slot_request_irq	Request a slot IRQ
 * @slot_free_irq	Free a slot IRQ
 * @slot_read_uchar	Read an unsigned char value
 * @slot_read_ushort	Read an unsigned short value
 * @slot_read_uint	Read an unsigned int value
 * @slot_write_uchar	Write an unsigned char value
 * @slot_write_ushort	Write an unsigned short value
 * @slot_write_uint	Write an unsigned int value
 * @slot_write_vector	Write the IRQ vector
 *
 */
struct str_carrier_api {
    int  (*carrier_register)   (struct carrier_board *carrier, void *dev_specific);
    void (*carrier_unregister) (struct carrier_board *carrier);
    int  (*slot_request_irq)    (struct carrier_board *carrier, struct slot_id *slot_id);
    void (*slot_free_irq)       (struct carrier_board *carrier, struct slot_id *slot_id);
    unsigned char  (*slot_read_uchar)  (void *address, unsigned long offset);
    unsigned short (*slot_read_ushort) (void *address, unsigned long offset);
    unsigned int   (*slot_read_uint)   (void *address, unsigned long offset);
    void (*slot_write_uchar)  (unsigned char value, void *address, unsigned long offset);
    void (*slot_write_ushort) (unsigned short value, void *address, unsigned long offset);
    void (*slot_write_uint)   (unsigned int value, void *address, unsigned long offset);
    int  (*slot_write_vector) (unsigned char vector, struct slot_id *slot_id, slot_space space, unsigned long offset);
};

#endif /* _CARRIERCOMMON_H_ */
