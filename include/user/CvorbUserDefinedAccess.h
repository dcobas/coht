/**
 * @file CvorbUserDefinedAccess.h
 *
 * @brief CVORB Library API
 *
 * @author Copyright (C) 2009 CERN. Yury GEORGIEVSKIY <ygeorgie@cern.ch>
 *
 * @date Created on 12/10/2009
 *
 * @section license_sec License
 *          Released under the GPL
 */
#ifndef _CVORB_USER_DEFINED_ACCESS_H_INCLUDE_
#define _CVORB_USER_DEFINED_ACCESS_H_INCLUDE_

#include <dg/dal.h>
#include <CvorbDefs.h>

#define MAX_CHAN_AM 16 //!< module can have up to 16 channels

/** @defgroup cvorberror Library Return Codes
 *@{
 */
enum cvorble {
	CVORB_OK  = 0,	    /**< all OK */
	CVORB_ERR = 100,    /**< internal lib error */
	CVORB_BAD_HANDLE,   /**< bad library handle */
        CVORB_DAL_ERR,      /**< DAL Error Occurs. You Can decode it using
			       DaGetErrCode() */
        CVORB_OUT_OF_RANGE, /**< value out-of-range */
	CVORB_POISONED,	    /**< step size (in us) is @b not a factor
			       of delta t. Vector table is invalid */
	CVORB_IOCTL_FAILED  /**< ioctl call failed */
};
/*@} end of group*/

#ifdef __cplusplus
extern "C" {
#endif
/** @defgroup cvorblib Library API functions
 *@{
 */
	int cvorb_init(int);
	int cvorb_exit(int);
	int cvorb_rd_mconfig(int, int, uint*);
	int cvorb_rd_mconfig_struct(int, int, struct mcr*);
	int cvorb_wr_mconfig(int, int, uint);
	int cvorb_wr_mconfig_struct(int, int, struct mcr*);
	int cvorb_rd_cconfig(int, int, uint*);
	int cvorb_rd_cconfig_struct(int, int, struct ccr*);
	int cvorb_wr_cconfig(int, int, uint);
	int cvorb_wr_cconfig_struct(int, int, struct ccr*);
	int cvorb_rd_mstat(int, int, uint*);
	int cvorb_rd_mstat_struct(int, int, struct mstat*);
	int cvorb_rd_cstat(int, int, uint*);
	int cvorb_rd_cstat_struct(int, int, struct chstat*);
	int cvorb_rd_vhdl_vers(int, char*);
	int cvorb_rd_pcb_sn(int, char*);
	int cvorb_rd_temp(int, int*);
	int cvorb_rst_module(int, int);
	int cvorb_rst_fpga(int, int);
	int cvorb_rd_fem(int, int, unsigned long long *);
	int cvorb_rd_fem_arr(int, int, char[64]);
	int cvorb_wr_fem(int, int, unsigned long long);
	int cvorb_func_load(int, int, int, struct fv*, int);
	int cvorb_func_read(int, int, int, struct fv*, int);
	int cvorb_func_en(int, int, int);
	int cvorb_func_dis(int, int, int);
	int cvorb_func_sel(int, int, int);
	int cvorb_func_get(int, int);
	char* cvorb_perr(int);
	int cvorb_swp(int, int, int);
	int cvorb_rd_rcyc(int, int, uint *);
	int cvorb_wr_rcyc(int, int, uint);
	int cvorb_sram_ok(int, int, int);
/*@} end of group*/

#ifdef __cplusplus
}
#endif

#endif /* _CVORB_USER_DEFINED_ACCESS_H_INCLUDE_ */
