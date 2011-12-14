/**
 * @file CvorbUserDefinedAccess.c
 *
 * @brief CVORB library
 *
 * @mainpage CVORB Library Index Page

 * Current API was derived from the old interface for GFAS module, which is an
 * ancestor of CVORB.
 * Extra funcions were added to exploit new features, that were not available
 * in the old HW.
 *
 * @author Copyright (C) 2009-2010 CERN. Yury GEORGIEVSKIY <ygeorgie@cern.ch>
 *
 * @date Created on 12/10/2009
 *
 * @section license_sec License
 *          Released under the GPL
 *
 * @section extra_docs Related Documents.
 * You can find <b> Library API functions description </b>
 * @ref cvorblib "here".\n
 * You can visit <a href="http://wikis.cern.ch/display/HT/CVORB+V2+-+Arbitrary+waveform+generator"><b>CVORB homepage</b></a> for complete board details.\n
 * Hardware-specific information is not provided here as you can find it in the <a href="http://wikis.cern.ch/download/attachments/16779455/CVORB+-+technical+guide.pdf"><b>CVORB techical guide</b></a> manual.\n
 * Check @b DAL <a href="http://ab-dep-co-ht.web.cern.ch/ab-dep-co-ht/ProjectDocuments/dal/man3/dal.3.html"><b>manual pages</b></a>
 * for complete information on DAL library.
 *
 * @section intro_sec Introduction.
 *
 * * Board is made of two identical submodules. Each of them has 8 channels
 *   to play functions.
 *
 * * Library operates on the Board as a whole, i.e. it doesn't distinguishing
 *   between submodules.
 *
 * * There are situations nevertheless, when a specific submodule register
 *   should be adressed.\n
 *   In this case channel numbers are used to distinguish between submodules.\n
 *   [1 - 8]  -- to address first submodule\n
 *   [9 - 16] -- to address second submodule
 *
 * * CVORB Driver bench (including generic module libraries) was generated usind\n
 *   driver-genV2 and therefore complyes with all its rules and limitations.\n
 *   Current library -- is a user-defined (among 3 possible for the module) one.\n
 *
 * * This Library is working on top of  DAL library, so user application should
 *   be compiled\n
 *   with @b -lcvorbuda and @b -ldal libraries.\n
 *   Note, that library order is relevant during compilation.
 *
 * @section install_sec Initialization.
 *
 * Before using @b any library functions - user should call
 * cvorb_init() in order to initialize the library and get library handle
 * to manage the device. You should provide module <b>Logical Unit Number</b>
 * you want to work with. If this is a real driver -- @b LUN is positive,
 * if this is simulator -- @b LUN should be negative.
 *
 * When the user is done with the library - cvorb_exit() should
 * be called in order to gracefully free library context.
 *
 */
#include <sys/ioctl.h>
#include <stdint.h>
#include <CvorbUserDefinedAccess.h>
#include <CvorbDrvrDefs.h>
#include "ad9516o.h"

#define MAX_HNDLS 32 //!< MAX supported users

/** @brief library handle */
static struct {
	HANDLE h;	//!< DAL handle
	int    fd;	//!< driver node file descriptor
} _lh[MAX_HNDLS] = {
	[0 ... MAX_HNDLS-1] {
		.h  =  0,
		.fd = -1
	}
};

/**
 * @brief Initialize library and returns handle
 *
 * This function should be called before using any library function
 *
 * @param lun -- module LUN to open (Negative in case of simulator)
 *
 * @return lib handle that should be use in subsequent calls -- if OK
 * @return -CVORB_DAL_ERR -- DAL error. Use cvorb_perr() to get error info
 * @return -CVORB_ERR     -- no more free handles
 */
int cvorb_init(int lun)
{
	HANDLE h;
	int i, fd;

	h = DaEnableAccess("CVORB", IOCTL, lun, 0);
	if (h < 0)
		return -CVORB_DAL_ERR;

	fd = DaGetNodeFd(h);
	if (fd < 0)
		return -CVORB_DAL_ERR;

	for (i = 0; i < MAX_HNDLS; i++)
		if (_lh[i].fd == -1) {
			_lh[i].h  = h;
			_lh[i].fd = fd;
			return i+1;
		}

	/* no more free handles */
	return -CVORB_ERR;
}

/**
 * @brief Free library context.
 *
 * @param h -- handle returned by cvorb_init()
 *
 * This function should be called when user is done using the library.
 *
 * @return  0                -- SUCCESS
 * @return -CVORB_BAD_HANDLE -- bad library handle
 * @return -CVORB_DAL_ERR    -- DAL error. Use cvorb_perr() to get error info
 */
int cvorb_exit(int h)
{
	if (!WITHIN_RANGE(1, h, MAX_HNDLS))
		return -CVORB_BAD_HANDLE;

	if (DaDisableAccess(_lh[h-1].h))
		return -CVORB_DAL_ERR;

	/* free handle */
	_lh[h-1].h  =  0;
	_lh[h-1].fd = -1;
	return 0;
}

/**
 * @brief Read Module Configuration Register.
 *
 * @param h  -- handle
 * @param ch -- channel [1-8]  -- submodule#1 \n
 *                      [9-16] -- submodule#2 \n
 *              if 0 -- configuration registers of all (2) submodules
 *                      will be read
 * @param cr -- Module Configuration register value goes here
 *
 * As module consists of 2 submodules -- we need to distinguish between
 * them. \n
 * If you want to read 2 configuration registers at once -- @b cr parameter
 * should point to the massive of 2 uint. \n
 * See module config register layout and its bits to have more details.
 *
 * @return   0 -- if OK
 * @return < 0 -- if FAILED
 */
