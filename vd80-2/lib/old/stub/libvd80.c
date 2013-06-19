/* ==================================================================== */
/* Implement general vd80 library stub                                  */
/* Julian Lewis Tue 23rd April 2013                                     */
/* ==================================================================== */

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sched.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <dlfcn.h>

#include <libvd80.h>
#include <libvd80P.h>

/**
 * Defined in the makefile we use the following variables
 * LDVER The version of libvd80.a stub library
 * SOVER The version of libvd80.so shared object library
 * SUBVER The sub version of libvd80.so shared object library
 */

#ifndef LDVER
#define LDVER 2.0
#endif

#ifndef SOVER
#define SOVER 2.0
#endif

#ifndef SUBVER
#define SUBVER 1
#endif

#define SO_PATH_DEFAULT "/usr/local/vd80"

void swab(const void *from, void *to, ssize_t n);

/* ==================================================================== */
/* Device and Error string static texts                                 */

static char *ErrTexts[Vd80ERRORS] = {
	"All went OK, No error                      ",  // Vd80ErrSUCCESS,
	"Function is not implemented on this device ",  // Vd80ErrNOT_IMP,
	"Invalid Start value for this device        ",  // Vd80ErrSTART,
	"Invalid Mode  value for this device        ",  // Vd80ErrMODE,
	"Invalid Clock value for this device        ",  // Vd80ErrCLOCK,
	"Can't open a driver file handle, fatal     ",  // Vd80ErrOPEN,
	"Can't connect to that interrupt            ",  // Vd80ErrCONNECT,
	"No connections to wait for                 ",  // Vd80ErrWAIT,
	"Timeout in wait                            ",  // Vd80ErrTIMEOUT,
	"Queue flag must be set 0 queueing is needed",  // Vd80ErrQFLAG,
	"IO or BUS error                            ",  // Vd80ErrIO,
	"Module is not enabled                      ",  // Vd80ErrNOT_ENAB
	"Not available for INTERNAL sources         ",  // Vd80ErrSOURCE,
	"Value out of range                         ",  // Vd80ErrVOR,
	"Bad device type                            ",  // Vd80ErrDEVICE,
	"Bad address                                ",  // Vd80ErrADDRESS,
	"Can not allocate memory                    ",  // Vd80ErrMEMORY,
	"Shared library error, can't open object    ",  // Vd80ErrDLOPEN,
	"Shared library error, can't find symbol    ",  // Vd80ErrDLSYM,
	"Can't open sampler device driver           ",  // Vd80ErrDROPEN,
	"Invalid handle                             ",  // Vd80ErrHANDLE,
	"Invalid module number, not installed       ",  // Vd80ErrMODULE,
	"Invalid channel number for this module     "   // Vd80ErrCHANNEL,
 };

/* ==================================================================== */
/* If library dosn't implement a function use this.                     */
/* Make sure there are enough argument places to avoid stack problems.  */

int NotImplemented()
{
	return Vd80ErrNOT_IMP;
}

/* ==================================================================== */
/* Bind a symbol to a function                                          */

int BindSymbol(Handle *hand, void **func, char *symb)
{
	char psym[64];
	void *Ni = NotImplemented;

	sprintf(psym,"vd80%s",symb);

	*func = dlsym(hand->DllSo,psym);
	if (*func == NULL) {
		*func = Ni;
		return 0;
	}
	return 1;
}

/* ==================================================================== */
/* Initialize the callers API                                           */
/* These routines MUST be instantiated, they are allowed to return the  */
/* error NOT-IMPLEMENTED.                                               */

