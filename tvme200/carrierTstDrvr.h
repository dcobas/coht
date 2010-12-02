/**
 * @file carrierTstDrvr.h
 *
 * @brief Test driver for the Carrier driver API
 *
 * @author Nicolas Serafini, EIC2 SA
 *
 * @date Created on 16/02/2009
 *
 * This driver provide some IOCTL that can be used to
 * test the carrier driver interface.
 *
 * @version $Id$
 */
#ifndef _CARRIERTSTDRVR_H_
#define _CARRIERTSTDRVR_H_
/*============================================================================*/
/* Include                                                                    */
/*============================================================================*/
#include "carrier.h"

/*============================================================================*/
/* Structure definitions                                                      */
/*============================================================================*/
/**
 * \struct slot_t
 * \brief Slot data that can be used through IOCTL
 */
typedef struct {
    char         boardName[SLOT_BOARD_NAME_SIZE]; /**< IP board name. */
    unsigned int carrierNumber;             /**< Logical board (VME, PCI) position */
    unsigned int slotPosition;              /**< Slot position on the board\n
                                                 (A->0, B->1, C->2, D->3, ...) */
    slotSpace_e  space;                     /**< Space to map */
    unsigned int memory;                    /**< Size to map (only used in memory space) */
    unsigned long offset;                   /**< Offset for read and write action */
    unsigned char ucharValue;               /**< Value used for read and write */
    unsigned short ushortValue;             /**< Value used for read and write */
    unsigned int uintValue;                 /**< Value used for read and write */
    int vector;                             /**< IRQ vector */
    unsigned int counter;                   /**< IRQ counter */
} slot_t;

/*============================================================================*/
/* IOCTL definitions                                                          */
/*============================================================================*/
/** IOCTL Magic number */
#define CARRIER_IOCTL_MAGIC_NUMBER 0xAE

/** Register slot IOCTL */
#define CARRIER_SLOT_REGISTER         _IOW(CARRIER_IOCTL_MAGIC_NUMBER,   0, slot_t)
#define CARRIER_SLOT_UNREGISTER       _IOW(CARRIER_IOCTL_MAGIC_NUMBER,   1, slot_t)
#define CARRIER_SLOT_MAP_SPACE        _IOW(CARRIER_IOCTL_MAGIC_NUMBER,   2, slot_t)
#define CARRIER_SLOT_UNMAP_SPACE      _IOW(CARRIER_IOCTL_MAGIC_NUMBER,   3, slot_t)
#define CARRIER_SLOT_READ_UCHAR       _IOWR(CARRIER_IOCTL_MAGIC_NUMBER,  4, slot_t)
#define CARRIER_SLOT_READ_USHORT      _IOWR(CARRIER_IOCTL_MAGIC_NUMBER,  5, slot_t)
#define CARRIER_SLOT_READ_UINT        _IOWR(CARRIER_IOCTL_MAGIC_NUMBER,  6, slot_t)
#define CARRIER_SLOT_WRITE_UCHAR      _IOW(CARRIER_IOCTL_MAGIC_NUMBER,   7, slot_t)
#define CARRIER_SLOT_WRITE_USHORT     _IOW(CARRIER_IOCTL_MAGIC_NUMBER,   8, slot_t)
#define CARRIER_SLOT_WRITE_UINT       _IOW(CARRIER_IOCTL_MAGIC_NUMBER,   9, slot_t)
#define CARRIER_SLOT_IRQ_INSTALL      _IOW(CARRIER_IOCTL_MAGIC_NUMBER,  10, slot_t)
#define CARRIER_SLOT_IRQ_FREE         _IOW(CARRIER_IOCTL_MAGIC_NUMBER,  11, slot_t)
#define CARRIER_SLOT_GET_IRQ_COUNTER  _IOWR(CARRIER_IOCTL_MAGIC_NUMBER, 12, slot_t)

#endif /* _CARRIERTSTDRVR_H_ */
