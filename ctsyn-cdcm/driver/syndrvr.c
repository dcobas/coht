/* ************************************************************************ */
/* CtSync Timing receiver device driver (SYN-Version)                       */
/* Julian Lewis 1st/April/2010  Linux Men A20 Version Only                  */
/* ************************************************************************ */

#include <libinstkernel.h>	/* Routines to work with XML tree */
#include <cdcm/cdcm.h>

#include <syndrvr.h>
#include <syndrvrP.h>

#include <vmebus.h>

static int AddModule(SynDrvrModuleContext * mcon, int index, int flag);
static unsigned int GetStatus(SynDrvrModuleContext * mcon);
int IntrHandler(void *cookie);

/*========================================================================*/

/* Drivers working area global pointer */

static SynDrvrWorkingArea *Wa = NULL;

/*========================================================================*/

static int GetClrBusErrCnt(void);
static unsigned short HRd(void *x);
static void HWr(unsigned short v, void *x);

/*========================================================================*/
/* Perform VME access and check bus errors, always                        */
/*========================================================================*/

#define CLEAR_BUS_ERROR ((int) 1)
#define BUS_ERR_PRINT_THRESHOLD 10

static int bus_error_count = 0;	/* For all modules */
static int isr_bus_error = 0;	/* Bus error in an ISR */
static int last_bus_error = 0;	/* Last printed bus error */

/* ==================== */

static void BusErrorHandler(struct vme_bus_error *error)
{

	bus_error_count++;
}

/* ==================== */

static int GetClrBusErrCnt()
{
	int res;

	res = bus_error_count;

	bus_error_count = 0;
	last_bus_error = 0;
	isr_bus_error = 0;

	return res;
}

/* ==================== */

static unsigned short IHRd(void *x)
{
	unsigned short res;

	isr_bus_error = 0;
	res = ioread16be(x);
	if (bus_error_count > last_bus_error)
		isr_bus_error = 1;
	return res;
}

/* ==================== */

static inline void check_bus_error(void *vaddr)
{
	if (bus_error_count > last_bus_error &&
	    bus_error_count <= BUS_ERR_PRINT_THRESHOLD) {
		cprintf("SynDrvr:BUS_ERROR:A16D16:READ-Address:0x%p\n", vaddr);
		if (isr_bus_error)
			cprintf("SynDrvr:BUS_ERROR:In ISR occured\n");
		if (bus_error_count == BUS_ERR_PRINT_THRESHOLD)
			cprintf("SynDrvr:BUS_ERROR:PrintSuppressed\n");
		isr_bus_error = 0;
		last_bus_error = bus_error_count;
	}
}

static unsigned short HRd(void *vaddr)
{
	unsigned short res;

	res = ioread16be(vaddr);
	check_bus_error(vaddr);
	return res;
}

/* ==================== */

static void HWr(unsigned short v, void *x)
{

	iowrite16be(v, x);
	if (bus_error_count > last_bus_error) {
		if (bus_error_count <= BUS_ERR_PRINT_THRESHOLD) {
			cprintf("SynDrvr:BUS_ERROR:A16D16:WRITE-Address:0x%p Data:0x%x\n", x, v);
			if (bus_error_count == BUS_ERR_PRINT_THRESHOLD)
				cprintf("SynDrvr:BUS_ERROR:PrintSuppressed\n");
			last_bus_error = bus_error_count;
		}
	}
	return;
}

/* ========================================================= */
/* Convert the standard XML tree into a module address array */

static SynDrvrInfoTable tinfo;

void SynConvertInfo(InsLibDrvrDesc * drvrd, SynDrvrInfoTable * addresses)
{

	InsLibModlDesc *modld = NULL;
	InsLibVmeModuleAddress *vmema = NULL;
	InsLibVmeAddressSpace *vmeas = NULL;
	InsLibIntrDesc *intrd = NULL;

	int index;
	int vme = 0;

	bzero((void *) addresses, (size_t) sizeof(SynDrvrModuleAddress));

	modld = drvrd->Modules;
	while (modld) {
		if (modld->BusType != InsLibBusTypeVME)
			return;

		index = addresses->Modules;
		addresses->Modules += 1;

		intrd = modld->Isr;

		vmema = modld->ModuleAddress;
		if (vmema) {
			vmeas = vmema->VmeAddressSpace;
			while (vmeas) {

				/*
				 * WARNING: This logic assumes the FIRST address 0x29 space is for JTAG
				 *          and the SECOND is the base address. There is no way I can
				 *          tell ... perhaps a specific word in the comment !
				 */

				if ((!vme) && (vmeas->AddressModifier == 0x29)) {
					addresses->Addresses[index].
					    JTGAddress =
					    (unsigned short *) vmeas->
					    BaseAddress;
					vme = 1;
				}
				if ((vme) && (vmeas->AddressModifier == 0x29)) {
					addresses->Addresses[index].
					    VMEAddress =
					    (unsigned short *) vmeas->
					    BaseAddress;
					vme = 0;
				}
				if (intrd) {
					addresses->Addresses[index].
					    InterruptVector =
					    (unsigned int) intrd->Vector;
					addresses->Addresses[index].
					    InterruptLevel =
					    (unsigned int) intrd->Level;
				}

				vmeas = vmeas->Next;
			}
		}
		modld = modld->Next;
	}
}

/* ================================================= */
/* Build a syn info object from the pointer infoaddr */

int BuildInfoObject(InsLibDrvrDesc * info)
{

	InsLibDrvrDesc *drvrd = NULL;
	int rc = 1;

	InsLibSetCopyRoutine((void (*)(void *, const void *, int n))
			     cdcm_copy_from_user);

	drvrd = InsLibCloneOneDriver(info);

	if (drvrd) {
		if ((strcmp(drvrd->DrvrName, "SYN") == 0)
		    || (strcmp(drvrd->DrvrName, "syn") == 0)) {

			SynConvertInfo(drvrd, &tinfo);
			rc = 0;
		}
	}
	InsLibFreeDriver(drvrd);
	return rc;
}

/*========================================================================*/
/* Reset the module                                                       */
/*========================================================================*/

