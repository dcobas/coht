/**
 * @file carrier.h
 *
 * @brief External interface header file.
 *
 * @author Nicolas Serafini, EIC2 SA
 *
 * @date Created on 21/01/2009
 *
 * This is the external interface for IndustryPack boards driver.
 *
 * @version $Id: carrier.h,v 1.1 2009/12/01 14:13:03 lewis Exp dcobas $
 */

#ifndef _CARRIER_H_
#define _CARRIER_H_

/*============================================================================*/
/* Define Section                                                             */
/*============================================================================*/
/** IndustryPack board name maximum size. */
#define SLOT_BOARD_NAME_SIZE    50
#define SLOT_IRQ_NAME_SIZE      50

/** \name Slot ID Prom register offset */
/* @{ */
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
/* @} */

/*============================================================================*/
/* Enum Section                                                               */
/*============================================================================*/
/**
 * \enum slotSpace_e
 * \brief Different slot spaces.
 */
typedef enum {
    SLOT_IO_SPACE    = 0, /**< Slot IO space */
    SLOT_ID_SPACE    = 1, /**< Slot ID space */
    SLOT_MEM_SPACE   = 2, /**< Slot Memory space */
} slotSpace_e;

/*============================================================================*/
/* Structure Section                                                          */
/*============================================================================*/

/**
 * \struct slotAddrSpace_t
 * \brief Virtual address space mapped for a specified type.
 *
 * slotAddrSpace_t contains information about the mapped space memory of the
 * slot
 *
 */
typedef struct {
    void         *address;  /**< adress */
    unsigned int size;      /**< size */
} slotAddrSpace_t;

/**
 * \struct slotId_t
 * \brief IndustryPack slot structure.
 *
 * slotId_t contain all configuration information and mapped memory informations
 * of the slot and is returned as a handle during IndustryPack board
 * registration.
 * <b>Warning</b> : Direct access to mapped memory is possible but the endianness
 * is not the same with PCI carrier or VME carrier.
 */
typedef struct {
    char         boardName[SLOT_BOARD_NAME_SIZE]; /**< IP board name. */
    unsigned int carrierNumber;                   /**< Logical board (VME, PCI) position (0..N)*/
    unsigned int slotPosition;                    /**< Slot position on the board\n
                                                       (A->0, B->1, C->2, D->3, ...) */
    slotAddrSpace_t idSpace;                      /**< Mapped ID space address */
    slotAddrSpace_t ioSpace;                      /**< Mapped IO space address */
    slotAddrSpace_t memSpace;                     /**< Mapped Memory space address */
} slotId_t;

/*============================================================================*/
/* Exported function prototype                                                */
/*============================================================================*/
#ifdef __KERNEL__
/*
 * Those definitions are for drivers only and are not visible to userspace.
 */
/**
 * @brief Register a new IP Slot,\n
 *
 * @param boardName     - Name of the slot board
 * @param carrierNumber - Carrier board number (0..N)
 * @param slotPosition  - Position on the carrier (0..N)
 *
 * @return slotId_t structure - if success.
 * @return NULL               - if fails.
 */
extern slotId_t* ipSlotRegister(char *boardName, unsigned int carrierNumber,
                         unsigned int slotPosition);

/**
 * @brief Unregister a IP Slot.
 *
 * @param pSlotId - Slot to unregister
 *
 * @return  0 - success
 * @return -1 - on error
 */
extern int ipSlotUnregister(slotId_t *pSlotId);

/**
 * @brief Map an IP Slot specific space.\n
 *        If space address is NULL after mapping it's because space size is 0 or an error occured.
 *
 * @param pSlotId    - Slot to map space
 * @param memorySize - Memory Size needed by the IP module.\n
 *                     <i>This parameter is only used to map memory space</i>\n
 *                     <i>For other space size is hardware fixed</i>
 * @param space      - specific space to map
 *
 * @return  0 - success
 * @return <0 - on error
 */
extern int ipSlotMapSpace(slotId_t *pSlotId, unsigned int memorySize, slotSpace_e space);

/**
 * @brief Unmap an IP Slot specific space.
 *
 * @param pSlotId - Slot to map space
 * @param space   - specific space to unmap
 *
 * @return  0 - success
 * @return -1 - on error
 */
