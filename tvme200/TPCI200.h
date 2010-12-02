/**
 * @file TPCI200.h
 *
 * @brief TEWS TPCI200 registers definitions
 *
 * @author Nicolas Serafini, EIC2 SA
 *
 * @date Created on 21/01/2009
 *
 * All registers definitions of the TPCI200 carrier.
 *
 * @version $Id: TPCI200.h,v 1.1 2009/12/01 14:13:03 lewis Exp dcobas $
 */

#ifndef _TPCI200_H_
#define _TPCI200_H_

/*============================================================================*/
/* Names                                                                      */
/*============================================================================*/
/** Long name of the carrier board */
#define TPCI200_LONGNAME "TEWS Technologies - PCI TPCI200 Carrier"
/** Short name of the carrier board */
#define TPCI200_SHORTNAME "TPCI200"

/*============================================================================*/
/* Carrier informations                                                       */
/*============================================================================*/
/** Number of Slot of the carrier board */
#define TPCI200_NB_SLOT               0x4
/** Number of Bar of the carrier board */
#define TPCI200_NB_BAR                0x6

/** TPCI200 vendor ID */
#define TPCI200_VENDOR_ID             0x1498
/** TPCI200 device ID */
#define TPCI200_DEVICE_ID             0x30C8
/** TPCI200  SubVendor ID */
#define TPCI200_SUBVENDOR_ID          0x1498
/** TPCI200 SubDevice ID */
#define TPCI200_SUBDEVICE_ID          0x300A

/** IP interface registers BAR */
#define TPCI200_IP_INTERFACE_BAR      2
/** Slot A-D IO ID INT spaces BAR */
#define TPCI200_IO_ID_INT_SPACES_BAR  3
/** Slot A-D Memory space (16bit) BAR */
#define TPCI200_MEM16_SPACE_BAR       4
/** Slot A-D Memory space (8bit) BAR */
#define TPCI200_MEM8_SPACE_BAR        5

/*============================================================================*/
/* IP Interface Register Definitions                                          */
/*============================================================================*/
/** TPCI200 Revision register offset */
#define TPCI200_REVISION_REG          0x00
/** TPCI200 IP A control register offset */
#define TPCI200_CONTROL_A_REG         0x02
/** TPCI200 IP B control register offset */
#define TPCI200_CONTROL_B_REG         0x04
/** TPCI200 IP C control register offset */
#define TPCI200_CONTROL_C_REG         0x06
/** TPCI200 IP D control register offset */
#define TPCI200_CONTROL_D_REG         0x08
/** TPCI200 reset register offset */
#define TPCI200_RESET_REG             0x0A
/** TPCI200 status register offset */
#define TPCI200_STATUS_REG            0x0C

/** TPCI200 interface register size */
#define TPCI200_IFACE_SIZE            0x100

/*============================================================================*/
/* IP IO/ID/INT Space Definitions                                             */
/*============================================================================*/
/** TPCI200 IO space base offset */
#define TPCI200_IO_SPACE_OFF          0x0000
/** TPCI200 IO space IP gap */
#define TPCI200_IO_SPACE_GAP          0x0100
/** TPCI200 IO space size per IP */
#define TPCI200_IO_SPACE_SIZE         0x0080
/** TPCI200 ID space base offset */
#define TPCI200_ID_SPACE_OFF          0x0080
/** TPCI200 ID space IP gap */
#define TPCI200_ID_SPACE_GAP          0x0100
/** TPCI200 ID space size per IP */
#define TPCI200_ID_SPACE_SIZE         0x0040
/** TPCI200 INT space base offset */
#define TPCI200_INT_SPACE_OFF         0x00C0
/** TPCI200 INT space IP gap */
#define TPCI200_INT_SPACE_GAP         0x0100
/** TPCI200 INT space size per IP */
#define TPCI200_INT_SPACE_SIZE        0x0040

