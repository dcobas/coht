/**
 * @file skeldrvr.c
 *
 * @brief Skel driver framework
 *
 * This framework implements a standard SKEL compliant driver.
 * To implement a specific driver for a given hardware module
 * you only need to declare the specific part.
 * See the documentation at: ???
 *
 * Copyright (c) 2008-09 CERN
 * @author Julian Lewis <julian.lewis@cern.ch>
 * @author Emilio G. Cota <emilio.garcia.cota@cern.ch>
 *
 * @section license_sec License
 * Released under the GPL v2. (and only v2, not any later version)
 *
 * Introduced module parameters and got rid of CDCM
 * Julian 5/Dec/2011 This is a Linux Only implementation
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/mutex.h>
#include <asm/current.h>
#include <linux/sched.h>
#include <linux/wait.h>

#include <skeluser.h>
#include <skeluser_ioctl.h>
#include <skel.h>
#include <skeldrvrP.h>

/*
 * Common variables
 * These variables are shared between the user and the generic parts of skel
 */

SkelDrvrWorkingArea *Wa;	/* working area */

/*
 * Basic skeleton driver standard IOCTL names used by both the driver
 * and by the test program and library.
 */

static const char *SkelStandardIoctlNames[] = {
	[_IOC_NR(SkelDrvrIoctlSET_DEBUG)] = "SET_DEBUG",
	[_IOC_NR(SkelDrvrIoctlGET_DEBUG)] = "GET_DEBUG",

	[_IOC_NR(SkelDrvrIoctlGET_VERSION)] = "GET_VERSION",

	[_IOC_NR(SkelDrvrIoctlSET_TIMEOUT)] = "SET_TIMEOUT",
	[_IOC_NR(SkelDrvrIoctlGET_TIMEOUT)] = "GET_TIMEOUT",

	[_IOC_NR(SkelDrvrIoctlSET_QUEUE_FLAG)] = "SET_QUEUE_FLAG",
	[_IOC_NR(SkelDrvrIoctlGET_QUEUE_FLAG)] = "GET_QUEUE_FLAG",
	[_IOC_NR(SkelDrvrIoctlGET_QUEUE_SIZE)] = "GET_QUEUE_SIZE",
	[_IOC_NR(SkelDrvrIoctlGET_QUEUE_OVERFLOW)] = "GET_QUEUE_OVERFLOW",

	[_IOC_NR(SkelDrvrIoctlSET_MODULE)] = "SET_MODULE",
	[_IOC_NR(SkelDrvrIoctlGET_MODULE)] = "GET_MODULE",
	[_IOC_NR(SkelDrvrIoctlGET_MODULE_COUNT)] = "GET_MODULE_COUNT",
	[_IOC_NR(SkelDrvrIoctlGET_MODULE_MAPS)] = "GET_MODULE_MAPS",

	[_IOC_NR(SkelDrvrIoctlCONNECT)] = "CONNECT",
	[_IOC_NR(SkelDrvrIoctlGET_CLIENT_LIST)] = "GET_CLIENT_LIST",
	[_IOC_NR(SkelDrvrIoctlGET_CLIENT_CONNECTIONS)] = "GET_CLIENT_CONNECTIONS",

	[_IOC_NR(SkelDrvrIoctlENABLE)] = "ENABLE",
	[_IOC_NR(SkelDrvrIoctlRESET)] = "RESET",

	[_IOC_NR(SkelDrvrIoctlGET_STATUS)] = "GET_STATUS",
	[_IOC_NR(SkelDrvrIoctlGET_CLEAR_STATUS)] = "GET_CLEAR_STATUS",

	[_IOC_NR(SkelDrvrIoctlRAW_READ)] = "RAW_READ",
	[_IOC_NR(SkelDrvrIoctlRAW_WRITE)] = "RAW_WRITE",

	[_IOC_NR(SkelDrvrIoctlRAW_BLOCK_READ)] = "RAW_BLOCK_READ",
	[_IOC_NR(SkelDrvrIoctlRAW_BLOCK_WRITE)] = "RAW_BLOCK_WRITE",
};

#define SkelDrvrSTANDARD_IOCTL_CALLS ARRAY_SIZE(SkelStandardIoctlNames)

/**
 * Debug flags
 */

static const char *SkelDrvrDebugNames[] = {
	"AssertionViolation",
	"IoctlTrace",
	"ClientWarning",
	"ModuleWarning",
	"Information",
	"EmulationOn"
};

const char *GetDebugFlagName(SkelDrvrDebugFlag debf)
{
	int i;
	uint32_t msk;

	for (i = 0; i < SkelDrvrDEBUG_NAMES; i++) {
		msk = 1 << i;
		if (msk & debf)
			return SkelDrvrDebugNames[i];
	}
	return SkelDrvrDebugNames[0];
}

/**
 * @brief check if an IOCTL belongs to the user's bottom-half
 * @param cm - IOCTL number number
 * @return 1 - if the IOCTL belongs to the user
 * @return 0 - otherwise
 */

#define WITHIN_RANGE(MIN,ARG,MAX) ( ((MAX) >= (ARG)) && ((MIN) <= (ARG)) )

static int is_user_ioctl(int nr)
{
	return WITHIN_RANGE(_IOC_NR(SkelUserIoctlFIRST), _IOC_NR(nr),
			    _IOC_NR(SkelUserIoctlLAST)) &&
	    _IOC_TYPE(nr) == SKELUSER_IOCTL_MAGIC;
}

static const char *GetIoctlName(int nr)
{
	static char dtxt[128];
	int first;

	/* 1st skel's IOCTL */

	first = _IOC_NR(nr) - _IOC_NR(SkelDrvrIoctlSET_DEBUG);

	/* skel's IOCTL */

	if (WITHIN_RANGE(0, first, _IOC_NR(SkelDrvrIoctlLAST_STANDARD)) &&
	    _IOC_TYPE(nr) == SKEL_IOCTL_MAGIC)
		return SkelStandardIoctlNames[_IOC_NR(nr)];

	/* non-skel IOCTL */

	if (is_user_ioctl(nr))
		return SkelUserGetIoctlName(nr);

	/* cannot recognise the IOCTL number */

	sprintf(dtxt, "(illegal)");
	return dtxt;
}

/**
 * @brief debug an IOCTL call
 * @param ccon - client context
 * @param cm - IOCTL command number
 * @param lav - long argument value passed to the IOCTL call
 */

static void DebugIoctl(SkelDrvrClientContext * ccon, int cm, int32_t lav)
{
	if (!(ccon->Debug & SkelDrvrDebugFlagTRACE))
		return;

	report_client(ccon, SkelDrvrDebugFlagTRACE,
		      "module#%d:IOCTL:%s:Arg:%d[0x%x]",
		      ccon->ModuleNumber, GetIoctlName(cm), lav, lav);
}

/**
 * Set global error number
 */

static int drv_errno = 0;

void pseterr(int err)
{
	drv_errno = err;
}

/**
 * @brief Handle timeout callback from a clients read
 * @param ccon points to the clients context
 */

static void client_timeout(SkelDrvrClientContext * ccon)
{
	SkelDrvrQueue *q = &ccon->Queue;
	unsigned long flags;

	spin_lock_irqsave(&q->lock, flags);
	ccon->Queue.Size = 0;
	ccon->Queue.Missed = 0;
	spin_unlock_irqrestore(&q->lock, flags);

	report_client(ccon, SkelDrvrDebugFlagWARNING, "Client Timeout");
}

/**
 * @brief set module context status
 * @param mcon - module context
 * @param flag - flag to set it to
 * @return 0 - on success
 * @return -1 - on failure
 *
 * Note: this function acquires the module's mutex
 */