static void Reset(SynDrvrModuleContext * mcon)
{

	SynDrvrModuleAddress *moad;	/* Module address, vector, level, ssch */
	SynDrvrMap *mmap;	/* Module Memory map */
	SynDrvrMsc *msc;
	SynDrvrIrq *irq;

	unsigned short source, setup;
	unsigned int synstat;

	moad = &(mcon->Address);
	if (moad->VMEAddress == 0) {
		if (AddModule(mcon, mcon->ModuleIndex, 1)) {
			moad->VMEAddress = 0;
			return;
		}
		cprintf("SynDrvr: New module detected: Address: %x\n",
			(uint32_t) moad->VMEAddress);
	}

	mmap = (SynDrvrMap *) moad->VMEAddress;
	msc = &(mmap->Msc);

	irq = &(mmap->Irq);
	HWr((unsigned short) 0, &irq->Enable);	/* Disable all interrupts */
	source = HRd(&irq->Enable);	/* Clear on read -> clear all interrupt sources */

	/* Setup interrupt vector and level */

	setup =
	    (moad->InterruptLevel & 0xFF) | (moad->InterruptVector << 8);
	HWr(setup, &irq->Setup);

	/* Setup the output delay value */

	if ((mcon->OutputDelay.Upr == 0)
	    && (mcon->OutputDelay.Lwr == 0)) {
		mcon->OutputDelay.Upr = SynDrvrDEFAULT_OUTPUT_DELAY >> 16;
		mcon->OutputDelay.Lwr =
		    SynDrvrDEFAULT_OUTPUT_DELAY & 0xFFFF;
	}
	HWr(mcon->OutputDelay.Upr, &mmap->OutputDelay.Upr);
	HWr(mcon->OutputDelay.Lwr, &mmap->OutputDelay.Lwr);

	/* Setup the sync period */

	if (mcon->SyncPeriod == 0)
		mcon->SyncPeriod = SynDrvrDEFAULT_SYNC_PERIOD;
	HWr(mcon->SyncPeriod, &mmap->SyncPeriod);

	/* Set up PLL */

	if ((mcon->Phase.Upr == 0) && (mcon->Phase.Lwr == 0)) {
		mcon->Phase.Upr = SynDrvrDEFAULT_PHASE >> 16;
		mcon->Phase.Lwr = SynDrvrDEFAULT_PHASE & 0xFFFF;
	}
	HWr(mcon->Phase.Upr, &mmap->Pll.PllPhase.Upr);
	HWr(mcon->Phase.Lwr, &mmap->Pll.PllPhase.Lwr);

	if ((mcon->NumAverage.Upr == 0) && (mcon->NumAverage.Lwr == 0)) {
		mcon->NumAverage.Upr = SynDrvrDEFAULT_NUMBER_AVS >> 16;
		mcon->NumAverage.Lwr = SynDrvrDEFAULT_NUMBER_AVS & 0xFFFF;
	}
	HWr(mcon->NumAverage.Upr, &mmap->Pll.PllNumAverage.Upr);
	HWr(mcon->NumAverage.Lwr, &mmap->Pll.PllNumAverage.Lwr);

	if ((mcon->KP.Upr == 0) && (mcon->KP.Lwr == 0)) {
		mcon->KP.Upr = SynDrvrDEFAULT_KP >> 16;
		mcon->KP.Lwr = SynDrvrDEFAULT_KP & 0xFFFF;
	}
	HWr(mcon->KP.Upr, &mmap->Pll.PllKP.Upr);
	HWr(mcon->KP.Lwr, &mmap->Pll.PllKP.Lwr);

	if ((mcon->KI.Upr == 0) && (mcon->KI.Lwr == 0)) {
		mcon->KI.Upr = SynDrvrDEFAULT_KI >> 16;
		mcon->KI.Lwr = SynDrvrDEFAULT_KI & 0xFFFF;
	}
	HWr(mcon->KI.Upr, &mmap->Pll.PllKI.Upr);
	HWr(mcon->KI.Lwr, &mmap->Pll.PllKI.Lwr);

	mcon->PllAsPrdNs = SynDrvrDEFAULT_ASYNC_PERIOD;

	/* Re-establish output state and resync */

	setup = SynDrvrMscControlRESYNC_PPS;
	synstat = GetStatus(mcon);
	if (synstat & SynDrvrStatusOUTPUTS_ENABLED) {
		mcon->EnabledOutput = 1;
		setup |= SynDrvrMscControlENABLE_OUTPUT;
	}
	HWr(setup, &msc->Control);

	/* Clear module context */

	mcon->BusError = 0;	/* No bus errors */
	mcon->FlashOpen = 0;	/* Jtag flash closed */

	/* Re-enable connected interrupts */

	HWr(mcon->EnabledInterrupts, &irq->Enable);

	return;
}

/*========================================================================*/
/* Raw IO                                                                 */
/*========================================================================*/

static int RawIo(SynDrvrModuleContext * mcon,
		 SynDrvrRawIoBlock * riob, unsigned int flag)
{

	SynDrvrModuleAddress *moad;	/* Module address, vector, level, ssch */
	unsigned short *mmap;
	int berr;

	moad = &(mcon->Address);
	mmap = (unsigned short *) moad->VMEAddress;

	if (flag)
		HWr(riob->Data, &(mmap[riob->Offset]));
	else
		riob->Data = HRd(&(mmap[riob->Offset]));

	berr = GetClrBusErrCnt();
	if (berr)
		cprintf("Syn:RawIo:**BusError**\n");

	return OK;
}

/*========================================================================*/
/* Get status                                                             */
/*========================================================================*/

static unsigned int GetStatus(SynDrvrModuleContext *mcon)
{

	SynDrvrModuleAddress *moad;	/* Module address, vector, level, ssch */
	SynDrvrMap *mmap;	/* Module Memory map */
	SynDrvrMsc *msc;	/* Control Status block */

	unsigned short stat;

	moad = &(mcon->Address);
	mmap = (SynDrvrMap *) moad->VMEAddress;
	msc = &(mmap->Msc);

	/* Read hardware status */

	stat = HRd(&msc->Status) & SynDrvrMscStatusMASK;

	/* OR in software status bits */

	mcon->BusErrorCount += GetClrBusErrCnt();
	if (mcon->BusErrorCount)
		stat |= SynDrvrStatusBUS_FAULT;
	if (mcon->FlashOpen)
		stat |= SynDrvrStatusFLASH_OPEN;
	mcon->BusErrorCount = 0;

	return (unsigned int) stat;
}

/*========================================================================*/
/* Add a module to the driver, called per module from install             */
/*========================================================================*/

