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

void print_load_sram_ioctl_args(ushort *args)
{
	//int i;
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
        struct sram_params p; /* load/read SRAM ioctl paramters */
	int *sp; /* swap pointer */
	uint sar = 0; /* Start Address Register */
	uint *ptr;
	ushort *vect = NULL, *vp;
	int fvsz; /* function vector massive size */
	int sz = 0;   /* size (in words) of a function memory */
	int i, rc = OK;
	int dt;		/* delta t in us */
	ushort ss, nos; /* step size, number of steps */
	struct fv *fv;

	if (cdcm_copy_from_user(&p, arg, sizeof(p)))
		return SYSERR;

	/* get functioin vectors */
	fvsz = sizeof(*fv) * ((p.am+1/*incl [t0,V0]*/));
	fv = (struct fv *)sysbrk(fvsz);
	if (!fv)
		return SYSERR;

	if (cdcm_copy_from_user(fv, p.fv, fvsz)) {
		rc = SYSERR;
		goto out_load_sram;
	}

	sz = (p.am * VSZ) + MIN_V;
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
	vect[1] = p.am;
	vect[3] = fv->v; /* set V0 */
	for (i = 1; i <= p.am; i++) {
		if (!fv[i].t) { /* internal stop */
			ss = (ushort) -1;
			nos = 0;
		} else {
			dt = (fv[i].t - fv[i-1].t)*1000; /* delta t in us */
			ss = ((dt-1)/MTMS) + 1;
			nos = dt/(ss * MIN_STEP);
		}
#if 0
		/* step size (in us) should be a factor of dt!
		   should be already checked by the library! */
		if (dt%ss)
			poison;
#endif

		if (i&1) { /* odd */
			vp[0] = nos;
			vp[2] = fv[i].v;
			vp[3] = ss;
			vp += 4;
		} else { /* even */
			vp[0] = ss;
			vp[1] = nos;
			vp[3] = fv[i].v;
			vp += 2;
		}
        }

	ptr = (uint *)vect;

#ifdef __linux__
	/* we should swap words inside the vector table,
           so that dwords will be written into SRAM memory correctly */
        for (i = 0, sp = (int *)vect; i < sz; i += 4, ++sp)
		swahw32s(sp);
#endif

	/* set Start Address Register */
	sar |= (((p.chan-1)<<SRAM_CHAN_SHIFT) |
		((p.func-1)<<SRAM_FUNC_SHIFT));

	_wr(p.module-1, SRAM_SA, sar);
	_wrr(p.module-1, SRAM_DATA, (ulong *)vect, sz/4);

 out_load_sram:
	if (fv)   sysfree((char *)fv, fvsz);
	if (vect) sysfree((char *)vect, sz);
	return rc;
}

int read_sram(CVORBUserStatics_t *usp, char *arg)
{
	struct sram_params p; /* ioctl paramters */
	int i, rc = OK;
	int sz; /* function size (in words) in SRAM */
	uint sram = 0, *ptr;
	ushort *nov;		/* number of vectors */
	ushort *vect = NULL, *vp; /* vector table && its pointer */
	uint sar = 0;
	struct fv *fv = NULL;
	int fvsz; /* function vector size in bytes */
	int dt = 0;  /* time in ms from the beginning of the function */

	if (cdcm_copy_from_user(&p, arg, sizeof(p)))
		return SYSERR;

	/* set Start Address Register */
	sar |= (((p.chan-1)<<SRAM_CHAN_SHIFT) |
		((p.func-1)<<SRAM_FUNC_SHIFT));

	_wr(p.module-1, SRAM_SA, sar);

	/* get number of vectors in the function
	   and compute how many dwords we should read */
#ifdef __linux__
	sram = swahb32(ioread32((void *)
				&(usp->md[p.module-1].md->SRAM_DATA)));
#else  /* lynx */
	sram = __inl((__port_addr_t)&(usp->md[p.module-1].md->SRAM_DATA));
#endif
	nov = (ushort *)&sram;	/* number of vectors in the function */
	++nov;

	/* safety check for garbage SRAM */
	if (!(WITHIN_RANGE(1, *nov, MAX_F_VECT)))
		return SYSERR;	/* was no initialized */

	if (*nov > p.am) /* provided storage is not big enough */
		return SYSERR;

	fvsz = (*nov+1/*t0,V0*/)*sizeof(*fv); /* function vector size,
						 including [t0,V0] */

	/* allocate space to store function vector data */
	fv = (struct fv *) sysbrk(fvsz);
	if (!fv)
		return SYSERR;
	memset(fv, 0, fvsz);

	/* how many bytes we should read from SRAM */
	sz = (*nov*VSZ)+MIN_V;
	if (sz&1) ++sz;
	sz >>= 1;	/* convert to dwords */
	--sz;		/* as we already read first dword */

	/* allocate storage room for SRAM data */
	vect = (ushort *)sysbrk(sz*4);
	if (!vect) {
		rc = SYSERR;
		goto out;
	}
	memset(vect, 0, sz*4);

	/* read SRAM && store data in the vector table */
	ptr = (uint *)vect;
	for (i = 0; i < sz; i++, ptr++) {
#ifdef __linux__
		*ptr = swahb32(ioread32((void *)&(usp->md[p.module-1].md->SRAM_DATA)));
#else  /* lynx */
		*ptr = __inl((__port_addr_t)&(usp->md[p.module-1].md->SRAM_DATA));
#endif
	}

	/* now convert vector table to [function vectors] table
	   and put it in the user space */
	fv->t = *nov+1; /* return number of vectors (plus [t0, V0]) */
	fv->v = vect[1]; /* V0 */
	vp = &vect[0]; /* set to first Number of Steps */
	for ( i = 1; i <= *nov; i++) {
		if (i&1) { /* odd */
			fv[i].t = ((vp[0] * vp[3] * 5))/1000 + dt; /* in ms */
			fv[i].v = vp[2];
			vp += 4;
		} else { /* even */
			fv[i].t = ((vp[0] * vp[1] * 5))/1000 + dt; /* in ms */
			fv[i].v = vp[3];
			vp += 2;
		}
		dt = fv[i].t;
	}

	/* give results to the user */
	if (cdcm_copy_to_user(p.fv, fv, fvsz))
		rc = SYSERR;
 out:
	if (vect) sysfree((char *)vect, sz*4);
	if (fv) sysfree((char *)fv, fvsz);
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
