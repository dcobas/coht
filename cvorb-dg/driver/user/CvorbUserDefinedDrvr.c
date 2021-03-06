#include "CvorbUserDefinedDrvr.h"
#include "CvorbDrvrDefs.h"
#include "utilsDrvr.h"
#include "ad9516o-drvr.h"

#ifdef __powerpc__
#include <vme.h>
#endif

#if defined (__LYNXOS)
/* from linux/irqreturn.h */
/**
 * enum irqreturn
 * @IRQ_NONE            interrupt was not from this device
 * @IRQ_HANDLED         interrupt was handled by this device
 * @IRQ_WAKE_THREAD     handler requests to wake the handler thread
 */
enum irqreturn {
	IRQ_NONE,
	IRQ_HANDLED,
	IRQ_WAKE_THREAD,
};
#endif

struct cvorb_module _m[2];

#define FW_VERSION	0x420317
#define FW_MODEL	((FW_VERSION & 0xffff0000) >> 16)
#define MIN_FW_VERSION	(FW_VERSION & 0xffff)

/**
 * @brief Interrupt Service Routine.
 *
 * @param stPtr -- Statics table pointer
 *
 * Implementation is @b completely up to the user.
 *
 * @return IRQ_NONE        --  interrupt was not from this device
 * @return IRQ_HANDLED     --  interrupt was handled by this device
 * @return IRQ_WAKE_THREAD --  handler requests to wake the handler thread
 */
static int __attribute__((unused)) CvorbISR(CVORBStatics_t *ptr)
{
	CVORBStatics_t *stPtr;
	stPtr = (CVORBStatics_t *)ptr;

	return IRQ_HANDLED;
}


/**
 * @brief User entry point in driver/simulator open routine.
 *
 * @param proceed -- if standard code execution should be proceed
 * @param sptr    -- statics table pointer
 * @param d       --
 * @param f       -- file pointer. Lynx/Linux specific.
 *                   See (sys/file.h) for Lynx and (linux/fs.h) for Linux.
 *
 * It's up to user to set kernel-level errno (by means of @e pseterr call).
 * @e proceed parameter denotes if further standard actions should be proceed
 * after function returns. @b FALSE - means that user-desired operation done
 * all that user wants and there is no further necessaty to perfom any standard
 * operations that follow function call. @b TRUE - means that code that follows
 * function call will be executed.
 *
 * @return return value is the same as in entry point function\n
 *         OK     - if succeed.\n
 *         SYSERR - in case of failure.
 */
int CvorbUserOpen(int *proceed, register CVORBStatics_t *sptr,
			int d, struct file *f)
{
	CVORBUserStatics_t *usp; /* user statistics table */

	usp = sptr->usrst;


	/*
	  +-------------------------------+
	  | INSERT USER-DEFINED CODE HERE |
	  +-------------------------------+
	*/


	if (proceed)
		*proceed = TRUE; /* continue standard code execution */

	return OK; /* succeed */
}

/**
 * @brief User entry point in driver/simulator close routine.
 *
 * @param proceed -- if standard code execution should be proceed
 * @param sptr    -- statics table pointer
 * @param f       -- file pointer. Lynx/Linux specific.
 *                   See (sys/file.h) for Lynx and (linux/fs.h) for Linux.
 *
 * It's up to user to set kernel-level errno (by means of @e pseterr call).
 * @e proceed parameter denotes if further standard actions should be proceed
 * after function returns. @b FALSE - means that user-desired operation done
 * all that user wants and there is no further necessaty to perfom any standard
 * operations that follow function call. @b TRUE - means that code that follows
 * function call will be executed.
 *
 * @return return value is the same as in entry point function\n
 *         OK     - if succeed.\n
 *         SYSERR - in case of failure.
 */
int CvorbUserClose(int *proceed, register CVORBStatics_t *sptr,
			 struct file *f)
{
	CVORBUserStatics_t *usp; /* user statistics table */

	usp = sptr->usrst;


	/*
	  +-------------------------------+
	  | INSERT USER-DEFINED CODE HERE |
	  +-------------------------------+
	*/


	if (proceed)
		*proceed = TRUE; /* continue standard code execution */

	return OK; /* succeed */
}

/**
 * @brief User entry point in driver/simulator read routine.
 *
 * @param proceed -- if standard code execution should be proceed
 * @param sptr    -- statics table pointer
 * @param f       -- file pointer. Lynx/Linux specific.
 *                   See (sys/file.h) for Lynx and (linux/fs.h) for Linux.
 * @param buff    -- character buffer pointer
 * @param count   -- number of bytes to copy
 *
 * It's up to user to set kernel-level errno (by means of @e pseterr call).
 * @e proceed parameter denotes if further standard actions should be proceed
 * after function returns. @b FALSE - means that user-desired operation done
 * all that user wants and there is no further necessaty to perfom any standard
 * operations that follow function call. @b TRUE - means that code that follows
 * function call will be executed.
 *
 * @return return value is the same as in entry point function.\n
 *         number of bytes actually copied, including zero - if succeed.\n
 *         SYSERR                                          - if fails.
 */
