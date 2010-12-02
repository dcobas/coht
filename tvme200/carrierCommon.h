/**
 * @file carrierCommon.h
 *
 * @brief Common interface header file.
 *
 * @author Nicolas Serafini, EIC2 SA
 *
 * @date Created on 21/01/2009
 *
 * This is the common interface between all specific Carrier board.
 *
 * @version $Id: carrierCommon.h,v 1.2 2009/12/01 17:33:57 lewis Exp $
 */
#ifndef _CARRIERCOMMON_H_
#define _CARRIERCOMMON_H_
/*============================================================================*/
/* Include Section                                                            */
/*============================================================================*/
#include <linux/version.h>
#include <linux/limits.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <asm/io.h>

#if LINUX_VERSION_CODE >= 0x02061d
#include <linux/swab.h>		
#else
#include <linux/byteorder/swabb.h>		
#endif

#include "carrier.h"

/*============================================================================*/
/* Define Section                                                             */
/*============================================================================*/
/** SWAPSLB(x) Swap word value for Little-Endian Carrier with Big-Endian IndustryPack Slot
 * on Intel or PowerPc CPU.\n
 * Intel CPU : No swap needed\n
 * PowerPc CPU : Bytes lanes swap (AABB -> BBAA)
 */
#if defined(__LITTLE_ENDIAN)
#define SWAPSLB(x)  x
#endif
#if defined(__BIG_ENDIAN)
#define SWAPSLB(x)  swab16(x)
#endif

/** SWAPLLB(x) Swap long word value for Little-Endian Carrier with Big-Endian IndustryPack Slot
 * on Intel or PowerPc CPU.\n
 * Intel CPU : swaps word lanes of the long word (AABBCCDD -> CCDDAABB)\n
 * PowerPc CPU : swaps byte lanes within word lanes of the long word (AABBCCDD -> BBAADDCC)\n
 */
#if defined(__LITTLE_ENDIAN)
#define SWAPLLB(x)  swahw32(x)
#endif
#if defined(__BIG_ENDIAN)
#define SWAPLLB(x)  swahb32(x)
#endif

/** SWAPSBB(x) Swap word value for Big-Endian Carrier with Big-Endian IndustryPack Slot
 * on Intel or PowerPc CPU.\n
 * Intel CPU : Bytes lanes swap (AABB -> BBAA)\n
 * PowerPc CPU : No swap needed
 */
#if defined(__LITTLE_ENDIAN)
#define SWAPSBB(x)  swab16(x)
#endif
#if defined(__BIG_ENDIAN)
#define SWAPSBB(x)  x
#endif

/** SWAPLBB(x) Swap long word value for Big-Endian Carrier with Big-Endian IndustryPack Slot
 * on Intel or PowerPc CPU.\n
 * Intel CPU : swaps word and byte lanes of the long word (AABBCCDD -> DDCCBBAA)\n
 * PowerPc CPU : No swap needed\n
 */
#if defined(__LITTLE_ENDIAN)
#define SWAPLBB(x)  swab32(x)
#endif
#if defined(__BIG_ENDIAN)
#define SWAPLBB(x)  x
#endif


/** IoId space identifier for installation */
#define VME_IOID_SPACE  "IOID"
/** MEM space identifier for installation */
#define VME_MEM_SPACE  "MEM"
/*============================================================================*/
/* Enum Section                                                               */
/*============================================================================*/
/**
 * \enum carrierType_e
 * \brief Supported carrier board.
 *
 * All the carrier board supported by the driver.
 */
typedef enum {
    CARRIER_TPCI200 = 0, /**< TEWS TPCI200 carrier board. */
    CARRIER_TVME200 = 1, /**< TEWS TVME200 carrier board. (same as VIPC626) */
    CARRIER_VIPC610 = 2, /**< SBS VIPC610 carrier board. */
    CARRIER_VIPC616 = 3, /**< SBS VIPC616 carrier board. */
    CARRIER_VIPC626 = 4, /**< SBS VIPC626 carrier board. (same as TVME200) */
} carrierType_e;

/*============================================================================*/
/* Structure Section                                                          */
/*============================================================================*/
/**
 * \struct paramsPCI_t
 * \brief Parameters of PCI.
 *
 * paramsPCI_t informations specific for installation of PCI carrier boards.
 *
 */
typedef struct {
    unsigned int busNumber;   /**< PCI bus number */
    unsigned int slotNumber;  /**< PCI slot number */
    unsigned int vendorId;    /**< PCI board vendor Id */
    unsigned int deviceId;    /**< PCI board device Id */
    unsigned int subVendorId; /**< PCI board subvendor Id */
    unsigned int subDeviceId; /**< PCI board subdevice Id */
} paramsPCI_t;