static int AddModule(SynDrvrModuleContext *mcon, 	/* Module Context */
		     int index, 			/* Module index (0-based) */
		     int flag) 				/* Check again */
{

	SynDrvrModuleAddress *moad;	/* Module address, vector, level, ssch */
	unsigned int addr;	/* VME base address */
	unsigned int coco;	/* Completion code */

	struct vme_bus_error berr;
	struct pdparam_master param;
	void *vmeaddr;		/* For CES PowerPC */

	/* Compute Virtual memory address as seen from system memory mapping */

	moad = &(mcon->Address);

	/* Check module has re-appeared after JTAG modification */

	if (!flag) {

		/* CES: build an address window (64 kbyte) for VME A16-D16 accesses */

		addr = (unsigned int) moad->JTGAddress;
		addr &= 0x0000ffff;	/* A16 */

		param.iack = 1;	/* no iack */
		param.rdpref = 0;	/* no VME read prefetch option */
		param.wrpost = 0;	/* no VME write posting option */
		param.swap = 1;	/* VME auto swap option */
		param.dum[0] = 0;	/* window is sharable */
		param.dum[1] = 0;	/* XPC ADP-type */
		param.dum[2] = 0;	/* window is sharable */
		vmeaddr = (void *) find_controller(addr, 	/* Vme base address */
						   (unsigned int) 0x1000, 	/* Module address space */
						   (unsigned int) 0x29, 	/* Address modifier A16 */
						   0, 	/* Offset */
						   2, 	/* Size is D16 */
						   &param);	/* Parameter block */
		if (vmeaddr == (void *) (-1)) {
			cprintf
			    ("SynDrvr: find_controller: ERROR: Module:%d. JTAG Addr:%x\n",
			     (uint32_t) index + 1,
			     (uint32_t) moad->JTGAddress);
			pseterr(ENXIO);	/* No such device or address */
			return 1;
		}
		moad->JTGAddress = (unsigned short *) vmeaddr;
	}

	/* CES: build an address window (64 kbyte) for VME A16-D16 accesses */

	addr = (unsigned int) moad->VMEAddress;
	if (addr == 0)
		addr = (unsigned int) moad->CopyAddress;
	addr &= 0x0000ffff;	/* A16 */

	param.iack = 1;		/* no iack */
	param.rdpref = 0;	/* no VME read prefetch option */
	param.wrpost = 0;	/* no VME write posting option */
	param.swap = 1;		/* VME auto swap option */
	param.dum[0] = 0;	/* window is sharable */
	param.dum[1] = 0;	/* XPC ADP-type */
	param.dum[2] = 0;	/* window is sharable */
	vmeaddr = (void *) find_controller(addr, 	/* Vme base address */
					   (unsigned int) 0x1000, 	/* Module address space */
					   (unsigned int) 0x29, 	/* Address modifier A16 */
					   0, 	/* Offset */
					   2, 	/* Size is D16 */
					   &param);	/* Parameter block */
	if (vmeaddr == (void *) (-1)) {
		cprintf
		    ("SynDrvr: find_controller: ERROR: Module:%d. VME Addr:%x\n",
		     (uint32_t) index + 1, (uint32_t) moad->VMEAddress);
		pseterr(ENXIO);	/* No such device or address */
		return 2;
	}
	moad->VMEAddress = (unsigned short *) vmeaddr;

	berr.address = (int) vmeaddr;
	berr.am = 0x29;
	mcon->BerrHandler =
	    vme_register_berr_handler(&berr, 0x1000, BusErrorHandler);
	if (IS_ERR(mcon->BerrHandler)) {
		cprintf("SynDrvr: Cant register bus error handler\n");
	}

	/* Now set up the interrupt handler */

	coco = vme_intset(moad->InterruptVector, 	/* Vector */
			  IntrHandler, 	/* Address of ISR */
			  (void *) mcon, 	/* Parameter to pass */
			  NULL);	/* Don't save previous */
	if (coco < 0) {
		cprintf("SynDrvr: vme_intset: ERROR %d, MODULE %d\n", coco,
			index + 1);
		pseterr(EFAULT);	/* Bad address */
		return 3;
	}

	Reset(mcon);		/* Soft reset and initialize module */

	return 0;
}

/*
 * =====================================================
 * Remove a VME module
 * =====================================================
 */

static void RemoveModule(SynDrvrModuleContext * mcon)
{

	vme_intclr(mcon->Address.InterruptVector, NULL);
	return_controller((unsigned long) mcon->Address.VMEAddress,
			  0x1000);
	return_controller((unsigned long) mcon->Address.JTGAddress,
			  0x1000);
}

/*========================================================================*/
/* Cancel a timeout in a safe way                                         */
/*========================================================================*/

static void CancelTimeout(int *t)
{

	int v;
	unsigned long flags;

	cdcm_spin_lock_irqsave(&(Wa->lock), flags);
	if ((v = *t)) {
		*t = 0;
		cancel_timeout(v);
	}
	cdcm_spin_unlock_irqrestore(&(Wa->lock), flags);
}

/*========================================================================*/
/* Handel read timeouts                                                   */
/*========================================================================*/

static int ReadTimeout(void *cookie)
{

	SynDrvrClientContext *ccon = cookie;
	unsigned long flags;

	cdcm_spin_lock_irqsave(&(Wa->lock), flags);
	ccon->Timer = 0;
	sreset(&(ccon->Semaphore));
	cdcm_spin_unlock_irqrestore(&(Wa->lock), flags);
	return 0;
}

/*========================================================================*/
/* Connect... Adding new sources                                          */
/*========================================================================*/

static int Connect(SynDrvrClientContext *ccon, SynDrvrConnection *conx)
{

	SynDrvrModuleAddress *moad;	/* Module address, vector, level, ssch */
	SynDrvrMap *mmap;	/* Module Memory map */
	SynDrvrIrq *irq;	/* Interrupt block */
	SynDrvrModuleContext *mcon;

	unsigned int ci;
	unsigned short msk;
	unsigned long flags;

	if (conx == NULL) {
		pseterr(EINVAL);	/* EINVAL = "Invalid argument" */
		return SYSERR;
	}

	if (conx->Module == 0)
		mcon = &(Wa->ModuleContexts[ccon->ModuleIndex]);	/* Default module selected */
	else
		mcon = &(Wa->ModuleContexts[conx->Module - 1]);

	moad = &(mcon->Address);
	mmap = (SynDrvrMap *) moad->VMEAddress;
	irq = &(mmap->Irq);

	ci = ccon->ClientIndex;
	msk = (conx->ConMask & SynDrvrConMaskMASK);

	cdcm_spin_lock_irqsave(&(mcon->lock), flags);
	mcon->ConMask[ci] |= msk;
	mcon->Clients[ci] = ccon;
	mcon->EnabledInterrupts |= msk;
	HWr(mcon->EnabledInterrupts, &irq->Enable);
	cdcm_spin_unlock_irqrestore(&(mcon->lock), flags);

	return OK;
}

/*========================================================================*/
/* Disconnect                                                             */
/*========================================================================*/