extern int ipSlotUnmapSpace(slotId_t *pSlotId, slotSpace_e space);

/**
 * @brief Install a IRQ for the slot
 *
 * @param pSlotId   - Slot to install IRQ
 * @param vector    - Interrupt vector
 * @param handler   - Interrupt handler
 * @param arg       - Interrupt handler argument
 * @param name      - Interrupt name
 *
 * <b> WARNING </b> :\n
 * Setup the Interrupt Vector in the IndustryPackdevice before an IRQ Request.\n
 * This is necessary for VMEbus carrier interrupt identification.\n
 * Read the User Manual of your IndustryPack device to know where writing the vector in memory.
 *
 * @return  0 - success
 * @return -1 - on error
 */
extern int ipSlotRequestIRQ(slotId_t *pSlotId, int vector,
                            int (*handler)(void *), void *arg,
                            char *name);


/**
 * @brief Write the vector into the specified offset of memory space.
 *
 * @param vector  - Vector to write
 * @param pSlotId - Slot to write
 * @param space   - space of the slot to write
 * @param offset  - Offset
 *
 * This function is like ipSlotWriteUchar but work only for VME carrier.\n
 * PCI carrier doesn't need vector value but to have a standard interface use this API
 * to write vector.
 *
 * @return  0 - success
 * @return <0 - on error
 */
extern int ipSlotWriteVector(unsigned char vector, slotId_t *pSlotId, slotSpace_e space, unsigned long offset);

/**
 * @brief Free the IRQ of the slot
 *
 * @param pSlotId - Slot to uninstall IRQ
 *
 * @return  0 - success
 * @return -1 - on error
 */
extern int ipSlotFreeIRQ(slotId_t *pSlotId);

/**
 * @brief Read an unsigned char value on space with offset.
 *
 * @param value   - Readed value
 * @param pSlotId - Slot to read
 * @param space   - space of the slot to read
 * @param offset  - Offset
 *
 * @return  0 - success
 * @return <0 - on error
 */
extern int ipSlotReadUchar(unsigned char *value, slotId_t *pSlotId, slotSpace_e space, unsigned long offset);

/**
 * @brief Read an unsigned short value on space with offset.
 *
 * @param value   - Readed value
 * @param pSlotId - Slot to read
 * @param space   - space of the slot to read
 * @param offset  - Offset
 *
 * @return  0 - success
 * @return <0 - on error
 */
extern int ipSlotReadUshort(unsigned short *value, slotId_t *pSlotId, slotSpace_e space, unsigned long offset);

/**
 * @brief Read an unsigned int value on space with offset.
 *
 * @param value   - Readed value
 * @param pSlotId - Slot to read
 * @param space   - space of the slot to read
 * @param offset  - Offset
 *
 * @return  0 - success
 * @return <0 - on error
 */
extern int ipSlotReadUint(unsigned int *value, slotId_t *pSlotId, slotSpace_e space, unsigned long offset);

/**
 * @brief Write an unsigned char value on space with offset.
 *
 * @param value   - Value to write
 * @param pSlotId - Slot to write
 * @param space   - space of the slot to write
 * @param offset  - Offset
 *
 * @return  0 - success
 * @return <0 - on error
 */
extern int ipSlotWriteUchar(unsigned char value, slotId_t *pSlotId, slotSpace_e space, unsigned long offset);

/**
 * @brief Write an unsigned short value on space with offset.
 *
 * @param value   - Value to write
 * @param pSlotId - Slot to write
 * @param space   - space of the slot to write
 * @param offset  - Offset
 *
 * @return  0 - success
 * @return <0 - on error
 */
extern int ipSlotWriteUshort(unsigned short value, slotId_t *pSlotId, slotSpace_e space, unsigned long offset);

/**
 * @brief Write an unsigned int value on space with offset.
 *
 * @param value   - Value to write
 * @param pSlotId - Slot to write
 * @param space   - space of the slot to write
 * @param offset  - Offset
 *
 * @return  0 - success
 * @return <0 - on error
 */
extern int ipSlotWriteUint(unsigned int value, slotId_t *pSlotId, slotSpace_e space, unsigned long offset);

#endif /* __KERNEL__ */
#endif /* _CARRIER_H_ */
