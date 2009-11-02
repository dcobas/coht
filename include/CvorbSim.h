#ifndef _CVORB_SIM_H_INCLUDE_
#define _CVORB_SIM_H_INCLUDE_

#ifdef __LYNXOS
/* only for kernel */
#include <dldd.h>
#include <kernel.h>
#include <kern_proto.h>

#include <errno.h>
#include <sys/types.h>
#include <conf.h>
#include <io.h>
#include <sys/ioctl.h>
#include <time.h>
#include <param.h>
#include <string.h>
#include <proto.h>
#include <proto_int.h>
#include <fcntl.h>
#include <dir.h>
#include <string.h>
#include <unistd.h>
#include <param.h>
#include <sys/stat.h>
#include <stdio.h>
#include <signal.h>
#include <sys/file.h>
#include <termio.h>
#include <termios.h>
#include <ces/vmelib.h>	  /* for 'pdparam_xxx' structure */
#include <inode.h>

/* system thread */
#include <conf.h>
#include <st.h>
#endif	/* __LYNXOS */

#include "dg/vme_am.h" /* for VMEbus Address Modifier Codes */
#include "dg/ServiceRegIoctl.h"
#include "dg/ModuleHeader.h"

#if defined(__linux__) && defined (__KERNEL__)
#undef sel
#define sel poll_table_struct	/* for select entry point */
/* only for Linux kernel */
#include <cdcm/cdcm.h>
#endif

/* Provides debug information printing. */
#define DBG_BEG(f_nm)							\
do {									\
  if (s->info->debugFlag & DBGIOCTL) {					\
    swait(&sem_drvr, SEM_SIGIGNORE); /* lock calling thread */		\
    cprintf("(pid %d) (tid %d) %s() ", getpid(), st_getstid(), f_nm);	\
  }									\
} while (0)

/* information about the address that will be read/write */
#define DBG_ADDR(ra, idx, en)					   \
do {								   \
  if (s->info->debugFlag & DBGIOCTL) {				   \
    unsigned addr = (unsigned)ra;				   \
    cprintf("[(elIdx %d) (elAm %d) @ 0x%x] ", idx, (int)en, addr); \
  }								   \
} while (0)

/* ioctl completion tag */
#define DBG_OK()							\
do {									\
	if (s->info->debugFlag & DBGIOCTL) {				\
		cprintf(" - OK.\n");					\
		ssignal(&sem_drvr);	/* wake up waiting thread */	\
	}								\
 } while (0)

/* ioctl completion tag */
#define DBG_FAIL()					\
do {							\
  if (s->info->debugFlag & DBGIOCTL) {			\
    cprintf(" - FAIL.\n");				\
    ssignal(&sem_drvr);	/* wake up waiting thread */	\
  }							\
} while (0)

/* start timing measurements. Different in case of Lynx/Linux */
#ifdef __Lynx__

#define START_TIME_STAT()								\
do {											\
  if (s->info->debugFlag & DBGTIMESTAT) {						\
    /* because of the spatial locality reference phenomenon, fill in cache		\
       memory so that it will keep recently accessed values (in our case		\
       it is 'nanotime' sys function stuff), thus trying to avoid additional		\
       time loss induced by cache miss.							\
       Normally, '__builtin_prefetch' gcc function should be used to minimize		\
       cache-miss latency by moving data into a cache before it is accessed.		\
       But with currently used Lynx gcc (version 2.95.3 20010323 (Lynx) as		\
       of 24.04.06) it's not possible, as it's probably not supported.			\
       For more info see gcc 4.0.1 reference manual at:					\
       http://gcc.gnu.org/onlinedocs/gcc-4.0.1/gcc/Other-Builtins.html#Other-Builtins	\
       So let's wait until newer gcc will be installed and then try the new		\
       approach. */									\
    GlobStartN = nanotime(&GlobStartS);							\
											\
    /* wait for the better times to implement... */					\
    /*__builtin_prefetch(&GlobStartN, 1, 1); */						\
											\
    /* now we can start real timing measurements */					\
    GlobStartN = nanotime(&GlobStartS);							\
  }											\
} while (0)

#else  /* linux */

#define START_TIME_STAT() \
	if (s->info->debugFlag & DBGTIMESTAT) getnstimeofday(&_t_beg)