static int DisConnect(SynDrvrClientContext *ccon, SynDrvrConnection *conx)
{
	SynDrvrModuleAddress *moad;	/* Module address, vector, level, ssch */
	SynDrvrMap *mmap;	/* Module Memory map */
	SynDrvrIrq *irq;	/* Interrupt block */
	SynDrvrModuleContext *mcon;

	unsigned int ci;
	unsigned short msk, cmsk;
	int i;
	unsigned long flags;

	if (conx == NULL) {
		pseterr(EINVAL);	/* EINVAL = "Invalid argument" */
		return SYSERR;
	}

	if (conx->Module == 0)
		mcon = &(Wa->ModuleContexts[ccon->ModuleIndex]);	/* Default module selected */
	else
		mcon = &(Wa->ModuleContexts[conx->Module - 1]);

	moad = &(mcon->Address);
	mmap = (SynDrvrMap *) moad->VMEAddress;
	irq = &(mmap->Irq);

	ci = ccon->ClientIndex;
	cmsk = conx->ConMask;

	/* See if other clients are using some of the bits this client */
	/* is disconnecting from, in that case, do not disable these   */
	/* bits in the interrupt enable mask */

	cdcm_spin_lock_irqsave(&(mcon->lock), flags);
	for (i = 0; i < SynDrvrCLIENT_CONTEXTS; i++) {
		if (i != ci) {
			msk = cmsk & mcon->ConMask[i];
			if (msk)
				cmsk &= ~msk;	/* Other clients are still connected */
		}
	}
	if (cmsk) {
		mcon->EnabledInterrupts &= ~cmsk;
		HWr(mcon->EnabledInterrupts, &irq->Enable);
	}

	/* Remove this clients connections from the module */

	msk = mcon->ConMask[ci] & ~conx->ConMask;
	mcon->ConMask[ci] = msk;
	if (msk == 0)
		mcon->Clients[ci] = NULL;
	cdcm_spin_unlock_irqrestore(&(mcon->lock), flags);

	return OK;
}

/*========================================================================*/
/* Disconnect all a clients conections, used in close                     */
/*========================================================================*/

static void DisConnectAll(SynDrvrClientContext * ccon)
{

	SynDrvrModuleContext *mcon;
	SynDrvrConnection conx;
	int i, ci, cc;

	ci = ccon->ClientIndex;
	for (i = 0; i < SynDrvrMODULE_CONTEXTS; i++) {
		mcon = &(Wa->ModuleContexts[i]);
		if (mcon->Clients[ci]) {
			conx.Module = i + 1;
			conx.ConMask = mcon->ConMask[i];
			cc = DisConnect(ccon, &conx);
		}
	}
}

/***************************************************************************/
/* Interrupt handler                                                       */
/***************************************************************************/

int IntrHandler(void *cookie)
{

	SynDrvrMap *mmap;
	SynDrvrIrq *irq;

	unsigned short src;
	unsigned short msk;
	SynDrvrClientContext *ccon;
	SynDrvrClientQueue *queue;
	SynDrvrReadBuf rb;
	unsigned int iq, ci;
	unsigned long flags;

	SynDrvrModuleContext *mcon = (SynDrvrModuleContext *) cookie;

	irq = (SynDrvrIrq *) mcon->Address.VMEAddress;
	src = IHRd(&irq->Source);

	mmap = (SynDrvrMap *) irq;
	rb.Status = mmap->Msc.Status;

	for (ci = 0; ci < SynDrvrCLIENT_CONTEXTS; ci++) {
		ccon = mcon->Clients[ci];
		if (ccon) {
			msk = mcon->ConMask[ci] & src;
			if (msk) {
				rb.Connection.Module =
				    mcon->ModuleIndex + 1;
				rb.Connection.ConMask = msk;

				cdcm_spin_lock_irqsave(&(ccon->lock),
						       flags);
				queue = &(ccon->Queue);
				if (queue->Size < SynDrvrCLIENT_QUEUE_SIZE) {
					queue->Size++;
					iq = queue->Head;
					queue->Queue[iq++] = rb;
					ssignal(&(ccon->Semaphore));
					if (iq < SynDrvrCLIENT_QUEUE_SIZE)
						queue->Head = iq;
					else
						queue->Head = 0;
				} else {
					queue->Missed++;
				}
				cdcm_spin_unlock_irqrestore(&(ccon->lock),
							    flags);
			}
		}
	}
	return OK;
}

/***************************************************************************/
/* INSTALL                                                                 */
/***************************************************************************/

char *SynDrvrInstall(void *cookie)
{				/* Driver info table */

	SynDrvrWorkingArea *wa;

	SynDrvrModuleContext *mcon;
	SynDrvrModuleAddress *moad;
	int i, erlv;

	InsLibDrvrDesc **drvrinfo = cookie;

	/* Allocate the driver working area. */

	wa = (SynDrvrWorkingArea *) sysbrk(sizeof(SynDrvrWorkingArea));
	if (!wa) {
		cprintf
		    ("SynDrvrInstall: NOT ENOUGH MEMORY(WorkingArea)\n");
		pseterr(ENOMEM);	/* Not enough core */
		return ((char *) SYSERR);
	}
	Wa = wa;		/* Global working area pointer */

   /****************************************/
	/* Initialize the driver's working area */
	/* and add the modules ISR into LynxOS  */
   /****************************************/

	bzero((void *) wa, sizeof(SynDrvrWorkingArea));	/* Clear working area */

	bzero((void *) &tinfo, sizeof(SynDrvrInfoTable));

	if (BuildInfoObject(*drvrinfo) /* Try to use the XML object tree */
	    ) {

		/* Failed so do a default install */

		cprintf
		    ("SynDrvrInstall: Will auto-install:1 SYN modules\n");
		tinfo.Modules = 1;

		for (i = 0; i < tinfo.Modules; i++) {
			moad = &tinfo.Addresses[i];
			moad->VMEAddress =
			    (unsigned short *) 0x8800 + (0x1000 * i);
			moad->JTGAddress =
			    (unsigned short *) 0x8100 + (0x1000 * i);
			moad->InterruptVector =
			    (unsigned char) 0x80 + (0x1 * i);
			moad->InterruptLevel = 2;
		}
	}

	for (i = 0; i < tinfo.Modules; i++) {
		mcon = &(wa->ModuleContexts[i]);
		moad = &mcon->Address;
		*moad = tinfo.Addresses[i];
		mcon->ModuleIndex = i;
		erlv = AddModule(mcon, i, 0);
		if (erlv == 0) {
			cprintf
			    ("SynDrvr: Module %d. VME Addr: %x JTAG Addr: %x Vect: %x Level: %x Installed OK\n",
			     i + 1, (unsigned int) moad->VMEAddress,
			     (unsigned int) moad->JTGAddress,
			     (unsigned int) moad->InterruptVector,
			     (unsigned int) moad->InterruptLevel);

			mcon->InUse = 1;
		} else if (erlv == 2) {
			moad->VMEAddress = 0;
			cprintf
			    ("SynDrvr: Module: %d WARNING: JTAG Only: JTAG Addr: %x\n",
			     i + 1, (unsigned int) moad->JTGAddress);

			mcon->InUse = 1;
		} else {
			moad->VMEAddress = 0;
			moad->JTGAddress = 0;
			cprintf
			    ("SynDrvr: Module: %d ERROR: Not Installed\n",
			     i + 1);
		}
	}
	wa->Modules = tinfo.Modules;

	/*
	 * Initialize all driver spin locks
	 */

	cdcm_spin_lock_init(&Wa->lock);	/* Global driver spin lock */
	for (i = 0; i < SynDrvrCLIENT_CONTEXTS; i++) {
		cdcm_spin_lock_init(&Wa->ClientContexts[i].lock);	/* Client spin lock */
	}
	for (i = 0; i < SynDrvrMODULE_CONTEXTS; i++) {
		cdcm_spin_lock_init(&Wa->ModuleContexts[i].lock);	/* Module spin lock */
	}

	cprintf("SynDrvr: Installed: %d GMT Modules in Total\n",
		(int) wa->Modules);

	return ((char *) wa);
}

