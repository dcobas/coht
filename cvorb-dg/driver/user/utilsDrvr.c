/**
 * @file utilsDrvr.c
 *
 * @brief Extra CVORB functions
 *
 * Housekeeping routines used in driver entry points are defined here.
 *
 * @author Copyright (C) 2009 CERN. Yury GEORGIEVSKIY <ygeorgie@cern.ch>
 *
 * @date Created on 21/10/2009
 *
 * @section license_sec License
 *          Released under the GPL
 */
#include "CvorbUserDefinedDrvr.h"
#include "CvorbDrvrDefs.h"
#include "utilsDrvr.h"
#include "ad9516o-drvr.h"

void print_load_sram_ioctl_args(ushort *args)
{
	int *sarp = (int *)&args[FSZ];

	kkprintf("Start address register is 0x%x\n"
		 "Chan#     --> %d\n"
		 "Module#   --> %d\n"
		 "Function# --> %d\n",
		 *sarp,
		 (*sarp & SRAM_CHAN_MASK)>>SRAM_CHAN_SHIFT,
		 sarp[1],
		 (*sarp & SRAM_FUNC_MASK)>>SRAM_FUNC_SHIFT);
}

/*
  1. SRAM Memory organization inside the function:
  ------------------------------------------------

  0 2   4    6  8  a   c ... <-- address offset
  +-+---+----+--+--+---+---+----+----+--+--+---+---+----+----+--+--+---+
  |0|nov|nos1|v0|v1|ss1|ss2|nos2|nos3|v2|v3|ss3|ss4|nos4|nos5|v4|v5|ss5| ...
  +-+---+----+--+--+---+---+----+----+--+--+---+---+----+----+--+--+---+
   0  1   2   3   4  5  ... <-- word number (element idx)

  where:
  nov  -- number of vectors in the function
  nos# -- number of vector steps
  ss#  -- vector step size
  V#   -- vector 16-bit value

  The algorithm to fill in the memory is the following:
  p = el[2];  (nos1)
  for (number of vectors) {
    if (odd) {
       p[0] = nos;
       p[2] = v;
       p[3] = ss;
       p += 4;
    } else (even) {
       p[0] = ss;
       p[1] = nos;
       p[3] = v;
       p += 2;
    }
  }


  2. Memory allocation computation:
  ---------------------------------

     1  |  2    |   3  |    4   |   5   |  6   |    7   |   8   |  9   |<-dwords
  +-+---+----+--+--+---+---+----+----+--+--+---+---+----+----+--+--+---+
  |0|nov|nos1|v0|v1|ss1|ss2|nos2|nos3|v2|v3|ss3|ss4|nos4|nos5|v4|v5|ss5|+3dwords
  +-+---+----+--+--+---+---+----+----+--+--+---+---+----+----+--+--+---+
   1  2 | 3   4 | 5  6 | 7    8 |9   10 |11  12| 13  14 |15  16 |17  18|<-words

  Size of one vector -- is 3 words           <-- VSZ
  Min size possible -- is 3 words (nov,0,V0) <-- MIN_V

  (vector_amount * VSZ) + MIN_V

  Memory should be dword aligned (4bytes)
  Ex:
  vector amount | size in words | 4-byte aligned size (in dwords)
  --------------+---------------+--------------------------------
        1       |       6       |              3
  --------------+---------------+--------------------------------
        2       |       9       |              5
  --------------+---------------+--------------------------------
        3       |       12      |              6
  --------------+---------------+--------------------------------
        4       |       15      |              8
  --------------+---------------+--------------------------------
        5       |       18      |              9
  --------------+---------------+--------------------------------
      ...             ...                      ...
 */