Vd80Err InitAPI(Handle *hand)
{
	BindSymbol(hand,(void **) &hand->API.CloseHandle,"CloseHandle");
	BindSymbol(hand,(void **) &hand->API.ResetMod,"ResetMod");
	BindSymbol(hand,(void **) &hand->API.GetAdcValue,"GetAdcValue");
	BindSymbol(hand,(void **) &hand->API.CallibrateChn,"CallibrateChn");
	BindSymbol(hand,(void **) &hand->API.GetState,"GetState");
	BindSymbol(hand,(void **) &hand->API.GetStatus,"GetStatus");
	BindSymbol(hand,(void **) &hand->API.GetDatumSize,"GetDatumSize");
	BindSymbol(hand,(void **) &hand->API.SetDebug,"SetDebug");
	BindSymbol(hand,(void **) &hand->API.GetDebug,"GetDebug");
	BindSymbol(hand,(void **) &hand->API.GetModuleCount,"GetModuleCount");
	BindSymbol(hand,(void **) &hand->API.GetChannelCount,"GetChannelCount");
	BindSymbol(hand,(void **) &hand->API.GetVersion,"GetVersion");
	BindSymbol(hand,(void **) &hand->API.SetTrigger,"SetTrigger");
	BindSymbol(hand,(void **) &hand->API.GetTrigger,"GetTrigger");
	BindSymbol(hand,(void **) &hand->API.SetClock,"SetClock");
	BindSymbol(hand,(void **) &hand->API.GetClock,"GetClock");
	BindSymbol(hand,(void **) &hand->API.SetStop,"SetStop");
	BindSymbol(hand,(void **) &hand->API.GetStop,"GetStop");
	BindSymbol(hand,(void **) &hand->API.SetClockDivide,"SetClockDivide");
	BindSymbol(hand,(void **) &hand->API.GetClockDivide,"GetClockDivide");
	BindSymbol(hand,(void **) &hand->API.StrtAcquisition,"StrtAcquisition");
	BindSymbol(hand,(void **) &hand->API.TrigAcquisition,"TrigAcquisition");
	BindSymbol(hand,(void **) &hand->API.StopAcquisition,"StopAcquisition");
	BindSymbol(hand,(void **) &hand->API.Connect,"Connect");
	BindSymbol(hand,(void **) &hand->API.SetTimeout,"SetTimeout");
	BindSymbol(hand,(void **) &hand->API.GetTimeout,"GetTimeout");
	BindSymbol(hand,(void **) &hand->API.Wait,"Wait");
	BindSymbol(hand,(void **) &hand->API.SimulateInterrupt,"SimulateInterrupt");
	BindSymbol(hand,(void **) &hand->API.SetBufferSize,"SetBufferSize");
	BindSymbol(hand,(void **) &hand->API.GetBuffer,"GetBuffer");
	BindSymbol(hand,(void **) &hand->API.SetTriggerLevels,"SetTriggerLevels");
	BindSymbol(hand,(void **) &hand->API.GetTriggerLevels,"GetTriggerLevels");
	BindSymbol(hand,(void **) &hand->API.SetTriggerConfig,"SetTriggerConfig");
	BindSymbol(hand,(void **) &hand->API.GetTriggerConfig,"GetTriggerConfig");
	return Vd80ErrSUCCESS;
}

/* ==================================================================== */
/* Round up to the nearest chunk. The DMA mechanism of some hardware is */
/* in minimum sized chunks. This routine rounds up to chunk size.       */
/* Units are in datum size, I.E. samples.                               */

#define CHUNK 32

int GetChunkSize(int sze)
{
	int n, m;

	n = sze/CHUNK;
	m = n * CHUNK;
	if (m == sze) return sze;
	return (n+1) * CHUNK;
}

/* ==================================================================== */
/* Convert an Vd80Error to a string suitable for printing.               */
/* This routine can not return a NULL pointer. If the error is out of   */
/* range it returns a string saying just that. The pointer returned is  */
/* thread safe and there is no need to free it, the allocated memory is */
/* kept in the callers address space.                                   */
/* If the handle pointer is NULL, the error text is kept in static      */
/* memory, and is not thread safe. In this case copy it yourself.       */

