/*
 * CVORB_hw.h
 *
 * CVORB hardware description
 */

#ifndef _CVORB_HW_H_
#define _CVORB_HW_H_

#ifndef BIT
#define BIT(nr)		(1UL << (nr))
#endif

/*
 * VME configuration
 */

#define CVORB_ADDRESS_MODIFIER 	0x29
#define CVORB_WINDOW_LENGTH	0X400
#define CVORB_DATA_SIZE		4

/*
 * SubModule's offset
 */
#define CVORB_SUBMODULE_OFFSET	0x200
#define CVORB_SUBMODULES	2
/* Number of channels per submodule. */
#define CVORB_CHANNELS		8

/**********************************************************************************/
/**
 * CVORB board registers common to the 2 submodules
 * All the registers in this card are 32-bits long
 */

/*
 * VHDL Version (R)
 * Encoded in BCD -- Upper 16 bits are not used
 * |15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
 * |   Tens    |   Units   |   Tenth   |Hundredths |
 */
#define CVORB_VERSION			0x010
#define CVORB_VERSION_FW_SHIFT		16
#define CVORB_VERSION_FW_MASK		0xffff
#define CVORB_VERSION_TENS_SHIFT	12
#define CVORB_VERSION_UNITS_SHIFT	8
#define CVORB_VERSION_TENTHS_SHIFT	4
#define CVORB_VERSION_HUNDREDTHS_SHIFT	0
#define CVORB_VERSION_AS_STRING_SZ      8

/*
 * PCB serial number MSBs (R)
 * Each board has a unique ID, given by a 64-bit serial number
 */
#define CVORB_PCB_MSB	0x014

/*
 * PCB serial number LSBs (R)
 */
#define CVORB_PCB_LSB	0x018

/*
 * Temperature (R)
 * Format: 12 bits signed fixed point (bits 0-10)
 * Range: -55 to 125 [C]
 * Bits 11-31: sign --> they all have the same value
 */
#define CVORB_TEMP	0x01C

/**********************************************************************************/
/**
 * CVORB registers per submodule (0x000-0x1ff per submodule)
 * All the registers in this card are 32-bits long
 */

/*
 * CVORB Software Control (W)
 * Note that only one bit can be written at a time on this register
 */
#define CVORB_SUBMODULE_CTL	0x024
#define CVORB_SUBMODULE_RESET	BIT(0) 	/* Submodule software reset */
#define CVORB_FPGA_RESET	BIT(3) 	/* FPGA hardware reset: reset both submodules in one goal*/
#define CVORB_SUBMODULE_START	BIT(4) 	/* Simulate submodule Start trigger: start execution of a function */
#define CVORB_SUBMODULE_EVSTART	BIT(5) 	/* Simulate submodule event Start trigger : allows to continue the execution of a function suspended by a stop event*/
#define CVORB_SUBMODULE_STOP	BIT(6) 	/* Simulate submodule Stop trigger: allows to suspend the execution of a function */
#define CVORB_SUBMODULE_EVSTOP	BIT(7) 	/* Simulate submodule event Stop trigger: ends execution of a function */

/*
 * chunk of 4 bytes not used: from 0x0030 to 0x003c
 */

/*
 * Status (R)
 */
#define CVORB_SUBMODULE_STATUS	        0x040
#define CVORB_SUBMODULE_READY	        BIT(0) 	/* Ready to run (on 0) */
#define CVORB_SUBMODULE_SRAM_BUSY	BIT(1) 	/* SRAM data write in progress: raised don't write to SRAM register */
#define CVORB_SUBMODULE_BUSY	        BIT(2) 	/* waveform is being played: "logical or" of all channels of the module */
#define CVORB_SUBMODULE_IDLED	        BIT(3) 	/* no waveform being executed: "logical or" of all channels of the module */
/* remaining bits not used */

/*
 * Configuration (R/W)
 */