#define VSZ 3   /* vector size in words */
#define MIN_V 3 /* minimun size of memory inside the function in words */
int load_sram(CVORBUserStatics_t *usp, char *arg) //struct sram_params *p)
{
        struct sram_params *p; /* load/read SRAM ioctl paramters */
	int *sp; /* swap pointer */
	uint sar = 0; /* Start Address Register */
	ushort *vect = NULL, *vp;
	int sz = 0;   /* size (in words) of a function memory */
	int i, rc = OK;

	p = (struct sram_params *)sysbrk(sizeof(*p));
	if (!p)
		return SYSERR;

	if (cdcm_copy_from_user(p, arg, sizeof(*p)))
		return SYSERR;

	sz = (p->am * VSZ) + MIN_V;
	if (sz&1) ++sz;	 /* should be even number of words
			    (in order to be 4-byte aligned  */
	sz += 6; /* 6 words of "0" are mandatory at the end */
	sz <<= 1; /* convert words to bytes */

	vect = (ushort *)sysbrk(sz);
	if (!vect) {
		rc = SYSERR;
		goto out_load_sram;
	}
	memset(vect, 0, sz);

	vp = &vect[2];
	vect[1] = p->am;
	vect[3] = p->fv[0].v; /* set V0 */
	for (i = 1; i <= p->am; i++) {
		if (i&1) { /* odd */
			vp[0] = p->fv[i].nos;
			vp[2] = p->fv[i].v;
			vp[3] = p->fv[i].ss;
			vp += 4;
		} else { /* even */
			vp[0] = p->fv[i].ss;
			vp[1] = p->fv[i].nos;
			vp[3] = p->fv[i].v;
			vp += 2;
		}
        }

#ifdef __linux__
	/* we should swap words inside the vector table,
           so that dwords will be written into SRAM memory correctly */
        for (i = 0, sp = (int *)vect; i < sz; i += 4, ++sp)
		swahw32s(sp);
#endif

	/* set Start Address Register */
	sar |= (((p->chan-1)<<SRAM_CHAN_SHIFT) |
		((p->func-1)<<SRAM_FUNC_SHIFT));

	/* enter critical region -- r/w ioaccess protection */
	cdcm_mutex_lock(&usp->md[p->module-1].iol);
	/* Should wait on Channel Status register bit[9] -- function
	   copy in progress, when data is copying into local SRAM. */
	while(_rcr(p->module-1, p->chan-1, CH_STAT) & 1<<9)
		usec_sleep(1);
	_wr(p->module-1, SRAM_SA, sar);
	_wrr(p->module-1, SRAM_DATA, (ulong *)vect, sz/4);
	/* leave critical region */
	cdcm_mutex_unlock(&usp->md[p->module-1].iol);

 out_load_sram:
	if (p)   sysfree((char *)p, sizeof(*p));
	if (vect) sysfree((char *)vect, sz);
	return rc;
}