char *Vd80ErrToStr(Vd80Handle handle, Vd80Err error)
{
	char         *ep;    /* Error text string pointer */
	char         *cp;    /* Char pointer */
	unsigned int i;
	Handle       *hand;
	static char  err[Vd80ErrSTRING_SIZE]; /* Static text area when null handle */

	i = (unsigned int) error;
	if (i >= Vd80ERRORS) return "Invalid error code";

	hand = (Handle *) handle;
	if (hand) {
		if (hand->Magic != Vd80MAGIC) return "Invalid non void handle";
		ep = hand->Err;
	} else ep = err;

	bzero((void *) ep, Vd80ErrSTRING_SIZE);

	strncpy(ep,ErrTexts[i],(size_t) Vd80ErrSTRING_SIZE);

	/* Extra info available from loader */

	if ((error == Vd80ErrDLOPEN)
	||  (error == Vd80ErrDLSYM)) {
		cp = dlerror();
		if (cp) {
			strcat(ep,": ");
			i = Vd80ErrSTRING_SIZE - strlen(ep) -1;
			strncat(ep,cp,i);
		}
	}

	/* Extra info available from errno */

	if ((error == Vd80ErrIO)
	||  (error == Vd80ErrDROPEN)
	||  (error == Vd80ErrMEMORY)) {
		cp = strerror(errno);
		if (cp) {
			strcat(ep,": ");
			i = Vd80ErrSTRING_SIZE - strlen(ep) -1;  /* -1 is 0 terminator byte */
			strncat(ep,cp,i);
		}
	}
	return ep;
}

/* ==================================================================== */
/* Each thread in an application should get a handle and use it in      */
/* all subsequent calls.                                                */
/*                                                                      */
/* Example:                                                             */
/*                                                                      */
/*    SlbHandle my_handle;                                              */
/*    SlbErr    my_error;                                               */
/*                                                                      */
/*    my_error = Vd80OpenHandle(&my_handle,"2.0");                      */
/*    if ( != Vd80ErrSUCCESS) {                                         */
/*       stprintf(stderr,"Fatal: %s\n",Vd80ErrTopStr(NULL, my_error));  */
/*       exit((int) my_error);                                          */
/*    }                                                                 */
/*                                                                      */

Vd80Err Vd80OpenHandle(Vd80Handle *handle, char *version)
{
	Handle *hand;
	char path[64];
	int i;

	if (handle == NULL)
		return Vd80ErrADDRESS;

	hand = (Handle *) malloc(sizeof(Handle));
	if (hand == NULL)
		return Vd80ErrMEMORY;

	*handle = (Vd80Handle) hand;              /* Store the address of the handle */

	bzero((void *) hand, sizeof(Handle));
	hand->Magic = Vd80MAGIC;

	/**
	 * First try to open with the default system path
	 */

	sprintf(path,"libvd80.so.%1.1f",SOVER);
	hand->DllSo = dlopen(path, RTLD_LOCAL | RTLD_LAZY);
	if (hand->DllSo == NULL) {

		/**
		 * If a version is not specified try to open the default symbolic link
		 */

		if ((version == NULL) || (strlen(version) < strlen("2.0")))
			sprintf(path,"%s/libvd80.so.%1.1f",SO_PATH_DEFAULT,SOVER);
		else
			/**
			 * Try for the specified version symbolic link
			 */

			sprintf(path,"%s/libvd80.so.%s",SO_PATH_DEFAULT,version);

		hand->DllSo = dlopen(path, RTLD_LOCAL | RTLD_LAZY);
		if (hand->DllSo == NULL)
			return Vd80ErrDLOPEN;
	}
	if (BindSymbol(hand,(void **) &hand->API.OpenHandle,"OpenHandle") == 0)
		return Vd80ErrDLSYM;

	for (i=0; i<Vd80MAX_DEVICES; i++)
		hand->FD[i] = hand->API.OpenHandle(i);

	return InitAPI(hand);
}

/* ==================================================================== */
/* Close handle                                                         */