#define CVORB_SUBMODULE_CONFIG	                0x044
/*bit 6: input pulse polarity: 0 positive pulses, 1 negative */
#define CVORB_SUBMODULE_INPOL_SHIFT             6
#define CVORB_SUBMODULE_INPOL_MASK		(1<<CVORB_SUBMODULE_INPOL_SHIFT)
/* bit7-8-9: allows to specify which channel is connected to the DAC output */
#define CVORB_SUBMODULE_DAC_SEL_SHIFT 	        7
#define CVORB_SUBMODULE_DAC_SEL_MASK            (0x7<<CVORB_SUBMODULE_DAC_SEL_SHIFT)
/* bit10-11-12: allows to specify which channel is connected to the Optical output */
#define CVORB_SUBMODULE_OPTICAL_SEL_SHIFT 	10
#define CVORB_SUBMODULE_OPTICAL_SEL_MASK        (0x7<<CVORB_SUBMODULE_OPTICAL_SEL_SHIFT)
/* bit13-14-15: allows to specify which channel is connected to the Front panel led */
#define CVORB_SUBMODULE_LEDS_SEL_SHIFT 	        13
#define CVORB_SUBMODULE_LEDS_SEL_MASK           (0x7<<CVORB_SUBMODULE_LEDS_SEL_SHIFT)
/* bit 25: Enable optical output */
#define CVORB_SUBMODULE_OPT_EN_SHIFT		25
#define CVORB_SUBMODULE_OPT_EN_MASK             (1<<CVORB_SUBMODULE_OPT_EN_SHIFT)

/*
 * SRAM start address (R/W)
 * This register is a pointer to the external SRAM. The pointer auto-increments
 * on read or write of SRAMDATA. External SRAM is 512k x 32 bits.
 */
#define CVORB_SUBMODULE_SRAM_ADDR       0x04c   /* Byte addresses must be written into this reg */
/* SRAM address is computed by applying to channel nr and function nr respective shift*/
#define CVORB_SUBMODULE_SRAM_CHAN_SHIFT 18
#define CVORB_SUBMODULE_SRAM_FUNC_SHIFT 12

/*
 * SRAM data (R/W)
 * Used to read/write functions from/to SRAM
 * Note. SRAM cannot be directly mapped
 */
#define CVORB_SUBMODULE_SRAM_DATA	0x050

/*
 * DAC Control (R/W)
 * SPI interface to AD9707
 */
#define CVORB_DACCTL		0x060
#define CVORB_DACCTL_DATA 	(0xff<<0)	/* DAC register data */
#define CVORB_DACCTL_ADDR	(0x1ff<<8) 	/* DAC register address */
/* bits 13-14: do not care about them */
#define CVORB_DACCTL_RW		BIT(15) 	/* DAC register read/write (1/0) */
#define CVORB_DACCTL_UP2DATE	BIT(16) 	/* set when data in DACCTL are updated; cleared on SPI read */
/* bits 17-31: not used */

/**********************************************************************************/
/**
 * CVORB registers per channel
 * address of a channel: board address + CVORB_CHAN_STATUS + CVORB_CHAN_SIZE * chan number
 * where 0 <= chan number <= 7
 * All the registers in this card are 32-bits long
 */
#define CVORB_CH_SIZE           0x030

/*
 * Channel status
 */
#define CVORB_CH_STATUS         0x080

/*
 * Channel config
 */
#define CVORB_CH_CONFIG         0x084
#define CVORB_CH_ENABLE         0x2
#define CVORB_CH_DISABLE        0x1
#define CVORB_CH_ENABLE_MASK    0x3

/*
 * Channel Select Function
 */
#define CVORB_CH_SELECT_FCN     0x088

/*
 * Channel Function enable mask
 */
#define CVORB_CH_ENABLE_FCN_MASK_H      0x08C
#define CVORB_CH_ENABLE_FCN_MASK_L      0x090
#define CVORB_FCN_MAX_BYTE_SIZE         0x01000

/*
 * Channel Function occurrences used to specify how many times a function will be generated
 * the default value is 1.
 */
#define CVORB_CH_REPEAT_FCN     0x98

#endif /* _CVORB_HW_H_ */
