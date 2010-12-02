/**
 * @file VIPC61x.h
 *
 * @brief SBS VIPC610 and SBS VIPC616 registers definitions
 *
 * @author Nicolas Serafini, EIC2 SA
 *
 * @date Created on 21/01/2009
 *
 * All registers definitions of the VIPC610 and VIPC616 carrier.
 *
 * @version $Id: VIPC61x.h,v 1.1 2009/12/01 14:13:03 lewis Exp dcobas $
 */

#ifndef _VIPC61X_H_
#define _VIPC61X_H_

/*============================================================================*/
/* Names                                                                      */
/*============================================================================*/
/** Long name of the VIPC616 carrier board */
#define VIPC616_LONGNAME "SBS Technologies - VME VIPC616 Carrier"
/** Short name of the VIPC616 carrier board */
#define VIPC616_SHORTNAME "VIPC616"

/** Long name of the VIPC610 carrier board */
#define VIPC610_LONGNAME "SBS Technologies - VME VIPC610 Carrier"
/** Short name of the VIPC610 carrier board */
#define VIPC610_SHORTNAME "VIPC610"

/*============================================================================*/
/* Carrier informations                                                       */
/*============================================================================*/
/** Number of Slot of the carrier board */
#define VIPC61X_NB_SLOT               0x4

/*============================================================================*/
/* IP IO/ID/INT Space Definitions                                             */
/*============================================================================*/
/** VIPC61x IO space base offset */
#define VIPC61X_IO_SPACE_OFF          0x0000
/** VIPC61x IO space IP gap */
#define VIPC61X_IO_SPACE_GAP          0x0100
/** VIPC61x IO space size per IP */
#define VIPC61X_IO_SPACE_SIZE         0x0080
/** VIPC61x ID space base offset */
#define VIPC61X_ID_SPACE_OFF          0x0080
/** VIPC61x ID space IP gap */
#define VIPC61X_ID_SPACE_GAP          0x0100
/** VIPC61x ID space size per IP */
#define VIPC61X_ID_SPACE_SIZE         0x0080

/** VIPC61x IO ID and INT space size */
#define VIPC61X_IOIDINT_SIZE          0x0400

/*============================================================================*/
/* MEM Space Definitions                                                      */
/*============================================================================*/
/** VIPC61x 512k Total Carrier MEM size */
#define VIPC61X_MEM_512k            0x00080000
/** VIPC61x Slot MEM Gap for 512k Total Carrier MEM size */
#define VIPC61X_MEM_512k_SLOT_GAP   0x00020000
/** VIPC61x Slot MEM size for 512k Total Carrier MEM size */
#define VIPC61X_MEM_512k_SLOT_SIZE  0x00020000

/** VIPC61x 1M Total Carrier MEM size */
#define VIPC61X_MEM_1M              0x00100000
/** VIPC61x Slot MEM Gap for 1M Total Carrier MEM size */
#define VIPC61X_MEM_1M_SLOT_GAP     0x00040000
/** VIPC61x Slot MEM size for 1M Total Carrier MEM size */
#define VIPC61X_MEM_1M_SLOT_SIZE    0x00040000

/** VIPC61x 2M Total Carrier MEM size */
#define VIPC61X_MEM_2M              0x00200000
/** VIPC61x Slot MEM Gap for 2M Total Carrier MEM size */
#define VIPC61X_MEM_2M_SLOT_GAP     0x00080000
/** VIPC61x Slot MEM size for 2M Total Carrier MEM size */
#define VIPC61X_MEM_2M_SLOT_SIZE    0x00080000

/** VIPC61x 4M Total Carrier MEM size */
#define VIPC61X_MEM_4M              0x00400000
/** VIPC61x Slot MEM Gap for 4M Total Carrier MEM size */
#define VIPC61X_MEM_4M_SLOT_GAP     0x00100000
/** VIPC61x Slot MEM size for 4M Total Carrier MEM size */
#define VIPC61X_MEM_4M_SLOT_SIZE    0x00100000

/** VIPC61x 8M Total Carrier MEM size */
#define VIPC61X_MEM_8M              0x00800000
/** VIPC61x Slot MEM Gap for 8M Total Carrier MEM size */
#define VIPC61X_MEM_8M_SLOT_GAP     0x00200000
/** VIPC61x Slot MEM size for 8M Total Carrier MEM size */
#define VIPC61X_MEM_8M_SLOT_SIZE    0x00200000

/** VIPC61x 32M Total Carrier MEM size */
#define VIPC61X_MEM_32M             0x02000000
/** VIPC61x Slot MEM Gap for 32M Total Carrier MEM size */
#define VIPC61X_MEM_32M_SLOT_GAP    0x00800000
/** VIPC61x Slot MEM size for 32M Total Carrier MEM size */
#define VIPC61X_MEM_32M_SLOT_SIZE   0x00800000

#endif /* _VIPC61X_H_ */