int cvorb_rd_mconfig(int h, int ch, uint *cr)
{
	if (!WITHIN_RANGE(1, h, MAX_HNDLS))
		return -CVORB_BAD_HANDLE;

	if (!WITHIN_RANGE(0, ch, MAX_CHAN_AM))
		return -CVORB_OUT_OF_RANGE;

	 /* set ioctl parameter -- submodule idx
	    if ch == 0 -- then config registers of all
	    submodules will be read */
	if (!ch)
		*cr = SMAM;
	else
		*cr = (ch > CHAM) ? 1 : 0;

	/* call the driver */
	if (ioctl(_lh[h-1].fd, CVORB_MOD_CFG_RD, cr))
		return -CVORB_IOCTL_FAILED;
	return 0;
}

/**
 * @brief Read channel Configuration Register bit settings
 *
 * @param h  -- handle
 * @param ch -- channel [1-8]  -- submodule#1 \n
 *                      [9-16] -- submodule#2
 * @param cr -- if not NULL - configuration Register bit
 *              settings goes here
 *
 * @return Module Config Register Value -- if SUCCESS
 * @return -1                           -- FAILED
 */
int cvorb_rd_mconfig_struct(int h, int ch, struct mcr *cr)
{
	uint mcr; /* Module Configuration register */
	int rc = cvorb_rd_mconfig(h, ch, &mcr);

	if (rc)
		return -1;

	if (!cr)
		return mcr;

	*cr = (struct mcr) {
		.ms    = mcr & 15, /* module selection */
		.ipp   = (mcr & (1<<6))>>6, /* input pulses polarity */
		.dss   = (mcr & (7<<7))>>7, /* DAC source selection */
		.oop   = (mcr & (7<<10))>>10, /* optical output selection */
		.fplss = (mcr & (7<<13))>>13, /* front panel LED source selection */
		.eoo   = (mcr & (1<<25))>>25  /* enable optical output */
	};

	return mcr;
}

/**
 * @brief Write Module Configuration Register.
 *
 * @param h    -- handle
 * @param ch   -- channel [1-8]  -- submodule#1 \n
 *                        [9-16] -- submodule#2
 * @param data -- data to write into register
 *
 * We need to distinguish between the submodules.
 * See module config register layout and its bits to have more details.
 *
 * @return   0 -- all OK
 * @return < 0 -- FAILED to write register
 */
int cvorb_wr_mconfig(int h, int ch, uint data)
{
	uint par[2];

	if (!WITHIN_RANGE(1, h, MAX_HNDLS))
		return -CVORB_BAD_HANDLE;

	if (!WITHIN_RANGE(1, ch, MAX_CHAN_AM))
		return -CVORB_OUT_OF_RANGE;

	 /* set ioctl parameters:
	    [0] -- module idx
	    [1] -- register data to set */
	par[0] = (ch > CHAM) ? 1 : 0;
	par[1] = data;

	/* call the driver */
	if (ioctl(_lh[h-1].fd, CVORB_MOD_CFG_WR, &par))
		return -CVORB_IOCTL_FAILED;
	return 0;
}

/**
 * @brief @b Write module Configuration Register bit settings
 *
 * @param h  -- handle
 * @param ch -- channel [1-8]  -- submodule#1 \n
 *                      [9-16] -- submodule#2
 * @param cr -- new Configuration Register settings are taken from here
 *
 * If value should not be set in the register -- user should set it to -1 \n
 * Othervise new value will be set inside the register
 *
 * @return   0 -- OK
 * @return < 0 -- FAILED
 */
int cvorb_wr_mconfig_struct(int h, int ch, struct mcr *cr)
{
	uint mcr = 0;

	if (cr->ms != (ushort)-1)
		mcr |= (cr->ms) & 15;
	if (cr->ipp != (ushort)-1)
		mcr |= ((cr->ipp) & 1) << 6;
	if (cr->dss != (ushort)-1)
		mcr |= ((cr->dss) & 7) << 7;
	if (cr->oop != (ushort)-1)
		mcr |= ((cr->oop) & 7) << 10;
	if (cr->fplss != (ushort)-1)
		mcr |= ((cr->fplss) & 7) << 13;
	if (cr->eoo != (ushort)-1)
		mcr |= ((cr->eoo) & 1) << 25;

        return cvorb_wr_mconfig(h, ch, mcr);
}

/**
 * @brief Read Channel Configuration Register.
 *
 * @param h  -- handle
 * @param ch -- channel [1-16]
 * @param cr -- current register value goes here
 *
 * See channel config register layout and its bits to have more details.
 *
 * @return   0 -- if OK
 * @return < 0 -- if FAILED
 */
int cvorb_rd_cconfig(int h, int ch, uint *cr)
{
	struct {
		ushort  m; /* submodule idx */
		ushort  c; /* channel idx */
		uint   *r; /* results goes here */
	} par;

	if (!WITHIN_RANGE(1, h, MAX_HNDLS))
		return -CVORB_BAD_HANDLE;

	if (!WITHIN_RANGE(1, ch, MAX_CHAN_AM))
		return -CVORB_OUT_OF_RANGE;

	/* init params */
	par.m = (ch > CHAM) ? 1 : 0;
	par.c = ((ch-1)%CHAM);
	par.r = cr;
	/* call the driver */
	if (ioctl(_lh[h-1].fd, CVORB_CH_CFG_RD, &par))
		return -CVORB_IOCTL_FAILED;
	return 0;
}

/**
 * @brief Read channel Configuration Register bit settings
 *
 * @param h  -- handle
 * @param ch -- channel [1-16]
 * @param cr -- Configuration Register bit settings goes here
 *
 * @return   0 -- if SUCCESS
 * @return < 0 -- FAILED
 */
int cvorb_rd_cconfig_struct(int h, int ch, struct ccr *cr)
{
	uint ccr;
	int rc = cvorb_rd_cconfig(h, ch, &ccr);

	if (rc)
		return rc;

	*cr = (struct ccr) {
		.chm      = ccr & 7,
		.slope_en = (ccr & (1<<3)) >> 3,
		.so_dis   = (ccr & (1<<4)) >> 4,
		.cov      = (ccr & (((1<<16)-1)<<16)) >> 16
	};

	return 0;
}