static int
update_mcon_status(SkelDrvrModuleContext * mcon,
		   SkelDrvrStandardStatus flag)
{
	if (mutex_lock_interruptible(&mcon->mutex)) {
		pseterr(EINTR);
		return SYSERR;
	}
	mcon->StandardStatus |= flag;
	mutex_unlock(&mcon->mutex);

	return OK;
}

static int GetVersion(SkelDrvrModuleContext * mcon, SkelDrvrVersion * ver)
{
	SkelUserReturn sker;

	ver->DriverVersion = COMPILE_TIME;
	sker = SkelUserGetModuleVersion(mcon, ver->ModuleVersion);
	if (sker) {
		pseterr(EACCES);
		return SYSERR;
	}
	return OK;
}

/*
 * This RawIo implementation permits transfer of blocks of data to or from user space
 * to the hardware via an inermiediate paging kernel buffer. The RawIoBlock structure
 * controls how the transfer takes place.
 *
 * RawIoBlock:
 *
 *    SpaceNumber;  Address space to read/write
 *    Offset;       Hardware address byte offset
 *    DataWidth;    Data width in bits: 0 Default, 8, 16, 32
 *    AddrIncr;     Address increment: 0=FIFO 1=Normal else skip
 *    BytesTr;      Byte size of the transfer
 *   *Data;         Data buffer pointer
 *
 * flag is zero on read and non zero on write
 *
 */

#define iowrite16le iowrite16
#define iowrite32le iowrite32

#define ioread16le ioread16
#define ioread32le ioread32

static unsigned int
RawIoBlock(SkelDrvrModuleContext * mcon,
	   SkelDrvrRawIoTransferBlock * riob, int flag)
{
#define TRANSFERS 1024

	InsLibAnyAddressSpace *anyas = NULL;
	InsLibModlDesc *modld = NULL;
	char *cp;

	int tszbt;		/* One transfer data item size in bits */
	int tszby;		/* One transfer data item size in bytes  */
	int tremg;		/* Transfer data items remaining  */
	int uindx;		/* Byte index into user array */
	int kindx;		/* Byte index in kernel buffer */
	int hskip;		/* Skip hardware byte index */
	int bendf;		/* Big Endian flag */
	int ksize;		/* Kernal buffer size */
	void *mmap;		/* Hardware module memory map */
	void *kbuf;		/* Kernel transfer buffer */
	void *kp;		/* Running kernel buffer pointer */
	void *mp;		/* Running modle buffer pointer */

	int cc;

	modld = mcon->Modld;
	anyas = InsLibGetAddressSpace(modld, riob->SpaceNumber);

	if (!anyas) {
		report_module(mcon, SkelDrvrDebugFlagMODULE,
			      "%s:IllegalAddressSpace", __FUNCTION__);
		pseterr(ENXIO);
		return SYSERR;
	}

	/*
	 * In some cases (mainly PCI), the size of the mapping is not
	 * provided in the XML file. In Lynx there's no easy way to
	 * get the mapping size, so we just leave it up to the user
	 * not to cause a bus error.
	 */

	if (anyas->WindowSize && riob->Offset >= anyas->WindowSize) {
		report_module(mcon, SkelDrvrDebugFlagMODULE,
			      "%s: Offset out of range", __FUNCTION__);
		pseterr(EINVAL);
		return SYSERR;
	}

	tszbt = riob->DataWidth & 0x3F;			   /* Can be 8, 16, 32 etc */
	if (tszbt <= 0)
		tszbt = anyas->DataWidth;		   /* Not specified take default */
	tszby = tszbt / 8;				   /* Size of one transfer entity in bytes */
	tremg = riob->BytesTr / tszby;			   /* Number of transfers to perform */
	if (tremg <= 0)
		tremg = 1;				   /* At least one transfer */

	if (tremg < TRANSFERS)
		ksize = tremg * tszby;			   /* One transfer from small buffer */
	else
		ksize = TRANSFERS * tszby;		   /* Multiple transfers from kernel buffer */
	kbuf = kmalloc(ksize, GFP_KERNEL);
	if (kbuf == NULL) {
		pseterr(ENOMEM);
		return SYSERR;
	}

	cp = anyas->Mapped;				   /* Point to address map */
	mmap = &cp[riob->Offset];			   /* and get the address at offset */

	bendf = (anyas->Endian == InsLibEndianBIG);	   /* Set BIG endian boolean */
	uindx = 0;
	kindx = 0;
	hskip = 0;					   /* Index into map array */

	/*
	 * If writing fill up the kernel buffer from user space
	 */

	if (flag) {
		cc = copy_from_user(kbuf, riob->Data, ksize);   /* Get User memory if writing */
		if (cc) {
			pseterr(EACCES);
			return SYSERR;
		}
	}

	/*
	 * While transfers remaining loop until all done
	 */

	while (tremg-- > 0) {

		kp = kbuf + kindx;			   /* Next kernel buffer address */
		mp = mmap + hskip;			   /* Next module address */

		if (flag) {
			switch (tszbt) {
			default:
			case 8:
				iowrite8(*(char *) kp, mp);
				break;
			case 16:
				if (bendf)
					iowrite16be(*(short *) kp, mp);
				else
					iowrite16le(*(short *) kp, mp);
				break;
			case 32:
				if (bendf)
					iowrite32be(*(int *) kp, mp);
				else
					iowrite32le(*(int *) kp, mp);
				break;
			}
		} else {
			switch (tszbt) {
			default:
			case 8:
				*(char *) kp = ioread8(mmap);
				break;
			case 16:
				if (bendf)
					*(short *) kp = ioread16be(mp);
				else
					*(short *) kp = ioread16le(mp);
				break;
			case 32:
				if (bendf)
					*(int *) kp = ioread32be(mp);
				else
					*(int *) kp = ioread32le(mp);
				break;
			}
		}

		/*
		 * Deal wit indexes into buffers
		 */

		hskip += riob->AddrIncr * tszby;	   /* Module hardware skip index */
		kindx += tszby;				   /* Kernel buffer index */

		/*
		 * Flush the kernel buffer
		 */

		if (kindx >= ksize) {
			if (flag)
				cc = copy_from_user(kbuf, &riob->Data[uindx], ksize); /* Flush out data to user */
			else
				cc = copy_to_user(&riob->Data[uindx], kbuf, ksize);   /* Flush in data from user */
			if (cc) {
				pseterr(EACCES);
				return SYSERR;
			}

			uindx += ksize;			   /* Next block to copy */
			kindx = 0;			   /* Running kernel buffer index reset for next loop or exit */
		}
	}

	/*
	 * If reading copy any remaining data to user space
	 */

	if ((!flag) && (kindx))
		cc = copy_to_user(&riob->Data[uindx], kbuf, kindx);
		if (cc) {
			pseterr(EACCES);
			return SYSERR;
		}

	kfree(kbuf);
	return OK;
}

/*
 * Single item transfer legacy routine
 */

