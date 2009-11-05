/**
 * @file CvorbDrvrDefs.h
 *
 * @brief Definitions and structures used by the driver/simulator and library
 *
 * They are not exported to the user library header file though
 *
 * @author Copyright (C) 2009 CERN. Yury GEORGIEVSKIY <ygeorgie@cern.ch>
 *
 * @date Created on 27/10/2009
 *
 * @section license_sec License
 *          Released under the GPL
 */
#ifndef _CVORB_DRVR_DEFS_H_INCLUDE_
#define _CVORB_DRVR_DEFS_H_INCLUDE_

#include <CvorbDefs.h>

/** @defgroup cvorbsram SRAM organization
 *@{
 */
#define SRAM_CHAN_SHIFT 18
#define SRAM_FUNC_SHIFT 12
#define SRAM_ADDR_SHIFT 0

#define SRAM_CHAN_MASK (7<<18)
#define SRAM_FUNC_MASK (63<<12)
#define SRAM_ADDR_MASK (2047<<0)

/* SRAM start address offset for each channel */
static int _sram_ch[] __attribute__ ((unused)) = {
	0x0,
	0x040000,
	0x080000,
	0x0c0000,
	0x100000,
	0x140000,
	0x180000,
	0x1c0000
};

enum {
	SRAM_CH1 = 0x0,
	SRAM_CH2 = 0x040000,
	SRAM_CH3 = 0x080000,
	SRAM_CH4 = 0x0c0000,
	SRAM_CH5 = 0x100000,
	SRAM_CH6 = 0x140000,
	SRAM_CH7 = 0x180000,
	SRAM_CH8 = 0x1c0000
};
/*@} end of group*/

/* channel offsets from the Submodule Base Address */
static int _ch_offset[] __attribute__((unused)) = {
	0x80,
	0xb0,
	0xe0,
	0x110,
	0x140,
	0x170,
	0x1a0,
	0x1d0
};

/* CVORB_LOAD_SRAM && CVORB_GET_SRAM ioctl parameters */
struct sram_params {
	ushort module;	/**< sub module [1, 2] */
	ushort chan;	/**< channel [1 - 8] */
	ushort func;	/**< function [1 - 64] */
	struct fv *fv;	/**< function vectors
			   fv[0] == [t0 == 0, V0] -- in case of write
			   fv[0] == [number of vectors(including this one), v0]
			            -- in case of read */
	ushort am;	/**< in case of write -- number of vectors
			   (excluding fv[0] == [t=0, V0]!), that is
			   not a real vector. 679 max

			   in case of read -- actual fv capacity,
			   i.e. how many vectors can be stored in it.
			   Special case (-1) is possible in case of read.
			   Only Vector Amount will be returned to the user in
			   fv[0].t
			   All other fields are invalid in this case */
};


#endif /* _CVORB_DRVR_DEFS_H_INCLUDE_ */
