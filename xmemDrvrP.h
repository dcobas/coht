/**
 * @file xmemDrvrP.h
 *
 * @brief Private definitions used by the Xmem driver.
 *
 * @author Julian Lewis AB/CO/HT CERN
 *
 * @date Created on 02/09/2004
 *
 * This header file is *not* meant to be exported to clients--it's just
 * for the driver itself.
 *
 * @version 2.0 Emilio G. Cota 15/10/2008, CDCM-compliant.
 *
 * @version 1.0 Julian Lewis 31/08/2005, Lynx/Linux compatible.
 */
#ifndef XMEM_DRVR_P
#define XMEM_DRVR_P

#include "plx9656_layout.h"
#include "vmic5565_layout.h"
#include "xmemDrvr.h"

/*! @name Global definitions
 *
 * TransferDirection (Read,Write) and endianness.
 */
//@{
typedef enum {
  XmemDrvrREAD  = 1, //!< Do a read operation
  XmemDrvrWRITE = 2  //!< Do a write operation
} XmemDrvrIoDir;

typedef enum {
  XmemDrvrEndianNOT_SET = 0,
  XmemDrvrEndianBIG,
  XmemDrvrEndianLITTLE
} XmemDrvrEndian;
//@}

/*! @name Client Context
 *
 * Structures and definitions for the client's context
 */
//@{

//!< TIMEOUT = 1 --> delay of 10ms
#define XmemDrvrMINIMUM_TIMEOUT 10
#define XmemDrvrDEFAULT_TIMEOUT 1000
#define XmemDrvrBUSY_TIMEOUT 200

//!< Maximum queue size:
#define XmemDrvrQUEUE_SIZE 128

typedef struct {
  unsigned short  QueueOff;
  unsigned short  Missed;
  unsigned short  Size;
  XmemDrvrReadBuf Entries[XmemDrvrQUEUE_SIZE];
} XmemDrvrQueue;

typedef struct {
  //!< Non zero when context is being used, else zero
  unsigned long InUse;

  XmemDrvrDebug Debug;           //!< Clients debug settings mask
  unsigned long Pid;             //!< Clients Process ID

  //!< Position of this client in array of clients
  unsigned long ClientIndex;

  unsigned long ModuleIndex;     //!< The VMIC module he is working with
  unsigned long UpdatedSegments; //!< Updated segments mask
  unsigned long Timeout;         //!< Timeout value in ms or zero
  int  Timer;                    //!< Timer being used by client
  int  Semaphore;                //!< Semaphore to block on read
  XmemDrvrQueue Queue;           //!< Interrupt queue
} XmemDrvrClientContext;
//@}

/*! @name Module context
 *
 * Structures and definitions for the module's context
 */
//@{

/* TIMEOUT = 1 --> delay of 10ms. */
#define XmemDrvrDMA_TIMEOUT 10

typedef struct {
  char *Buffer;
  cdcm_dma_t Dma;
  unsigned long Len;
  unsigned short Flag;
} XmemDmaOp;

typedef struct {
  unsigned long InUse;			//!< Module context in use
  struct drm_node_s *Handle;    	//!< Handle from DRM
  unsigned long IVector;		//!< Resulting interrupt vector
  unsigned long LocalOpen;		//!< Plx9656 local conig open
  unsigned long ConfigOpen;		//!< Plx9656 configuration open
  unsigned long RfmOpen;		//!< VMIC RFM register space
  unsigned long RawOpen;		//!< VMIC SDRAM space
  unsigned long PciSlot;		//!< Module geographic ID PCI Slot
  unsigned long ModuleIndex;		//!< Which module we are
  unsigned long NodeId;			//!< Node 1..256

  //!< {Rd,Wr}DmaSemaphores are used for tracking the DMA transfer time
  int		RdDmaSemaphore;		//!< DMA 0 engine sem, used for reading
  int		WrDmaSemaphore;		//!< DMA 1 engine sem, used for writing

  //!< the two semaphores below are mutexes
  int		BusySemaphore;		//!< Module is busy
  int		TempbufSemaphore;	//!< Tempbuf is being used

  int     	RdTimer;		//!< Read DMA timer
  int     	WrTimer;		//!< Write DMA timer
  int    	BusyTimer;		//!< Module busy timer
  int     	TempbufTimer;		//!< Temp buffer timer
  VmicRfmMap  	*Map;			//!< Pointer to the real hardware
  unsigned char	*SDRam;			//!< Direct access to VMIC SD Ram
  PlxLocalMap	*Local;			//!< Local Plx register space
  XmemDrvrScr	Command;		//!< Command bits settings
  VmicLier	InterruptEnable;	//!< Enabled interrupts mask

  //!< Clients interrupts
  XmemDrvrIntr	Clients[XmemDrvrCLIENT_CONTEXTS];

  XmemDmaOp	DmaOp;			//!< DMA mapping info
  struct cdcm_dmabuf Dma;		//!< For CDCM internal use only

  //!< temporary buffer, allocated during the installation of the driver
  void *Tempbuf;
} XmemDrvrModuleContext;
//@}

/* ============================================================ */
/* Driver Working Area                                          */
/* ============================================================ */

/*! @name Driver's Working Area
 *
 * Structures and definitions for the driver's working area (WA).
 * All the module and client contexts are kept inside the WA; memory space
 * for the WA is allocated during the installation of the driver.
 */
//@{
#define XmemDrvrDEFAULT_DMA_THRESHOLD 1024

#if XmemDrvrDEFAULT_DMA_THRESHOLD > PAGESIZE
#define XmemDrvrDEFAULT_DMA_THRESHOLD PAGESIZE
#endif

typedef struct {
  unsigned long		Modules;
  unsigned long		DmaThreshold;
  unsigned long		Nodes;		//!< One bit for each node
  XmemDrvrEndian	Endian;
  XmemDrvrSegTable	SegTable;
  XmemDrvrModuleContext	ModuleContexts[XmemDrvrMODULE_CONTEXTS];
  XmemDrvrClientContext	ClientContexts[XmemDrvrCLIENT_CONTEXTS];
} XmemDrvrWorkingArea;
//@}


#endif
