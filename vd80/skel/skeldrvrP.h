/* ===================================================================== */
/* Standard Private definitions for all drivers. Basis driver skeleton.  */
/* Julian Lewis 25th/Nov/2008                                            */
/* ===================================================================== */

#ifndef SKELDRVRP
#define SKELDRVRP

#include <skeldefs.h>
#include <skeldrvr.h>

InsLibDrvrDesc *build_drvr(void);
void set_tree_defaults(InsLibDrvrDesc * dp);

/*
 * Note. Common variables are declared at the end of this source file, because
 * some of them need types defined in this header file.
 */

/* =============================================== */
/* Up to 32 incomming events per client are queued */

typedef struct {
	spinlock_t lock;
	int QueueOff;
	U16 Missed;
	int Size;
	U32 RdPntr;
	U32 WrPntr;
	SkelDrvrReadBuf Entries[SkelDrvrQUEUE_SIZE];
} SkelDrvrQueue;

/**
 * @brief The client context
 *
 * @param waitq        --  Wait queue
 * @param waitc        --  Wait counter
 * @param Queue        --  Skell queue
 * @param Pid          --
 * @param Timeout      --
 * @param Debug        --
 * @param ModuleNumber --
 * @param ChannelNr    --
 * @param UserData     --
 * @param filep        --
 */
typedef struct {
	wait_queue_head_t waitq;
	unsigned int      waitc;
	SkelDrvrQueue     Queue;

	U32 Pid;
	U32 Timeout;
	SkelDrvrDebugFlag Debug;
	U32 ModuleNumber;
	U32 ChannelNr;
	void *UserData;
	struct file *filep;
} SkelDrvrClientContext;

/**
 * Lists of clients are needed in various places in the driver
 */

struct client_link {
	struct list_head list;
	SkelDrvrClientContext *context;	/* Clients context */
};

/**
 * \brief keep client's connections on a module
 * @lock - protects the struct
 * @clients      - connected clients kept in an array of linked lists, one list per interrupt
 * @enabled_ints - mask of enabled interrupts
 *
 * When a client_link is in the @clients list, that means the client exists --
 * this is guaranteed by the fact that a client is always disconnected from
 * ALL interrupts BEFORE being removed (see SkelDrvrClose()).
 * Note: @lock may be acquired with Wa->list_lock already held. But not the
 * other way around!
 */
typedef struct {
	spinlock_t lock;
	struct list_head clients[SkelDrvrINTERRUPTS];	/* List of connected clients per interrupt */
	uint32_t enabled_ints;
} SkelDrvrModConn;

/* =============================================== */
/* The module context                              */

typedef struct {
	struct mutex mutex;	/* Module lock */
	U32 ModuleNumber;	/* Drivers module logical unit number */
	U32 InUse;		/* Context in Use, module installed */
	InsLibModlDesc *Modld;	/* Module hardware address */
	U32 Timeout;		/* Transaction timeout value */
	S32 Timer;		/* Transaction timer */
	SkelDrvrModConn Connected;
	U32 Registers[SkelDrvrREGISTERS];	/* Copy of module registers, Emulation and Reset */
	SkelDrvrDebugFlag Debug;	/* Global debug options */
	SkelDrvrStandardStatus StandardStatus;	/* Standard status */
	void *UserData;
} SkelDrvrModuleContext;

/* =============================================== */
/* Drivers working area                            */

/*
 * @list_lock protects the @clients list.
 */
typedef struct {
	InsLibDrvrDesc *Drvrd;
	InsLibEndian Endian;
	U32 InstalledModules;
	SkelDrvrModuleContext Modules[SkelDrvrMODULE_CONTEXTS];
	void *UserData;

	spinlock_t list_lock;           /* Lock for clients list */
	struct list_head clients;       /* Linked list of clients */

} SkelDrvrWorkingArea;

/* =========================================================== */
/* Return code for user routines                               */

typedef enum {
	SkelUserReturnOK,
	SkelUserReturnFAILED,
	SkelUserReturnNOT_IMPLEMENTED
} SkelUserReturn;

/* =========================================================== */
/* The module version can be the compilation date for the FPGA */
/* code or some other identity readable from the hardware.     */

extern SkelUserReturn SkelUserGetModuleVersion(SkelDrvrModuleContext *
					       mcon, char *mver);

/* =========================================================== */
/* The specific hardware status is read here. From this it is  */
/* possible to build the standard SkelDrvrStatus inthe module  */
/* context at location mcon->StandardStatus                    */

extern SkelUserReturn SkelUserGetHardwareStatus(SkelDrvrModuleContext *
						mcon, U32 * hsts);

/* =========================================================== */
/* Get the UTC time, called from the ISR                       */

extern SkelUserReturn SkelUserGetUtc(SkelDrvrModuleContext * mcon,
				     SkelDrvrTime * utc);

/* =========================================================== */
/* This is the hardware reset routine. Don't forget to copy    */
/* the registers block in the module context to the hardware.  */

extern SkelUserReturn SkelUserHardwareReset(SkelDrvrModuleContext * mcon);

/* =========================================================== */
/* This routine gets and clears the modules interrupt source   */
/* register. Try to do this with a Read/Modify/Write cycle.    */
/* Its a good idea to return the time of interrupt.            */

extern SkelUserReturn SkelUserGetInterruptSource(SkelDrvrModuleContext *
						 mcon, SkelDrvrTime * itim,
						 U32 * isrc);

/* =========================================================== */
/* Define howmany interrupt sources you want and provide this  */
/* routine to enable them.                                     */