static unsigned int
RawIo(SkelDrvrModuleContext * mcon, SkelDrvrRawIoBlock * riob, int flag)
{
	InsLibAnyAddressSpace *anyas = NULL;
	InsLibModlDesc *modld = NULL;
	void *mmap = NULL;
	char *cp;

	modld = mcon->Modld;
	anyas = InsLibGetAddressSpaceWidth(modld, riob->SpaceNumber,
					   riob->DataWidth);

	if (!anyas) {
		report_module(mcon, SkelDrvrDebugFlagMODULE,
			      "%s:IllegalAddressSpace", __FUNCTION__);
		pseterr(ENXIO);
		return SYSERR;
	}

	/*
	 * In some cases (mainly PCI), the size of the mapping is not
	 * provided in the XML file. In Lynx there's no easy way to
	 * get the mapping size, so we just leave it up to the user
	 * not to cause a bus error.
	 */

	if (anyas->WindowSize && riob->Offset >= anyas->WindowSize) {
		report_module(mcon, SkelDrvrDebugFlagMODULE,
			      "%s: Offset out of range", __FUNCTION__);
		pseterr(EINVAL);
		return SYSERR;
	}

	if (!riob->DataWidth)
		riob->DataWidth = anyas->DataWidth;
	cp = anyas->Mapped;
	mmap = &cp[riob->Offset];

	if (flag) {
		switch (riob->DataWidth) {
		case 8:
			if (anyas->Endian == InsLibEndianBIG)
				iowrite8(riob->Data, mmap);
			else
				iowrite8(riob->Data, mmap);
			return OK;
		case 16:
			if (anyas->Endian == InsLibEndianBIG)
				iowrite16be(riob->Data, mmap);
			else
				iowrite16le(riob->Data, mmap);
			return OK;
		case 32:
			if (anyas->Endian == InsLibEndianBIG)
				iowrite32be(riob->Data, mmap);
			else
				iowrite32le(riob->Data, mmap);
			return OK;
		default:
			pseterr(EINVAL);
			return SYSERR;
		}
	} else {
		switch (riob->DataWidth) {
		case 8:
			if (anyas->Endian == InsLibEndianBIG)
				riob->Data = ioread8(mmap);
			else
				riob->Data = ioread8(mmap);
			return OK;
		case 16:
			if (anyas->Endian == InsLibEndianBIG)
				riob->Data = ioread16be(mmap);
			else
				riob->Data = ioread16le(mmap);
			return OK;
		case 32:
			if (anyas->Endian == InsLibEndianBIG)
				riob->Data = ioread32be(mmap);
			else
				riob->Data = ioread32le(mmap);
			return OK;
		default:
			pseterr(EINVAL);
			return SYSERR;
		}
	}
}

static int Reset(SkelDrvrModuleContext * mcon)
{
	SkelDrvrModConn *connected = &mcon->Connected;
	SkelUserReturn sker;

	if (mutex_lock_interruptible(&mcon->mutex)) {
		pseterr(EINTR);
		return SYSERR;
	}
	sker = SkelUserHardwareReset(mcon);
	if (sker == SkelUserReturnFAILED)
		return SYSERR;

	sker = SkelUserEnableInterrupts(mcon, connected->enabled_ints);
	if (sker == SkelUserReturnFAILED)
		return SYSERR;

	mutex_unlock(&mcon->mutex);
	return OK;
}

static int GetStatus(SkelDrvrModuleContext * mcon, SkelDrvrStatus * ssts)
{
	SkelUserReturn sker;

	sker = SkelUserGetHardwareStatus(mcon, &ssts->HardwareStatus);
	if (sker == SkelUserReturnFAILED)
		return SYSERR;

	ssts->StandardStatus = mcon->StandardStatus;
	return OK;
}

/*
 * call with the queue's lock held
 */

static inline void
__q_put(const SkelDrvrReadBuf * rb, SkelDrvrClientContext * ccon)
{
	SkelDrvrQueue *q = &ccon->Queue;

	q->Entries[q->WrPntr] = *rb;
	q->WrPntr = (q->WrPntr + 1) % SkelDrvrQUEUE_SIZE;

	if (q->Size < SkelDrvrQUEUE_SIZE) {
		q->Size++;             /** Skel queue size */
		ccon->waitc++;         /** wait_queue counter */
		wake_up(&ccon->waitq);
	} else {
		q->Missed++;
		q->RdPntr = (q->RdPntr + 1) % SkelDrvrQUEUE_SIZE;
	}
}

/*
 * put read buffer on the queue of a client
 */

static inline void
q_put(const SkelDrvrReadBuf * rb, SkelDrvrClientContext * ccon)
{
	unsigned long flags;

	spin_lock_irqsave(&ccon->Queue.lock, flags);
	__q_put(rb, ccon);
	spin_unlock_irqrestore(&ccon->Queue.lock, flags);
}

/**
 * @brief put the 'read buffer' @rb on the queues of clients
 * @param rb - read buffer to be put in the queues
 * @param client_list - list of clients whos queues are to be appended
 */

static inline void
put_queues(const SkelDrvrReadBuf * rb, struct list_head *client_list)
{
	struct client_link *entry;

	list_for_each_entry(entry, client_list, list)
		q_put(rb, entry->context);
}

/* Note: call this function with @connected's lock held */

static inline void
__fill_clients_queues(SkelDrvrModConn * connected,
		      const SkelDrvrReadBuf * rb, const uint32_t imask)
{
	struct list_head *client_list;
	uint32_t interrupt;
	SkelDrvrReadBuf rbuf = *rb;	/* local copy of rb */
	int i;

	for (i = 0; i < SkelDrvrINTERRUPTS; i++) {

		interrupt = imask & (1 << i);
		client_list = &connected->clients[i];
		if (!interrupt || list_empty(client_list))
			continue;

		/* set one bit at a time on the clients' queues */

		rbuf.Connection.ConMask = interrupt;
		put_queues(&rbuf, client_list);
	}
}

/**
 * @brief fill the queues of clients connected to the interrupt given by mask
 * @param connected - connections on the module struct
 * @param rb - read buffer to put in the queues
 * @param imask - interrupt mask
 */

static inline void
fill_clients_queues(SkelDrvrModConn * connected,
		    const SkelDrvrReadBuf * rb, const uint32_t imask)
{
	unsigned long flags;

	spin_lock_irqsave(&connected->lock, flags);
	__fill_clients_queues(connected, rb, imask);
	spin_unlock_irqrestore(&connected->lock, flags);
}

/**
 * @brief interrupt handler---reads the HW and update clients' queues
 * @param cookie - module context
 * @return 0 - on success
 * @return -1 - on failure
 */

static irqreturn_t IntrHandler(void *cookie)
{
	SkelDrvrModuleContext *mcon = (SkelDrvrModuleContext *) cookie;
	SkelDrvrModConn *connected;
	SkelDrvrReadBuf rb;
	uint32_t interrupt;
	SkelUserReturn sker;

	/* initialise read buffer */

	rb.Connection.Module = mcon->ModuleNumber;

	/* read interrupt mask from hardware */

	sker = SkelUserGetInterruptSource(mcon, &rb.Time, &interrupt);
	if (sker || !interrupt)
		return IRQ_NONE;

	connected = &mcon->Connected;

	fill_clients_queues(connected, &rb, interrupt);

	return IRQ_HANDLED;
}

/**
 * @brief skel interrupt handler switch
 * @param cookie - cookie passed to the ISR
 * @return returned value from the interrupt handler
 *
 * Depending on skel's configuration, either the user's or the default
 * interrupt handler is used to serve interrupts.
 */

int skel_isr(void *cookie)
{
	int cc;
	if (SkelConf.intrhandler) {
		cc = SkelConf.intrhandler(cookie);
		if (cc) return IRQ_NONE;
		return IRQ_HANDLED;
	}
	return IntrHandler(cookie);
}

/**
 * @brief remove a module, given by its module context
 * @param mcon - module context
 *
 * This function is called before freeing a module context. It unhooks
 * the user's bottom-half, unregisters the ISR and unmaps any memory
 * mappings that may have been done. All this when applicable and
 * for any kind of supported bus.
 */

static void RemoveModule(SkelDrvrModuleContext * mcon)
{
	if (mcon->StandardStatus & SkelDrvrStandardStatusEMULATION)
		return;					   /* no ISR, no mappings.. exit */

	/* unhook user's stuff */

	SkelUserModuleRelease(mcon);

	switch (mcon->Modld->BusType) {
	case InsLibBusTypePMC:
	case InsLibBusTypePCI:
		return RemovePciModule(mcon);
	case InsLibBusTypeVME:
		return RemoveVmeModule(mcon);
	case InsLibBusTypeCARRIER:
		return RemoveCarModule(mcon);
	default:
		break;
	}
}