Vd80Err Vd80CloseHandle(Vd80Handle handle) {

Handle *hand;
int i;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   for (i=0; i<Vd80MAX_DEVICES; i++)
      if (hand->FD[i] > 0)
	 hand->API.CloseHandle(hand->FD[i]);

   dlclose(hand->DllSo);

   bzero((void *) hand, sizeof(Handle));
   free((void *) hand);

   return Vd80ErrSUCCESS;
}

/* ==================================================================== */
/* Get the Driver file descriptor array to be used within select calls. */

Vd80Err Vd80GetFileDescriptor(Vd80Handle handle, int **fd) {

Handle *hand;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   *fd = hand->FD;

   return Vd80ErrSUCCESS;
}

/* ==================================================================== */
/* Set debug level                                                      */

Vd80Err Vd80SetDebug(Vd80Handle handle, Vd80Debug *deb) {

Handle *hand;
int i;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   for (i=0; i<Vd80MAX_DEVICES; i++)
      if (hand->FD[i] > 0)
	 hand->API.SetDebug(hand->FD[i],deb);

   return Vd80ErrSUCCESS;
}

/* ==================================================================== */
/* Get debug level                                                      */

Vd80Err Vd80GetDebug(Vd80Handle handle, Vd80Debug *deb) {

Handle *hand;
int i;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   for (i=0; i<Vd80MAX_DEVICES; i++)
      if (hand->FD[i] > 0)
	 return hand->API.GetDebug(hand->FD[i],deb);

   return Vd80ErrOPEN;
}

/* ==================================================================== */
/* Get the sampler device name string.                                  */

Vd80Err Vd80GetDeviceName(Vd80Handle handle, char **dname) {

Handle *hand;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   *dname = "vd80";

   return Vd80ErrSUCCESS;
}

/* ==================================================================== */
/* Get the Dll Version string.                                          */

Vd80Err Vd80GetDllVersion(Vd80Handle handle, char **dver)
{
	Handle *hand;
	static char res[32];

	hand = (Handle *) handle;
	if (hand == NULL) return Vd80ErrADDRESS;
	if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

	sprintf(res,"%1.1f",SOVER);
	*dver = res;

	return Vd80ErrSUCCESS;
}

/* ==================================================================== */
/* The address of any library function can be got by name. This can be  */
/* useful if you want to call a device library function directly, for   */
/* example a specific device function.                                  */
/*                                                                      */
/* Example:                                                             */
/*                                                                      */
/* Vd80Err (*my_funct)(int p);                                          */
/* Vd80Err my_error;                                                    */
/*                                                                      */
/*    my_error = SlbGetSymbol(my_handle,"Vd80LibSpecial",&my_funct);    */
/*    if (my_error != Vd80ErrSUCCESS) {                                 */
/*       stprintf(stderr,"Fatal: %s\n",Vd80ErrTopStr(NULL, my_error));  */
/*       exit((int) my_error);                                          */
/*    }                                                                 */
/*                                                                      */
/*    my_error = my_funct(1); ...                                       */

Vd80Err Vd80GetSymbol(Vd80Handle handle, char *name, void **func)
{
	Handle *hand;

	hand = (Handle *) handle;
	if (hand == NULL) return Vd80ErrADDRESS;
	if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

	*func = dlsym(hand->DllSo,name);
	if (*func == NULL) return Vd80ErrDLSYM;

	return Vd80ErrSUCCESS;
}

/* ==================================================================== */
/* Reset is the equivalent to a VME bus reset. All settings and data on */
/* the module will be lost.                                             */

Vd80Err Vd80ResetMod(Vd80Handle handle, int module) {

Handle *hand;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   return hand->API.ResetMod(hand->FD[module -1]);
}

/* ==================================================================== */
/* Calibrate requires that zero and Max volts are applied to an input   */
/* and the Adc value then read back. Once done the values obtained can  */
/* be used to calibrate the module.                                     */

Vd80Err Vd80GetAdcValue(Vd80Handle handle, int module, int channel, int *adc) {

Handle *hand;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   return hand->API.GetAdcValue(hand->FD[module -1],channel,adc);
}

