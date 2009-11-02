#ifndef _CVORB_IOCTL_ACCESS_H_INCLUDE_
#define _CVORB_IOCTL_ACCESS_H_INCLUDE_

#include "./user/CvorbUserDefinedAccess.h"

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
int  CvorbGetWAVE_L(int fd, unsigned long *result);
int  CvorbSetWAVE_L(int fd, unsigned long arg);
int  CvorbGetWAVE_SA(int fd, unsigned long *result);
int  CvorbSetWAVE_SA(int fd, unsigned long arg);
int  CvorbGetREC_CYC(int fd, unsigned long *result);
int  CvorbSetREC_CYC(int fd, unsigned long arg);
int  CvorbGetDAC_CNTL(int fd, unsigned long *result);
int  CvorbSetDAC_CNTL(int fd, unsigned long arg);
int  CvorbGetWindowCH1(int fd, unsigned int elOffs, unsigned int depth, unsigned long *result);
int  CvorbGetCH1(int fd, unsigned long result[7]);
int  CvorbSetWindowCH1(int fd, unsigned int elOffs, unsigned int depth, unsigned long *arg);
int  CvorbSetCH1(int fd, unsigned long arg[7]);
int  CvorbGetWindowCH2(int fd, unsigned int elOffs, unsigned int depth, unsigned long *result);
int  CvorbGetCH2(int fd, unsigned long result[7]);
int  CvorbSetWindowCH2(int fd, unsigned int elOffs, unsigned int depth, unsigned long *arg);
int  CvorbSetCH2(int fd, unsigned long arg[7]);
int  CvorbGetWindowCH3(int fd, unsigned int elOffs, unsigned int depth, unsigned long *result);
int  CvorbGetCH3(int fd, unsigned long result[7]);
int  CvorbSetWindowCH3(int fd, unsigned int elOffs, unsigned int depth, unsigned long *arg);
int  CvorbSetCH3(int fd, unsigned long arg[7]);
int  CvorbGetWindowCH4(int fd, unsigned int elOffs, unsigned int depth, unsigned long *result);
int  CvorbGetCH4(int fd, unsigned long result[7]);
int  CvorbSetWindowCH4(int fd, unsigned int elOffs, unsigned int depth, unsigned long *arg);
int  CvorbSetCH4(int fd, unsigned long arg[7]);
int  CvorbGetWindowCH5(int fd, unsigned int elOffs, unsigned int depth, unsigned long *result);
int  CvorbGetCH5(int fd, unsigned long result[7]);
int  CvorbSetWindowCH5(int fd, unsigned int elOffs, unsigned int depth, unsigned long *arg);
int  CvorbSetCH5(int fd, unsigned long arg[7]);
int  CvorbGetWindowCH6(int fd, unsigned int elOffs, unsigned int depth, unsigned long *result);
int  CvorbGetCH6(int fd, unsigned long result[7]);
int  CvorbSetWindowCH6(int fd, unsigned int elOffs, unsigned int depth, unsigned long *arg);
int  CvorbSetCH6(int fd, unsigned long arg[7]);
int  CvorbGetWindowCH7(int fd, unsigned int elOffs, unsigned int depth, unsigned long *result);
int  CvorbGetCH7(int fd, unsigned long result[7]);
int  CvorbSetWindowCH7(int fd, unsigned int elOffs, unsigned int depth, unsigned long *arg);
int  CvorbSetCH7(int fd, unsigned long arg[7]);
int  CvorbGetWindowCH8(int fd, unsigned int elOffs, unsigned int depth, unsigned long *result);
int  CvorbGetCH8(int fd, unsigned long result[7]);
int  CvorbSetWindowCH8(int fd, unsigned int elOffs, unsigned int depth, unsigned long *arg);
int  CvorbSetCH8(int fd, unsigned long arg[7]);
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
int  CvorbGetWindowALL_CH(int fd, unsigned int elOffs, unsigned int depth, unsigned long *result);
int  CvorbGetALL_CH(int fd, unsigned long result[56]);
int  CvorbSetWindowALL_CH(int fd, unsigned int elOffs, unsigned int depth, unsigned long *arg);
int  CvorbSetALL_CH(int fd, unsigned long arg[56]);

#ifdef __cplusplus
}
#endif

#endif /* _CVORB_IOCTL_ACCESS_H_INCLUDE_ */
