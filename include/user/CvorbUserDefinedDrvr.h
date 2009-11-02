#ifndef _CVORB_USER_DEFINED_DRVR_H_INCLUDE_
#define _CVORB_USER_DEFINED_DRVR_H_INCLUDE_

#include "CvorbDrvr.h"

#ifdef __LYNXOS
#include <dg/port_ops_lynx.h>
#elif defined(__linux__) && defined(__KERNEL__)
#include <dg/port_ops_linux.h>
#endif /* __LYNXOS */

/* for kernel only */
#if defined(__LYNXOS) || defined(__KERNEL__)
struct sel; /* preliminary structure declaration to supress warnings during
	       user code compilation */

typedef CVORBBlock09_t chd;
typedef CVORBBlock00_t mod;

/**
 * @brief Description of each module
 *
 * Each module consists of 8 channels, and has general registers.
 * md -- generic module registers
 * cd -- channel registers
 *
 */
struct cvorb_module {
	mod *md;
	chd *cd[8];
};

/* user-defined statics data table for CVORB module */
struct CVORBUserStatics_t {
	struct cvorb_module md[2];
};

/** @defgroup Low level r/w operations
 *
 *@{
 */
/* read register (_rr), read channel register (_rcr),
   read channel register repetitive (_rcrr), repetitive read register (_rrr)
   m -- module [0, 1]
   c -- channel [0 - 7]
   r -- register to read
   b -- buffer to put results into
   t -- times to read (counter)
 */
#define _rcr(m, c, r)							\
	({								\
		/*udelay(5);*/						\
		/*kkprintf("\n_rcr @ 0x%x\n", (uint)&(usp->md[m].cd[c]->r));*/ \
		__inl((__port_addr_t)&(usp->md[m].cd[c]->r));	\
	})

#define _rr(m, r)							\
	({								\
		/*udelay(5);*/						\
		/*kkprintf("\n_rr @ 0x%x\n", (uint)&(usp->md[m].md->r));*/ \
		__inl((__port_addr_t)&(usp->md[m].md->r));		\
	})

#define _rcrr(m, c, r, b, t)						\
	({								\
		/*udelay(5);*/						\
		/*kkprintf("\n_rcrr @ 0x%x\n", (uint)&(usp->md[m].cd[c]->r));*/	\
		__rep_inl((__port_addr_t)&(usp->md[m].cd[c]->r), b, t); \
	})

#define _rrr(m, r, b, t)						\
	({								\
		/*udelay(5);*/						\
		/*kkprintf("\n_rrr @ 0x%x\n", (uint)&(usp->md[m].md->r));*/ \
		__rep_inl((__port_addr_t)&(usp->md[m].md->r), b, t); \
	})

/* write register (_wr),  write channel register (_wcr),
   write channel register repetitive (_wcrr), write register repetitive (_wrr)
   m -- module [0, 1]
   c -- channel [0 - 7]
   r -- register to write
   v -- value to write into hw
   b -- buffer to get data from
   t -- times to write (counter)
 */
#define _wcr(m, c, r, v)						\
	({								\
		/*udelay(5);*/						\
		/*kkprintf("\n_wcr @ 0x%x\n", (uint)&(usp->md[m].cd[c]->r));*/ \
		__outl((__port_addr_t)&(usp->md[m].cd[c]->r), v);	\
	})

#define _wr(m, r, v)							\
	({								\
		/*udelay(5);*/						\
		/*kkprintf("\n_wr @ 0x%x\n", (uint)&(usp->md[m].md->r));*/ \
		__outl((__port_addr_t)&(usp->md[m].md->r), v);	\
	})

#define _wcrr(m, c, r, b, t)						\
	({								\
		/*udelay(5);*/						\
		/*kkprintf("\n_wcrr @ 0x%x\n", (uint)&(usp->md[m].cd[c]->r));*/ \
		__rep_outl((__port_addr_t)&(usp->md[m].cd[c]->r), b, t); \
	})

#define _wrr(m, r, b, t)						\
	({								\
		/*udelay(5);*/						\
		/*kkprintf("\n_wrr @ 0x%x\n", (uint)&(usp->md[m].md->r));*/ \
		__rep_outl((__port_addr_t)&(usp->md[m].md->r), b, t); \
	})
/*@} end of group*/

int CvorbUserOpen(int*, register CVORBStatics_t*, int, struct file*);
int CvorbUserClose(int*, register CVORBStatics_t*, struct file*);
int CvorbUserRead(int*, register CVORBStatics_t*, struct file*, char*, int);
int CvorbUserWrite(int*, register CVORBStatics_t*, struct file*, char*, int);
int CvorbUserSelect(int*, register CVORBStatics_t*, struct file*, int, struct sel*);
int CvorbUserIoctl(int*, register CVORBStatics_t*, struct file*, int, int, char*);
char* CvorbUserInst(int*, register DevInfo_t*, register CVORBStatics_t*);
int CvorbUserUnInst(int*, CVORBStatics_t*);
#endif /* __LYNXOS || __KERNEL__ */

#endif /* _CVORB_USER_DEFINED_DRVR_H_INCLUDE_ */
