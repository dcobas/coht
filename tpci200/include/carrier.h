/**
 * carrier.h
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

#ifndef _CARRIER_H_
#define _CARRIER_H_

#define SLOT_BOARD_NAME_SIZE    50
#define SLOT_IRQ_NAME_SIZE      50

#define SLOT_IDPROM_OFFSET_I                  0x01
#define SLOT_IDPROM_OFFSET_P                  0x03
#define SLOT_IDPROM_OFFSET_A                  0x05
#define SLOT_IDPROM_OFFSET_C                  0x07
#define SLOT_IDPROM_OFFSET_MANUFACTURER_ID    0x09
#define SLOT_IDPROM_OFFSET_MODEL              0x0B
#define SLOT_IDPROM_OFFSET_REVISION           0x0D
#define SLOT_IDPROM_OFFSET_RESERVED           0x0F
#define SLOT_IDPROM_OFFSET_DRIVER_ID_L        0x11
#define SLOT_IDPROM_OFFSET_DRIVER_ID_H        0x13
#define SLOT_IDPROM_OFFSET_NUM_BYTES          0x15
#define SLOT_IDPROM_OFFSET_CRC                0x17

typedef enum {
    SLOT_IO_SPACE    = 0,
    SLOT_ID_SPACE    = 1,
    SLOT_MEM_SPACE   = 2,
} slot_space;

/**
 * struct slot_addr_space - Virtual address space mapped for a specified type.
 * @address
 * @size
 *
 */
struct slot_addr_space {
    void         *address;  /**< adress */
    unsigned int size;      /**< size */
};

/**
 * struct slot_id - contain all configuration information and mapped memory informations
 * @board_name		IP Board name
 * @carrier_number	Logical board number (VME, PCI)
 * @slot_position	Slot position
 * @id_space		Mapped ID space address
 * @io_space		Mapped IO space address
 * @mem_space		Mapped Memory space address
 *
 * Warning: Direct access to mapped memory is possible but the endianness
 * is not the same with PCI carrier or VME carrier.
 */
struct slot_id {
    char         board_name[SLOT_BOARD_NAME_SIZE];
    unsigned int carrier_number;
    unsigned int slot_position;
    struct slot_addr_space id_space;
    struct slot_addr_space io_space;
    struct slot_addr_space mem_space;
};

#ifdef __KERNEL__

/**
 * ip_slot_register - Register a new IP Slot,\n
 *
 * @board_name		Name of the slot board
 * @carrier_number	Carrier board number (0..N)
 * @slot_position	Position on the carrier (0..N)
 *
 */
extern struct slot_id* ip_slot_register(char *board_name, unsigned int carrier_number,
                         unsigned int slot_position);

/**
 * ip_slot_unregister - Unregister a IP Slot.
 *
 * @slot_id	Slot to unregister
 *
 */
extern int ip_slot_unregister(struct slot_id *slot_id);

/**
 * ip_slot_map_space - Map an IP Slot specific space.\n
 *        If space address is NULL after mapping it's because space size is 0 or an error occured.
 *
 * @slot_id		Slot to map space
 * @memory_size		Memory Size needed by the IP module.
 *			This parameter is only used to map memory space
 *			For other space size is hardware fixed
 * @space		Specific space to map
 *
 */
extern int ip_slot_map_space(struct slot_id *slot_id, unsigned int memory_size, slot_space space);

/**
 * ip_slot_unmap_space - Unmap an IP Slot specific space.
 *
 * @slot_id	Slot to map space
 * @space	Specific space to unmap
 *
 */
extern int ip_slot_unmap_space(struct slot_id *slot_id, slot_space space);

/**
 * ip_slot_request_irq - Install a IRQ for the slot
 *
 * @slot_id	Slot to install IRQ
 * @vector	Interrupt vector
 * @handler	Interrupt handler
 * @arg		Interrupt handler argument
 * @name	Interrupt name
 *
 * WARNING:
 * Setup the Interrupt Vector in the IndustryPackdevice before an IRQ Request.
 * This is necessary for VMEbus carrier interrupt identification.
 * Read the User Manual of your IndustryPack device to know where writing the vector in memory.
 *
 */
extern int ip_slot_request_irq(struct slot_id *slot_id, int vector,
                            int (*handler)(void *), void *arg,
                            char *name);


/**
 * ip_slot_write_vector - Write the vector into the specified offset of memory space.
 *
 * @vector	Vector to write
 * @slot_id	Slot to write
 * @space	Space of the slot to write
 * @offset	Offset
 *
 * This function is like ip_slot_write_uchar but work only for VME carrier.\n
 * PCI carrier doesn't need vector value but to have a standard interface use this API
 * to write vector.
 *
 */
extern int ip_slot_write_vector(unsigned char vector, struct slot_id *slot_id, slot_space space, unsigned long offset);

/**
 * ip_slot_free_irq - Free the IRQ of the slot
 *
 * @slot_id	Slot to uninstall IRQ
 *
 */
extern int ip_slot_free_irq(struct slot_id *slot_id);

/**
 * ip_slot_read_uchar - Read an unsigned char value on space with offset.
 *
 * @value	Readed value
 * @slot_id	Slot to read
 * @space	Space of the slot to read
 * @offset	Offset
 *
 */
extern int ip_slot_read_uchar(unsigned char *value, struct slot_id *slot_id, slot_space space, unsigned long offset);

/**
 * ip_slot_read_ushort - Read an unsigned short value on space with offset.
 *
 * @value	Read value
 * @slot_id	Slot to read
 * @space	Space of the slot to read
 * @offset	Offset
 *
 */
extern int ip_slot_read_ushort(unsigned short *value, struct slot_id *slot_id, slot_space space, unsigned long offset);

/**
 * ip_slot_read_ushort - Read an unsigned int value on space with offset.
 *
 * @value	Read value
 * @slot_id	Slot to read
 * @space	Space of the slot to read
 * @offset	Offset
 *
 */
extern int ip_slot_read_uint(unsigned int *value, struct slot_id *slot_id, slot_space space, unsigned long offset);

/**
 * ip_slot_write_uchar - Write an unsigned char value on space with offset.
 *
 * @value	Value to write
 * @slot_id	Slot to write
 * @space	Space of the slot to write
 * @offset	Offset
 *
 * @return  0 - success
 * @return <0 - on error
 */
extern int ip_slot_write_uchar(unsigned char value, struct slot_id *slot_id, slot_space space, unsigned long offset);

/**
 * ip_slot_write_ushort - Write an unsigned short value on space with offset.
 *
 * @value	Value to write
 * @slot_id	Slot to write
 * @space	Space of the slot to write
 * @offset	Offset
 *
 */
extern int ip_slot_write_ushort(unsigned short value, struct slot_id *slot_id, slot_space space, unsigned long offset);

/**
 * ip_slot_write_uint - Write an unsigned int value on space with offset.
 *
 * @value	Value to write
 * @slot_id	Slot to write
 * @space	Space of the slot to write
 * @offset	Offset
 *
 */
extern int ip_slot_write_uint(unsigned int value, struct slot_id *slot_id, slot_space space, unsigned long offset);

#endif /* __KERNEL__ */
#endif /* _CARRIER_H_ */