#endif /* defined __Lynx__ */

/* printout measured time statistics */
#define FINISH_TIME_STAT()						\
do {									\
	if (s->info->debugFlag & DBGTIMESTAT) {				\
		unsigned int retN; /* for timing measurements */	\
		retN = ComputeTime(GlobTimeStatS);			\
		if (retN == -1)						\
			cprintf("Done in %s\n", GlobTimeStatS);		\
		else							\
			cprintf("Done in %dns (%s)\n", retN, GlobTimeStatS); \
	}								\
 } while (0)


#define CVORB_MAX_NUM_CARDS 21 /* VME crates have 21 slots max */

/* CVORB module ioctl numbers */
#define CVORB_IOCTL_MAGIC 'C'

#define CVORB_IO(nr)      _IO(CVORB_IOCTL_MAGIC, nr)
#define CVORB_IOR(nr,sz)  _IOR(CVORB_IOCTL_MAGIC, nr, sz)
#define CVORB_IOW(nr,sz)  _IOW(CVORB_IOCTL_MAGIC, nr, sz)
#define CVORB_IOWR(nr,sz) _IOWR(CVORB_IOCTL_MAGIC, nr, sz)

#define CVORB_GET_INT_SRC (_FIRST__IOCTL_ + 0)
#define CVORB_GET_INT_EN (_FIRST__IOCTL_ + 1)
#define CVORB_SET_INT_EN (_FIRST__IOCTL_ + 2)
#define CVORB_GET_INT_L (_FIRST__IOCTL_ + 3)
#define CVORB_SET_INT_L (_FIRST__IOCTL_ + 4)
#define CVORB_GET_INT_V (_FIRST__IOCTL_ + 5)
#define CVORB_SET_INT_V (_FIRST__IOCTL_ + 6)
#define CVORB_GET_VHDL_V (_FIRST__IOCTL_ + 7)
#define CVORB_GET_PCB_SN_H (_FIRST__IOCTL_ + 8)
#define CVORB_GET_PCB_SN_L (_FIRST__IOCTL_ + 9)
#define CVORB_GET_TEMP (_FIRST__IOCTL_ + 10)
#define CVORB_GET_ADC (_FIRST__IOCTL_ + 11)
#define CVORB_GET_HISTORY_SOFT_PULSE (_FIRST__IOCTL_ + 12)
#define CVORB_SET_SOFT_PULSE (_FIRST__IOCTL_ + 13)
#define CVORB_GET_EXT_CLK_FREQ (_FIRST__IOCTL_ + 14)
#define CVORB_GET_CLK_GEN_CNTL (_FIRST__IOCTL_ + 15)
#define CVORB_SET_CLK_GEN_CNTL (_FIRST__IOCTL_ + 16)
#define CVORB_GET_MOD_STAT (_FIRST__IOCTL_ + 17)
#define CVORB_GET_MOD_CFG (_FIRST__IOCTL_ + 18)
#define CVORB_SET_MOD_CFG (_FIRST__IOCTL_ + 19)
#define CVORB_GET_DAC_VAL (_FIRST__IOCTL_ + 20)
#define CVORB_SET_DAC_VAL (_FIRST__IOCTL_ + 21)
#define CVORB_GET_SRAM_SA (_FIRST__IOCTL_ + 22)
#define CVORB_SET_SRAM_SA (_FIRST__IOCTL_ + 23)
#define CVORB_GET_SRAM_DATA (_FIRST__IOCTL_ + 24)
#define CVORB_SET_SRAM_DATA (_FIRST__IOCTL_ + 25)
#define CVORB_GET_WAVE_L (_FIRST__IOCTL_ + 26)
#define CVORB_SET_WAVE_L (_FIRST__IOCTL_ + 27)
#define CVORB_GET_WAVE_SA (_FIRST__IOCTL_ + 28)
#define CVORB_SET_WAVE_SA (_FIRST__IOCTL_ + 29)
#define CVORB_GET_REC_CYC (_FIRST__IOCTL_ + 30)
#define CVORB_SET_REC_CYC (_FIRST__IOCTL_ + 31)
#define CVORB_GET_DAC_CNTL (_FIRST__IOCTL_ + 32)
#define CVORB_SET_DAC_CNTL (_FIRST__IOCTL_ + 33)
#define CVORB_GET_CH1 (_FIRST__IOCTL_ + 34)
#define CVORB_SET_CH1 (_FIRST__IOCTL_ + 35)
#define CVORB_GET_CH2 (_FIRST__IOCTL_ + 36)
#define CVORB_SET_CH2 (_FIRST__IOCTL_ + 37)
#define CVORB_GET_CH3 (_FIRST__IOCTL_ + 38)
#define CVORB_SET_CH3 (_FIRST__IOCTL_ + 39)
#define CVORB_GET_CH4 (_FIRST__IOCTL_ + 40)
#define CVORB_SET_CH4 (_FIRST__IOCTL_ + 41)
#define CVORB_GET_CH5 (_FIRST__IOCTL_ + 42)
#define CVORB_SET_CH5 (_FIRST__IOCTL_ + 43)
#define CVORB_GET_CH6 (_FIRST__IOCTL_ + 44)
#define CVORB_SET_CH6 (_FIRST__IOCTL_ + 45)
#define CVORB_GET_CH7 (_FIRST__IOCTL_ + 46)
#define CVORB_SET_CH7 (_FIRST__IOCTL_ + 47)
#define CVORB_GET_CH8 (_FIRST__IOCTL_ + 48)
#define CVORB_SET_CH8 (_FIRST__IOCTL_ + 49)
#define CVORB_GET_CH_STAT (_FIRST__IOCTL_ + 50)
#define CVORB_GET_CH_CFG (_FIRST__IOCTL_ + 51)
#define CVORB_SET_CH_CFG (_FIRST__IOCTL_ + 52)
#define CVORB_GET_FUNC_SEL (_FIRST__IOCTL_ + 53)
#define CVORB_SET_FUNC_SEL (_FIRST__IOCTL_ + 54)
#define CVORB_GET_FCT_EM_H (_FIRST__IOCTL_ + 55)
#define CVORB_SET_FCT_EM_H (_FIRST__IOCTL_ + 56)
#define CVORB_GET_FCT_EM_L (_FIRST__IOCTL_ + 57)
#define CVORB_SET_FCT_EM_L (_FIRST__IOCTL_ + 58)
#define CVORB_GET_SLOPE (_FIRST__IOCTL_ + 59)
#define CVORB_SET_SLOPE (_FIRST__IOCTL_ + 60)
#define CVORB_GET_CH_REC_CYC (_FIRST__IOCTL_ + 61)
#define CVORB_SET_CH_REC_CYC (_FIRST__IOCTL_ + 62)
#define CVORB_GET_ALL_CH (_FIRST__IOCTL_ + 63)
#define CVORB_SET_ALL_CH (_FIRST__IOCTL_ + 64)