/**
 * @brief Write Channel Configuration Register.
 *
 * @param h    -- handle
 * @param ch   -- channel [1-16]
 * @param data -- data to write into register
 *
 * See channel config register layout and its bits to have more details.
 *
 * @return   0 -- all OK
 * @return < 0 -- FAILED to write register
 */
int cvorb_wr_cconfig(int h, int ch, uint data)
{
	struct {
		ushort m; /* module idx */
		ushort c; /* channel idx */
		uint   d; /* new data to set */
	} par;

	if (!WITHIN_RANGE(1, h, MAX_HNDLS))
		return -CVORB_BAD_HANDLE;

	if (!WITHIN_RANGE(1, ch, MAX_CHAN_AM))
		return -CVORB_OUT_OF_RANGE;

	/* init params */
	par.m = (ch > CHAM) ? 1 : 0;
	par.c = ((ch-1)%CHAM);
	par.d = data;

	/* call the driver */
	if (ioctl(_lh[h-1].fd, CVORB_CH_CFG_WR, &par))
		return -CVORB_IOCTL_FAILED;
	return 0;
}

/**
 * @brief @b Write channel Configuration Register bit settings
 *
 * @param h  -- handle
 * @param ch -- channel [1-16]
 * @param cr -- new Configuration Register settings are taken from here
 *
 * If value should not be set in the register -- user should set it to -1 \n
 * Othervise new value will be set inside the register
 *
 * @return   0 -- OK
 * @return < 0 -- FAILED
 */
int cvorb_wr_cconfig_struct(int h, int ch, struct ccr *cr)
{
	uint ccr = 0;

	if (cr->chm != (ushort)-1)
		ccr |= (cr->chm) & 7;
	if (cr->slope_en != (ushort)-1)
		ccr |= ((cr->slope_en) & 1) << 3;
	if (cr->so_dis != (ushort)-1)
		ccr |= ((cr->so_dis) & 1) << 4;
	if (cr->cov != (ushort)-1)
		ccr |= ((cr->cov) & ((1<<16)-1)) << 16;

	return cvorb_wr_cconfig(h, ch, ccr);
}

/**
 * @brief Read Module Status Register.
 *
 * @param h  -- handle
 * @param ch -- channel [1-8]  -- submodule#1 \n
 *                      [9-16] -- submodule#2
 * @param ms -- current module status register goes here
 *
 * We need to distinguish between the submodules.
 * See module status register layout and its bits to have more details.
 *
 * @return   0 -- if OK
 * @return < 0 -- if FAILED
 */
int cvorb_rd_mstat(int h, int ch, uint *ms)
{
	if (!WITHIN_RANGE(1, h, MAX_HNDLS))
		return -CVORB_BAD_HANDLE;

	if (!WITHIN_RANGE(1, ch, MAX_CHAN_AM))
		return -CVORB_OUT_OF_RANGE;

	/* set ioctl parameter -- submodule idx */
	*ms = (ch > CHAM) ? 1 : 0;

	/* call the driver */
	if (ioctl(_lh[h-1].fd, CVORB_MOD_STAT, ms))
		return -CVORB_IOCTL_FAILED;
	return 0;
}

/**
 * @brief Read module Status Register bit settings
 *
 * @param h     -- handle
 * @param ch    -- channel [1-8]  -- submodule#1 \n
 *                         [9-16] -- submodule#2
 * @param mstat -- Status Register bit settings goes here
 *
 * Each status bit, described with its own variable
 *
 * @return   0 -- if OK
 * @return < 0 -- if FAILED
 */
int cvorb_rd_mstat_struct(int h, int ch, struct mstat *mstat)
{
	uint s;
	int rc = cvorb_rd_mstat(h, ch, &s);

	if (rc)
		return rc;

	*mstat = (struct mstat) {
		.ready = s & 1,
		.wrip  = s & (1<<1),
		.mb    = s & (1<<2),
		.mp    = s & (1<<3),
		.merr  = s & (1<<4)
	};

	return 0;
}

/**
 * @brief Read Channel Status Register.
 *
 * @param h   -- handle
 * @param ch  -- channel [1-16]
 * @param chs -- current channel status register goes here
 *
 * See channel status register layout and its bits to have more details.
 *
 * @return   0 -- if OK
 * @return < 0 -- if FAILED
 */
int cvorb_rd_cstat(int h, int ch, uint *chs)
{
	struct {
		ushort  m; /* submodule idx */
		ushort  c; /* channel idx */
		uint   *r; /* results goes here */
	} par;

	if (!WITHIN_RANGE(1, h, MAX_HNDLS))
		return -CVORB_BAD_HANDLE;

	if (!WITHIN_RANGE(1, ch, MAX_CHAN_AM))
		return -CVORB_OUT_OF_RANGE;

	/* init params */
	par.m = (ch > CHAM) ? 1 : 0;
	par.c = ((ch-1)%CHAM);
	par.r = chs;

	/* call the driver */
	if (ioctl(_lh[h-1].fd, CVORB_CH_STAT, &par))
		return -CVORB_IOCTL_FAILED;
	return 0;
}

/**
 * @brief Read channel Status Register bit settings
 *
 * @param h      -- handle
 * @param ch     -- channel [1-16]
 * @param chstat -- Status Register bit settings goes here
 *
 * Each status bit, described with its own variable
 *
 * @return   0 -- if OK
 * @return < 0 -- if FAILED
 */
int cvorb_rd_cstat_struct(int h, int ch, struct chstat *chstat)
{
	uint s;
	int rc = cvorb_rd_cstat(h, ch, &s);

	if (rc)
		return rc;

	*chstat = (struct chstat) {
		.chb  = s & 1,
		.fp   = s & (1<<1),
		.err  = s & (1<<2),
		.ws   = s & (1<<3),
		.slf  = s & (1<<4),
		.chsm = s & (15<<5),
		.fcp  = s & (1<<9)
	};

	return 0;
}

