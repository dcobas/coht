/**
 * tpci200.h
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

#ifndef _TPCI200_H_
#define _TPCI200_H_

#define TPCI200_LONGNAME "TEWS Technologies - PCI TPCI200 Carrier"
#define TPCI200_SHORTNAME "TPCI200"

#define TPCI200_NB_SLOT               0x4
#define TPCI200_NB_BAR                0x6

#define TPCI200_VENDOR_ID             0x1498
#define TPCI200_DEVICE_ID             0x30C8
#define TPCI200_SUBVENDOR_ID          0x1498
#define TPCI200_SUBDEVICE_ID          0x300A

#define TPCI200_IP_INTERFACE_BAR      2
#define TPCI200_IO_ID_INT_SPACES_BAR  3
#define TPCI200_MEM16_SPACE_BAR       4
#define TPCI200_MEM8_SPACE_BAR        5

#define TPCI200_REVISION_REG          0x00
#define TPCI200_CONTROL_A_REG         0x02
#define TPCI200_CONTROL_B_REG         0x04
#define TPCI200_CONTROL_C_REG         0x06
#define TPCI200_CONTROL_D_REG         0x08
#define TPCI200_RESET_REG             0x0A
#define TPCI200_STATUS_REG            0x0C

#define TPCI200_IFACE_SIZE            0x100

#define TPCI200_IO_SPACE_OFF          0x0000
#define TPCI200_IO_SPACE_GAP          0x0100
#define TPCI200_IO_SPACE_SIZE         0x0080
#define TPCI200_ID_SPACE_OFF          0x0080
#define TPCI200_ID_SPACE_GAP          0x0100
#define TPCI200_ID_SPACE_SIZE         0x0040
#define TPCI200_INT_SPACE_OFF         0x00C0
#define TPCI200_INT_SPACE_GAP         0x0100
#define TPCI200_INT_SPACE_SIZE        0x0040
#define TPCI200_IOIDINT_SIZE          0x0400

#define TPCI200_MEM8_GAP              0x00400000
#define TPCI200_MEM8_SIZE             0x00400000
#define TPCI200_MEM16_GAP             0x00800000
#define TPCI200_MEM16_SIZE            0x00800000

#define TPCI200_INT0_EN               0x0040
#define TPCI200_INT1_EN               0x0080
#define TPCI200_INT0_EDGE             0x0010
#define TPCI200_INT1_EDGE             0x0020
#define TPCI200_ERR_INT_EN            0x0008
#define TPCI200_TIME_INT_EN           0x0004
#define TPCI200_RECOVER_EN            0x0002
#define TPCI200_CLK32                 0x0001

#define TPCI200_A_RESET               0x0001
#define TPCI200_B_RESET               0x0002
#define TPCI200_C_RESET               0x0004
#define TPCI200_D_RESET               0x0008

#define TPCI200_A_TIMEOUT             0x1000
#define TPCI200_B_TIMEOUT             0x2000
#define TPCI200_C_TIMEOUT             0x4000
#define TPCI200_D_TIMEOUT             0x8000

#define TPCI200_A_ERROR               0x0100
#define TPCI200_B_ERROR               0x0200
#define TPCI200_C_ERROR               0x0400
#define TPCI200_D_ERROR               0x0800

#define TPCI200_A_INT0                0x0001
#define TPCI200_A_INT1                0x0002
#define TPCI200_B_INT0                0x0004
#define TPCI200_B_INT1                0x0008
#define TPCI200_C_INT0                0x0010
#define TPCI200_C_INT1                0x0020
#define TPCI200_D_INT0                0x0040
#define TPCI200_D_INT1                0x0080

#define TPCI200_SLOT_INT_MASK         0x00FF

#endif /* _TPCI200_H_ */