/* First allowed number for user-defined ioctl */
#define CVORB_FIRST_USR_IOCTL (_FIRST__IOCTL_ + 65)

/* keeps last written value of the 'write only' registers */
typedef  struct {
  /* last written value */ unsigned long SOFT_PULSE;
} CVORBWriteonly_t;


/* user-defined extraneous registers */
typedef  struct {
} CVORBExtraneous_t;


/* Blk[#0]@addr[#1] Offs 0x0. Sz 84 bytes. 21 reg(s). 1 gap(s) */
typedef volatile struct {
  /* 0x0 */ unsigned long INT_SRC;
  /* 0x4 */ unsigned long INT_EN;
  /* 0x8 */ unsigned long INT_L;
  /* 0xc */ unsigned long INT_V;
  /* 0x10 */ unsigned long VHDL_V;
  /* 0x14 */ unsigned long PCB_SN_H;
  /* 0x18 */ unsigned long PCB_SN_L;
  /* 0x1c */ unsigned long TEMP;
  /* 0x20 */ unsigned long ADC;
  /* 0x24 */ unsigned long SOFT_PULSE;
  /* 0x28 */ unsigned long EXT_CLK_FREQ;
  /* 0x2c */ unsigned long CLK_GEN_CNTL;
  /* 0x40 */ unsigned long MOD_STAT;
  /* 0x44 */ unsigned long MOD_CFG;
  /* 0x48 */ unsigned long DAC_VAL;
  /* 0x4c */ unsigned long SRAM_SA;
  /* 0x50 */ unsigned long SRAM_DATA;
  /* 0x54 */ unsigned long WAVE_L;
  /* 0x58 */ unsigned long WAVE_SA;
  /* 0x5c */ unsigned long REC_CYC;
  /* 0x60 */ unsigned long DAC_CNTL;
} CVORBBlock00_t;

