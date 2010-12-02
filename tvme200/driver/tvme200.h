/*
 * tvme200.h
 *
 * driver for the carrier TEWS TVME-200 
 * Copyright (c) 2009 Nicolas Serafini, EIC2 SA
 * Copyright (c) 2010,2011 Samuel Iglesias Gonsalvez <siglesia@cern.ch>
 *
 * Released under the GPL v2. (and only v2, not any later version)
 */
#ifndef _TVME200_H_
#define _TVME200_H_

#define TVME200_LONGNAME "TEWS Technologies - VME TVME200 Carrier"
#define TVME200_SHORTNAME "TVME200"
#define VIPC626_LONGNAME "SBS Technologies - VME VIPC626 Carrier"
#define VIPC626_SHORTNAME "VIPC626"

#define TVME200_NB_SLOT               0x4
#define TVME200_IO_SPACE_OFF          0x0000
#define TVME200_IO_SPACE_GAP          0x0100
#define TVME200_IO_SPACE_SIZE         0x0080
#define TVME200_ID_SPACE_OFF          0x0080
#define TVME200_ID_SPACE_GAP          0x0100
#define TVME200_ID_SPACE_SIZE         0x0080
#define TVME200_INT_SPACE_OFF         0x00C0
#define TVME200_INT_SPACE_GAP         0x0100
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

#endif 