/**
 * \struct vmeAddrSpace_t
 * \brief VME addresse space.
 *
 * vmeAddrSpace_t VME address space definition.
 *
 */
typedef struct {
    unsigned int addressModifier; /**< Address modifier */
    unsigned int dataWidth;       /**< Data width */
    unsigned int baseAddress;     /**< Base address */
    unsigned int windowSize;      /**< Window size */
} vmeAddrSpace_t;

/**
 * \struct paramsVME_t
 * \brief Parameters of VME.
 *
 * paramsVME_t informations specific for installation of VME carrier boards.
 *
 */
typedef struct {
    vmeAddrSpace_t ioidSpace; /**< IO and ID address space informations */
    vmeAddrSpace_t memSpace;  /**< Memory address space informations */
} paramsVME_t;

/**
 * \struct slotIrq_t
 * \brief IP slot IRQ definition.
 *
 * slotIrq_t the slot IRQ definition.
 *
 */
typedef struct {
    int          vector;                     /**< Vector number */
    int         (*handler)(void*);          /**< Handler called on IRQ */
    void         *arg;                       /**< Handler argument */
    char         name[SLOT_IRQ_NAME_SIZE];   /**< IRQ name */
} slotIrq_t;

/**
 * \struct carrierSlot_t
 * \brief Slot of the carrier.
 *
 * carrierSlot_t data specific to the carrier slot.
 *
 */
typedef struct {
    slotId_t*       slotId;   /**< Slot identification gived to external interface */
    slotIrq_t*      irq;      /**< Slot IRQ infos */
    slotAddrSpace_t ioPhys;   /**< IO physical base address register of the slot */
    slotAddrSpace_t idPhys;   /**< ID physical base address register of the slot */
    slotAddrSpace_t memPhys;  /**< MEM physical base address register of the slot */
} carrierSlot_t;

/**
 * \struct carrierBoard_t
 * \brief Carrier board structure.
 *
 * carrierBoard_t contain all configuration information about a board.
 *
 */
struct str_carrierAPI;
typedef struct {
    unsigned int          carrierNumber;          /**< Logical board position */
    char                  procEntryName[NAME_MAX];/**< The proc entry name of the carrier */
    struct str_carrierAPI *carrierAPI;            /**< Carrier board API */
    void                  *carrierSpecific;       /**< Board specific data (PCI or VME) */
    struct mutex          carrierMutex;           /**< Carrier mutex to prevent concurent access */
    carrierSlot_t         *slots;                 /**< Array of slots\n
                                                       (A->0, B->1, C->2, D->3, ...) */
} carrierBoard_t;

/**
 * \struct carrierAPI_t
 * \brief Carrier board specific API structure.
 *
 * carrierAPI_t is the standard API of each boards.
 *
 */
typedef struct str_carrierAPI {
    const char    *longName;    /**< Carrier board long name */
    const char    *shortName;    /**< Carrier board short name */
    unsigned int  numberOfSlot; /**< Number of slots of the carrier */
    carrierType_e type;         /**< Type of the board */
    int  (*carrierRegister)   (carrierBoard_t *pCarrier, void *devSpecific); /**< Carrier board specific initialisation */
    void (*carrierUnregister) (carrierBoard_t *pCarrier);                    /**< Carrier board specific initialisation */
    int  (*carrierProcShow)   (carrierBoard_t *pCarrier, char *page);        /**< For procfs printout */
    int  (*slotRequestIrq)    (carrierBoard_t *pCarrier, slotId_t *pSlotId); /**< Request a slot IRQ */
    void (*slotFreeIrq)       (carrierBoard_t *pCarrier, slotId_t *pSlotId); /**< Free a slot IRQ */
    unsigned char  (*slotReadUchar)  (void *address, unsigned long offset);  /**< Read an unsigned char value */
    unsigned short (*slotReadUshort) (void *address, unsigned long offset);  /**< Read an unsigned short value */
    unsigned int   (*slotReadUint)   (void *address, unsigned long offset);  /**< Read an unsigned int value */
    void (*slotWriteUchar)  (unsigned char value, void *address, unsigned long offset);  /**< Write an unsigned char value */
    void (*slotWriteUshort) (unsigned short value, void *address, unsigned long offset); /**< Write an unsigned short value */
    void (*slotWriteUint)   (unsigned int value, void *address, unsigned long offset);   /**< Write an unsigned int value */
    int  (*slotWriteVector) (unsigned char vector, slotId_t *pSlotId, slotSpace_e space, unsigned long offset); /**< Write an the vector */
} carrierAPI_t;

#endif /* _CARRIERCOMMON_H_ */