/* Blk[#1]@addr[#1] Offs 0x80. Sz 28 bytes. 1 reg(s). 0 gap(s) */
typedef volatile struct {
  /* [0x0 - 0x18] */ unsigned long CH1[7];
} CVORBBlock01_t;

/* Blk[#2]@addr[#1] Offs 0xb0. Sz 28 bytes. 1 reg(s). 0 gap(s) */
typedef volatile struct {
  /* [0x0 - 0x18] */ unsigned long CH2[7];
} CVORBBlock02_t;

/* Blk[#3]@addr[#1] Offs 0xe0. Sz 28 bytes. 1 reg(s). 0 gap(s) */
typedef volatile struct {
  /* [0x0 - 0x18] */ unsigned long CH3[7];
} CVORBBlock03_t;

/* Blk[#4]@addr[#1] Offs 0x110. Sz 28 bytes. 1 reg(s). 0 gap(s) */
typedef volatile struct {
  /* [0x0 - 0x18] */ unsigned long CH4[7];
} CVORBBlock04_t;

/* Blk[#5]@addr[#1] Offs 0x140. Sz 28 bytes. 1 reg(s). 0 gap(s) */
typedef volatile struct {
  /* [0x0 - 0x18] */ unsigned long CH5[7];
} CVORBBlock05_t;

/* Blk[#6]@addr[#1] Offs 0x170. Sz 28 bytes. 1 reg(s). 0 gap(s) */
typedef volatile struct {
  /* [0x0 - 0x18] */ unsigned long CH6[7];
} CVORBBlock06_t;

/* Blk[#7]@addr[#1] Offs 0x1a0. Sz 28 bytes. 1 reg(s). 0 gap(s) */
typedef volatile struct {
  /* [0x0 - 0x18] */ unsigned long CH7[7];
} CVORBBlock07_t;

/* Blk[#8]@addr[#1] Offs 0xd0. Sz 28 bytes. 1 reg(s). 0 gap(s) */
typedef volatile struct {
  /* [0x0 - 0x18] */ unsigned long CH8[7];
} CVORBBlock08_t;

/* Blk[#9]@addr[#1] Offs 0x80. Sz 28 bytes. 7 reg(s). 0 gap(s) */
typedef volatile struct {
  /* 0x0 */ unsigned long CH_STAT;
  /* 0x4 */ unsigned long CH_CFG;
  /* 0x8 */ unsigned long FUNC_SEL;
  /* 0xc */ unsigned long FCT_EM_H;
  /* 0x10 */ unsigned long FCT_EM_L;
  /* 0x14 */ unsigned long SLOPE;
  /* 0x18 */ unsigned long CH_REC_CYC;
} CVORBBlock09_t;

/* Blk[#10]@addr[#1] Offs 0x80. Sz 224 bytes. 1 reg(s). 0 gap(s) */
typedef volatile struct {
  /* [0x0 - 0xdc] */ unsigned long ALL_CH[56];
} CVORBBlock10_t;

/* CVORB module master topology */
typedef struct {
  CVORBWriteonly_t  *wo; /* write only registers last history */
  CVORBExtraneous_t *ex; /* extraneous registers */
  CVORBBlock00_t *block00;
  CVORBBlock01_t *block01;
  CVORBBlock02_t *block02;
  CVORBBlock03_t *block03;
  CVORBBlock04_t *block04;
  CVORBBlock05_t *block05;
  CVORBBlock06_t *block06;
  CVORBBlock07_t *block07;
  CVORBBlock08_t *block08;
  CVORBBlock09_t *block09;
  CVORBBlock10_t *block10;
} CVORBTopology_t;

/* compiler should know variable type. So get rid of incomplete type */
typedef struct CVORBUserStatics_t CVORBUserStatics_t;

/* CVORB statics data table */
typedef struct {
  CVORBTopology_t *card; /* hardware module topology */
  DevInfo_t *info; /* device information table */
  CVORBUserStatics_t *usrst; /* user-defined statics data table */
} CVORBStatics_t;

#endif /* _CVORB_SIM_H_INCLUDE_ */