/**
 * @brief Get current VHDL version of the Board.
 *
 * @param h  -- handle
 * @param vv -- VHDL version goes here
 *
 * User should provide enough space to store VHDL version string
 *
 * @return   0 -- if 0K
 * @return < 0 -- if FAILED
 */
int cvorb_rd_vhdl_vers(int h, char *vv)
{
	uint v;
	if (ioctl(_lh[h-1].fd, CVORB_VHDL, &v))
		return -CVORB_IOCTL_FAILED;

	/* bcd2ascii */
	sprintf(vv, "%hx%hx.%hx%hx",
		(v>>12)&15,
		(v>>8)&15,
		(v>>4)&15,
		(v>>0)&15);

	return 0;
}

/**
 * @brief Get "Printed Circuit Board" serial number.
 *
 * @param h  -- handle
 * @param sn -- s.n. goes here
 *
 * User should provide enough space to store serial number string
 *
 * @return   0 -- if OK
 * @return < 0 -- if FAILED
 */
int cvorb_rd_pcb_sn(int h, char *sn)
{
	uint pcb[2];
	if (ioctl(_lh[h-1].fd, CVORB_PCB, &pcb))
		return -CVORB_IOCTL_FAILED;

	sprintf(sn, "%08x%08x", pcb[0], pcb[1]);
	return 0;
}

/**
 * @brief Get Board Temperature.
 *
 * @param h -- handle
 * @param t -- temperature in Celsius [-55 - 120] goes here
 *
 * @return   0 -- if OK
 * @return < 0 -- if FAILED
 */
int cvorb_rd_temp(int h, int *t)
{
	if (ioctl(_lh[h-1].fd, CVORB_TEMP, t))
		return -CVORB_IOCTL_FAILED;
	return 0;
}

/**
 * @brief Module Software Reset.
 *
 * @param h  -- handle
 * @param ch -- channel [1-8]  -- submodule#1 \n
 *                      [9-16] -- submodule#2
 *
 * We need to distinguish between the submodules.
 *
 * @return   0 -- OK
 * @return < 0 -- FAILED
 */
int cvorb_rst_module(int h, int ch)
{
	if (!WITHIN_RANGE(1, ch, MAX_CHAN_AM))
		return -CVORB_OUT_OF_RANGE;

	return cvorb_swp(h, (ch>CHAM)?2:1, SPR_MSR);
}

/**
 * @brief FPGA reset (Global Reset).
 *
 * @param h  -- handle
 * @param ch -- channel [1-8]  -- submodule#1 \n
 *                      [9-16] -- submodule#2
 *
 * We need to distinguish between the submodules.
 *
 * @return   0 -- OK
 * @return < 0 -- FAILED
 */
int cvorb_rst_fpga(int h, int ch)
{
	if (!WITHIN_RANGE(1, ch, MAX_CHAN_AM))
                return -CVORB_OUT_OF_RANGE;

        return cvorb_swp(h, (ch>CHAM)?2:1, SPR_FGR);
}

/**
 * @brief Read Channel Function Enable Mask.
 *
 * @param h  -- handle
 * @param ch -- channel [1-16]
 * @param m  -- current function mask goes here \n
 *
 * Allows to burst-check function state (enabled/disabled)
 *
 * Each bit corresponds to a function. If the bit is set, the function
 * is enabled and is played if a start comes when selected.
 *
 * @return   0 -- OK
 * @return < 0 -- FAILED
 */
int cvorb_rd_fem(int h, int ch, unsigned long long *m)
{
	uint fem[2] = { 0 }; /* function enable bits goes here
				[0] -- bits[63-32]
				[1] -- bits[31-0] */
	struct {
		ushort m; /* submodule idx */
		ushort c; /* channel idx */
		uint *p;  /* results goes here */
	} par;

	if (!WITHIN_RANGE(1, h, MAX_HNDLS))
		return -CVORB_BAD_HANDLE;

	if (!WITHIN_RANGE(1, ch, MAX_CHAN_AM))
		return -CVORB_OUT_OF_RANGE;

	par.m = (ch > CHAM) ? 1 : 0;
	par.c = ((ch-1)%CHAM);
	par.p = fem;

	/* call the driver */
	if (ioctl(_lh[h-1].fd, CVORB_FEN_RD, &par))
		return -CVORB_IOCTL_FAILED;

	*m = 0;
	*m |= fem[0];
	*m <<= 32;
	*m |= fem[1];

	return 0;
}

/**
 * @brief @b Read Channel Function Enable bits
 *
 * @param h   -- handle
 * @param ch  -- channel [1-16]
 * @param fem -- Channel bitmaks goes here, one char for each bit \n
 *               Can be 0 or 1
 *
 * @return   0 -- OK
 * @return < 0 -- FAILED
 */
int cvorb_rd_fem_arr(int h, int ch, char fem[64])
{
	int i;
	unsigned long long data = 0;
	int rc = cvorb_rd_fem(h, ch, &data);

	if (rc)
		return rc;

	for (i = 0; i < 64; i++)
		fem[i] = (data >> i) & 1;

	return 0;
}

/**
 * @brief @b Write Channel Function Enable Mask.
 *
 * @param h    -- handle
 * @param ch   -- channel [1-16]
 * @param mask -- new function mask to set
 *
 * Allows to burst-set function state.
 *
 * Each bit corresponds to a function. If the bit is set, the function
 * is enabled and is played if a start comes when selected.
 *
 * @return   0 -- OK
 * @return < 0 -- FAILED
 */
