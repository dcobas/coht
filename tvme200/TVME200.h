/**
 * @file TVME200.h
 *
 * @brief TEWS TVME200 registers definitions
 *
 * @author Nicolas Serafini, EIC2 SA
 *
 * @date Created on 21/01/2009
 *
 * All registers definitions of the TVME200 and VIPC626 carrier.
 *
 * @version $Id: TVME200.h,v 1.1 2009/12/01 14:13:03 lewis Exp dcobas $
 */

#ifndef _TVME200_H_
#define _TVME200_H_

/*============================================================================*/
/* Names                                                                      */
/*============================================================================*/
/** Long name of the TVME200 carrier board */
#define TVME200_LONGNAME "TEWS Technologies - VME TVME200 Carrier"
/** Short name of the TVME200 carrier board */
#define TVME200_SHORTNAME "TVME200"

/** Long name of the VIPC626 carrier board */
#define VIPC626_LONGNAME "SBS Technologies - VME VIPC626 Carrier"
/** Short name of the VIPC626 carrier board */
#define VIPC626_SHORTNAME "VIPC626"

/*============================================================================*/
/* Carrier informations                                                       */
/*============================================================================*/
/** Number of Slot of the carrier board */
#define TVME200_NB_SLOT               0x4

/*============================================================================*/
/* IP IO/ID/INT Space Definitions                                             */
/*============================================================================*/
/** TVME200 IO space base offset */
#define TVME200_IO_SPACE_OFF          0x0000
/** TVME200 IO space IP gap */
#define TVME200_IO_SPACE_GAP          0x0100
/** TVME200 IO space size per IP */
#define TVME200_IO_SPACE_SIZE         0x0080
/** TVME200 ID space base offset */
#define TVME200_ID_SPACE_OFF          0x0080
/** TVME200 ID space IP gap */
#define TVME200_ID_SPACE_GAP          0x0100
/** TVME200 ID space size per IP */
#define TVME200_ID_SPACE_SIZE         0x0080
/** TVME200 INT space base offset */
#define TVME200_INT_SPACE_OFF         0x00C0
/** TVME200 INT space IP gap */
#define TVME200_INT_SPACE_GAP         0x0100
/** TVME200 INT space size per IP */
#define TVME200_INT_SPACE_SIZE        0x0040

/** TVME200 IO ID and INT space size */
#define TVME200_IOIDINT_SIZE          0x0400

/*============================================================================*/
/* MEM Space Definitions                                                      */
/*============================================================================*/
/** TVME200 128k Total Carrier MEM size */
#define TVME200_MEM_128k            0x00020000
/** TVME200 Slot MEM Gap for 128k Total Carrier MEM size */
#define TVME200_MEM_128k_SLOT_GAP   0x00008000
/** TVME200 Slot MEM size for 128k Total Carrier MEM size */
#define TVME200_MEM_128k_SLOT_SIZE  0x00008000

/** TVME200 256k Total Carrier MEM size */
#define TVME200_MEM_256k            0x00040000
/** TVME200 Slot MEM Gap for 256k Total Carrier MEM size */
#define TVME200_MEM_256k_SLOT_GAP   0x00010000
/** TVME200 Slot MEM size for 256k Total Carrier MEM size */
#define TVME200_MEM_256k_SLOT_SIZE  0x00010000

/** TVME200 512k Total Carrier MEM size */
#define TVME200_MEM_512k            0x00080000
/** TVME200 Slot MEM Gap for 512k Total Carrier MEM size */
#define TVME200_MEM_512k_SLOT_GAP   0x00020000
/** TVME200 Slot MEM size for 512k Total Carrier MEM size */
#define TVME200_MEM_512k_SLOT_SIZE  0x00020000

/** TVME200 1M Total Carrier MEM size */
#define TVME200_MEM_1M              0x00100000
/** TVME200 Slot MEM Gap for 1M Total Carrier MEM size */
#define TVME200_MEM_1M_SLOT_GAP     0x00040000
/** TVME200 Slot MEM size for 1M Total Carrier MEM size */
#define TVME200_MEM_1M_SLOT_SIZE    0x00040000

/** TVME200 2M Total Carrier MEM size */
#define TVME200_MEM_2M              0x00200000
/** TVME200 Slot MEM Gap for 2M Total Carrier MEM size */
#define TVME200_MEM_2M_SLOT_GAP     0x00080000
/** TVME200 Slot MEM size for 2M Total Carrier MEM size */
#define TVME200_MEM_2M_SLOT_SIZE    0x00080000

/** TVME200 4M Total Carrier MEM size */
#define TVME200_MEM_4M              0x00400000
/** TVME200 Slot MEM Gap for 4M Total Carrier MEM size */
#define TVME200_MEM_4M_SLOT_GAP     0x00100000
/** TVME200 Slot MEM size for 4M Total Carrier MEM size */
#define TVME200_MEM_4M_SLOT_SIZE    0x00100000

/** TVME200 8M Total Carrier MEM size */
#define TVME200_MEM_8M              0x00800000
/** TVME200 Slot MEM Gap for 8M Total Carrier MEM size */
#define TVME200_MEM_8M_SLOT_GAP     0x00200000
/** TVME200 Slot MEM size for 8M Total Carrier MEM size */
#define TVME200_MEM_8M_SLOT_SIZE    0x00200000

/** TVME200 32M Total Carrier MEM size */
#define TVME200_MEM_32M             0x02000000
/** TVME200 Slot MEM Gap for 32M Total Carrier MEM size */
#define TVME200_MEM_32M_SLOT_GAP    0x00800000
/** TVME200 Slot MEM size for 32M Total Carrier MEM size */
#define TVME200_MEM_32M_SLOT_SIZE   0x00800000

#endif /* _TVME200_H_ */