int CvorbUserRead(int *proceed, register CVORBStatics_t *sptr,
			struct file *f, char *buff, int count)
{
	CVORBUserStatics_t *usp; /* user statistics table */

	usp = sptr->usrst;


	/*
	  +-------------------------------+
	  | INSERT USER-DEFINED CODE HERE |
	  +-------------------------------+
	*/


	if (proceed)
		*proceed = TRUE; /* continue standard code execution */

	return count; /* number of read bytes */
}

/**
 * @brief User entry point in driver/simulator write routine.
 *
 * @param proceed -- if standard code execution should be proceed
 * @param sptr    -- statics table pointer
 * @param f       -- file pointer. Lynx/Linux specific.
 *                   See (sys/file.h) for Lynx and (linux/fs.h) for Linux.
 * @param buff    -- char buffer pointer
 * @param count   -- the number of bytes to copy
 *
 * It's up to user to set kernel-level errno (by means of @e pseterr call).
 * @e proceed parameter denotes if further standard actions should be proceed
 * after function returns. @b FALSE - means that user-desired operation done
 * all that user wants and there is no further necessaty to perfom any standard
 * operations that follow function call. @b TRUE - means that code that follows
 * function call will be executed.
 *
 * @return return value is the same as in entry point function.\n
 *         number of bytes actually copied, including zero - if succeed.\n
 *         SYSERR                                          - if fails.
 */
int CvorbUserWrite(int *proceed, register CVORBStatics_t *sptr,
			 struct file *f, char *buff, int count)
{
	CVORBUserStatics_t *usp; /* user statistics table */

	usp = sptr->usrst;


	/*
	  +-------------------------------+
	  | INSERT USER-DEFINED CODE HERE |
	  +-------------------------------+
	*/


	if (proceed)
		*proceed = TRUE; /* continue standard code execution */

	return count; /* number of written bytes */
}

/**
 * @brief User entry point in driver/simulator select routine.
 *
 * @param proceed -- if standard code execution should be proceed
 * @param sptr    -- statics table pointer
 * @param f       -- file pointer. Lynx/Linux specific.
 *                   See (sys/file.h) for Lynx and (linux/fs.h) for Linux.
 * @param which   -- condition to monitor. Not valid in case of Linux!\n
 *                   <b> SREAD, SWRITE, SEXCEPT </b> in case of Lynx.
 * @param ffs     -- select data structure in case of Lynx.\n
 *                   struct poll_table_struct in case of Linux.
 *
 * It's up to user to set kernel-level errno (by means of @e pseterr call).
 * @e proceed parameter denotes if further standard actions should be proceed
 * after function returns. @b FALSE - means that user-desired operation done
 * all that user wants and there is no further necessaty to perfom any standard
 * operations that follow function call. @b TRUE - means that code that follows
 * function call will be executed.
 *
 * @return return value is the same as in entry point function\n
 *         OK     - if succeed.\n
 *         SYSERR - in case of failure.
 */
int CvorbUserSelect(int *proceed, register CVORBStatics_t *sptr,
			  struct file *f, int which, struct sel *ffs)
{
	CVORBUserStatics_t *usp; /* user statistics table */

	usp = sptr->usrst;


	/*
	  +-------------------------------+
	  | INSERT USER-DEFINED CODE HERE |
	  +-------------------------------+
	*/


	if (proceed)
		*proceed = TRUE; /* continue standard code execution */

	return OK; /* succeed */
}

/**
 * @brief User entry point in driver/simulator ioctl routine.
 *
 * @param proceed -- if standard code execution should be proceed
 * @param sptr    -- statics table pointer
 * @param f       -- file pointer. Lynx/Linux specific.
 *                   See (sys/file.h) for Lynx and (linux/fs.h) for Linux.
 * @param lun     -- minor number (LUN)
 * @param com     -- ioctl number
 * @param arg     -- ioctl arguments
 *
 * It's up to user to set kernel-level errno (by means of @e pseterr call).
 * @e proceed parameter denotes if further standard actions should be proceed
 * after function returns. @b FALSE - means that user-desired operation done
 * all that user wants and there is no further necessaty to perfom any standard
 * operations that follow function call. @b TRUE - means that code that follows
 * function call will be executed.
 *
 * @return return value is the same as in entry point function\n
 *         OK     - if succeed.\n
 *         SYSERR - in case of failure.
 */