int cvorb_wr_fem(int h, int ch, unsigned long long mask)
{
	struct {
		ushort m; /* submodule idx */
                ushort c; /* channel idx */
                unsigned long long p;  /* new value to set */
        } par;

	if (!WITHIN_RANGE(1, h, MAX_HNDLS))
                return -CVORB_BAD_HANDLE;

        if (!WITHIN_RANGE(1, ch, MAX_CHAN_AM))
                return -CVORB_OUT_OF_RANGE;

        par.m = (ch > CHAM) ? 1 : 0;
        par.c = ((ch-1)%CHAM);
        par.p = mask;

        /* call the driver */
        if (ioctl(_lh[h-1].fd, CVORB_FEN_WR, &par))
                return -CVORB_IOCTL_FAILED;

	return 0;
}

/* for testing only */
static void  __attribute__((unused)) prnt_vtf(struct sram_params *ptr)
{
	int i;

	printf("Number of vectors is %d\n", ptr->am);
	for (i = 0; i < ptr->am + 1; i++)
		printf("V%d: nos --> %d ss --> %d v --> %d\n",
		       i, ptr->fv[i].nos, ptr->fv[i].ss, ptr->fv[i].v);
}

/**
 * @brief Load Channel Function in device SRAM.
 *
 * @param h    -- handle
 * @param ch   -- channel [1-16]
 * @param func -- function to load [1-64]
 * @param fv   -- data to load \n
 *                MAX allowed vector amount in this array -- is
 *		  @ref MAX_F_VECT + 1 (which is [t0 == 0, V0]) \n
 *                Element[0] -- should be @b always [t0 ==0, V0] \n
 *                Actual element amout passed by the user in @b sz parameter
 *                can be smaller.
 * @param sz   -- how many data elements were passed by the user
 *                (including fv[0] -- which is @b always [t0==0 V0])
 *
 * See struct fv description for more details on the data types
 * Channel function can hold up to @ref MAX_F_VECT.
 * Element[0] should @b ALWAYS hold start time == 0 and
 * initial physical value. \n
 *
 * @b NOTE on rounding down time between two vectors. \n
 *
 * MIN step possible between two vectors is 5us.
 * Every 327.675ms -- step granularity is lowered by 5us.
 * I.e the finest time step resolution possible only if dt is less
 * then 327.675ms.
 * For example, in 2nd dt range [327.680 - 655.350]ms -- step will be 10us.
 * It means that dt requested should be multipe of 10.
 * I.e. if you'll request let's say dt to be 400.355ms -- it will be rounded
 * down to 400.350ms.
 *
 * In 3rd time range -- dt must be multiple of 15 and so on...
 *
 * If it is not the case, (i.e. dt is not a multiple of current step size)
 * -- time between vectors will be rounded down to be
 * divisible by step size of the current range.
 * Step size will always be a factor of dt.
 *
 *
 * @return  0                  -- OK
 * @return -CVORB_BAD_HANDLE   -- library handle is bad
 * @return -CVORB_OUT_OF_RANGE -- MAX vector amount exceeded
 * @return -CVORB_IOCTL_FAILED -- ioctl failed
 */
int cvorb_func_load(int h, int ch, int func, struct fv *fv, int sz)
{
	struct sram_params par = { 0 };
	int i;
	double lldt;
	uint64_t lldt_int;

	if (!WITHIN_RANGE(1, h, MAX_HNDLS))
		return -CVORB_BAD_HANDLE;

	if (!WITHIN_RANGE(1, ch, MAX_CHAN_AM))
		return -CVORB_OUT_OF_RANGE;

	if (!WITHIN_RANGE(1, func, FAM))
		return -CVORB_OUT_OF_RANGE;

	/* how many vectors do we have */
	if (sz > MAX_F_VECT+1/* t=0,V0 */)
		return -CVORB_OUT_OF_RANGE;

 	par.module = (ch > CHAM) ? 2 : 1;
	par.chan = ((ch-1)%CHAM)+1;
	par.func = func;
	par.am = sz-1;	/* set actual vector amount,
			   exclude first element, which is [t0 == 0, V0] */

	/* convert [struct sv] into ioctl params  */
	par.fv[0].v = fv[0].v;
	for (i = 1; i <= sz-1; i++) {
		par.fv[i].v = fv[i].v;
		if (!fv[i].t) { /* internal stop */
			par.fv[i].ss = (ushort) -1;
			continue;
		}

		if (fv[i].t <= fv[i-1].t)
			/* vector table is wrong */
			return -CVORB_OUT_OF_RANGE;

		/* convert to us in order to not lose precision */
		lldt = (fv[i].t - fv[i-1].t) * 1000;	/* delta t in us */

#if 0
		/* TODO. Should we return wit error if
		   MIN_STEP is not a factor of delta t???
		   Meanwhile averaging is done within the driver */
		if (lldt%MIN_STEP)
			printf("5us is not a factor of MIN dt!\n");
#endif
		/* TODO. Handle the MAX time between two vectors
		   (2hrs 58min 56sec 926ms 725us)
		   Vector should be split in two vectors.
		   (see 4.2 Vector definition in CVORB Tech Guide) */
		if (lldt > (uint64_t)((1<<16)-1)*((1<<15)-1)*5)
			return -CVORB_OUT_OF_RANGE;

		/* first -- round to the nearest multiple of
		   MIN_STEP (5us) size */
		lldt_int = MIN_STEP * (uint64_t)(lldt / MIN_STEP + 0.5);

		/* get minimum step size needed to hit dt range */
		par.fv[i].ss = ((lldt_int-1)/MTMS) + 1;

		/* Round delta t if needed.
		   Ensure step size (in us) is a factor of dt,
		   i.e. no remainder */
		lldt_int -= lldt_int % (par.fv[i].ss * MIN_STEP);

		/* compute && save number of steps */
		par.fv[i].nos = lldt_int/(par.fv[i].ss * MIN_STEP);
	}

	//prnt_vtf(&par); /* For testing only */

	/* call the driver */
	if (ioctl(_lh[h-1].fd, CVORB_LOAD_SRAM, &par))
		return -CVORB_IOCTL_FAILED;
	return 0;
}