int read_sram(CVORBUserStatics_t *usp, char *arg)
{
	struct sram_params *p; /* ioctl paramters */
	int i, rc = OK;
	int sz = 0; /* function size (in words) in SRAM */
	uint sram = 0, *ptr;
	ushort nov;		/* number of vectors */
	ushort *vect = NULL, *vp; /* vector table && its pointer */
	uint sar = 0;

	p = (struct sram_params *)sysbrk(sizeof(*p));
        if (!p)
                return SYSERR;

	memset(p, 0, sizeof(*p));

	if (cdcm_copy_from_user(p, arg, sizeof(*p)))
		return SYSERR;

	/* set Start Address Register */
	sar |= (((p->chan-1)<<SRAM_CHAN_SHIFT) |
		((p->func-1)<<SRAM_FUNC_SHIFT));

	/* enter critical region -- r/w ioaccess protection */
	cdcm_mutex_lock(&usp->md[p->module-1].iol);

	_wr(p->module-1, SRAM_SA, sar);

	/* get number of vectors in the function
	   and compute how many dwords we should read */
#ifdef __linux__
	sram = swahb32(ioread32((void *)
				&(usp->md[p->module-1].md->SRAM_DATA)));
#else  /* lynx */
	sram = __inl((__port_addr_t)&(usp->md[p->module-1].md->SRAM_DATA));
#endif
	vp = (ushort *)&sram;
	++vp; /* move pointer to number of vectors in the function */
	nov = *vp;

	if (p->am == (ushort)-1) { /* just return number of vectors
				     in the function to the user */
		cdcm_mutex_unlock(&usp->md[p->module-1].iol);
		rc = nov;
		goto out;
	}

	/* safety check for garbage SRAM */
	if (!(WITHIN_RANGE(1, nov, MAX_F_VECT))) {
		rc = SYSERR;
		goto outcritical; /* was no initialized */
	}

	/* how many bytes we should read from SRAM */
	sz = (nov * VSZ) + MIN_V;
	if (sz&1) ++sz;
	sz >>= 1;	/* convert to dwords */
	--sz;		/* as we already read first dword */

	/* allocate storage room for SRAM data */
	vect = (ushort *)sysbrk(sz*4);
	if (!vect) {
		rc = SYSERR;
		goto outcritical;
	}
	memset(vect, 0, sz*4);

	/* read SRAM && store data in the vector table */
	ptr = (uint *)vect;
	for (i = 0; i < sz; i++, ptr++) {
#ifdef __linux__
		*ptr = swahb32(ioread32((void *)&(usp->md[p->module-1].md->SRAM_DATA)));
#else  /* lynx */
		*ptr = __inl((__port_addr_t)&(usp->md[p->module-1].md->SRAM_DATA));
#endif
	}

 outcritical:
	/* leave critical region */
	cdcm_mutex_unlock(&usp->md[p->module-1].iol);
	if (rc < 0)
		goto out;

	/* now convert vector table to [function vectors] table
	   and put it in the user space */
	p->fv[0].nos = nov+1; /* return number of vectors (plus [t0, V0]) */
	p->fv[0].v = vect[1]; /* V0 */
	vp = &vect[0]; /* set to first Number of Steps */
	for ( i = 1; i <= nov; i++) {
		if (i&1) { /* odd */
			p->fv[i].nos = vp[0];
                        p->fv[i].v   = vp[2];
                        p->fv[i].ss  = vp[3];
			vp += 4;
		} else { /* even */
			p->fv[i].ss  = vp[0];
			p->fv[i].nos = vp[1];
                        p->fv[i].v   = vp[3];
			vp += 2;
		}
	}

	/* give results to the user */
	if (cdcm_copy_to_user(arg, p, sizeof(*p)))
		rc = SYSERR;
 out:
	if (vect) sysfree((char *)vect, sz*4);
	if (p) sysfree((char *)p, sizeof(*p));
	return rc;
}

int read_vhdl(CVORBUserStatics_t *usp, char *arg)
{
	uint rv;
	rv =  _rr(0, VHDL_V);
	return cdcm_copy_to_user(arg, &rv, sizeof(rv));
}

int read_pcb(CVORBUserStatics_t *usp, char *arg)
{
	uint32_t rv[2];

	rv[0] =  _rr(0, PCB_SN_H);
	rv[1] =  _rr(0, PCB_SN_L);
	return cdcm_copy_to_user(arg, rv, sizeof(rv));
}


int read_temp(CVORBUserStatics_t *usp, char *arg)
{
	uint32_t rv;
	int temp;

	rv =  _rr(0, TEMP);
	temp = (rv & 0x7FF) >> 4; /* get rid of fraction */
        if (rv & 0x800) /* sign bit */
                temp *= -1;
	return cdcm_copy_to_user(arg, &temp, sizeof(temp));
}

int read_mod_config_reg(CVORBUserStatics_t *usp, char *arg)
{
	uint mc[2]; /* module configuration */
	if (cdcm_copy_from_user(&mc, arg, sizeof(uint)))
		return SYSERR;

	switch (mc[0]) {
		case SMAM: /* need to read configuration registers
			   from both submodules  */
			mc[0] = _rr(0, MOD_CFG);
			mc[1] = _rr(1, MOD_CFG);
			return cdcm_copy_to_user(arg, &mc, sizeof(mc));
		case 0:
		case 1:
			mc[0] = _rr(mc[0], MOD_CFG);
			return cdcm_copy_to_user(arg, &mc, sizeof(uint));
		};

	return SYSERR;
}