/**
 * @brief get the address of a module's context
 * @param modnr - module number (verbatim from the XML file)
 * @return address of the modules' mcon - on success
 * @return NULL - if the module is not found
 */

SkelDrvrModuleContext *get_mcon(int modnr)
{
	SkelDrvrModuleContext *m;
	int i;

	for (i = 0; i < Wa->InstalledModules; i++) {
		m = &Wa->Modules[i];
		if (m->ModuleNumber == modnr)
			return m;
	}
	return NULL;
}

/* Search the list for a given client context in the client list */
/* WARNING: Must be locked else where */
/* Returns pointer to entry if found or null */

static struct client_link *__get_client(SkelDrvrClientContext * ccon,
					struct list_head *client_list)
{

	struct client_link *found = NULL;
	struct client_link *entry;

	list_for_each_entry(entry, client_list, list) {
		if (entry->context == ccon) {
			found = entry;
			break;
		}
	}
	return found;
}

/**
 * @brief connect a client to an interrupt
 * @param ccon - client context
 * @param conx - connection description
 */

static void Connect(SkelDrvrClientContext * ccon,
		    SkelDrvrConnection * conx)
{
	SkelDrvrModuleContext *mcon;
	SkelDrvrModConn *connected;
	struct client_link *alloc_link;
	struct client_link *link;
	unsigned int j, imsk;
	unsigned long flags;
	SkelUserReturn sker;

	if (!conx->Module)
		mcon = get_mcon(ccon->ModuleNumber);
	else
		mcon = get_mcon(conx->Module);

	if (mcon == NULL)
		return;

	connected = &mcon->Connected;
	imsk = conx->ConMask;

	for (j = 0; j < SkelDrvrINTERRUPTS; j++) {
		if (!(imsk & (1 << j)))
			continue;
		/*
		 * We may not need this link, but we cannot allocate memory
		 * (sleep) in an atomic section, so we pre-allocate it here.
		 */

		alloc_link = (struct client_link *)
		    kmalloc(sizeof(*alloc_link), GFP_KERNEL);

		spin_lock_irqsave(&connected->lock, flags);

		/*
		 * Algorithm:
		 * Check if the client is already in the list of connections
		 * If it is, free the allocated link
		 * If it isn't, add it to the list using the allocated link
		 */

		link = __get_client(ccon, &connected->clients[j]);
		if (link) {
			if (alloc_link) {
				kfree((void *) alloc_link);
				alloc_link = NULL;
			}
		} else {
			if (alloc_link == NULL) {
				SK_WARN("ENOMEM adding a client link");
			} else {
				alloc_link->context = ccon;
				list_add(&alloc_link->list,
					 &connected->clients[j]);
			}
		}

		/* if any of these is set, we succeeded in adding the connection */

		if (link || alloc_link) {
			connected->enabled_ints |= imsk;
			sker = SkelUserEnableInterrupts(mcon,
							connected->enabled_ints);
		}
		spin_unlock_irqrestore(&connected->lock, flags);
	}
}

/**
 * @brief disconnect a client from certain interrupts
 * @param ccon - client context
 * @param conx - module and interrupts to disconnect from
 *
 * If no module is specificied, current client's module is taken.
 * An empty interrupt mask means 'disconnect from all the interrupts for
 * this module'
 */

static void DisConnect(SkelDrvrClientContext * ccon,
		       SkelDrvrConnection * conx)
{
	SkelDrvrModuleContext *mcon;
	SkelDrvrModConn *connected;
	struct client_link *link;
	unsigned int j, imsk;
	unsigned long flags;
	SkelUserReturn sker;

	if (!conx->Module)
		mcon = get_mcon(ccon->ModuleNumber);
	else
		mcon = get_mcon(conx->Module);

	if (mcon == NULL)
		return;

	connected = &mcon->Connected;

	/* Disconnect from all the ints on this module */

	if (!conx->ConMask)
		conx->ConMask = ~conx->ConMask;

	imsk = conx->ConMask;

	spin_lock_irqsave(&connected->lock, flags);

	for (j = 0; j < SkelDrvrINTERRUPTS; j++) {

		/* check interrupt mask and that there are clients connected */

		if (!(imsk & (1 << j))
		||  list_empty(&connected->clients[j]))
			continue;

		link = __get_client(ccon, &connected->clients[j]);
		if (!link)
			continue;
		list_del(&link->list);
		kfree((void *) link);

		/*
		 * if there are no more clients connected to it, disable the
		 * interrupt on the module
		 */

		if (list_empty(&connected->clients[j])) {
			connected->enabled_ints &= ~imsk;
			sker = SkelUserEnableInterrupts(mcon,
							connected->enabled_ints);
		}
	}

	spin_unlock_irqrestore(&connected->lock, flags);
}

/**
 * @brief disconnect a client from all the interrupts on any module
 * @param ccon - client context
 */

static void DisConnectAll(SkelDrvrClientContext * ccon)
{

	SkelDrvrConnection conx;
	unsigned int i;

	conx.ConMask = 0;
	for (i = 0; i < Wa->InstalledModules; i++) {
		conx.Module = Wa->Modules[i].ModuleNumber;
		DisConnect(ccon, &conx);
	}
}

static int set_mcon_defaults(SkelDrvrModuleContext * mcon)
{
	int i;

	/* default timeout */

	mcon->Timeout = SkelDrvrDEFAULT_MODULE_TIMEOUT;

	/* debugging */

	mcon->Debug = Wa->Drvrd->DebugFlags;

	/* emulation */

	if (Wa->Drvrd->EmulationFlags) {
		mcon->Debug |= SkelDrvrDebugFlagEMULATION;
		mcon->StandardStatus |= SkelDrvrStandardStatusEMULATION;
	}

	/* initialise the connection's lock */

	spin_lock_init(&mcon->Connected.lock);

	/* initialise the mutex */

	mutex_init(&mcon->mutex);

	for (i = 0; i < SkelDrvrINTERRUPTS; i++)
		INIT_LIST_HEAD(&mcon->Connected.clients[i]);

	/*
	 * Set the module status to NO_ISR as a default.
	 * If there's an ISR, the ISR installers will clear this bit.
	 */

	return(update_mcon_status(mcon, SkelDrvrStandardStatusNO_ISR));
}

/**
 * @brief unmap mapping flagged as 'FreeModifierFlag'
 * @param mcon - module context
 *
 * An address space with the 'FreeModifierFlag' set will be erased shortly after
 * being mapped; in between, a SkelUser function to initialise the module is
 * called.
 */

static void unmap_flagged(SkelDrvrModuleContext * mcon)
{
	InsLibBusType bus = mcon->Modld->BusType;

	switch (bus) {
	case InsLibBusTypeVME:

		/* The zero in par 2 only releases flagged */

		unmap_vmeas(mcon, 0);
		break;

	/* do nothing for PCI or Carrier */

	case InsLibBusTypePMC:
	case InsLibBusTypePCI:
	case InsLibBusTypeCARRIER:
		break;
	default:
		break;
	}
}

/**
 * @brief start-up a module; put it into a 'ready-to-use' state
 * @param mcon - module context
 * @return 0 - on failure
 * @return 1 - on success
 */

static int AddModule(SkelDrvrModuleContext * mcon)
{
	InsLibBusType bus;
	int mod_ok;

	bus = mcon->Modld->BusType;

	switch (bus) {

	case InsLibBusTypeVME:
		mod_ok = AddVmeModule(mcon);
		break;

	case InsLibBusTypePMC:
	case InsLibBusTypePCI:
		mod_ok = AddPciModule(mcon);
		break;

	case InsLibBusTypeCARRIER:
		mod_ok = AddCarModule(mcon);
		break;

	default:
		SK_ERROR("Unsupported BUS type: %d", bus);
		mod_ok = 0;
	}

	if (!mod_ok)					   /* AddModule didn't work */
		return 0;

	/* user's module installation bottom-half */

	mod_ok = SkelUserModuleInit(mcon);

	if (mod_ok == SkelUserReturnFAILED) {

		/* abort: delete mappings, unregister ISR */

		RemoveModule(mcon);
		return 0;
	}

	/*
	 * Sometimes the user just wants to:
	 * 1. Map a certain hardware region
	 * 2. Configure it (that's why SkelUserModuleInit is for)
	 * 3. Unmap it (so that we don't run out of address space)
	 * So at this stage we check for each of the mappings and unmap
	 * them if the flag 'FreeModifierFlag' is set.
	 */

	unmap_flagged(mcon);

	/* everything OK */

	Wa->InstalledModules++;

	return 1;
}

