/* ==================================================================== */
/* Implement general stub library for generic devices                   */
/* Julian Lewis Wed 15th April 2009                                     */
/* ==================================================================== */

#ifndef LIBSTUB_P
#define LIBSTUB_P

/* ==================================================================== */
/* The common API that each device library should try to implement      */

typedef struct {
   int (*OpenHandle)(int dev);
   void (*CloseHandle)(int fd);
   int (*ResetMod)(int fd);
   int (*GetAdcValue)(int fd, int chn, int *adc);
   int (*CallibrateChn)(int fd, int chn, int low, int hih);
   int (*GetState)(int fd, Vd80State *ste);
   int (*GetStatus)(int fd, Vd80Status *sts);
   int (*SetDebug)(int fd, Vd80Debug *deb);
   int (*GetDebug)(int fd, Vd80Debug *deb);
   int (*GetDatumSize)(int fd, Vd80DatumSize *dsz);
   int (*GetModuleCount)(int fd, int *cnt);
   int (*GetChannelCount)(int fd, int *cnt);
   int (*GetVersion)(int fd, Vd80Version *ver);
   int (*SetTrigger)(int fd, Vd80Input *inp);
   int (*GetTrigger)(int fd, Vd80Input *inp);
   int (*SetClock)(int fd, Vd80Input *inp);
   int (*GetClock)(int fd, Vd80Input *inp);
   int (*SetStop)(int fd, unsigned int chns, Vd80Input *inp);
   int (*GetStop)(int fd, int chn, Vd80Input *inp);
   int (*SetClockDivide)(int fd, unsigned int dvd);
   int (*GetClockDivide)(int fd, unsigned int *dvd);
   int (*StrtAcquisition)(int fd);
   int (*TrigAcquisition)(int fd);
   int (*ReadAcquisition)(int fd);
   int (*StopAcquisition)(int fd);
   int (*Connect)(int fd, unsigned int imsk);
   int (*SetTimeout)(int fd, int tmo);
   int (*GetTimeout)(int fd, int *tmo);
   int (*Wait)(int fd, Vd80Intr *intr);
   int (*SimulateInterrupt)(int fd, Vd80Intr *intr);
   int (*SetBufferSize)(int fd, int post);
   int (*GetBuffer)(int fd, int chn, Vd80Buffer *buf);
   int (*SetTriggerLevels)(int fd, unsigned int chns, Vd80AnalogTrig *atrg);
   int (*GetTriggerLevels)(int fd, int chn, Vd80AnalogTrig *atrg);
   int (*SetTriggerConfig)(int fd, Vd80TrigConfig *ctrg);
   int (*GetTriggerConfig)(int fd, Vd80TrigConfig *ctrg);
 } Vd80API;

/* ==================================================================== */
/* Private definition for handle.                                       */

#define Vd80MAGIC 0xCE8A4954
#define Vd80MAX_DEVICES 16

typedef struct {
   int           Magic;                   /* Magic number for a real handle */
   Vd80DatumSize DSize;                   /* Datum size */
   void         *DllSo;                   /* Handle for shared object */
   int           FD[Vd80MAX_DEVICES];     /* Users file descriptor from driver */
   char          Err[Vd80ErrSTRING_SIZE]; /* Users last error text */
   Vd80API       API;                     /* API pointers */
   int           BSze;                    /* Buffer Size */
   int           Post;                    /* Post trigger samples */
 } Handle;

#endif