int write_mod_config_reg(CVORBUserStatics_t *usp, char *arg)
{
	uint par[2]; /* 0 -- submodule idx, 1 -- new val */

	if (cdcm_copy_from_user(par, arg, sizeof(par)))
		return SYSERR;
	_wr(par[0], MOD_CFG, par[1]);
	return OK;
}

int  read_ch_config_reg(CVORBUserStatics_t *usp, char *arg)
{
	uint cc; /* channel configuration */
	struct { /* ioctl params */
		ushort  m; /* submodule idx */
		ushort  c; /* channel idx */
		uint   *r; /* results goes here */
	} par;

	if (cdcm_copy_from_user(&par, arg, sizeof(par)))
		return SYSERR;
	cc = _rcr(par.m, par.c, CH_CFG);
	return cdcm_copy_to_user(par.r, &cc, sizeof(cc));
}

int write_ch_config_reg(CVORBUserStatics_t *usp, char *arg)
{
        struct { /* ioctl params */
                ushort m; /* submodule idx */
                ushort c; /* channel idx */
                uint   d; /* new data to set */
        } par;

	if (cdcm_copy_from_user(&par, arg, sizeof(par)))
		return SYSERR;

	_wcr(par.m, par.c, CH_CFG, par.d);
	return OK;
}

int read_mod_stat(CVORBUserStatics_t *usp, char *arg)
{
	uint ms; /* module status */
	if (cdcm_copy_from_user(&ms, arg, sizeof(ms)))
		return SYSERR;
	ms = _rr(ms, MOD_STAT);
	return cdcm_copy_to_user(arg, &ms, sizeof(ms));

}

int read_ch_stat(CVORBUserStatics_t *usp, char *arg)
{
	uint cs; /* channel status */
	struct { /* ioctl params */
		ushort  m; /* submodule idx */
		ushort  c; /* channel idx */
		uint   *r; /* results goes here */
	} par;

	if (cdcm_copy_from_user(&par, arg, sizeof(par)))
		return SYSERR;
	cs = _rcr(par.m, par.c, CH_STAT);
	return cdcm_copy_to_user(par.r, &cs, sizeof(cs));
}

int enable_function(CVORBUserStatics_t *usp, char *arg)
{
	uint rv; /* register value */
	ushort par[3]; /* [0] -- submodule idx
			  [1] -- chan idx
			  [2] -- func idx */

	if (cdcm_copy_from_user(par, arg, sizeof(par)))
		return SYSERR;

	/* if function idx is > FAM -- enable all functions */
	if (par[2] > FAM) {
		_wcr(par[0], par[1], FCT_EM_L, (uint)-1);
		_wcr(par[0], par[1], FCT_EM_H, (uint)-1);
		return OK;
	}

	/* read current mask */
	if (par[2] < 32)
		rv = _rcr(par[0], par[1], FCT_EM_L);
	else
		rv = _rcr(par[0], par[1], FCT_EM_H);

	/* enable bit && set new mask */
	rv |= 1<<par[2];
	if (par[2] < 32)
		_wcr(par[0], par[1], FCT_EM_L, rv);
	else
		_wcr(par[0], par[1], FCT_EM_H, rv);

	return OK;
}

int disable_function(CVORBUserStatics_t *usp, char *arg)
{
	uint rv; /* register value */
	ushort par[3]; /* [0] -- submodule idx
			  [1] -- chan idx
			  [2] -- func idx */

	if (cdcm_copy_from_user(&par, arg, sizeof(par)))
		return SYSERR;

	/* if function idx is > FAM -- disable all functions */
	if (par[2] > FAM) {
		_wcr(par[0], par[1], FCT_EM_L, 0);
		_wcr(par[0], par[1], FCT_EM_H, 0);
		return OK;
	}

	/* read current mask */
	if (par[2] < 32)
		rv = _rcr(par[0], par[1], FCT_EM_L);
	else
		rv = _rcr(par[0], par[1], FCT_EM_H);

	/* disable bit && set new mask */
	rv &= ~(1<<par[2]);
	if (par[2] < 32)
		_wcr(par[0], par[1], FCT_EM_L, rv);
	else
		_wcr(par[0], par[1], FCT_EM_H, rv);

	return OK;
}