/**
 * @brief allocate and initialise the working area
 * @param drvrd - driver descriptor
 * @return 0 - on failure
 * @return 1 - on success
 *
 * Note: do not use SK_INFO and friends before Wa is properly set.
 */

static int wa_init(InsLibDrvrDesc * drvrd)
{
	SkelDrvrWorkingArea *wa;

	/* Allocate the driver working area. */

	wa = (SkelDrvrWorkingArea *) kmalloc(sizeof(SkelDrvrWorkingArea),
					     GFP_KERNEL);
	if (wa == NULL) {
		printk
		    ("Skel: Cannot allocate enough memory (WorkingArea)");
		pseterr(ENOMEM);
		return 0;
	}

	Wa = wa;					   /* Global working area pointer */

	/* initialise Wa */

	memset(Wa, 0, sizeof(SkelDrvrWorkingArea));

#ifdef __BIG_ENDIAN__
	Wa->Endian = InsLibEndianBIG;
	printk("Skel:Big endian platform\n");
#else
	Wa->Endian = InsLibEndianLITTLE;
	printk("Skel:Little endian platform\n");
#endif

	/* hook driver description onto Wa */

	Wa->Drvrd = drvrd;

	return 1;
}

/**
 * @brief install modules defined in the parameter list
 * @return -1 -- didn't install all the modules requested
 * @return -2 -- requested module amount exeeds supported one
 * @return  0 -- all OK
 */

static int modules_install(void)
{
	SkelDrvrModuleContext *mcon;
	InsLibModlDesc *modld;
	int mod_ok;
	int i, rc = 0;

	/* safety first */

	if (Wa->Drvrd->ModuleCount > SkelDrvrMODULE_CONTEXTS) {
		SK_ERROR("Requested Module amount (%d) exeeds supported"
			 " one (%d). SkelDrvrMODULE_CONTEXTS should be"
			 " extended\n",
			 Wa->Drvrd->ModuleCount, SkelDrvrMODULE_CONTEXTS);
		return -2;
	}

	modld = Wa->Drvrd->Modules;

	/*
	 * go through the module descriptions and install the hardware.
	 * In case of failure, the subscript 'i' is not incremented.
	 */

	for (i = 0; i < Wa->Drvrd->ModuleCount;) {
		if (!modld)
			break;

		/* initialise the module context */

		mcon = &Wa->Modules[i];
		mcon->Modld = modld;
		mcon->ModuleNumber = modld->ModuleNumber;

		/* the rest of mcon are the default values */

		if (set_mcon_defaults(mcon)) {
			rc = -1;
			break;
		}

		/*
		 * once mcon is set-up, create the mappings, register the ISR
		 * and initialise the hardware
		 */

		mod_ok = AddModule(mcon);
		if (mod_ok) {
			SK_INFO("Module#%d installed OK",
				modld->ModuleNumber);

			mcon->InUse = 1;
			i++;				   /* ensure that there are no gaps in Wa->Modules */
		} else {
			SK_WARN("Error installing Module#%d",
				modld->ModuleNumber);
			rc = -1;
		}

		modld = modld->Next;
	}

	return rc;
}

/**
 * Provide standard driver entries
 * ===============================
 */