int CvorbUserIoctl(int *proceed, register CVORBStatics_t *sptr,
			 struct file *f, int lun, int com, char *arg)
{
	CVORBUserStatics_t *usp = sptr->usrst; /* user statistics table */
	ushort edp[3]; /* [select/get function] ioctl parameters
			  [0] -- module idx
			  [1] -- chan idx
			  [2] -- func idx */
	*proceed = FALSE;

	switch (com) {
	case CVORB_VHDL:
		return read_vhdl(usp, arg);
	case CVORB_PCB:
		return read_pcb(usp, arg);
	case CVORB_TEMP:
		return read_temp(usp, arg);
	case CVORB_MOD_CFG_RD:
		return read_mod_config_reg(usp, arg);
	case CVORB_MOD_CFG_WR:
		return write_mod_config_reg(usp, arg);
	case CVORB_CH_CFG_RD:
		return read_ch_config_reg(usp, arg);
	case CVORB_CH_CFG_WR:
		return write_ch_config_reg(usp, arg);
	case CVORB_MOD_STAT:
		return read_mod_stat(usp, arg);
	case CVORB_CH_STAT:
		return read_ch_stat(usp, arg);
	case CVORB_LOAD_SRAM:
		return load_sram(usp, arg);
	case CVORB_READ_SRAM:
		return read_sram(usp, arg);
	case CVORB_FEN: /* enable function in the function bitmask */
		return enable_function(usp, arg);
	case CVORB_FDIS: /* disable function in the function bitmask */
		return disable_function(usp, arg);
	case CVORB_FEN_RD: /* read Funciton Enable Mask */
		{
			uint m[2]; /* [0] -- bits[63-32]
				      [1] -- bits[31-0] */

			/* ioctl parameters */
			struct {
				ushort m; /* module idx */
				ushort c; /* channel idx */
				uint  *p; /* results goes here */
			} par;

			if (cdcm_copy_from_user(&par, arg, sizeof(par)))
				return SYSERR;

			m[0] = _rcr(par.m, par.c, FCT_EM_H);
			m[1] = _rcr(par.m, par.c, FCT_EM_L);
			return cdcm_copy_to_user(par.p, m, sizeof(m));
		}
	case CVORB_FEN_WR: /* write Function Enable Mask */
		return write_fem_regs(usp, arg);
	case CVORB_FUNC_SEL: /* select function to be played */
		if (cdcm_copy_from_user(&edp, arg, sizeof(edp)))
			return SYSERR;
		_wcr(edp[0], edp[1], FUNC_SEL, edp[2]);
		/* Should wait on Channel Status register bit[9] -- function
		   copy in progress, when data is copying into local SRAM. */
		while(_rcr(edp[0], edp[1], CH_STAT) & 1<<9)
			usec_sleep(1);
		return OK;
	case CVORB_FUNC_GET:	/* get currently selected function */
		if (cdcm_copy_from_user(&edp, arg, sizeof(edp)))
			return SYSERR;
		return _rcr(edp[0], edp[1], FUNC_SEL);
	case CVORB_WRSWP: /* action register.
			     Simulate front panel pulse inputs */
		return write_swp(usp, arg);
	case CVORB_RC_RD:
		return read_recurrent_cycles_reg(usp, arg);
	case CVORB_RC_WR:
		return write_recurrent_cycles_reg(usp, arg);
	case CVORB_DAC_ON:
		return dac_on(usp, arg);
	case CVORB_DAC_OFF:
		/* disable on-board clock generator */
		_wr(0, CLK_GEN_CNTL, AD9516_OFF);
		return OK;
	case AD9516_GET_PLL:
		return get_pll(usp, arg);
	case CVORB_WR_SAR:
		return write_sar(usp, arg);
	default:
		*proceed = TRUE; /* continue standard code execution */
	}

	return OK;
}

int firmware_ok(CVORBUserStatics_t *usp, int lun)
{
	uint serial;
	uint model;
	uint version;

	serial = _rr(0, VHDL_V);
	model = (serial & 0xffff0000) >> 16;
	version = (serial & 0x0000ffff);
	kkprintf("cvorb firmware version: 0x%08x\n", serial);
	if (model != FW_MODEL) {
		kkprintf("Board with lun %d has a wrong FW model: 0x%x. Should be 0x%x\n",
							lun, model, FW_MODEL);
		return 0;
	}
	if (version < MIN_FW_VERSION) {
		kkprintf("Board with lun %d has too old FW version: "
			"0x%x. Should be 0x%x or higher\n",
			lun, version, MIN_FW_VERSION);
		return 1;
	}
	return 1;
}
/**
 * @brief User entry point in driver/simulator installation routine.
 *
 * @param proceed -- if standard code execution should be proceed
 * @param info    -- driver info table
 * @param sptr    -- statics table
 *
 * It's up to user to set kernel-level errno (by means of @e pseterr call).
 * @e proceed parameter denotes if further standard actions should be proceed
 * after function returns. @b FALSE - means that user-desired operation done
 * all that user wants and there is no further necessaty to perfom any standard
 * operations that follow function call. @b TRUE - means that code that follows
 * function call will be executed.
 *
 * @return return value is the same as in entry point function.\n
 *         pointer to a statics data structure - if succeed.\n
 *         SYSERR                              - in case of failure.
 */