extern SkelUserReturn SkelUserEnableInterrupts(SkelDrvrModuleContext *
					       mcon, U32 imsk);

/* =========================================================== */
/* Provide a way to enable or disable the hardware module.     */

extern SkelUserReturn SkelUserHardwareEnable(SkelDrvrModuleContext * mcon,
					     U32 flag);

/* =========================================================== */
/* Standard CO FPGA JTAG IO, Read a byte                       */

extern SkelUserReturn SkelUserJtagReadByte(SkelDrvrModuleContext * mcon,
					   U32 * byte);

/* =========================================================== */
/* Standard CO FPGA JTAG IO, Write a byte                      */

extern SkelUserReturn SkelUserJtagWriteByte(SkelDrvrModuleContext * mcon,
					    U32 byte);

/* =========================================================== */
/* Then decide on howmany IOCTL calls you want, and fill their */
/* debug name strings.                                         */

extern SkelUserReturn SkelUserIoctls(SkelDrvrClientContext * ccon,	/* Calling clients context */
				     SkelDrvrModuleContext * mcon,	/* Calling clients selected module */
				     int cm,	/* Ioctl number */
				     char *arg);	/* Pointer to argument space */

/*
 * Get the name of an IOCTL
 */
extern char *SkelUserGetIoctlName(int cmd);

/*
 * Module initialisation
 */
extern SkelUserReturn SkelUserModuleInit(SkelDrvrModuleContext * mcon);

/*
 * Module uninstallation
 */
extern void SkelUserModuleRelease(SkelDrvrModuleContext * mcon);

/*
 * Client initialisation
 */
extern SkelUserReturn SkelUserClientInit(SkelDrvrClientContext * ccon);

/*
 * Client's close()
 */
extern void SkelUserClientRelease(SkelDrvrClientContext * ccon);

#ifdef __linux__
#define SKEL_ERR KERN_ERR
#define SKEL_WARN KERN_WARNING
#define SKEL_INFO KERN_INFO
#define SKEL_DEBUG KERN_DEBUG
#else				/* lynx */
#define SKEL_ERR
#define SKEL_WARN
#define SKEL_INFO
#define SKEL_DEBUG
#endif				/* __linux__ */

/* error printout */
#define SK_ERROR(format...)						\
        do {								\
		printk(SKEL_ERR "[%s_ERROR] ", Wa->Drvrd->DrvrName);  \
		printk(format);                                       \
		printk("\n");                                         \
        } while (0)

/* warning printout */
#define SK_WARN(format...)						\
        do {								\
		printk(SKEL_WARN "[%s_WARN] ", Wa->Drvrd->DrvrName);  \
		printk(format);                                       \
		printk("\n");                                         \
        } while (0)

/* informational printout */
#define SK_INFO(format...)						\
        do {								\
		printk(SKEL_INFO "[%s] ", Wa->Drvrd->DrvrName);       \
		printk(format);                                       \
		printk("\n");                                         \
        } while (0)

/* debugging printout */
#define SK_DEBUG(format...)						\
        do {								\
		printk(SKEL_DEBUG "[%s_DEBUG] %s()@%d: ", Wa->Drvrd->DrvrName, __FUNCTION__, __LINE__); \
		printk(format);                                       \
		printk("\n");                                         \
        } while (0)

#define __report_client(___ccon, ___dbflag, format...)			\
	do {								\
		if ((___ccon)->Debug & (___dbflag) || !(___dbflag)) {	\
			printk(SKEL_INFO "[%s] ", Wa->Drvrd->DrvrName); \
			printk("(%s) pid=%d -- ",                     \
				GetDebugFlagName((___dbflag)),		\
				(___ccon)->Pid);                        \
			printk(format);                               \
			printk("\n");                                 \
		}							\
	} while (0)

#define __report_module(___mcon, ___dbflag, format...)	    		\
	do {								\
		if ((___mcon)->Debug & (___dbflag) || !(___dbflag)) {	\
			printk(SKEL_INFO "[%s] ", Wa->Drvrd->DrvrName);\
			printk("(%s) module#%d -- \n",                 \
				GetDebugFlagName((___dbflag)),          \
				(___mcon)->ModuleNumber);               \
			printk(format);                                \
			printk("\n");                                  \
		}                                                       \
	}                                                               \
	while (0)

/*
 * Common functions
 */
void *get_vmemap_addr(SkelDrvrModuleContext * mcon, int am, int dw);
int get_vme_addr(SkelDrvrModuleContext * mcon, int am, int dw,
		 uint32_t * addr);
SkelDrvrModuleContext *get_mcon(int modnr);
SkelDrvrClientContext *get_ccon(struct file *f);
const char *GetDebugFlagName(SkelDrvrDebugFlag debf);

/**
* @brief report debugging info to a client
*
* @param c - client context
* @param dbf - debug flag
* @param fmt - format
*/
#define report_client(c, dbf, fmt...) __report_client(c, dbf, fmt)

/**
* @brief report debugging info related to a module
*
* @param m - module context
* @param dbf - debug flag
* @param fmt - format
*/
#define report_module(m, dbf, fmt...) __report_module(m, dbf, fmt)

/*
 * skel's configuration struct
 */
struct skel_conf {
	int (*read) (void *, struct file *, char *, int);
	int (*write) (void *, struct file *, char *, int);
	int (*intrhandler) (void *);
	int (*rawio) (SkelDrvrModuleContext * mcon,
		      SkelDrvrRawIoBlock * riob, int write);
};

/*
 * Common variables
 */
extern SkelDrvrWorkingArea *Wa;	/* Drivers' working area */
extern struct skel_conf SkelConf;	/* skel's configuration */

#endif