/* ==================================================================== */
/* Callibrate a channels ADC                                            */

Vd80Err Vd80CalibrateChn(Vd80Handle handle, int module, int channel, int low, int high) {

Handle *hand;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   return hand->API.CallibrateChn(hand->FD[module -1],channel,low,high);
}

/* ==================================================================== */
/* A channel has a state, the state must be IDLE to read back data.     */

Vd80Err Vd80GetState(Vd80Handle handle, int module, int channel, Vd80State *state) {

Handle *hand;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   return hand->API.GetState(hand->FD[module -1],state);
}

/* ==================================================================== */
/* Mod status indicates hardware failures, enable state etc.            */

Vd80Err Vd80GetStatus(Vd80Handle handle, int module, Vd80Status *status) {

Handle *hand;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   return hand->API.GetStatus(hand->FD[module -1],status);
}

/* ==================================================================== */
/* This is a fixed value for each device type.                          */

Vd80Err Vd80GetDatumSize(Vd80Handle handle, Vd80DatumSize *datsze) {

Handle *hand;
int i;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   for (i=0; i<Vd80MAX_DEVICES; i++)
      if (hand->FD[i] > 0)
	 return hand->API.GetDatumSize(hand->FD[i],datsze);

   return Vd80ErrOPEN;
}

/* ==================================================================== */
/* Discover how many modules and channels are installed.                */

Vd80Err Vd80GetModCount(Vd80Handle handle, int *count) {

Handle *hand;
int i;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   for (i=0; i<Vd80MAX_DEVICES; i++)
      if (hand->FD[i] > 0)
	 return hand->API.GetModuleCount(hand->FD[i],count);

   return Vd80ErrOPEN;
}

/* ==================================================================== */
/* Discover how many modules and channels are installed.                */

Vd80Err Vd80GetChnCount(Vd80Handle handle, int *count) {

Handle *hand;
int i;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   for (i=0; i<Vd80MAX_DEVICES; i++)
      if (hand->FD[i] > 0)
	 return hand->API.GetChannelCount(hand->FD[i],count);

   return Vd80ErrOPEN;
}

/* ==================================================================== */
/* This returns the hardware, driver and library versions.              */

Vd80Err Vd80GetVersion(Vd80Handle handle, int module, Vd80Version *version) {

Handle *hand;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   return hand->API.GetVersion(hand->FD[module -1],version);
}

/* ==================================================================== */
/* Get the version of this stub library                                 */

#ifndef COMPILE_TIME
#define COMPILE_TIME 0
#endif

Vd80Err Vd80GetStubVersion(unsigned int *version) {
   *version = COMPILE_TIME;
   return Vd80ErrSUCCESS;
}

/* ==================================================================== */
/* Setup the trigger input                                              */

Vd80Err Vd80SetTrigger(Vd80Handle handle, Vd80Mod modules, Vd80Chn channels, Vd80Input *trigger) {

Handle *hand;
int i, msk;
Vd80Err ser;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   for (i=0; i<Vd80MAX_DEVICES; i++) {
      msk = 1 << i;
      if (modules & msk) {
	 ser = hand->API.SetTrigger(hand->FD[i],trigger);
	 if (ser != Vd80ErrSUCCESS)
	    return ser;
      }
   }
   return Vd80ErrSUCCESS;
}

/* ==================================================================== */
/* Get the trigger input settings                                       */

Vd80Err Vd80GetTrigger(Vd80Handle handle, int module, int channel, Vd80Input *trigger) {

Handle *hand;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   return hand->API.GetTrigger(hand->FD[module -1],trigger);
}

/* ==================================================================== */
/* Setup the clock input                                                */

