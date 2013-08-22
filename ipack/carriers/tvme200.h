#ifndef _TVME200_H_
#define _TVME200_H_

#include <linux/version.h>
#include <linux/limits.h>
#include <linux/spinlock.h>
#include <linux/swab.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include "linux/ipack.h"
#include "vmebus.h"


#define TVME200_LONGNAME "TEWS Technologies - VME TVME200 Carrier"
#define TVME200_SHORTNAME "TVME200"
#define VIPC626_LONGNAME "SBS Technologies - VME VIPC626 Carrier"
#define VIPC626_SHORTNAME "VIPC626"

#define TVME200_NB_SLOT               0x4
#define TVME200_IO_SPACE_OFF          0x0000
#define TVME200_IO_SPACE_INTERVAL     0x0100
#define TVME200_IO_SPACE_SIZE         0x0080
#define TVME200_ID_SPACE_OFF          0x0080
#define TVME200_ID_SPACE_INTERVAL     0x0100
#define TVME200_ID_SPACE_SIZE         0x0080
#define TVME200_INT_SPACE_OFF         0x00C0
#define TVME200_INT_SPACE_INTERVAL    0x0100
#define TVME200_INT_SPACE_SIZE        0x0040
#define TVME200_IOIDINT_SIZE          0x0400

#define TVME200_MEM_128k            0x00020000
#define TVME200_MEM_128k_SLOT_GAP   0x00008000
#define TVME200_MEM_128k_SLOT_SIZE  0x00008000
#define TVME200_MEM_256k            0x00040000
#define TVME200_MEM_256k_SLOT_GAP   0x00010000
#define TVME200_MEM_256k_SLOT_SIZE  0x00010000
#define TVME200_MEM_512k            0x00080000
#define TVME200_MEM_512k_SLOT_GAP   0x00020000
#define TVME200_MEM_512k_SLOT_SIZE  0x00020000
#define TVME200_MEM_1M              0x00100000
#define TVME200_MEM_1M_SLOT_GAP     0x00040000
#define TVME200_MEM_1M_SLOT_SIZE    0x00040000
#define TVME200_MEM_2M              0x00200000
#define TVME200_MEM_2M_SLOT_GAP     0x00080000
#define TVME200_MEM_2M_SLOT_SIZE    0x00080000
#define TVME200_MEM_4M              0x00400000
#define TVME200_MEM_4M_SLOT_GAP     0x00100000
#define TVME200_MEM_4M_SLOT_SIZE    0x00100000
#define TVME200_MEM_8M              0x00800000
#define TVME200_MEM_8M_SLOT_GAP     0x00200000
#define TVME200_MEM_8M_SLOT_SIZE    0x00200000
#define TVME200_MEM_32M             0x02000000
#define TVME200_MEM_32M_SLOT_GAP    0x00800000
#define TVME200_MEM_32M_SLOT_SIZE   0x00800000

#define VME_IOID_SPACE  "IOID"
#define VME_MEM_SPACE  "MEM"

/**
 * struct params_pci - informations specific for installation of PCI carrier boards.
 * @bus_number		PCI bus number
 * @slot_number		PCI slot number
 * @vendor_id		PCI board vendor ID
 * @device_id		PCI board device ID
 * @sub_vendor_id	PCI board subvendor ID
 * @sub_device_id	PCI board subdevice ID
 *
 */
struct params_pci {
    unsigned int bus_number;
    unsigned int slot_number;
    unsigned int vendor_id;
    unsigned int device_id;
    unsigned int sub_vendor_id;
    unsigned int sub_device_id;
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

struct params_vme {
    struct vme_addr_space ioid_space;
    struct vme_addr_space mem_space;
};

#define SLOT_BOARD_NAME_SIZE    50
#define SLOT_IRQ_NAME_SIZE      50

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
	struct ipack_device *holder;
	int  (*handler)(void*);
	void         *arg;
	char         name[SLOT_IRQ_NAME_SIZE];
};

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

/**
 * struct carrier_slot - data specific to the carrier slot.
 * @slot_id	Slot identification gived to external interface
 * @irq		Slot IRQ infos
 * @io_phys	IO physical base address register of the slot
 * @id_phys	ID physical base address register of the slot
 * @mem_phys	MEM physical base address register of the slot
 *
 */
struct tvme200_slot {
    struct slot_id		*slot_id;
    struct slot_irq		*irq;
    struct slot_addr_space 	io_phys;
    struct slot_addr_space 	id_phys;
    struct slot_addr_space 	mem_phys;
};

/**
 * struct tpci200_infos - informations specific of the TPCI200 tpci200.
 * @ioid_mapping        inteface with the vmebridge driver for the IoIdInt space
 * @mem_mapping         inteface with the vmebridge driver for the Mem space
 */
struct tvme200_infos {
	struct device			*dev;
        struct vme_mapping		ioid_mapping;
        struct vme_mapping		mem_mapping;
	struct ipack_bus_device		*ipack_bus;
};

/**
 * struct tvme200_board - all configuration information about a board.
 * @_number	Logical board position
 * @mutex	Carrier mutex to prevent concurrent access
 * @slots 		Array os slots
 *
 */
struct tvme200_board {
	unsigned int          	number;
	struct mutex          	mutex;
	spinlock_t		regs_lock;
	struct tvme200_slot   	*slots;
	struct tvme200_infos	*info;
	phys_addr_t             mod_mem[IPACK_SPACE_COUNT];
};


#endif