/***************************************************************************/
/* UNINSTALL (Not suported)                                                */
/***************************************************************************/

int SynDrvrUninstall(void *wa)
{				/* Drivers working area pointer */
	int i;
	SynDrvrModuleContext *mcon;
	SynDrvrClientContext *ccon;

	for (i = 0; i < SynDrvrCLIENT_CONTEXTS; i++) {
		ccon = &(Wa->ClientContexts[i]);
		if (ccon->InUse) {
			pseterr(EBUSY);	/* EBUSY = "Resource busy" Can't uninstall */
			return SYSERR;
		}
	}
	for (i = 0; i < SynDrvrMODULE_CONTEXTS; i++) {
		mcon = &(Wa->ModuleContexts[i]);
		if (mcon->InUse)
			RemoveModule(mcon);
	}
	sysfree((void *) Wa, sizeof(SynDrvrWorkingArea));
	Wa = NULL;
	cprintf("SynDrvr: UnInstalled\n");
	return OK;
}

/***************************************************************************/
/* OPEN                                                                    */
/***************************************************************************/

int SynDrvrOpen(void *wa, 	/* Working area */
		int dnm, 	/* Device number */
		struct cdcm_file *flp)
{				/* File pointer */

	int cnum;		/* Client number */
	SynDrvrClientContext *ccon;	/* Client context */
	SynDrvrClientQueue *queue;
	unsigned long flags;

	/* We allow one client per minor device, we use the minor device */
	/* number as an index into the client contexts array. */

	cnum = minor(flp->dev) - 1;
	if ((cnum < 0) || (cnum >= SynDrvrCLIENT_CONTEXTS)) {
		pseterr(EFAULT);	/* EFAULT = "Bad address" */
		return SYSERR;
	}
	ccon = &(Wa->ClientContexts[cnum]);

	/* If already open by someone else, give a permission denied error */

	if (ccon->InUse) {

		/* This next error is normal */

		pseterr(EBUSY);	/* EBUSY = "Resource busy" File descriptor already open */
		return SYSERR;
	}

	/* Initialize a client context */

	cdcm_spin_lock_irqsave(&(ccon->lock), flags);
	ccon->ClientIndex = cnum;
	ccon->Timeout = SynDrvrDEFAULT_TIMEOUT;
	ccon->InUse = 1;
	ccon->Pid = getpid();
	queue = &ccon->Queue;
	bzero((void *) queue, sizeof(SynDrvrClientQueue));
	cdcm_spin_unlock_irqrestore(&(ccon->lock), flags);

	return OK;
}

/***************************************************************************/
/* CLOSE                                                                   */
/***************************************************************************/

int SynDrvrClose(void *wa, 	/* Working area */
		 struct cdcm_file *flp)
{				/* File pointer */

	int cnum;		/* Client number */
	SynDrvrClientContext *ccon;	/* Client context */

	/* We allow one client per minor device, we use the minor device */
	/* number as an index into the client contexts array.            */

	cnum = minor(flp->dev) - 1;
	if ((cnum < 0) || (cnum >= SynDrvrCLIENT_CONTEXTS)) {
		pseterr(EFAULT);	/* EFAULT = "Bad address" */
		return SYSERR;
	}
	ccon = &(Wa->ClientContexts[cnum]);

	CancelTimeout(&ccon->Timer);
	DisConnectAll(ccon);

	ccon->InUse = 0;
	ccon->Pid = 0;
	ccon->Timer = 0;
	sreset(&(ccon->Semaphore));

	return (OK);
}

/*
 * =====================================
 */

void GetVersion(SynDrvrMap * mmap, SynDrvrVersion * ver)
{

	unsigned short upr, lwr;

	upr = HRd(&mmap->VhdlDate.Upr);
	lwr = HRd(&mmap->VhdlDate.Lwr);
	ver->VhdlVersion = (upr << 16) | lwr;
	ver->DrvrVersion = COMPILE_TIME;
}

/*
 * =====================================
 */

void EnableOutput(SynDrvrModuleContext * mcon, int lav)
{

	SynDrvrMsc *msc;
	SynDrvrModuleAddress *moad;
	SynDrvrMap *mmap;

	moad = &(mcon->Address);
	mmap = (SynDrvrMap *) moad->VMEAddress;
	msc = &mmap->Msc;

	if (lav) {
		mcon->EnabledOutput = 1;
		HWr(SynDrvrMscControlENABLE_OUTPUT, &msc->Control);
	} else {
		mcon->EnabledOutput = 0;
		HWr(0, &msc->Control);
	}
}

/*
 * =====================================
 */

void ResyncPps(SynDrvrModuleContext * mcon)
{

	short sav;
	SynDrvrModuleAddress *moad;
	SynDrvrMap *mmap;
	SynDrvrMsc *msc;

	moad = &(mcon->Address);
	mmap = (SynDrvrMap *) moad->VMEAddress;
	msc = &mmap->Msc;

	sav = SynDrvrMscControlRESYNC_PPS;
	if (mcon->EnabledOutput)
		sav |= SynDrvrMscControlENABLE_OUTPUT;
	HWr(sav, &msc->Control);
}