Vd80Err Vd80SetClock(Vd80Handle handle, Vd80Mod modules, Vd80Chn channels, Vd80Input *clock) {

Handle *hand;
int i, msk;
Vd80Err ser;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   for (i=0; i<Vd80MAX_DEVICES; i++) {
      msk = 1 << i;
      if (modules & msk) {
	 ser = hand->API.SetClock(hand->FD[i],clock);
	 if (ser != Vd80ErrSUCCESS)
	    return ser;
      }
   }
   return Vd80ErrSUCCESS;
}

/* ==================================================================== */
/* Get the clock input settings                                         */

Vd80Err Vd80GetClock(Vd80Handle handle, int module, int channel, Vd80Input *clock) {

Handle *hand;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   return hand->API.GetClock(hand->FD[module -1],clock);
}

/* ==================================================================== */
/* Some hardware has an external stop input.                            */

Vd80Err Vd80SetStop(Vd80Handle handle, Vd80Mod modules, Vd80Chn channels, Vd80Input *stop) {

Handle *hand;
int i, msk;
Vd80Err ser;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   for (i=0; i<Vd80MAX_DEVICES; i++) {
      msk = 1 << i;
      if (modules & msk) {
	 ser = hand->API.SetStop(hand->FD[i],channels,stop);
	 if (ser != Vd80ErrSUCCESS)
	    return ser;
      }
   }
   return Vd80ErrSUCCESS;
}

/* ==================================================================== */
/* Some hardware has an external stop input.                            */

Vd80Err Vd80GetStop(Vd80Handle handle, int module, int channel, Vd80Input *stop) {

Handle *hand;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   return hand->API.GetStop(hand->FD[module -1],channel,stop);
}

/* ==================================================================== */
/* Divide the clock frequency by an integer.                            */
/* One is added to the divisor, so 0 means divide by 1.                 */

Vd80Err Vd80SetClockDivide(Vd80Handle handle, Vd80Mod modules, Vd80Chn channels, unsigned int divisor) {

Handle *hand;
int i, msk;
Vd80Err ser;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   for (i=0; i<Vd80MAX_DEVICES; i++) {
      msk = 1 << i;
      if (modules & msk) {
	 ser = hand->API.SetClockDivide(hand->FD[i],divisor);
	 if (ser != Vd80ErrSUCCESS)
	    return ser;
      }
   }
   return Vd80ErrSUCCESS;
}

/* ==================================================================== */
/* Divide the clock frequency by an integer.                            */
/* One is added to the divisor, so 0 means divide by 1.                 */

Vd80Err Vd80GetClockDivide(Vd80Handle handle, int module, int channel, unsigned int *divisor) {

Handle *hand;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   return hand->API.GetClockDivide(hand->FD[module -1],divisor);
}

/* ==================================================================== */
/* Start acquisition by software.                                       */

Vd80Err Vd80StrtAcquisition(Vd80Handle handle, Vd80Mod modules, Vd80Chn channels) { /* Zero all channels */

Handle *hand;
int i, msk;
Vd80Err ser;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   for (i=0; i<Vd80MAX_DEVICES; i++) {
      msk = 1 << i;
      if (modules & msk) {
	 ser = hand->API.StrtAcquisition(hand->FD[i]);
	 if (ser != Vd80ErrSUCCESS)
	    return ser;
      }
   }
   return Vd80ErrSUCCESS;
}

/* ==================================================================== */
/* Trigger acquisition by software.                                     */

Vd80Err Vd80TrigAcquisition(Vd80Handle handle, Vd80Mod modules, Vd80Chn channels) {

Handle *hand;
int i, msk;
Vd80Err ser;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   for (i=0; i<Vd80MAX_DEVICES; i++) {
      msk = 1 << i;
      if (modules & msk) {
	 ser = hand->API.TrigAcquisition(hand->FD[i]);
	 if (ser != Vd80ErrSUCCESS)
	    return ser;
      }
   }
   return Vd80ErrSUCCESS;
}

/* ==================================================================== */
/* Force reading to stop                                                */

