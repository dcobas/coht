#ifndef _CVORB_IOCTL_ACCESS_H_INCLUDE_
#define _CVORB_IOCTL_ACCESS_H_INCLUDE_

#include "CvorbUserDefinedAccess.h"

/* this API is obsolete! */
#warning WARNING! Deprecated library. Use DrvrAccess library instead.

#ifdef __cplusplus
extern "C" {
#endif

/* see CvorbIoctlAccess.c for precise parameter description */

int  CvorbEnableAccess(int, int); /* open  Device driver */
void CvorbDisableAccess(int);     /* close Device driver */

int  CvorbGetINT_SRC(int fd, unsigned long *result);
int  CvorbGetINT_EN(int fd, unsigned long *result);
int  CvorbSetINT_EN(int fd, unsigned long arg);
int  CvorbGetINT_L(int fd, unsigned long *result);
int  CvorbSetINT_L(int fd, unsigned long arg);
int  CvorbGetINT_V(int fd, unsigned long *result);
int  CvorbSetINT_V(int fd, unsigned long arg);
int  CvorbGetVHDL_V(int fd, unsigned long *result);
int  CvorbGetPCB_SN_H(int fd, unsigned long *result);
int  CvorbGetPCB_SN_L(int fd, unsigned long *result);
int  CvorbGetTEMP(int fd, unsigned long *result);
int  CvorbGetADC(int fd, unsigned long *result);
int  CvorbGetLastHisSOFT_PULSE(int fd, unsigned long *result);
int  CvorbSetSOFT_PULSE(int fd, unsigned long arg);
int  CvorbGetEXT_CLK_FREQ(int fd, unsigned long *result);
int  CvorbGetCLK_GEN_CNTL(int fd, unsigned long *result);
int  CvorbSetCLK_GEN_CNTL(int fd, unsigned long arg);
int  CvorbGetMOD_STAT(int fd, unsigned long *result);
int  CvorbGetMOD_CFG(int fd, unsigned long *result);
int  CvorbSetMOD_CFG(int fd, unsigned long arg);
int  CvorbGetDAC_VAL(int fd, unsigned long *result);
int  CvorbSetDAC_VAL(int fd, unsigned long arg);
int  CvorbGetSRAM_SA(int fd, unsigned long *result);
int  CvorbSetSRAM_SA(int fd, unsigned long arg);
int  CvorbGetSRAM_DATA(int fd, unsigned long *result);
int  CvorbSetSRAM_DATA(int fd, unsigned long arg);
int  CvorbGetDAC_CNTL(int fd, unsigned long *result);
int  CvorbSetDAC_CNTL(int fd, unsigned long arg);
int  CvorbGetCH_STAT(int fd, unsigned long *result);
int  CvorbGetCH_CFG(int fd, unsigned long *result);
int  CvorbSetCH_CFG(int fd, unsigned long arg);
int  CvorbGetFUNC_SEL(int fd, unsigned long *result);
int  CvorbSetFUNC_SEL(int fd, unsigned long arg);
int  CvorbGetFCT_EM_H(int fd, unsigned long *result);
int  CvorbSetFCT_EM_H(int fd, unsigned long arg);
int  CvorbGetFCT_EM_L(int fd, unsigned long *result);
int  CvorbSetFCT_EM_L(int fd, unsigned long arg);
int  CvorbGetSLOPE(int fd, unsigned long *result);
int  CvorbSetSLOPE(int fd, unsigned long arg);
int  CvorbGetCH_REC_CYC(int fd, unsigned long *result);
int  CvorbSetCH_REC_CYC(int fd, unsigned long arg);

#ifdef __cplusplus
}
#endif

#endif /* _CVORB_IOCTL_ACCESS_H_INCLUDE_ */