/**
 * @brief Read Channel Function.
 *
 * @param h    -- handle
 * @param ch   -- channel [1-16]
 * @param func -- funcion to read [1-64]
 * @param fv   -- results goes here
 *                Array should be big enough to hold data returned
 *                MAX element amount is @ref MAX_F_VECT, but depending on
 *                actual size -- can be less.
 *                Array  is of size @b sz.
 *                Funciton returns actual number of elements (function vectors)
 * @param sz   -- how many elements array can hold
 *
 * See struct fv description for more details on the data types
 *
 * @b NOTE Time written into the function can be different from one you wil
 * read back, because of the possible rounding of dt between two vectors.
 *
 * See cvorb_func_write() for more explanation on dt rounding down issue.
 *
 *
 * @return number of function vectors (plus [t0, V0]) -- if OK
 * @return < 0                                        -- if FAILED
 */
int cvorb_func_read(int h, int ch, int func, struct fv *fv, int sz)
{
	int rc, i;
	struct sram_params par = { 0 };

	if (!WITHIN_RANGE(1, h, MAX_HNDLS))
		return -CVORB_BAD_HANDLE;

	if (!WITHIN_RANGE(1, ch, MAX_CHAN_AM))
		return -CVORB_OUT_OF_RANGE;

	if (!WITHIN_RANGE(1, func, FAM))
		return -CVORB_OUT_OF_RANGE;

	/* how many vectors do we have */
	if (sz > MAX_F_VECT+1/* t=0,V0 */)
		return -CVORB_OUT_OF_RANGE;

	par.module = (ch > CHAM) ? 2 : 1;
	par.chan = ((ch-1)%CHAM)+1;
	par.func = func;
	par.am = sz; /* array capacity */

	/* call the driver */
	if (ioctl(_lh[h-1].fd, CVORB_READ_SRAM, &par))
		return -CVORB_IOCTL_FAILED;

	rc = par.fv[0].nos; /* number of vectors in the table */

	/* convert from ioctl function vector table
	   into user function vector table */
	fv[0].t = 0;
	fv[0].v = par.fv[0].v;
	for (i = 1; i < rc; i++) {
		if (par.fv[i].ss == (ushort) -1)
			fv[i].t = 0;
		else
			fv[i].t = ((par.fv[i].nos * par.fv[i].ss * MIN_STEP)
				   / 1000.0) + fv[i-1].t;

		fv[i].v = par.fv[i].v;
	}
	return rc;
}

/**
 * @brief Enable Channel Function.
 *
 * @param h    -- handle
 * @param ch   -- channel [1-16]
 * @param func -- function to enable [0-64] \n
 *                0 -- enable all of them
 *
 * Enabling function allows it to be played as soon as it's selected
 * using cvorb_func_sel()
 *
 * @return   0 -- if OK
 * @return < 0 -- if FAILED
 */
int cvorb_func_en(int h, int ch, int func)
{
	ushort par[3]; /* ioctl params */

	if (!WITHIN_RANGE(1, h, MAX_HNDLS))
		return -CVORB_BAD_HANDLE;

	if (!WITHIN_RANGE(1, ch, MAX_CHAN_AM))
		return -CVORB_OUT_OF_RANGE;

	if (!WITHIN_RANGE(0, func, FAM))
		return -CVORB_OUT_OF_RANGE;

	/*
	  init params
	   [0] -- module idx
	   [1] -- channel idx
	   [2] -- function idx
	*/
	par[0] = (ch > CHAM) ? 1 : 0;
	par[1] = ((ch-1)%CHAM);
	par[2] = (func)?func-1:FAM+1;

	/* call the driver */
	if (ioctl(_lh[h-1].fd, CVORB_FEN, &par))
		return -CVORB_IOCTL_FAILED;

	return 0;
}

/**
 * @brief Disable Channel Function.
 *
 * @param h    -- handle
 * @param ch   -- channel [1-16]
 * @param func -- functioin to disable [0-64] \n
 *                0 -- disable all of them
 *
 * Disabling function prevents it from being playing, even if
 * it was selected using cvorb_func_sel()
 *
 * @return   0 -- if OK
 * @return < 0 -- if FAILED
 */
int cvorb_func_dis(int h, int ch, int func)
{
	ushort par[3]; /* ioctl params */

	if (!WITHIN_RANGE(1, h, MAX_HNDLS))
		return -CVORB_BAD_HANDLE;

	if (!WITHIN_RANGE(1, ch, MAX_CHAN_AM))
		return -CVORB_OUT_OF_RANGE;

	if (!WITHIN_RANGE(0, func, FAM))
		return -CVORB_OUT_OF_RANGE;

	/*
	  init params
	   [0] -- module idx
	   [1] -- channel idx
	   [2] -- function idx
	*/
	par[0] = (ch > CHAM) ? 1 : 0;
	par[1] = ((ch-1)%CHAM);
	par[2] = (func)?func-1:FAM+1;

	/* call the driver */
	if (ioctl(_lh[h-1].fd, CVORB_FDIS, &par))
		return -CVORB_IOCTL_FAILED;

	return 0;
}

/**
 * @brief Select Channel Function to play.
 *
 * @param h    -- handle
 * @param ch   -- channel [1-16]
 * @param func -- function to play [1-64]
 *
 * Set function to be played, when next start event comes.
 * Note, that it will be played only if function is enabled
 * in Function Enable Mask Register.
 * See cvorb_rd_fem(), cvorb_wr_fem(), cvorb_func_en() and cvorb_func_dis()
 * for more details on howto set/get/check current channel mask.
 *
 * @return   0 -- if OK
 * @return < 0 -- if FAILED
 */