/*
 * =====================================
 */

void SetOutputDelay(SynDrvrModuleContext * mcon, int lav)
{

	unsigned short upr, lwr;
	SynDrvrModuleAddress *moad;
	SynDrvrMap *mmap;

	moad = &(mcon->Address);
	mmap = (SynDrvrMap *) moad->VMEAddress;

	upr = lav >> 16;
	lwr = lav & 0xFFFF;
	mcon->OutputDelay.Upr = upr;
	mcon->OutputDelay.Lwr = lwr;
	HWr(mcon->OutputDelay.Upr, &mmap->OutputDelay.Upr);
	HWr(mcon->OutputDelay.Lwr, &mmap->OutputDelay.Lwr);
}

/*
 * =====================================
 */

void GetOutputDelay(SynDrvrMap * mmap, int *lav)
{

	unsigned short upr, lwr;

	upr = HRd(&mmap->OutputDelay.Upr);
	lwr = HRd(&mmap->OutputDelay.Lwr);
	*lav = (upr << 16) | lwr;
}

/*
 * =====================================
 */

void GetPll(SynDrvrModuleContext * mcon, SynDrvrPllBuf * pllb)
{

	unsigned short upr, lwr;
	SynDrvrModuleAddress *moad;
	SynDrvrMap *mmap;

	moad = &(mcon->Address);
	mmap = (SynDrvrMap *) moad->VMEAddress;

	upr = HRd(&mmap->Pll.PllError.Upr);
	lwr = HRd(&mmap->Pll.PllError.Lwr);
	pllb->PllError = (upr << 16) | lwr;

	upr = HRd(&mmap->Pll.PllLastItLen.Upr);
	lwr = HRd(&mmap->Pll.PllLastItLen.Lwr);
	pllb->PllLastItLen = (upr << 16) | lwr;

	upr = HRd(&mmap->Pll.PllPhase.Upr);
	lwr = HRd(&mmap->Pll.PllPhase.Lwr);
	pllb->PllPhase = (upr << 16) | lwr;

	upr = HRd(&mmap->Pll.PllIntegrator.Upr);
	lwr = HRd(&mmap->Pll.PllIntegrator.Lwr);
	pllb->PllIntegrator = (upr << 16) | lwr;

	pllb->PllDac = HRd(&mmap->Pll.PllDac);

	upr = HRd(&mmap->Pll.PllNumAverage.Upr);
	lwr = HRd(&mmap->Pll.PllNumAverage.Lwr);
	pllb->PllNumAverage = (upr << 16) | lwr;

	upr = HRd(&mmap->Pll.PllKP.Upr);
	lwr = HRd(&mmap->Pll.PllKP.Lwr);
	pllb->PllKP = (upr << 16) | lwr;

	upr = HRd(&mmap->Pll.PllKI.Upr);
	lwr = HRd(&mmap->Pll.PllKI.Lwr);
	pllb->PllKI = (upr << 16) | lwr;

	if (*(int *) &mcon->PllAsPrdNs)
		mcon->PllAsPrdNs = SynDrvrDEFAULT_ASYNC_PERIOD;
	pllb->PllAsPrdNs = mcon->PllAsPrdNs;
}

/*
 * =====================================
 */

void GetPps(SynDrvrMap * mmap, SynDrvrPpsBuf * ppsb)
{

	unsigned short upr, lwr;

	upr = HRd(&mmap->Pps.PpsPeriod.Upr);
	lwr = HRd(&mmap->Pps.PpsPeriod.Lwr);
	ppsb->PpsPeriod = (upr << 16) | lwr;

	upr = HRd(&mmap->Pps.PpsPhase.Upr);
	lwr = HRd(&mmap->Pps.PpsPhase.Lwr);
	ppsb->PpsPhase = (upr << 16) | lwr;
}

/*
 * =====================================
 */

void SetPllPhase(SynDrvrModuleContext * mcon, int lav)
{

	unsigned short upr, lwr;
	SynDrvrModuleAddress *moad;
	SynDrvrMap *mmap;

	moad = &(mcon->Address);
	mmap = (SynDrvrMap *) moad->VMEAddress;

	upr = lav >> 16;
	lwr = lav & 0xFFFF;
	mcon->Phase.Upr = upr;
	mcon->Phase.Lwr = lwr;
	HWr(mcon->Phase.Upr, &mmap->Pll.PllPhase.Upr);
	HWr(mcon->Phase.Lwr, &mmap->Pll.PllPhase.Lwr);
}

/*
 * =====================================
 */

void SetPllNumAverage(SynDrvrModuleContext * mcon, int lav)
{

	unsigned short upr, lwr;
	SynDrvrModuleAddress *moad;
	SynDrvrMap *mmap;

	moad = &(mcon->Address);
	mmap = (SynDrvrMap *) moad->VMEAddress;

	upr = lav >> 16;
	lwr = lav & 0xFFFF;
	mcon->NumAverage.Upr = upr;
	mcon->NumAverage.Lwr = lwr;
	HWr(mcon->NumAverage.Upr, &mmap->Pll.PllNumAverage.Upr);
	HWr(mcon->NumAverage.Lwr, &mmap->Pll.PllNumAverage.Lwr);
}

/*
 * =====================================
 */

void SetPllKP(SynDrvrModuleContext * mcon, int lav)
{

	unsigned short upr, lwr;
	SynDrvrModuleAddress *moad;
	SynDrvrMap *mmap;

	moad = &(mcon->Address);
	mmap = (SynDrvrMap *) moad->VMEAddress;

	upr = lav >> 16;
	lwr = lav & 0xFFFF;
	mcon->KP.Upr = upr;
	mcon->KP.Lwr = lwr;
	HWr(mcon->KP.Upr, &mmap->Pll.PllKP.Upr);
	HWr(mcon->KP.Lwr, &mmap->Pll.PllKP.Lwr);
}

/*
 * =====================================
 */

void SetPllKI(SynDrvrModuleContext * mcon, int lav)
{

	unsigned short upr, lwr;
	SynDrvrModuleAddress *moad;
	SynDrvrMap *mmap;

	moad = &(mcon->Address);
	mmap = (SynDrvrMap *) moad->VMEAddress;

	upr = lav >> 16;
	lwr = lav & 0xFFFF;
	mcon->KI.Upr = upr;
	mcon->KI.Lwr = lwr;
	HWr(mcon->KI.Upr, &mmap->Pll.PllKI.Upr);
	HWr(mcon->KI.Lwr, &mmap->Pll.PllKI.Lwr);
}

/*
 * =====================================
 */

