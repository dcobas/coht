/**
 * @file CvorbDefs.h
 *
 * @brief Definitions, common for library and driver.
 *
 * @author Copyright (C) 2009 CERN. Yury GEORGIEVSKIY <ygeorgie@cern.ch>
 *
 * @date Created on 14/10/2009
 *
 * @section license_sec License
 *          Released under the GPL
 */

#ifndef _CVORB_DEFINITIONS_H_INCLUDE_
#define _CVORB_DEFINITIONS_H_INCLUDE_

#include <CvorbDrvr.h> /* for CVORB_FIRST_USR_IOCTL */

/** @defgroup cvorb_ioctl User-defined ioctl calls
 *@{
 */
/**< Read-only hardware VHDL version */
#define CVORB_VHDL       (CVORB_FIRST_USR_IOCTL + 0)

/**< Read-only Printed Curcuit Board number */
#define CVORB_PCB        (CVORB_FIRST_USR_IOCTL + 1)

/**< Module Temperature in Celsius */
#define CVORB_TEMP       (CVORB_FIRST_USR_IOCTL + 2)
/**< Read-only module status reg */
#define CVORB_MOD_STAT   (CVORB_FIRST_USR_IOCTL + 3)

/**< Read module config reg */
#define CVORB_MOD_CFG_RD (CVORB_FIRST_USR_IOCTL + 4)

/**< Write module config reg */
#define CVORB_MOD_CFG_WR (CVORB_FIRST_USR_IOCTL + 5)

/**< Read-only channel status reg */
#define CVORB_CH_STAT    (CVORB_FIRST_USR_IOCTL + 6)

/**< Read channel config reg */
#define CVORB_CH_CFG_RD  (CVORB_FIRST_USR_IOCTL + 7)

/**< Write channel config reg */
#define CVORB_CH_CFG_WR  (CVORB_FIRST_USR_IOCTL + 8)

/**<  */
#define CVORB_LOAD_SRAM  (CVORB_FIRST_USR_IOCTL + 9)

/**<  */
#define CVORB_READ_SRAM  (CVORB_FIRST_USR_IOCTL + 10)

/**< enable function */
#define CVORB_FEN        (CVORB_FIRST_USR_IOCTL + 11)

/**< disable function */
#define CVORB_FDIS       (CVORB_FIRST_USR_IOCTL + 12)

/**< Read enable function mask */
#define CVORB_FEN_RD     (CVORB_FIRST_USR_IOCTL + 13)

/**< Write enable function mask */
#define CVORB_FEN_WR     (CVORB_FIRST_USR_IOCTL + 14)

/**< select function */
#define CVORB_FUNC_SEL   (CVORB_FIRST_USR_IOCTL + 15)

/**< get currently selected function */
#define CVORB_FUNC_GET   (CVORB_FIRST_USR_IOCTL + 16)

/**< write into software pulses register. \n
   Allows to simulate front panel pulse inputs */
#define CVORB_WRSWP      (CVORB_FIRST_USR_IOCTL + 17)

/**< read Recurrent Cycles register */
#define CVORB_RC_RD      (CVORB_FIRST_USR_IOCTL + 18)

/**< write Recurrent Cycles register */
#define CVORB_RC_WR      (CVORB_FIRST_USR_IOCTL + 19)
/*@} end of group*/


#define MAX_F_VECT 679 /**< MAX function vectors amount */
#define CHAM 8 /**< submodule channel amount */
#define SMAM 2 /**< number of submodules */
#define FAM 64 /**< function amount per one channel */
#define FSZ 2048 /**< function size in shorts */
#define MIN_STEP 5 /**< function minimum step size is 5 us */
#define MTMS ((1<<16)*MIN_STEP) /**< MAX possible time between 2 vectors with
				   min step size (327680 us)  */


/** @defgroup cvorb_sw_pulses Software Pulses register bits
 *
 * Bits to provide as a paramter to @ref cvorb_swp() function \n
 * Only one bit at a time can be set. \n
 * Software Pulses register is used to simulate front panel pulse inputs.
 *@{
 */