int cvorb_func_sel(int h, int ch, int func)
{
	ushort par[3]; /* ioctl params */

	if (!WITHIN_RANGE(1, h, MAX_HNDLS))
		return -CVORB_BAD_HANDLE;

	if (!WITHIN_RANGE(1, ch, MAX_CHAN_AM))
		return -CVORB_OUT_OF_RANGE;

	if (!WITHIN_RANGE(1, func, FAM))
		return -CVORB_OUT_OF_RANGE;

	/*
	  init params
	   [0] -- module idx
	   [1] -- channel idx
	   [2] -- function idx
	*/
	par[0] = (ch > CHAM) ? 1 : 0;
	par[1] = ((ch-1)%CHAM);
	par[2] = func-1;

	/* call the driver */
	if (ioctl(_lh[h-1].fd, CVORB_FUNC_SEL, &par))
		return -CVORB_IOCTL_FAILED;

	return 0;
}

/**
 * @brief Read Channel Function, currently selected for playing.
 *
 * @param h  -- handle
 * @param ch -- channel [1-16]
 *
 * Read channel "Function Selection" register
 *
 * @return function number that is currently selected [1-64] -- if OK
 * @return < 0                                               -- if FAILED
 */
int cvorb_func_get(int h, int ch)
{
	ushort par[3]; /* ioctl params */
	int rc;

	if (!WITHIN_RANGE(1, h, MAX_HNDLS))
		return -CVORB_BAD_HANDLE;

	if (!WITHIN_RANGE(1, ch, MAX_CHAN_AM))
		return -CVORB_OUT_OF_RANGE;

	/*
	  init params
	   [0] -- module idx
	   [1] -- channel idx
	   [2] -- function idx
	*/
	par[0] = (ch > CHAM) ? 1 : 0;
	par[1] = ((ch-1)%CHAM);

	/* call the driver */
	rc = ioctl(_lh[h-1].fd, CVORB_FUNC_GET, &par);
	if (rc < 0)
		return -CVORB_IOCTL_FAILED;
	return rc+1;
}

/**
 * @brief Decode CVORB library error code in a human-readable form.
 *
 * @param e -- CVORB error, returned by the call
 *
 * Use this function to decode errors, returned by the library calls.
 *
 * @return error string
 */
char* cvorb_perr(int e)
{
	switch (e) {
	case -CVORB_OK:
		return "All OK";
	case -CVORB_ERR:
		return "Internal library error";
	case -CVORB_BAD_HANDLE:
		return "Bad library handle";
	case -CVORB_DAL_ERR:
		return "DAL error";
	case -CVORB_OUT_OF_RANGE:
		return "Value is out-of-range";
	case -CVORB_IOCTL_FAILED:
		return "IOCTL call failed";
	default:
		return "Unknown error";
	}
}

static inline __attribute__((const))
int is_pow_of_2(unsigned long n)
{
        return (n != 0 && ((n & (n - 1)) == 0));
}

/**
 * @brief Allows a program to simulate front panel pulse inputs
 *
 * @param h   -- handle
 * @param m   -- module [1,2]
 * @param bit -- bit to set \n
 *               See Software Pulses register bits description
 *               for description of each bit.
 *
 * Writes into Software Pulses register.
 * Only one bit per write can be set.
 *
 * @return   0 -- if OK
 * @return < 0 -- if FAILED
 */
int cvorb_swp(int h, int m, int bit)
{
	int rc;
	ushort par[2]; /* ioctl params */

	if (!WITHIN_RANGE(1, h, MAX_HNDLS))
		return -CVORB_BAD_HANDLE;

	if (!WITHIN_RANGE(1, m, SMAM))
		return -CVORB_OUT_OF_RANGE;

	if ((bit & ~SPR_MASK) || !is_pow_of_2(bit))
		return -CVORB_OUT_OF_RANGE;

	/*
	  init params
	   [0] -- module idx
	   [1] -- bit to set in Soft Pulses reg
	*/
	par[0] = m-1;
	par[1] = bit;

	/* call the driver */
	rc = ioctl(_lh[h-1].fd, CVORB_WRSWP, &par);
	if (rc < 0)
		return -CVORB_IOCTL_FAILED;

	return 0;
}

/**
 * @brief Get current value of the Recurrent Cycles register
 *
 * @param h --  handle
 * @param ch -- channel [1-8]  -- submodule#1 \n
 *                      [9-16] -- submodule#2
 * @param rc -- Register value goes here
 *
 * This register allows selecting the number of time a function will
 * be generated. If zero, the function is played an infinite number of times.
 *
 * @return   0 -- if OK
 * @return < 0 -- if FAILED
 */
int cvorb_rd_rcyc(int h, int ch, uint *rc)
{
	struct {
		ushort  m; /* submodule idx */
		ushort  c; /* channel idx */
		uint   *r; /* results goes here */
	} par;

	if (!WITHIN_RANGE(1, h, MAX_HNDLS))
		return -CVORB_BAD_HANDLE;

	if (!WITHIN_RANGE(1, ch, MAX_CHAN_AM))
		return -CVORB_OUT_OF_RANGE;

	/* init params */
	par.m = (ch > CHAM) ? 1 : 0;
	par.c = ((ch-1)%CHAM);
	par.r = rc;

	/* call the driver */
	if (ioctl(_lh[h-1].fd, CVORB_RC_RD, &par))
		return -CVORB_IOCTL_FAILED;

	return 0;
}

/**
 * @brief Set new value into the Recurrent Cycles register
 *
 * @param h --  handle
 * @param ch -- channel [1-8]  -- submodule#1 \n
 *                      [9-16] -- submodule#2
 * @param rc -- value to set
 *
 * This register allows selecting the number of time a function will
 * be generated. If zero, the function is played an infinite number of times.
 *
 * @return   0 -- if OK
 * @return < 0 -- if FAILED
 */