void SetPllIntegrator(SynDrvrMap * mmap, int lav)
{

	unsigned short upr, lwr;

	upr = lav >> 16;
	lwr = lav & 0xFFFF;
	HWr(upr, &mmap->Pll.PllIntegrator.Upr);
	HWr(lwr, &mmap->Pll.PllIntegrator.Lwr);
}

/*
 * =====================================
 */

void GetClientList(SynDrvrClientList * cls)
{

	int i;
	SynDrvrClientContext *ccon;

	bzero((void *) cls, sizeof(SynDrvrClientList));
	for (i = 0; i < SynDrvrCLIENT_CONTEXTS; i++) {
		ccon = &(Wa->ClientContexts[i]);
		if (ccon->InUse) {
			cls->Pid[cls->Size] = ccon->Pid;
			cls->Size++;
		}
	}
}

/*
 * =====================================
 */

void GetClientConnections(SynDrvrClientContext * ccon,
			  SynDrvrClientConnections * ccn)
{

	int i, j;
	SynDrvrModuleContext *mcon;

	if (ccn->Pid == 0)
		ccn->Pid = ccon->Pid;
	ccn->Size = 0;
	for (i = 0; i < SynDrvrMODULE_CONTEXTS; i++) {
		mcon = &(Wa->ModuleContexts[i]);
		for (j = 0; j < SynDrvrCLIENT_CONTEXTS; j++) {
			ccon = mcon->Clients[j];
			if (!((ccon) && (ccon->InUse) && (ccon->Pid == ccn->Pid))) continue;
			ccn->Connections[ccn->Size].Module = i + 1;
			ccn->Connections[ccn->Size].ConMask =
			    mcon->ConMask[j];
			ccn->Size++;
			if (ccn->Size >= SynDrvrCONNECTIONS)
				return;
			break;
		}
	}
}

/***************************************************************************/
/* IOCTL                                                                   */
/***************************************************************************/

int SynDrvrIoctl(void *wa, 	/* Working area */
		 struct cdcm_file *flp, 	/* File pointer */
		 int cm, 	/* IOCTL command */
		 char *cp)
{				/* Data for the IOCTL */

	SynDrvrModuleContext *mcon;	/* Module context */
	SynDrvrClientContext *ccon;	/* Client context */
	SynDrvrModuleAddress *moad;
	SynDrvrMap *mmap;

	int cnum;		/* Client number */
	int lav, *lap;		/* Long Value pointed to by Arg */
	unsigned short sav, sval;	/* Short argument and for Jtag IO */
	int num;

	void *arg = cp;

	num = _IOC_NR(cm);	/* Get pure ioctl enumeration number */

	if (arg != NULL) {
		lav = *((int *) arg);	/* Long argument value */
		lap = (int *) arg;	/* Long argument pointer */
	} else {
		lav = 0;
		lap = NULL;	/* Null arg = zero read/write counts */
	}
	sav = (unsigned short) lav;	/* Short argument value */

	/* We allow one client per minor device, we use the minor device */
	/* number as an index into the client contexts array. */

	cnum = minor(flp->dev) - 1;
	if ((cnum < 0) || (cnum >= SynDrvrCLIENT_CONTEXTS)) {
		pseterr(ENODEV);	/* No such device */
		return SYSERR;
	}

	/* We can't control a file which is not open. */

	ccon = &(Wa->ClientContexts[cnum]);
	if (ccon->InUse == 0) {
		cprintf("SynDrvrIoctl: DEVICE %2d IS NOT OPEN\n",
			cnum + 1);
		pseterr(EBADF);	/* Bad file number */
		return SYSERR;
	}

	/* Set up some useful module pointers */

	mcon = &(Wa->ModuleContexts[ccon->ModuleIndex]);	/* Default module selected */
	moad = &(mcon->Address);
	mmap = (SynDrvrMap *) moad->VMEAddress;

	/*************************************/
	/* Decode callers command and do it. */
	/*************************************/

	switch (num) {

	case SynNumGET_VERSION:
		GetVersion(mmap, arg);
		return OK;
		break;

	case SynNumSET_TIMEOUT:
		ccon->Timeout = lav;
		return OK;

	case SynNumGET_TIMEOUT:
		if (lap) {
			*lap = ccon->Timeout;
			return OK;
		}
		break;

	case SynNumSET_QUEUE_FLAG:
		if (lav)
			ccon->Queue.QOff = 1;
		else
			ccon->Queue.QOff = 0;
		return OK;

	case SynNumGET_QUEUE_FLAG:
		if (lap) {
			*lap = ccon->Queue.QOff;
			return OK;
		}
		break;

	case SynNumGET_QUEUE_SIZE:
		if (lap) {
			*lap = ccon->Queue.Size;
			return OK;
		}
		break;

	case SynNumGET_QUEUE_OVERFLOW:
		if (lap) {
			*lap = ccon->Queue.Missed;
			ccon->Queue.Missed = 0;
			return OK;
		}
		break;

	case SynNumSET_MODULE:	/* Select the module to work with */
		if ((lav >= 1)
		    && (lav <= Wa->Modules)) {
			ccon->ModuleIndex = lav - 1;
			return OK;
		}
		break;

	case SynNumGET_MODULE:
		if (lap) {
			*lap = ccon->ModuleIndex + 1;
			return OK;
		}
		break;

	case SynNumGET_MODULE_COUNT:
		if (lap) {
			*lap = Wa->Modules;
			return OK;
		}
		break;

	case SynNumENABLE_OUTPUTS:
		if (lap) {
			EnableOutput(mcon, lav);
			return OK;
		}
		break;

	case SynNumRESYNC_PPS:
		ResyncPps(mcon);
		return OK;

	case SynNumRESET:	/* Reset, re-establish connections */
		Reset(mcon);
		return OK;

	case SynNumGET_STATUS:	/* Read module status */
		if (lap) {
			*lap = GetStatus(mcon);
			return OK;
		}
		break;

	case SynNumSET_OUTPUT_DELAY:
		if ((lav >= SynDrvrMIN_OUTPUT_DELAY)
		    && (lav <= SynDrvrMAX_OUTPUT_DELAY)) {
			SetOutputDelay(mcon, lav);
			return OK;
		}
		break;

	case SynNumGET_OUTPUT_DELAY:
		if (lap) {
			GetOutputDelay(mmap, lap);
			return OK;
		}
		break;

	case SynNumSET_SYNC_PERIOD:
		if ((sav >= SynDrvrMIN_SYNC_PERIOD)
		    && (sav <= SynDrvrMAX_SYNC_PERIOD))
			mcon->SyncPeriod = sav;
		else
			mcon->SyncPeriod = SynDrvrDEFAULT_SYNC_PERIOD;
		HWr(mcon->SyncPeriod, &mmap->SyncPeriod);
		return OK;

	case SynNumGET_SYNC_PERIOD:
		if (lap) {
			sav = HRd(&mmap->SyncPeriod);
			lav = sav;
			*lap = lav;
			return OK;
		}
		break;

	case SynNumGET_PLL:
		GetPll(mcon, arg);
		return OK;

	case SynNumGET_PPS:
		GetPps(mmap, arg);
		return OK;

	case SynNumSET_PLL_PHASE:
		SetPllPhase(mcon, lav);
		return OK;

	case SynNumSET_PLL_NUM_AVERAGE:
		SetPllNumAverage(mcon, lav);
		return OK;

	case SynNumSET_PLL_KP:
		SetPllKP(mcon, lav);
		return OK;

	case SynNumSET_PLL_KI:
		SetPllKI(mcon, lav);
		return OK;

	case SynNumSET_PLL_INTG:
		SetPllIntegrator(mmap, lav);
		return OK;

	case SynNumSET_PLL_AP:
		mcon->PllAsPrdNs = *(float *) lap;
		return OK;

	case SynNumGET_CLIENT_LIST:
		GetClientList(arg);
		return OK;

	case SynNumCONNECT:
		return Connect(ccon, arg);

	case SynNumGET_CLIENT_CONNECTIONS:
		GetClientConnections(ccon, arg);
		return OK;

	case SynNumJTAG_OPEN:
		mcon->FlashOpen = 1;
		return OK;

	case SynNumJTAG_READ_BYTE:
		if (mcon->FlashOpen) {
			sval = HRd(mcon->Address.JTGAddress);
			sval = 0x00FF & sval;
			*cp = (unsigned char) sval;
			return OK;
		}
		pseterr(EBUSY);
		return SYSERR;

	case SynNumJTAG_WRITE_BYTE:
		if (mcon->FlashOpen) {
			sval = 0x00FF & sav;
			HWr(sval, mcon->Address.JTGAddress);
			return OK;
		}
		pseterr(EBUSY);
		return SYSERR;

	case SynNumJTAG_CLOSE:
		mcon->FlashOpen = 0;
		Reset(mcon);
		return OK;

	case SynNumGET_MODULE_ADDRESS:
		moad = (SynDrvrModuleAddress *) arg;
		*moad = mcon->Address;
		return OK;

	case SynNumRAW_READ:
		return RawIo(mcon, arg, 0);

	case SynNumRAW_WRITE:
		return RawIo(mcon, arg, 1);

	default:
		break;
	}

	pseterr(EINVAL);
	return SYSERR;
}