/** TPCI200 IO ID and INT space size */
#define TPCI200_IOIDINT_SIZE          0x0400

/*============================================================================*/
/* IP MEM Space Definitions                                                   */
/*============================================================================*/
/** TPCI200 8bit Mem gap per IP */
#define TPCI200_MEM8_GAP              0x00400000
/** TPCI200 8bit Mem size per IP */
#define TPCI200_MEM8_SIZE             0x00400000
/** TPCI200 16bit Mem gap per IP */
#define TPCI200_MEM16_GAP             0x00800000
/** TPCI200 16bit Mem size per IP */
#define TPCI200_MEM16_SIZE            0x00800000

/*============================================================================*/
/* IP Interface Control Register                                              */
/*============================================================================*/
/** INT0 enabled */
#define TPCI200_INT0_EN               0x0040
/** INT1 enabled */
#define TPCI200_INT1_EN               0x0080
/** INT0 edge sensitive */
#define TPCI200_INT0_EDGE             0x0010
/** INT1 edge sensitive */
#define TPCI200_INT1_EDGE             0x0020
/** ERR INT enabled */
#define TPCI200_ERR_INT_EN            0x0008
/** TIMEOUT INT enabled */
#define TPCI200_TIME_INT_EN           0x0004
/** IP recover tme enabled */
#define TPCI200_RECOVER_EN            0x0002
/** 32 MHz clock */
#define TPCI200_CLK32                 0x0001

/*============================================================================*/
/* IP Interface Reset Register                                                */
/*============================================================================*/
/** Assert IP RESET# signal at slot A*/
#define TPCI200_A_RESET               0x0001
/** Assert IP RESET# signal at slot B*/
#define TPCI200_B_RESET               0x0002
/** Assert IP RESET# signal at slot C*/
#define TPCI200_C_RESET               0x0004
/** Assert IP RESET# signal at slot D */
#define TPCI200_D_RESET               0x0008

/*============================================================================*/
/* IP Interface Status Register                                               */
/*============================================================================*/
/** IP A read: timeout has occured, write: clear timeout status */
#define TPCI200_A_TIMEOUT             0x1000
/** IP B read: timeout has occured, write: clear timeout status */
#define TPCI200_B_TIMEOUT             0x2000
/** IP C read: timeout has occured, write: clear timeout status */
#define TPCI200_C_TIMEOUT             0x4000
/** IP D read: timeout has occured, write: clear timeout status */
#define TPCI200_D_TIMEOUT             0x8000

/** IP A ERROR# signal asserted */
#define TPCI200_A_ERROR               0x0100
/** IP B ERROR# signal asserted */
#define TPCI200_B_ERROR               0x0200
/** IP C ERROR# signal asserted */
#define TPCI200_C_ERROR               0x0400
/** IP D ERROR# signal asserted */
#define TPCI200_D_ERROR               0x0800

/** IP A read: INT0 interrupt request, write: clear INT0 status */
#define TPCI200_A_INT0                0x0001
/** IP A read: INT1 interrupt request, write: clear INT1 status */
#define TPCI200_A_INT1                0x0002
/** IP B read: INT0 interrupt request, write: clear INT0 status */
#define TPCI200_B_INT0                0x0004
/** IP B read: INT1 interrupt request, write: clear INT1 status */
#define TPCI200_B_INT1                0x0008
/** IP C read: INT0 interrupt request, write: clear INT0 status */
#define TPCI200_C_INT0                0x0010
/** IP C read: INT1 interrupt request, write: clear INT1 status */
#define TPCI200_C_INT1                0x0020
/** IP D read: INT0 interrupt request, write: clear INT0 status */
#define TPCI200_D_INT0                0x0040
/** IP D read: INT1 interrupt request, write: clear INT1 status */
#define TPCI200_D_INT1                0x0080

/** Mask for all slot interrupt */
#define TPCI200_SLOT_INT_MASK         0x00FF

#endif /* _TPCI200_H_ */