int cvorb_wr_rcyc(int h, int ch, uint rc)
{
	struct {
		ushort m; /* submodule idx */
		ushort c; /* channel idx */
		uint   d; /* new data to set */
	} par;

	if (!WITHIN_RANGE(1, h, MAX_HNDLS))
		return -CVORB_BAD_HANDLE;

	if (!WITHIN_RANGE(1, ch, MAX_CHAN_AM))
		return -CVORB_OUT_OF_RANGE;

	/* init params */
	par.m = (ch > CHAM) ? 1 : 0;
	par.c = ((ch-1)%CHAM);
	par.d = rc;

	/* call the driver */
	if (ioctl(_lh[h-1].fd, CVORB_RC_WR, &par))
		return -CVORB_IOCTL_FAILED;

	return 0;
}

/**
 * @brief Check if SRAM memory is loaded with reasonable data.
 *
 * @param h    -- handle
 * @param ch   -- channel to check [1-16]
 * @param func -- funcion to check [1-64]
 *
 * Number of vectors in the function is checked. If it is within
 * allowed range [1 - 679] -- function vector table is considered
 * to be sane.
 *
 * @return number of vectors [1 - 679] -- SRAM is OK
 * @return -CVORB_ERR                  -- SRAM is not initialized
 * @return -CVORB_BAD_RANGE            -- bad handle
 * @return -CVORB_OUT_OF_RANGE         -- bad channel
 */
int cvorb_sram_ok(int h, int ch, int func)
{
	struct sram_params par;
	ushort va;

	if (!WITHIN_RANGE(1, h, MAX_HNDLS))
		return -CVORB_BAD_HANDLE;

	if (!WITHIN_RANGE(1, ch, MAX_CHAN_AM))
		return -CVORB_OUT_OF_RANGE;

	par = (struct sram_params) {
		.module = (ch > CHAM) ? 2 : 1,
		.chan = ((ch-1)%CHAM)+1,
		.func = func,
		.am = (ushort)-1 /* we need only number of vectors */
	};

	/* call the driver */
	va = ioctl(_lh[h-1].fd, CVORB_READ_SRAM, &par);
	if (!WITHIN_RANGE(1, va, MAX_F_VECT))
		return -CVORB_ERR;
	return va;
}

/**
 * @brief Enable onboard clock generator.
 *
 * @param h    -- handle
 * @param ch   -- channel [1-8]  -- submodule#1 \n
 *                        [9-16] -- submodule#2 \n
 * @param freq -- Clock generator frequency
 *                0 -- use default (that is 100MHz)
 *
 * <long-description>
 *
 * @return   0 -- if OK
 * @return < 0 -- if FAILED
 */
int cvorb_dac_on(int h, int ch, double freq)
{
	struct mcr cr;

	if (!WITHIN_RANGE(1, h, MAX_HNDLS))
		return -CVORB_BAD_HANDLE;

	if (!WITHIN_RANGE(0, ch, MAX_CHAN_AM))
		return -CVORB_OUT_OF_RANGE;

	/* TODO. For now set default freq only */
	if (ad9516o_on(_lh[h-1].fd, CVORB_DAC_ON, 0))
		return -CVORB_IOCTL_FAILED;

	/* now we can select on-board DAC source */

	/* read current configuration */
	cvorb_rd_mconfig_struct(h, ch, &cr);
	cr.dss   = ((ch-1)%CHAM); /* select new channel */
	cr.fplss = ((ch-1)%CHAM); /* Front panel LED's */
	cvorb_wr_mconfig_struct(h, ch, &cr);
	return 0;
}

/**
 * @brief Disable onboard Clock generator.
 *
 * @param h -- library handle
 *
 * @return   0 -- if OK
 * @return < 0 -- if FAILED
 */
int cvorb_dac_off(int h)
{
	if (!WITHIN_RANGE(1, h, MAX_HNDLS))
		return -CVORB_BAD_HANDLE;

	if (ioctl(_lh[h-1].fd, CVORB_DAC_OFF, NULL))
		return -CVORB_IOCTL_FAILED;

	return 0;
}

/**
 * @brief Get current onboard Clock generator frequency.
 *
 * @param h -- library handle
 *
 * <long-description>
 *
 * @return clk generator frequency -- if OK
 * @return < 0                     -- if FAILED
 */
double cvorb_get_clk_freq(int h)
{
	char *str;

	if (!WITHIN_RANGE(1, h, MAX_HNDLS))
		return -CVORB_BAD_HANDLE;

	return ad9516o_get_freq(_lh[h-1].fd, &str);
}

/**
 * @brief Write SRAM Start Address register
 *
 * @param h    -- handle
 * @param ch   -- channel [1-16]
 * @param func -- function  [1-64]
 *
 * Set SRAM address for r/w operations
 *
 * @return   0 -- if OK
 * @return < 0 -- if FAILED
 */
int cvorb_wr_sar(int h, int ch, int func)
{
	ushort par[3]; /* ioctl params */

	if (!WITHIN_RANGE(1, h, MAX_HNDLS))
		return -CVORB_BAD_HANDLE;

	if (!WITHIN_RANGE(1, ch, MAX_CHAN_AM))
		return -CVORB_OUT_OF_RANGE;

	if (!WITHIN_RANGE(1, func, FAM))
		return -CVORB_OUT_OF_RANGE;

	/*
	  init params
	   [0] -- module idx
	   [1] -- channel idx
	   [2] -- function idx
	*/
	par[0] = (ch > CHAM) ? 1 : 0;
	par[1] = ((ch-1)%CHAM);
	par[2] = func-1;

	/* call the driver */
	if (ioctl(_lh[h-1].fd, CVORB_WR_SAR, &par))
		return -CVORB_IOCTL_FAILED;

	return 0;
}

/**
 * @brief Get DAL handle
 *
 * @param h -- library handle
 *
 * Just in case if someone wants to work directly with DAL.
 *
 * @return -CVORB_BAD_HANDLE -- bad library handle
 * @return DAL handle        -- if OK
 */
HANDLE cvorb_get_dal_h(int h)
{
	if (!WITHIN_RANGE(1, h, MAX_HNDLS))
		return -CVORB_BAD_HANDLE;

	return _lh[h-1].h;
}