Vd80Err Vd80StopAcquisition(Vd80Handle handle, Vd80Mod modules, Vd80Chn channels) {

Handle *hand;
int i, msk;
Vd80Err ser;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   for (i=0; i<Vd80MAX_DEVICES; i++) {
      msk = 1 << i;
      if (modules & msk) {
	 ser = hand->API.StopAcquisition(hand->FD[i]);
	 if (ser != Vd80ErrSUCCESS)
	    return ser;
      }
   }
   return Vd80ErrSUCCESS;
}

/* ==================================================================== */
/* Connecting to zero disconnects from all previous connections         */

Vd80Err Vd80Connect(Vd80Handle handle, Vd80Mod modules, Vd80Chn channels, Vd80IntrMask imask) {

Handle *hand;
int i, msk;
Vd80Err ser;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   for (i=0; i<Vd80MAX_DEVICES; i++) {
      msk = 1 << i;
      if (modules & msk) {
	 ser = hand->API.Connect(hand->FD[i],imask);
	 if (ser != Vd80ErrSUCCESS)
	    return ser;
      }
   }
   return Vd80ErrSUCCESS;
}

/* ==================================================================== */

Vd80Err Vd80SetTimeout(Vd80Handle handle, int tmout) {

Handle *hand;
int i;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   for (i=0; i<Vd80MAX_DEVICES; i++)
      hand->API.SetTimeout(hand->FD[i],tmout);

   return Vd80ErrSUCCESS;
}

/* ==================================================================== */

Vd80Err Vd80GetTimeout(Vd80Handle handle, int *tmout) {

Handle *hand;
int i;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   for (i=0; i<Vd80MAX_DEVICES; i++)
      if (hand->FD[i] > 0)
	 return hand->API.GetTimeout(hand->FD[i],tmout);

   return Vd80ErrOPEN;
}

/* ==================================================================== */

Vd80Err Vd80SetQueueFlag(Vd80Handle handle, Vd80QueueFlag flag) {

   return Vd80ErrSUCCESS;
}

/* ==================================================================== */

Vd80Err Vd80GetQueueFlag(Vd80Handle handle, Vd80QueueFlag *flag) {

   *flag = Vd80QueueFlagOFF;
   return Vd80ErrSUCCESS;
}

/* ==================================================================== */

Vd80Err Vd80Wait(Vd80Handle handle, Vd80Intr *intr) {

Handle *hand;
int i;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   for (i=0; i<Vd80MAX_DEVICES; i++)
      if (hand->FD[i] > 0)
	 return hand->API.Wait(hand->FD[i],intr);

   return Vd80ErrOPEN;
}

/* ==================================================================== */

Vd80Err Vd80SimulateInterrupt(Vd80Handle handle, Vd80Intr *intr) {

Handle *hand;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   return hand->API.SimulateInterrupt(hand->FD[intr->Module -1],intr);
}

/* ==================================================================== */

Vd80Err Vd80SetTriggerLevels(Vd80Handle handle, unsigned int modules, unsigned int channels, Vd80AnalogTrig *atrg) {

Handle *hand;
int i, msk;
Vd80Err ser;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   for (i=0; i<Vd80MAX_DEVICES; i++) {
      msk = 1 << i;
      if (modules & msk) {
	 ser = hand->API.SetTriggerLevels(hand->FD[i],channels,atrg);
	 if (ser != Vd80ErrSUCCESS)
	    return ser;
      }
   }
   return Vd80ErrSUCCESS;
}

/* ==================================================================== */

Vd80Err Vd80GetTriggerLevels(Vd80Handle handle, int module, int channel, Vd80AnalogTrig *atrg) {

Handle *hand;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   return hand->API.GetTriggerLevels(hand->FD[module -1],channel,atrg);
}