/***************************************************************************/
/* READ                                                                    */
/***************************************************************************/

int SynDrvrRead(void *wa, 	/* Working area */
		struct cdcm_file *flp, 	/* File pointer */
		char *u_buf, 	/* Users buffer */
		int cnt)
{				/* Byte count in buffer */

	SynDrvrClientContext *ccon;	/* Client context */
	SynDrvrClientQueue *queue;
	SynDrvrReadBuf *rb;
	int cnum;		/* Client number */
	unsigned int iq;
	unsigned long flags;

	cnum = minor(flp->dev) - 1;
	ccon = &(Wa->ClientContexts[cnum]);

	cdcm_spin_lock_irqsave(&(ccon->lock), flags);
	queue = &(ccon->Queue);
	if (queue->QOff) {
		queue->Size = 0;
		queue->Tail = 0;
		queue->Head = 0;
		queue->Missed = 0;	/* ToDo: What to do with this info ? */
		sreset(&(ccon->Semaphore));
	}
	cdcm_spin_unlock_irqrestore(&(ccon->lock), flags);

	if (ccon->Timeout) {
		ccon->Timer =
		    timeout(ReadTimeout, (char *) ccon, ccon->Timeout);
		if (ccon->Timer < 0) {

			ccon->Timer = 0;

			/* EBUSY = "Device or resource busy" */

			pseterr(EBUSY);	/* No available timers */
			return 0;
		}
	}

	if (swait(&(ccon->Semaphore), SEM_SIGABORT)) {

		/* EINTR = "Interrupted system call" */

		pseterr(EINTR);	/* We have been signaled */
		return 0;
	}

	if (ccon->Timeout) {
		if (ccon->Timer) {
			CancelTimeout(&(ccon->Timer));
		} else {

			/* ETIME = "Timer expired */

			pseterr(ETIME);
			return 0;
		}
	}

	rb = (SynDrvrReadBuf *) u_buf;
	if (queue->Size) {
		cdcm_spin_lock_irqsave(&(ccon->lock), flags);
		iq = queue->Tail;
		*rb = queue->Queue[iq++];
		if (iq < SynDrvrCLIENT_QUEUE_SIZE)
			queue->Tail = iq;
		else
			queue->Tail = 0;
		queue->Size--;
		cdcm_spin_unlock_irqrestore(&(ccon->lock), flags);
		return sizeof(SynDrvrReadBuf);
	}

	pseterr(EINTR);
	return 0;
}

/***************************************************************************/
/* WRITE                                                                   */
/***************************************************************************/

int SynDrvrWrite(void *wa, 	/* Working area */
		 struct cdcm_file *flp, 	/* File pointer */
		 char *u_buf, 	/* Users buffer */
		 int cnt)
{				/* Byte count in buffer */

	pseterr(EPERM);		/* Not supported */
	return 0;
}

/***************************************************************************/
/* SELECT                                                                  */
/***************************************************************************/

int SynDrvrSelect(void *wa, 	/* Working area */
		  struct cdcm_file *flp, 	/* File pointer */
		  int wch, 	/* Read/Write direction */
		  struct sel *ffs)
{				/* Selection structurs */

	SynDrvrClientContext *ccon;
	int cnum;

	cnum = minor(flp->dev) - 1;
	ccon = &(Wa->ClientContexts[cnum]);

	if (wch == SREAD) {
		ffs->iosem = (int *) &(ccon->Semaphore);	/* Watch out here I hope   */
		return OK;	/* the system dosn't swait */
	}
	/* the read does it too !! */
	pseterr(EACCES);	/* Permission denied */
	return SYSERR;
}

/*************************************************************/
/* Dynamic loading information for driver install routine.   */
/*************************************************************/

struct dldd entry_points = {
	SynDrvrOpen, SynDrvrClose,
	SynDrvrRead, SynDrvrWrite,
	SynDrvrSelect, SynDrvrIoctl,
	SynDrvrInstall, SynDrvrUninstall
};