int read_recurrent_cycles_reg(CVORBUserStatics_t *usp, char *arg)
{
	uint rc; /* recurrent cycles */
	struct { /* ioctl params */
		ushort  m; /* submodule idx */
		ushort  c; /* channel idx */
		uint   *r; /* results goes here */
	} par;

	if (cdcm_copy_from_user(&par, arg, sizeof(par)))
		return SYSERR;
	rc = _rcr(par.m, par.c, CH_REC_CYC);
	return cdcm_copy_to_user(par.r, &rc, sizeof(rc));
}

int write_recurrent_cycles_reg(CVORBUserStatics_t *usp, char *arg)
{
        struct { /* ioctl params */
                ushort m; /* submodule idx */
                ushort c; /* channel idx */
                uint   d; /* new data to set */
        } par;

	if (cdcm_copy_from_user(&par, arg, sizeof(par)))
		return SYSERR;
	_wcr(par.m, par.c, CH_REC_CYC, par.d);
	return OK;
}

int write_fem_regs(CVORBUserStatics_t *usp, char *arg)
{
	struct {
		ushort m; /* submodule idx */
		ushort c; /* channel idx */
		unsigned long long p;  /* new value to set */
	} par;
	uint32_t *ptr = (uint32_t *) &par.p;

	if (cdcm_copy_from_user(&par, arg, sizeof(par)))
		return SYSERR;

#ifdef __linux__
	par.p = __swahl64(par.p);
#endif

	_wcr(par.m, par.c, FCT_EM_H, ptr[0]);
	_wcr(par.m, par.c, FCT_EM_L, ptr[1]);

	return OK;
}

int write_swp(CVORBUserStatics_t *usp, char *arg)
{
	ushort par[2]; /* [0] -- submodule idx, [1] -- new val */

	if (cdcm_copy_from_user(par, arg, sizeof(par)))
		return SYSERR;
	_wr(par[0], SOFT_PULSE, par[1]);

	/* will re-enable modules/channels */
	if (par[1] == SPR_MSR || par[1] == SPR_FGR)
		enable_modules(usp);

	return OK;
}

void enable_modules(CVORBUserStatics_t *usp)
{
	int i;

	for (i = 0; i < SMAM; i++)
		/* set normal mode operation */
		_wr(i, MOD_CFG, 1);

        /* enable all channels and
           set recurrent cycles to 1 (i.e. play function once) */
        for (i = 0; i < CHAM; i++) {
                _wcr(0, i, CH_CFG, 1);
                _wcr(0, i, CH_REC_CYC, 1);

                _wcr(1, i, CH_CFG, 1);
                _wcr(1, i, CH_REC_CYC, 1);
        }
}

/* enable on-board clock generator */
int  dac_on(CVORBUserStatics_t *usp, char *arg)
{
	struct pll pll;

	if (cdcm_copy_from_user(&pll, arg, sizeof(pll)))
		return SYSERR;

	/* for now just set a default config */
	//return ad9516o_clkgen_default_conf();
	return ad9516o_put_pll_conf(usp, &pll);
}

/* get current PLL settings */
int get_pll(CVORBUserStatics_t *usp, char *arg)
{
	struct pll pll;
	ad9516o_get_pll_conf(usp, &pll);
	return cdcm_copy_to_user(arg, &pll, sizeof(pll));
}

/* write SRAM Start Address register */
int write_sar(CVORBUserStatics_t *usp, char *arg)
{
	uint rv = 0; /* register value */
	ushort par[3]; /* [0] -- submodule idx
			  [1] -- chan idx
			  [2] -- func idx */

	if (cdcm_copy_from_user(&par, arg, sizeof(par)))
		return SYSERR;

	rv |= (par[1]<<SRAM_CHAN_SHIFT) | (par[2]<<SRAM_FUNC_SHIFT);
	_wr(par[0], SRAM_SA, rv);
	return OK;
}