/* ==================================================================== */
/* Buffers contain samples that occur before and after the trigger.     */
/* The buffer is allocated by the library in an optimum way for DMA.    */
/* Any byte ordering operations are performed by the library.           */
/* The user specifies the number of post trigger samples and the size   */
/* of the buffer. In some hardware such as the VD80 the DMA transfers   */
/* are carried out in chunks. Thus the exact triigger position within   */
/* the buffer is adjusted to the nearset size/post boundary. Hence the  */
/* actual number of pre/post trigger samples may vary.                  */
/* One buffer is allocated for each module/channel specified in the     */
/* module/channel masks. Calling SetBuffers will deallocate previous    */
/* memory allocations if the new size is larger and reallocate.         */

Vd80Err Vd80SetBufferSize(Vd80Handle handle, Vd80Mod modules, Vd80Chn channels, int *size, int  *post) {

int i, j;
unsigned int mski, mskj;
Vd80DatumSize dsze;
Vd80Err err;
Handle *hand;
Vd80Buffer buf;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;


   *size = GetChunkSize(*size);
   *post = GetChunkSize(*post);

   for (i=0; i<Vd80MODULES; i++) {
      mski = 1 << i;
      if (mski & modules) {

	 err = hand->API.GetDatumSize(hand->FD[i],&dsze);
	 if (err != Vd80ErrSUCCESS) return err; /* Bad handle */

	 for (j=0; j<Vd80CHANNELS; j++) {
	    mskj = 1 << j;
	    if (mskj & channels) {
	       bzero((void *) &buf, sizeof(Vd80Buffer));
	       buf.Post = *post; hand->Post = *post;
	       buf.BSze = *size; hand->BSze = *size;
	       err = hand->API.SetBufferSize(hand->FD[i], *post);
	       if (err != Vd80ErrSUCCESS) return err;
	    }
	 }
      }
   }
   return Vd80ErrSUCCESS;
}

/* ==================================================================== */

Vd80Err Vd80GetBufferSize(Vd80Handle handle, int module, int channel, int *size, int *post) {

Handle *hand;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   if ((module  < 1) || (module  > Vd80MODULES )) return Vd80ErrMODULE;
   if ((channel < 1) || (channel > Vd80CHANNELS)) return Vd80ErrCHANNEL;

   *size = hand->BSze;
   *post = hand->Post;

   return Vd80ErrSUCCESS;
}

/* ==================================================================== */
/* Get information about a buffer so that the data, if any, can be used */
/* This function transfers data to the user buffer.                     */

Vd80Err Vd80GetBuffer(Vd80Handle handle, int module, int channel, Vd80Buffer *buffer) {

Handle *hand;
Vd80Err err;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   if ((module  < 1) || (module  > Vd80MODULES )) return Vd80ErrMODULE;
   if ((channel < 1) || (channel > Vd80CHANNELS)) return Vd80ErrCHANNEL;

   err = hand->API.GetBuffer(hand->FD[module -1],channel,buffer); /* Read buffer */

   if ((buffer->Tpos + buffer->Post) > buffer->ASze) { /* Chopped ? */
       buffer->Post = buffer->ASze - buffer->Tpos;
   }

   return err;
}

/* ==================================================================== */

Vd80Err Vd80SetTriggerConfig(Vd80Handle handle, unsigned int modules, unsigned int channels, Vd80TrigConfig *ctrg) {

Handle *hand;
int i, msk;
Vd80Err ser;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   for (i=0; i<Vd80MAX_DEVICES; i++) {
      msk = 1 << i;
      if (modules & msk) {
	 ser = hand->API.SetTriggerConfig(hand->FD[i],ctrg); /* Set trigger config parameters */
	 if (ser != Vd80ErrSUCCESS)
	    return ser;
      }
   }
   return Vd80ErrSUCCESS;
}

/* ==================================================================== */

Vd80Err Vd80GetTriggerConfig(Vd80Handle handle, int module, int channel, Vd80TrigConfig *ctrg) {

Handle *hand;

   hand = (Handle *) handle;
   if (hand == NULL) return Vd80ErrADDRESS;
   if (hand->Magic != Vd80MAGIC) return Vd80ErrHANDLE;

   return hand->API.GetTriggerConfig(hand->FD[module -1],ctrg); /* Get trigger config parameters */
}