char* CvorbUserInst(int *proceed, register DevInfo_t *info,
			  register CVORBStatics_t *sptr)
{
	CVORBUserStatics_t *usp = sptr->usrst; /* user statistics table */
	int iVec = 0; /* interrupt vector */
	int m, c;

	iVec = info->iVector;		/* set up interrupt vector */

	/* map submodule address pointers */
	usp->md = (struct cvorb_module *)sysbrk(sizeof(_m));

	usp->md[0].md = (mod *)sptr->card->block00;
	usp->md[1].md = (mod *)((long)sptr->card->block00 + 0x200);
	
	if (!firmware_ok(usp, info->mlun)) {
		sysfree((char *)usp->md, sizeof(_m));
		return (char *)SYSERR;
	}
	for (m = 0; m < SMAM; m++) {
		/* reset subModules */
		_wr(m, SOFT_PULSE, SPR_FGR);

		/* initialize iolock mutex */
		cdcm_mutex_init(&usp->md[m].iol);

		/* set submodule channels addresses */
		for (c = 0; c < CHAM; c++)
			usp->md[m].cd[c] = (chd *)
				((long)usp->md[m].md + _ch_offset[c]);
	}

	/* init on-board DAC */
	ad9516o_init(usp);

	/* disable on-board clock generator */
	_wr(0, CLK_GEN_CNTL, AD9516_OFF);

	/* set normal mode operation, enable all channels and
	   set recurrent cycles to 1 (i.e. play function once) */
	enable_modules(usp);

	/* Uncomment the following code to register ISR */
#if 0
	if (iVec > 0) {
		int cc = 0; /* completion code */
		kkprintf("ISR ( vector number [%d] ) installation - ", iVec);
#ifdef __Lynx__
#ifdef __powerpc__ /* in this case we are using CES BSP */
		cc = vme_intset(iVec, (int (*)())CvorbISR,
				  (char*)sptr, 0);
#else  /* use standard system call otherwise */
		cc = iointset(iVec, (int (*)())CvorbISR, (char*)sptr);
#endif
#else  /* __linux__ */
		cc = vme_request_irq(iVec,
				     (int (*)(void *))CvorbISR,
				     (char *)sptr,
				     "CvorbD");
#endif /* __Lynx__ */
		if (cc < 0) {
			kkprintf("Failed.\n");
			pseterr(EFAULT); /* TODO. what error to set? */
			return (char*)SYSERR;	/* -1 */
		}
		kkprintf("interrupt vector managed.\n");
	}
#endif

	if (proceed)
		*proceed = TRUE; /* continue standard code execution */

	return (char *)sptr; /* succeed */
}

/**
 * @brief User entry point in driver/simulator unistall routine.
 *
 * @param proceed -- if standard code execution should be proceed
 * @param sptr    -- statics table pointer
 *
 * It's up to user to set kernel-level errno (by means of @e pseterr call).
 * @e proceed parameter denotes if further standard actions should be proceed
 * after function returns. @b FALSE - means that user-desired operation done
 * all that user wants and there is no further necessaty to perfom any standard
 * operations that follow function call. @b TRUE - means that code that follows
 * function call will be executed.
 *
 * @return return value is the same as in entry point function.\n
 *         OK     - if succeed.\n
 *         SYSERR - in case of failure.
 */
int CvorbUserUnInst(int *proceed, CVORBStatics_t *sptr)
{
	//CVORBUserStatics_t *usp = sptr->usrst; /* user statistics table */


	/* Uncomment the following code to unregister ISR */
#if 0
	kkprintf("Cvorb: Interrupt routine managment"
		 " cleanup ( vector number [%d] ) - ", sptr->info->iVector);
#ifdef __Lynx__
#ifdef __powerpc__
	/* in this case we are using CES BSP */
	vme_intclr(sptr->info->iVector, 0);
#else
	iointclr(sptr->info->iVector);
#endif
#else  /* __linux__ */
	vme_free_irq(sptr->info->iVector);
#endif	/* __Lynx__ */

	kkprintf("OK\n");
#endif

	sysfree((char*)sptr->usrst->md, sizeof(_m));
	if (proceed)
		*proceed = TRUE; /* continue standard code execution */

	return OK; /* succeed */
}