#define SPR_MSR    (1<<0) /**< Module software reset */
#define SPR_FGR    (1<<3) /**< FPGA reset (global reset) */
#define SPR_MSS	   (1<<4) /**< Module Software Start */
#define SPR_MSES   (1<<5) /**< Module Software event start */
#define SPR_MSSTP  (1<<6) /**< Module Software stop */
#define SPR_MSESTP (1<<7) /**< Module Software event stop */
#define SPR_MASK   249	  /**< bit mask */
/*@} end of group*/


/** @defgroup cvorb_reglayout CVORB Registers Layout
 *
 * Some bitfield registers are described here
 *@{
 */
/**
 * @brief Function Vectors
 *
 * * This structure is used to load or read current function to/from the module
 *   by cvorb_func_load() and cvorb_func_read()\n
 * * An array of Function Vectors is passed between the user and kernel
 *   space.\n
 * * When writing a new vector (using cvorb_func_load()) -- Element[0]
 *   should @b ALWAYS have t == 0 \n
 * * Array should not be bigger then @ref MAX_F_VECT + 1 (for Element[0])
 *   elements long \n
 */
struct fv {
	uint   t; /**< values of time from function start in microseconds */
	ushort v; /**< amplitudes in corresponding physical unit */
};

/** @brief Channel Configuration register data */
struct ccr {
	ushort chm; /**< Channel mode selection \n
		       0 -- OFF \n
		       1 -- ON \n
		       4 -- Test#1 (Triangle function) \n
		       5 -- Test#2 (Constant value [bits 16 to 31]) */
	ushort slope_en; /**< Slope enable flag
			    (1 == enable slope generation) */
	ushort so_dis;   /**< Disable serial output (0 == enable) */
	ushort cov;	 /**< Constant output value
			    (similar to DAC mode if GFAS) */
};

/** @brief Module Configuratioin register data */
struct mcr {
	ushort ms; /**< Mode selection \n
		      0 -- OFF \n
		      1 -- normal mode */
	ushort ipp; /**< Input Pulses Polarity \n
		       0 -- positive pulses \n
		       1 -- negative pulses */
	ushort dss; /**< On board DAC source selection \n
		       [0 - 7] -- Vector generator[0 - 7] */
	ushort oop; /**< Optical Output selection \n
		       [0 - 7] -- Vector generator[0 - 7] */
	ushort fplss; /**< Front panel LED source selection \n
			 [0 - 7] -- Channel[1 - 7] */
	ushort eoo; /**< Enable Optical Output */
};

/** @brief Module Status register data */
struct mstat {
	char ready; /**< 0 == Module is not ready \n
		       1 == Initialization finished */
	char wrip;  /**< 1 == SRAM data write in progress \n
		       (if [1] -- don't write to SRAM Data register) */
	char mb;   /**< Module is busy generating a function \n
		      This bit is an OR between all generators of the module */
	char mp;   /**< Module function paused \n
		      This bit is an OR between all generators of the module */
	char merr; /**< Module in error (One of the differential output is in
		      fault) */
};

/** @brief Channel Status register data */
struct chstat {
	char chb;  /**< Channel busy generating a function */
	char fp;  /**< Function paused */
	char err; /**< Error occurred */
	char ws;  /**< Waiting start (state machine waiting for a start
		     pulse) */
	char slf; /**< Serial link in fault */
	char chsm; /**< Channel state machine \n
		      0  -- Idle \n
		      1  -- Load internal RAM \n
		      2  -- Wait start \n
		      3  -- Init counters \n
		      4  -- Enable generation \n
		      5  -- Wait stop \n
		      6  -- Event stop \n
		      7  -- Internal stop \n
		      8  -- Load next vector \n
		      9  -- Reload internal RAM (internal stop) \n
		      10 -- Wait event start (internal stop) \n
		      11 -- Write current vector (internal stop) */
	char fcp; /**< Function copy in progress \n
		     1 == copying function into local RAM, selected function for
		     this generator @b MUST @b NOT be written! */
};
/*@} end of group*/

#endif /* _CVORB_DEFINITIONS_H_INCLUDE_ */