int skel_drv_open(struct inode *inode, struct file *filp);
int skel_drv_close(struct inode *inode, struct file *filp);
long skel_drv_ioctl_ulck(struct file *filp, unsigned int cmd, unsigned long arg);
int     skel_drv_ioctl_lck(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
ssize_t skel_drv_read(struct file *filp, char *buf, size_t count, loff_t * f_pos);
ssize_t skel_drv_write(struct file *filp, const char *buf, size_t count, loff_t * f_pos);

/**
 * Set up fops
 */

struct file_operations skel_drvr_fops = {

	.owner          = THIS_MODULE,
	.read           = skel_drv_read,
	.write          = skel_drv_write,
	.unlocked_ioctl = skel_drv_ioctl_ulck,
	.open           = skel_drv_open,
	.release        = skel_drv_close,
};

static int skel_major = 0;

int skel_drv_install(void)
{
	InsLibDrvrDesc *drvrd;
	int cc;

/**
 * Build the tree from module parameters. At this time only VME configurations are
 * supported. Some parameters such as address modifiers, mapping window sizes and
 * data item widths (D8,D16,D32...) must be set in skeluser.h as these values are
 * assumed to be the buisness of the driver implementor.
 * No PCI or CAR skel drivers are currently supported, build_driver would need
 * extending if this was needed.
 */

	drvrd = build_drvr();
	if (drvrd == NULL) {
		printk("Skel: build_drvr failed\n");
		pseterr(ENOMEM);
		return SYSERR;
	}

	skel_major = register_chrdev(0, SkelDrvrNAME, &skel_drvr_fops);
	if (skel_major < 0)
		return skel_major;

	printk("%s: driver major is:%d\n",SkelDrvrNAME,skel_major);

	if (!wa_init(drvrd)) {
		printk("Skel: Working Area Initialisation failed\n");
		pseterr(ENOMEM);
		goto out_err;
	}

	if (!Wa->Drvrd->ModuleCount) {
		SK_WARN
		    ("BUG in the descriptor tree: ModuleCount is empty");
		pseterr(EINVAL);
		goto out_err;
	}

	cc = modules_install();
	if (cc < 0) {
		SK_WARN("Module installation error:%d\n", cc);
		pseterr(EINVAL);
		goto out_err;
	}
	SK_INFO("Modules installed OK\n");

	/* initialise the client list spinlock */

	spin_lock_init(&Wa->list_lock);
	INIT_LIST_HEAD(&Wa->clients);

	if (Wa->InstalledModules != Wa->Drvrd->ModuleCount) {
		if (!Wa->InstalledModules) {
			SK_WARN("Not installed any modules");
			pseterr(ENODEV);
			goto out_err;
		} else {
			SK_WARN("Only installed %d out of %d modules",
				Wa->InstalledModules,
				Wa->Drvrd->ModuleCount);
		}
	} else {
		SK_INFO("Installed %d (out of %d) modules",
			Wa->InstalledModules, Wa->Drvrd->ModuleCount);
	}
	return OK;

	/** Error exit */

out_err:
	if (drvrd)
		InsLibFreeDriver(drvrd);
	if (Wa) {
		kfree((void *) Wa);
		Wa = NULL;
	}

	printk("%s: Failed to install\n",SkelDrvrNAME);

	return SYSERR;
}

/* Wa->list_lock should be held upon calling this function */

static void __skel_remove_ccon(SkelDrvrClientContext * ccon)
{
	struct client_link *client;

	client = __get_client(ccon, &Wa->clients);
	if (client) {
		list_del(&client->list);
		kfree((void *) client);
	}
}

/*
 * Close down a client context
 * Wa->list_lock should be called upon calling this function
 */

static void __do_close(SkelDrvrClientContext * ccon)
{
	DisConnectAll(ccon);
	SkelUserClientRelease(ccon);
	__skel_remove_ccon(ccon);
	kfree((void *) ccon);
}

static void do_close(SkelDrvrClientContext * ccon)
{
	unsigned long flags;

	spin_lock_irqsave(&Wa->list_lock, flags);
	__do_close(ccon);
	spin_unlock_irqrestore(&Wa->list_lock, flags);
}

/* Note: call with the queue's lock held */

static void __reset_queue(SkelDrvrClientContext * ccon)
{
	SkelDrvrQueue *q = &ccon->Queue;

	q->Size = 0;
	q->RdPntr = 0;
	q->WrPntr = 0;
	q->Missed = 0;
}

static void reset_queue(SkelDrvrClientContext * ccon)
{
	unsigned long flags;

	spin_lock_irqsave(&ccon->Queue.lock, flags);
	__reset_queue(ccon);
	spin_unlock_irqrestore(&ccon->Queue.lock, flags);
}

/*
 * initialise a client's context
 * @return 0 - on success
 * @return -1 - on failure
 *
 * Don't get too confused about wait_queues and skel_queues
 */

int client_init(SkelDrvrClientContext * ccon, struct file *flp, int modnum)
{
	int client_ok;

	memset(ccon, 0, sizeof(SkelDrvrClientContext));

	ccon->filep = flp;				   /* save file pointer */
	ccon->Timeout = SkelDrvrDEFAULT_CLIENT_TIMEOUT;
	ccon->Pid = current->pid;

	/* select by default the first installed module */

	if (Wa->InstalledModules) {
		if (!get_mcon(modnum))
			modnum = Wa->Modules[0].ModuleNumber;
		ccon->ModuleNumber = modnum;
	}

	spin_lock_init(&ccon->Queue.lock);

	init_waitqueue_head(&ccon->waitq);
	ccon->waitc = 0;

	reset_queue(ccon);

	/* user's client initialisation bottom-half */

	client_ok = SkelUserClientInit(ccon);
	if (client_ok == SkelUserReturnFAILED)             /* errno must be set */
		return -1;

	return 0;
}

static struct client_link *skel_add_ccon(SkelDrvrClientContext * ccon)
{
	struct client_link *entry;
	unsigned long flags;

	entry = (struct client_link *) kmalloc(sizeof(*entry), GFP_KERNEL);
	if (entry == NULL)
		return NULL;

	entry->context = ccon;
	spin_lock_irqsave(&Wa->list_lock, flags);
	list_add(&entry->list, &Wa->clients);
	spin_unlock_irqrestore(&Wa->list_lock, flags);

	return entry;
}

/**
 * Main skel Open logic, uses file private data to store
 * the client context pointer
 */

int SkelDrvrOpen(struct file *flp, int modnum)
{
	SkelDrvrClientContext *ccon = NULL;	/* Client context */

	if (Wa == NULL) {
		pseterr(ENODEV);
		return SYSERR;
	}

	ccon = (SkelDrvrClientContext *)
	    kmalloc(sizeof(SkelDrvrClientContext), GFP_KERNEL);
	if (ccon == NULL) {
		pseterr(ENOMEM);
		return SYSERR;
	}
	flp->private_data = ccon;

	if (client_init(ccon, flp, modnum)) {
		printk("%s: client_init failed\n",SkelDrvrNAME);
		pseterr(EBADF);
		goto out_free;
	}

	if (skel_add_ccon(ccon) == NULL) {
		printk("%s: skel_add_ccon failed\n",SkelDrvrNAME);
		pseterr(ENOMEM);
		goto out_free;
	}
	return OK;

out_free:
	if (ccon)
		kfree((void *) ccon);

	printk("%s: Failed to open\n",SkelDrvrNAME);
	return SYSERR;
}

/**
 * @brief Close down a client
 * @param wa  -- working area
 * @param flp -- file pointer
 * @return SYSERR -- failed
 * @return OK     -- success
 */

int SkelDrvrClose(void *wa, struct file *flp)
{
	SkelDrvrClientContext *ccon;	/* Client context */

	ccon = flp->private_data;
	if (ccon == NULL) {
		printk("Skel:Close:Bad File Descriptor\n");
		pseterr(EBADF);
		return SYSERR;
	}
	do_close(ccon);
	return OK;
}

/**
 * @brief uninstall all the modules
 */

static void modules_uninstall(void)
{
	SkelDrvrModuleContext *mcon;
	int i;

	/* Uninstall all the modules */

	for (i = 0; i < Wa->InstalledModules; i++) {
		mcon = &Wa->Modules[i];

		SK_INFO("Uninstalling Module#%d (out of %d)",
			mcon->ModuleNumber, Wa->InstalledModules);
		RemoveModule(mcon);
	}
}

void skel_drv_uninstall(void)
{
	unsigned long flags;

	if (Wa == NULL)
		return;

	spin_lock_irqsave(&Wa->list_lock, flags);

	if (!list_empty(&Wa->clients)) {

		/* A Non null client list means its still open */

		pseterr(EBUSY);

		spin_unlock_irqrestore(&Wa->list_lock, flags);
		return;
	}

	spin_unlock_irqrestore(&Wa->list_lock, flags);

	SK_INFO("Uninstalling driver...");

	modules_uninstall();

	SK_INFO("Driver uninstalled");

	InsLibFreeDriver(Wa->Drvrd);
	kfree((void *) Wa);
	Wa = NULL;

	unregister_chrdev(skel_major,SkelDrvrNAME);
	return;
}

/* Note: call with the queue's lock held */

static void __q_get(SkelDrvrReadBuf * rb, SkelDrvrQueue * q)
{
	*rb = q->Entries[q->RdPntr];
	q->RdPntr = (q->RdPntr + 1) % SkelDrvrQUEUE_SIZE;
	q->Size--;
}

/**
 * @brief read from a client's queue
 * @param rb - read buffer to put the read information in
 * @param ccon - client context
 */

static void q_get(SkelDrvrReadBuf * rb, SkelDrvrClientContext * ccon)
{
	SkelDrvrQueue *q = &ccon->Queue;
	unsigned long flags;

	spin_lock_irqsave(&q->lock, flags);
	__q_get(rb, q);
	spin_unlock_irqrestore(&q->lock, flags);
}

/**
 * Main skel read implementation. This logic now uses a wait queue.
 * Read is blocking and terminates on data on the skel queue or on
 * a timeout.
 */

static int SkelDrvrRead(void *wa, struct file *flp, char *u_buf, int len)
{
	SkelDrvrClientContext *ccon;
	SkelDrvrQueue *q;
	SkelDrvrReadBuf *rb;
	unsigned long flags;
	unsigned int waitc, cc;

	ccon = flp->private_data;
	if (ccon == NULL) {
		printk("Skel:Read:Bad File Descriptor\n");
		pseterr(EBADF);
		return SYSERR;
	}
	q = &ccon->Queue;

	spin_lock_irqsave(&q->lock, flags);
	if (q->QueueOff)
		__reset_queue(ccon);
	spin_unlock_irqrestore(&q->lock, flags);

	/**
	 * If there is data already on the skel queue just return it.
	 * The wait_event will only return when new data is available.
	 * This logic takes a snap shot of the wait_queue counter
	 * to be sure an interrupt occured.
	 */

	if ((ccon->Queue.Size == 0)            /* Empty ? */
	||  (ccon->Queue.QueueOff)) {          /* QueueOff ? */

		/* wait for something new in the queue */

		waitc = ccon->waitc;            /* Counter of wake_up calls */
		if (ccon->Timeout)
			cc = wait_event_interruptible_timeout(ccon->waitq,
							      waitc != ccon->waitc,
							      ccon->Timeout);
		else
			cc = wait_event_interruptible(ccon->waitq,
						      waitc != ccon->waitc);

		if (cc == -ERESTARTSYS) {
			client_timeout(ccon);
			pseterr(EINTR);
			return 0;
		}

		if (cc == 0 && ccon->Timeout)
			return -ETIME;

		if (cc < 0) {
			client_timeout(ccon);
			pseterr(EINTR);
			return 0;
		}
	}

	/* read from the queue */

	rb = (SkelDrvrReadBuf *) u_buf;
	q_get(rb, ccon);

	return sizeof(SkelDrvrReadBuf);
}

/*
 * read entry point switch
 * Depending on skel's configuration, either the user's or the default
 */

int skel_read(void *wa, struct file *f, char *buf, int len)
{
	if (SkelConf.read)
		return SkelConf.read(wa, f, buf, len);
	return SkelDrvrRead(wa, f, buf, len);
}

/*
 * The default write entry point simulates interrupts
 */

static int SkelDrvrWrite(void *wa, struct file *flp, char *u_buf, int len)
{
	SkelDrvrModuleContext *mcon;
	SkelDrvrConnection *conx;
	SkelDrvrModConn *connected;
	SkelDrvrReadBuf rb;
	SkelUserReturn sker;

	conx = (SkelDrvrConnection *) u_buf;

	mcon = get_mcon(conx->Module);
	if (mcon == NULL) {
		pseterr(EFAULT);
		return SYSERR;
	}
	connected = &mcon->Connected;

	memset(&rb, 0, sizeof(SkelDrvrReadBuf));

	/* initialise read buffer */

	rb.Connection = *conx;
	sker = SkelUserGetUtc(mcon, &rb.Time);

	fill_clients_queues(connected, &rb, conx->ConMask);

	return sizeof(SkelDrvrConnection);
}

/*
 * write entry point switch
 * Depending on skel's configuration, either the user's or the default
 * write() entry point is called.
 */

int skel_write(void *wa, struct file *f, char *buf, int len)
{
	if (SkelConf.write)
		return SkelConf.write(wa, f, buf, len);
	return SkelDrvrWrite(wa, f, buf, len);
}

static void get_module_maps_ioctl(SkelDrvrModuleContext * mcon, void *arg)
{
	InsLibAnyModuleAddress *ma;
	InsLibAnyAddressSpace *asp;
	InsLibModlDesc *modld;
	SkelDrvrMaps *m = (SkelDrvrMaps *) arg;
	int i = 0;

	memset(m, 0, sizeof(SkelDrvrMaps));

	modld = mcon->Modld;
	if (!modld)
		return;

	ma = (InsLibAnyModuleAddress *) modld->ModuleAddress;
	if (!ma)
		return;

	asp = ma->AnyAddressSpace;

	while (asp && i < SkelDrvrMAX_MAPS) {

		m->Maps[i].Mapped = (unsigned long) asp->Mapped;
		m->Maps[i].SpaceNumber = asp->SpaceNumber;
		m->Mappings++;

		i++;
		asp = asp->Next;
	}
}

static int get_queue_overflow(SkelDrvrQueue * q)
{
	unsigned long flags;
	int retval;

	spin_lock_irqsave(&q->lock, flags);
	retval = q->Missed;
	q->Missed = 0;
	spin_unlock_irqrestore(&q->lock, flags);

	return retval;
}

/*
 * The same PID can be connected more than once, so the idea of a client is not
 * the same as a PID. However we come in here with a PID list and I am not
 * breaking the interface. The way this is used is first GET_CLIENT_LIST
 * which returns a list of PIDs, then call this to see what thoes PIDs are
 * connected to. That is the correct behaviour we want, we should just
 * remove the _CLIENT_ part from the IOCTL names eventually.
 */

static int
skel_fill_client_connections(SkelDrvrModuleContext *mcon,
			     SkelDrvrClientConnections *ccn)
{
	struct list_head *client_list;
	struct client_link *entry;
	int i;

	for (i = 0; i < SkelDrvrINTERRUPTS; i++) {
		client_list = &mcon->Connected.clients[i];
		list_for_each_entry(entry, client_list, list) {
			if (entry->context->Pid != ccn->Pid)
				continue;
			ccn->Connections[ccn->Size].Module =
			    mcon->ModuleNumber;
			ccn->Connections[ccn->Size].ConMask = 1 << i;
			if (++ccn->Size >= SkelDrvrCONNECTIONS)
				return 1;
		}
	}
	return 0;
}

static int skel_get_client_list(SkelDrvrClientList * clients)
{
	SkelDrvrClientContext *ccon;
	struct client_link *link;
	unsigned long flags;

	memset(clients, 0, sizeof(*clients));

	spin_lock_irqsave(&Wa->list_lock, flags);
	list_for_each_entry(link, &Wa->clients, list) {

		/* avoid overflow */

		if (clients->Size >= SkelDrvrCLIENT_CONTEXTS)
			break;
		ccon = link->context;
		clients->Pid[clients->Size++] = ccon->Pid;
	}
	spin_unlock_irqrestore(&Wa->list_lock, flags);
	return OK;
}

static int
skel_get_client_connections(SkelDrvrClientContext * ccon,
			    SkelDrvrClientConnections * ccn)
{
	SkelDrvrModConn *connected;
	SkelDrvrModuleContext *module;
	unsigned long flags;
	int ret;
	int i;

	if (ccn->Pid == 0)
		ccn->Pid = ccon->Pid;
	ccn->Size = 0;

	for (i = 0; i < SkelDrvrMODULE_CONTEXTS; i++) {
		module = &Wa->Modules[i];
		if (!module->InUse)
			continue;
		connected = &module->Connected;
		spin_lock_irqsave(&connected->lock, flags);
		ret = skel_fill_client_connections(module, ccn);
		spin_unlock_irqrestore(&connected->lock, flags);
		if (ret)
			break;
	}
	return OK;
}

/**
 * Main ioctl logic that implements standard skell features
 * If its not a standard skel ioctl number the user ioctls
 * defined in skeluser_ioctl.h gets called if in range.
 */

int SkelDrvrIoctl(struct file *flp,     /* File pointer */
		  int cm,               /* IOCTL command */
		  void *arg)            /* Data for the IOCTL */
{
	SkelDrvrModuleContext *mcon;	/* Module context */
	SkelDrvrClientContext *ccon;	/* Client context */
	SkelDrvrConnection *conx;
	SkelDrvrVersion *ver;
	SkelDrvrClientConnections *ccn;
	SkelDrvrRawIoBlock *riob;
	SkelDrvrRawIoTransferBlock *riobt;
	SkelDrvrStatus *ssts;
	SkelDrvrDebug *db;

	long lav, *lap; /* Long Value pointed to by Arg */
	lap = arg;      /* Long argument pointer */
	lav = 0;
	if (lap)
		lav = *lap;     /* Long argument value */

	ccon = flp->private_data;
	if (ccon == NULL) {
		printk("Skel:Ioctl:Bad File Descriptor\n");
		pseterr(EBADF);
		return SYSERR;
	}

	mcon = get_mcon(ccon->ModuleNumber);
	if (mcon == NULL) {
		printk("Skel:ModuleContext:%d:Null\n",ccon->ModuleNumber);
		pseterr(EBADF);
		return SYSERR;
	}

	DebugIoctl(ccon, cm, lav);

	/* ================================= */
	/* Decode callers command and do it. */
	/* ================================= */

	switch (cm) {

	case SkelDrvrIoctlSET_DEBUG:
		db = (SkelDrvrDebug *) arg;
		ccon->Debug = db->DebugFlag;
		return OK;

	case SkelDrvrIoctlGET_DEBUG:
		db = (SkelDrvrDebug *) arg;
		db->DebugFlag = ccon->Debug;
		return OK;
		break;

	case SkelDrvrIoctlGET_VERSION:
		ver = (SkelDrvrVersion *) arg;
		GetVersion(mcon, ver);
		return OK;
		break;

	case SkelDrvrIoctlSET_TIMEOUT:
		ccon->Timeout = msecs_to_jiffies(lav);
		return OK;

	case SkelDrvrIoctlGET_TIMEOUT:
		if (lap) {
			*lap = jiffies_to_msecs(ccon->Timeout);
			return OK;
		}
		break;

	case SkelDrvrIoctlSET_QUEUE_FLAG:
		ccon->Queue.QueueOff = lav;
		return OK;

	case SkelDrvrIoctlGET_QUEUE_FLAG:
		if (lap) {
			*lap = ccon->Queue.QueueOff;
			return OK;
		}
		break;

	case SkelDrvrIoctlGET_QUEUE_SIZE:
		if (lap) {
			*lap = ccon->Queue.Size;
			return OK;
		}
		break;

	case SkelDrvrIoctlGET_QUEUE_OVERFLOW:
		if (lap) {
			*lap = get_queue_overflow(&ccon->Queue);
			return OK;
		}
		break;

	case SkelDrvrIoctlSET_MODULE:
		mcon = get_mcon(lav);
		if (mcon == NULL) {
			SK_WARN("SET_MODULE: Module %d doesn't exist",
				(int) lav);
			pseterr(ENODEV);
			return SYSERR;
		}
		ccon->ModuleNumber = lav;
		return OK;

	case SkelDrvrIoctlGET_MODULE:
		if (lap) {
			*lap = ccon->ModuleNumber;
			return OK;
		}
		break;

	case SkelDrvrIoctlGET_MODULE_COUNT:
		if (lap) {
			*lap = Wa->InstalledModules;
			return OK;
		}
		break;

	case SkelDrvrIoctlGET_MODULE_MAPS:
		get_module_maps_ioctl(mcon, arg);
		return OK;

	case SkelDrvrIoctlCONNECT:
		conx = (SkelDrvrConnection *) arg;
		if (conx->ConMask)
			Connect(ccon, conx);
		else {
			if (!conx->Module)
				DisConnectAll(ccon);
			else
				DisConnect(ccon, conx);
		}
		return OK;

	case SkelDrvrIoctlGET_CLIENT_LIST:
		return skel_get_client_list((void *) lap);

	case SkelDrvrIoctlGET_CLIENT_CONNECTIONS:
		ccn = (SkelDrvrClientConnections *) arg;
		return skel_get_client_connections(ccon, ccn);

	case SkelDrvrIoctlENABLE:
		if (SkelUserHardwareEnable(mcon, lav) != SkelUserReturnFAILED)
			mcon->StandardStatus &=
			    ~SkelDrvrStandardStatusDISABLED;
		else
			mcon->StandardStatus |=
			    SkelDrvrStandardStatusDISABLED;
		return OK;

	case SkelDrvrIoctlRESET:
		if (Reset(mcon))
			break;
		return OK;

	case SkelDrvrIoctlGET_STATUS:
		ssts = (SkelDrvrStatus *) arg;
		if (GetStatus(mcon, ssts))
			break;
		return OK;

	case SkelDrvrIoctlRAW_READ:
		riob = (SkelDrvrRawIoBlock *) arg;
		if (SkelConf.rawio)
			return SkelConf.rawio(mcon, riob, 0);
		return RawIo(mcon, riob, 0);
		break;

	case SkelDrvrIoctlRAW_WRITE:
		riob = (SkelDrvrRawIoBlock *) arg;
		if (SkelConf.rawio)
			return SkelConf.rawio(mcon, riob, 1);
		return RawIo(mcon, riob, 1);
		break;

	case SkelDrvrIoctlRAW_BLOCK_READ:
		riobt = (SkelDrvrRawIoTransferBlock *) arg;
		return RawIoBlock(mcon, riobt, 0);
		break;

	case SkelDrvrIoctlRAW_BLOCK_WRITE:
		riobt = (SkelDrvrRawIoTransferBlock *) arg;
		return RawIoBlock(mcon, riobt, 1);
		break;

	default:
		if (is_user_ioctl(cm)) {
			SkelUserReturn uret;

			if (mcon == NULL) {
				pseterr(EINVAL);
				return SYSERR;
			}

			uret = SkelUserIoctls(ccon, mcon, cm, arg);
			return uret == SkelUserReturnOK ? OK : SYSERR;
		}
		break;
	}

	pseterr(ENOTTY);				   /* IOCTL error - ENOTTY dictated by POSIX */
	return SYSERR;
}

/**
 * Implement linux IOCTL common logic used by locked and unlocked calls
 */

int
skel_drv_ioctl(struct inode *inode, struct file *filp, unsigned int cmd,
	       unsigned long arg)
{
	int cc;
	int iodr;		/* Io Direction */
	int iosz;		/* Io Size in bytes */
	int ionr;		/* Io Number */
	void *mem;              /* Argument buffer area */

	ionr = _IOC_NR(cmd);
	iodr = _IOC_DIR(cmd);
	iosz = _IOC_SIZE(cmd);

	if ((mem = kzalloc(iosz, GFP_KERNEL)) == NULL)
		return -ENOMEM;

	if (iodr & _IOC_WRITE) {
		if (copy_from_user(mem, (void *) arg, iosz) != 0)  {
			printk("%s: Failed copy_from_user\n",SkelDrvrNAME);
			kfree(mem);
			return -EACCES;
		}
	}

	cc = SkelDrvrIoctl(filp, cmd, mem);
	if (cc) {
		printk("%s: Failed SkelDrvrIoctl\n",SkelDrvrNAME);
		kfree(mem);
		return drv_errno;
	}

	if (iodr & _IOC_READ) {
		if (copy_to_user((void *) arg, mem, iosz) != 0) {
			printk("%s: Failed copy_to_user\n",SkelDrvrNAME);
			kfree(mem);
			return -EACCES;
		}
	}

	kfree(mem);
	return OK;
}

/**
 * Unlocked ioctl call
 */

long skel_drv_ioctl_ulck(struct file *filp, unsigned int cmd,
			 unsigned long arg)
{
	return skel_drv_ioctl(filp->f_dentry->d_inode, filp, cmd, arg);
}

/**
 * Older ioctl interface for locked ioctl calls
 */

int
skel_drv_ioctl_lck(struct inode *inode, struct file *filp,
		   unsigned int cmd, unsigned long arg)
{
	return skel_drv_ioctl(inode, filp, cmd, arg);
}

/**
 * Implement OPEN
 */

int skel_drv_open(struct inode *inode, struct file *filp)
{
	int modnum = MINOR(inode->i_rdev);;

	if (SkelDrvrOpen(filp,modnum))
		return drv_errno;
	return OK;
}

/**
 * Implement CLOSE
 */

int skel_drv_close(struct inode *inode, struct file *filp)
{
	if (SkelDrvrClose(Wa, filp))
		return drv_errno;
	return OK;
}

/**
 * Implement READ
 */

ssize_t
skel_drv_read(struct file *filp, char *buf, size_t count, loff_t * f_pos)
{
	SkelDrvrReadBuf rbuf;
	size_t len;
	int cc;

	len = sizeof(SkelDrvrReadBuf);
	if (count < len)
		len = count;

	cc = skel_read(Wa, filp, (char *) &rbuf, len);
	if (cc == 0)
		return 0; /* Timeout */

	cc = copy_to_user(buf, &rbuf, len);
	if (cc != 0)
		return -EACCES;
	return len;
}

/**
 * Implement WRITE
 */

ssize_t
skel_drv_write(struct file *filp, const char *buf, size_t count,
	       loff_t * f_pos)
{
	SkelDrvrReadBuf rbuf;
	int len, cc;

	len = sizeof(SkelDrvrReadBuf);
	if (count < len)
		len = count;

	cc = copy_from_user(&rbuf, (void *) buf, count);
	if (cc != 0)
		return -EACCES;

	if (skel_write(Wa, filp, (char *) &rbuf, len) == 0)
		return 0;

	return len;
}

/**
 * Module entries
 */

module_init(skel_drv_install);
module_exit(skel_drv_uninstall);
