/* =========================================================== */
/* SkelDriver user code frame. Fill in this frame to implement */
/* a specific driver for PCI/VME modules.                      */
/* Julian Lewis Tue 16th Dec 2008 AB/CO/HT                     */
/* =========================================================== */

#include <skeluser.h>
#include <skeluser_ioctl.h>
#include <skeldrvrP.h>

#include <vd80drvrP.h>
#include <vd80hard.h>

#include <vmebus.h>
#include <asm/page.h>
#include <linux/mm.h>
#include <linux/page-flags.h>
#include <linux/pagemap.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <asm/io.h>

/* =========================================================== */
/* Client and module asociated error messages                  */

#define MES_TEXT_LEN 128

/* =========================================================== */
/* Given a module context and an address modifier returned the */
/* corresponding address map pointer.                          */

uint32_t *GetVmeMappedAddress(SkelDrvrModuleContext *mcon,
			      int                    am) {

InsLibModlDesc         *modld = NULL;   /* Module descriptor linked list */
InsLibVmeModuleAddress *vmema = NULL;   /* The VD80 VME address space linked list */
InsLibVmeAddressSpace  *vmeas = NULL;   /* VME address spaces linked list */

   if (!mcon) {
      report_module(mcon, SkelDrvrDebugFlagWARNING, "GetVmeMappedAddress:NULL module context");
      return NULL;
   }
   modld = mcon->Modld;
   if (!modld) {
      report_module(mcon, SkelDrvrDebugFlagWARNING, "GetVmeMappedAddress:NULL module descriptor");
      return NULL;
   }
   vmema = modld->ModuleAddress;
   if (!vmema) {
      report_module(mcon, SkelDrvrDebugFlagWARNING, "GetVmeMappedAddress:NULL module address");
      return NULL;
   }

   /* Search for an address space for the given address modifier */

   vmeas = vmema->VmeAddressSpace;
   while (vmeas) {
      if (vmeas->AddressModifier == am) return vmeas->Mapped; /* Got it so return it */
      vmeas = vmeas->Next;
   }
   report_module(mcon, SkelDrvrDebugFlagWARNING, "GetVmeMappedAddress:Address modifier 0x%X not found", am);
   return NULL;
}

/* =========================================================== */
/* As above but return the corresponding base address instead  */

uint32_t *GetVmeBaseAddress(SkelDrvrModuleContext *mcon,
			    int                    am) {

InsLibModlDesc         *modld = NULL;
InsLibVmeModuleAddress *vmema = NULL;
InsLibVmeAddressSpace  *vmeas = NULL;

   if (!mcon) {
      report_module(mcon, SkelDrvrDebugFlagWARNING, "GetVmeBaseAddress:NULL module context");
      return NULL;
   }
   modld = mcon->Modld;
   if (!modld) {
      report_module(mcon, SkelDrvrDebugFlagWARNING, "GetVmeBaseAddress:NULL module descriptor");
      return NULL;
   }
   vmema = modld->ModuleAddress;
   if (!vmema) {
      report_module(mcon, SkelDrvrDebugFlagWARNING, "GetVmeBaseAddress:NULL module address");
      return NULL;
   }

   vmeas = vmema->VmeAddressSpace;
   while (vmeas) {
      if (vmeas->AddressModifier == am) return (uint32_t *) vmeas->BaseAddress;
      vmeas = vmeas->Next;
   }
   report_module(mcon, SkelDrvrDebugFlagWARNING, "GetVmeBaseAddress:Address modifier 0x%X not found", am);
   return NULL;
}

/* =========================================================== */
/* Access hardware/software registers with emulation           */

void SetReg(uint32_t *map, uint32_t offset, uint32_t valu, SkelDrvrModuleContext *mcon) {
   mcon->Registers[offset/4] = valu;
   if (mcon->StandardStatus & SkelDrvrStandardStatusEMULATION) return;
   iowrite32(cpu_to_be32((uint32_t) valu), &(map[offset/4]));
}

uint32_t GetReg(uint32_t *map, uint32_t offset, SkelDrvrModuleContext *mcon) {
   if (mcon->StandardStatus & SkelDrvrStandardStatusEMULATION)
      return (uint32_t) mcon->Registers[offset/4];
   return (uint32_t) be32_to_cpu(ioread32(&(map[offset/4])));
}

/* =========================================================== */
/* Get operational registers                                   */

uint32_t *GetRegs(SkelDrvrModuleContext *mcon) {

uint32_t *regs;

   if (mcon->StandardStatus & SkelDrvrStandardStatusEMULATION)
      return mcon->Registers;

   regs = GetVmeMappedAddress(mcon, VME_A24_USER_DATA_SCT);
   if (regs == NULL)
      report_module(mcon, SkelDrvrDebugFlagASSERTION, "GetRegs:Missing A24 Mapping");
   return regs;
}

/* =========================================================== */
/* Build a read DMA descriptor                                 */
/* Transfer one VME_DMA_BSIZE_4096 block or less               */

#define VME_NO_ADDR_INCREMENT 1
#define DMA_BLOCK_SIZE        4096
#define SAMPLES_IN_DMA_BLOCK  2048

void BuildDmaReadDesc(SkelDrvrModuleContext *mcon,
		      void                  *dest,
		      unsigned int           len,
		      struct vme_dma        *dma_desc) {

   memset(dma_desc, 0, sizeof(struct vme_dma));

   dma_desc->dir            = VME_DMA_FROM_DEVICE;
   dma_desc->src.data_width = VME_D32;
   dma_desc->src.am         = VME_A24_USER_MBLT;
   dma_desc->novmeinc       = VME_NO_ADDR_INCREMENT;

   dma_desc->ctrl.pci_block_size   = VME_DMA_BSIZE_4096;
   dma_desc->ctrl.pci_backoff_time = VME_DMA_BACKOFF_0;
   dma_desc->ctrl.vme_block_size   = VME_DMA_BSIZE_4096;
   dma_desc->ctrl.vme_backoff_time = VME_DMA_BACKOFF_0;

   dma_desc->src.addrl = (unsigned int) GetVmeBaseAddress(mcon,VME_A24_USER_DATA_SCT);
   dma_desc->dst.addrl = (unsigned int) dest;
   dma_desc->length    = len;

   if (mcon->Debug & SkelDrvrDebugFlagMODULE) {
      report_module(mcon, SkelDrvrDebugFlagWARNING, "DMA:src.addrl:0x%X dst.addrl:0x%X length:0x%X\n",
	      dma_desc->src.addrl,
	      dma_desc->dst.addrl,
	      dma_desc->length);
   }
}

/* =========================================================== */
/* The module version can be the compilation date for the FPGA */
/* code or some other identity readable from the hardware.     */

SkelUserReturn SkelUserGetModuleVersion(SkelDrvrModuleContext *mcon,
					char                  *mver) {
UserData *u;

   if (mver) {
      u = (UserData *) mcon->UserData;
      strncpy(mver,u->revis_id,VD80_CR_REV_ID_LEN);
      return SkelUserReturnOK;
   }
   report_module(mcon, SkelDrvrDebugFlagASSERTION, "NULL version string");
   return SkelUserReturnFAILED;
}

/* =========================================================== */
/* The specific hardware status is read here. From this it is  */
/* possible to build the standard SkelDrvrStatus inthe module  */
/* context at location mcon->StandardStatus                    */

SkelUserReturn SkelUserGetHardwareStatus(SkelDrvrModuleContext *mcon,
					 uint32_t              *hsts) {
uint32_t *regs = NULL;
uint32_t reg;

   *hsts = mcon->StandardStatus;

   if ((regs = GetRegs(mcon)) == NULL) return SkelUserReturnFAILED;
   reg = GetReg(regs,VD80_GSR,mcon);

   if (reg & VD80_STATE_IDLE) *hsts |= SkelDrvrStandardStatusIDLE;
   else                       *hsts |= SkelDrvrStandardStatusBUSY;
   if (reg & VD80_MEMOUTRDY)  *hsts |= SkelDrvrStandardStatusREADY;
   if (reg & VD80_IOERRCLK)   *hsts |= SkelDrvrStandardStatusHARDWARE_FAIL;
   if (reg & VD80_IOERRTRIG)  *hsts |= SkelDrvrStandardStatusHARDWARE_FAIL;

   reg = GetReg(regs,VD80_GCR2,mcon);

   if (reg & VD80_DBCNTSEL)   *hsts |= SkelDrvrStandardStatusHARDWARE_DBUG;

   return SkelUserReturnOK;
}

/* =========================================================== */
/* Get the UTC time, called from the ISR                       */

SkelUserReturn SkelUserGetUtc(SkelDrvrModuleContext *mcon,
			      SkelDrvrTime          *utc) {

   struct timespec curtime;
   curtime = current_kernel_time();

   utc->NanoSecond = curtime.tv_nsec;
   utc->Second     = curtime.tv_sec;
   return SkelUserReturnOK;
}

/* =========================================================== */
/* This is the hardware reset routine. Don't forget to copy    */
/* the registers block in the module context to the hardware.  */

SkelUserReturn SkelUserHardwareReset(SkelDrvrModuleContext *mcon) {

// return SkelUserConfig(Wa, mcon); /* Too dangerous */

   return SkelUserReturnOK;
}

/* =========================================================== */
/* This routine gets and clears the modules interrupt source   */
/* register. Try to do this with a Read/Modify/Write cycle.    */
/* Its a good idea to return the time of interrupt.            */
/* This is called from within the SKEL driver ISR handler.     */

SkelUserReturn SkelUserGetInterruptSource(SkelDrvrModuleContext *mcon,
					  SkelDrvrTime          *itim,
					  uint32_t              *isrc) {
uint32_t *regs = NULL;

   if ((regs = GetRegs(mcon)) == NULL)
      return SkelUserReturnFAILED;

   *isrc = (GetReg(regs,VD80_GSR,mcon) >> VD80_GSR_INTERRUPT_SHIFT)
	 & VD80_INTERRUPT_MASK;

   SkelUserGetUtc(mcon,itim);
   SetReg(regs,VD80_GCR1,VD80_INTCLR,mcon);

   return SkelUserEnableInterrupts(mcon,mcon->Connected.enabled_ints);
}

/* =========================================================== */
/* Define howmany interrupt sources you want and provide this  */
/* routine to enable them.                                     */

SkelUserReturn SkelUserEnableInterrupts(SkelDrvrModuleContext *mcon,
					uint32_t               imsk) {

uint32_t *regs = NULL;
uint32_t tval;

   if ((regs = GetRegs(mcon)) == NULL) return SkelUserReturnFAILED;
   if (mcon->Debug & SkelDrvrDebugFlagMODULE) {
      report_module(mcon, SkelDrvrDebugFlagWARNING, "SkelUserEnableInterrupts:0x%X", imsk);
   }
   tval = GetReg(regs,VD80_GCR2,mcon) & ~VD80_INTERRUPT_MASK;
   tval |= (imsk & VD80_INTERRUPT_MASK);
   SetReg(regs,VD80_GCR2,tval,mcon);
   return SkelUserReturnOK;
}

/* =========================================================== */
/* Provide a way to enable or disable the hardware module.     */

SkelUserReturn SkelUserHardwareEnable(SkelDrvrModuleContext *mcon,
				      uint32_t               flag) {
   return SkelUserReturnNOT_IMPLEMENTED;
}

/* =========================================================== */
/* Standard CO FPGA JTAG IO, Read a byte                       */

SkelUserReturn SkelUserJtagReadByte(SkelDrvrModuleContext *mcon,
				    uint32_t              *byte) {
   return SkelUserReturnNOT_IMPLEMENTED;
}

/* =========================================================== */
/* Standard CO FPGA JTAG IO, Write a byte                      */

SkelUserReturn SkelUserJtagWriteByte(SkelDrvrModuleContext *mcon,
				     uint32_t               byte) {
   return SkelUserReturnNOT_IMPLEMENTED;
}

/* =========================================================== */
/* Then decide on howmany IOCTL calls you want, and fill their */
/* debug name strings.                                         */

/* sleep until IDLE is settled or things time out */
void wait_for_idle(SkelDrvrModuleContext *mcon, int maxdelay)
{
	int state;
	int delay;

	for (delay = 0; delay < maxdelay; delay += 10) {
		state = GetReg(GetRegs(mcon), VD80_GSR, mcon) & VD80_STATE_MASK;
		if (state == VD80_STATE_IDLE)
			return;
		udelay(10);
	}
	printk("vd80: wait_for_idle timed out after %d us\n", maxdelay);
}

/**
 * @brief ioctl vector
 *
 * @param ccon -- Calling clients context
 * @param mcon -- Calling clients selected module
 * @param cm   -- Ioctl number
 * @param arg  -- Pointer to argument space
 *
 * <long-description>
 *
 * @return <ReturnValue>
 */
SkelUserReturn SkelUserIoctls(SkelDrvrClientContext *ccon,
			      SkelDrvrModuleContext *mcon,
			      int                    cm,
			      char                  *arg)
{

struct vme_dma dma_desc;    /* Vme driver DMA structure */
int    tcnt, psze;          /* Transfer count and short page size */

uint32_t *regs = NULL;   /* Mapped A24D32 address space for Vd80 module */
int *lap  = NULL;   /* Long-Value pointer to argument */
int lval, tval, bms;/* Long-Value from argument, temp-value, bit-mask */
int i;

int actpostrig = 0; /* The actual number of post trigger samples */
int actpretrig = 0; /* The actual number of pre  trigger samples */
int shotlength = 0; /* The total number od sample both pre and post trigger */
int samplestrt = 0; /* Start position for reading */

int remaining  = 0; /* Samples remaining to be transfered */
int samples    = 0; /* Samples transfered so far */

Vd80SampleBuf *sbuf = NULL; /* Users sample buffer */
Vd80Num num;                /* Enumeration over ioctl codes */

Vd80DrvrAnalogTrig *atrg;   /* Analog trigger */

Vd80DrvrTrigConfig *tcon;   /* Trigger configuration */
unsigned int        tsmp;   /* Min samples calculated */

int lvalue;                 /* For when arg is NULL */

   num = _IOC_NR(cm);       /* Get pure ioctl enumeration number */

   if ((mcon == NULL) || (ccon == NULL)) {
      printk("Vd80:Assertion Violation:Null Context\n");
      return SkelUserReturnFAILED;
   }

   /* Deal with emulation status, and make sure hardware is present if not */

   if (ccon->Debug & SkelDrvrDebugFlagEMULATION) {
      mcon->StandardStatus |= SkelDrvrStandardStatusEMULATION;
   } else {
      if (mcon->StandardStatus & SkelDrvrStandardStatusNO_HARDWARE) {
         report_client(ccon, 0, "Module hardware not installed");
	 return SkelUserReturnFAILED;
      }
      mcon->StandardStatus &= ~SkelDrvrStandardStatusEMULATION;
   }

   /* Get mapped A24D32 register address space */
   if ((regs = GetRegs(mcon)) == NULL)
	   return SkelUserReturnFAILED;

   /* Always get a long value and pointer from users arg */

   lap = (int *) arg;
   if (!lap) {
      lvalue = 0;
      lap = &lvalue;
      report_client(ccon, SkelDrvrDebugFlagWARNING, "Vd80:Null arg pointer in IOCTL");
   }
   lval = *lap;

   /* Set endian conversion, this is just a word swap, the user still needs */
   /* to swap bytes within these words */

   tval = GetReg(regs,VD80_GCR2,mcon);
   if (Wa->Endian == InsLibEndianLITTLE) tval |=  (VD80_BIGEND | VD80_BYTESWAP);
   else                                  tval &= ~(VD80_BIGEND | VD80_BYTESWAP);
   SetReg(regs,VD80_GCR2,tval,mcon);

   /* OK so do the users IOCTL command, notice its a Vd80Num not a Vd80 Ioctl */
   switch (num) {

      /* ============================ */
      /* Clock control                */

      case Vd80NumSET_CLOCK: /* Vd80Clock 0 = Internal, 1 = External */

	 tval = (GetReg(regs,VD80_CCR,mcon) & ~VD80_CLKSOURCE_MASK);
	 if (lval) bms = VD80_CLKSOURCE_EXT;
	 else      bms = VD80_CLKSOURCE_INT;
	 tval &= ~VD80_CLKSOURCE_EXT;
	 tval |= bms;
	 SetReg(regs,VD80_CCR,tval,mcon);
	 return SkelUserReturnOK;

      case Vd80NumSET_CLOCK_DIVIDE_MODE:  /* Vd80DivideMode 0 = Divide, 1 = Subsample */

	 tval = (GetReg(regs,VD80_CCR,mcon) & ~VD80_CLKDIVMODE_MASK);
	 if (lval) bms =  VD80_CLKDIVMODE_SUBSAMPLE;
	 else      bms =  VD80_CLKDIVMODE_DIVIDE;
	 tval &= ~VD80_CLKDIVMODE_SUBSAMPLE;
	 tval |= bms;
	 SetReg(regs,VD80_CCR,tval,mcon);
	 return SkelUserReturnOK;

      case Vd80NumSET_CLOCK_DIVISOR: /* 16 bit value to divide clock by (+1) */

	 tval  = (GetReg(regs,VD80_CCR,mcon) & ~VD80_CLKDIV_MASK);
	 tval |= ((lval << VD80_CLKDIV_SHIFT) & VD80_CLKDIV_MASK);
	 SetReg(regs,VD80_CCR,tval,mcon);
	 return SkelUserReturnOK;

      case Vd80NumSET_CLOCK_EDGE: /* Vd80Edge 0 = Rising, 1 = Falling */

	 tval = (GetReg(regs,VD80_CCR,mcon) & ~VD80_CLKEDGE_MASK);
	 if (lval) bms = VD80_CLKEDGE_FALLING;
	 else      bms = VD80_CLKEDGE_RISING;
	 tval &= ~VD80_CLKEDGE_FALLING;
	 tval |= bms;
	 SetReg(regs,VD80_CCR,tval,mcon);
	 return SkelUserReturnOK;

      case Vd80NumSET_CLOCK_TERMINATION: /* Vd80Termination 0 = None, 1 = 50-Ohms */

	 tval = GetReg(regs,VD80_CCR,mcon) & ~VD80_CLKTERM_MASK;
	 if (lval) bms = VD80_CLKTERM_50OHM;
	 else      bms = VD80_CLKTERM_NONE;
	 tval &= ~VD80_CLKTERM_50OHM;
	 tval |= bms;
	 SetReg(regs,VD80_CCR,tval,mcon);
	 return SkelUserReturnOK;

      case Vd80NumGET_CLOCK: /* Vd80Clock 0 = Internal, 1 = External, 2 = Illegal */

	 tval = (GetReg(regs,VD80_CCR,mcon) & VD80_CLKSOURCE_MASK);
	 if      (tval == VD80_CLKSOURCE_INT) *lap = Vd80DrvrClockINTERNAL;
	 else if (tval == VD80_CLKSOURCE_EXT) *lap = Vd80DrvrClockEXTERNAL;
	 else                                 *lap = Vd80DrvrCLOCKS;
	 return SkelUserReturnOK;

      case Vd80NumGET_CLOCK_DIVIDE_MODE: /* Vd80DivideMode 0 = Divide, 1 = Subsample */

	 tval = (GetReg(regs,VD80_CCR,mcon) & VD80_CLKDIVMODE_MASK);
	 if      (tval == VD80_CLKDIVMODE_DIVIDE)    *lap = Vd80DrvrDivideModeDIVIDE;
	 else if (tval == VD80_CLKDIVMODE_SUBSAMPLE) *lap = Vd80DrvrDivideModeSUBSAMPLE;
	 else                                        *lap = Vd80DrvrDivideMODES;
	 return SkelUserReturnOK;

      case Vd80NumGET_CLOCK_DIVISOR: /* 16 bit value to divide clock by (+1) */

	 tval = (GetReg(regs,VD80_CCR,mcon) & VD80_CLKDIV_MASK);
	 tval >>= VD80_CLKDIV_SHIFT;
	 *lap = tval;
	 return SkelUserReturnOK;

      case Vd80NumGET_CLOCK_EDGE: /* Vd80Edge 0 = Rising, 1 = Falling */

	 tval = (GetReg(regs,VD80_CCR,mcon) & VD80_CLKEDGE_FALLING);
	 if      (tval == VD80_CLKEDGE_RISING)  *lap = Vd80DrvrEdgeRISING;
	 else if (tval == VD80_CLKEDGE_FALLING) *lap = Vd80DrvrEdgeFALLING;
	 else                                   *lap = Vd80DrvrEDGES;
	 return SkelUserReturnOK;

      case Vd80NumGET_CLOCK_TERMINATION: /* Vd80Termination 0 = None 1 = 50_OHMS */

	 tval = (GetReg(regs,VD80_CCR,mcon) & VD80_CLKTERM_MASK);
	 if      (tval == VD80_CLKTERM_NONE)  *lap = Vd80DrvrTerminationNONE;
	 else if (tval == VD80_CLKTERM_50OHM) *lap = Vd80DrvrTermination50OHM;
	 else                                 *lap = Vd80DrvrTERMINATIONS;
	 return SkelUserReturnOK;

      /* ============================ */
      /* Trigger control              */

      case Vd80NumSET_TRIGGER: /* Vd80Trigger 0 = External, 1 = Internal */

	 tval = (GetReg(regs,VD80_TCR1,mcon) & ~VD80_TRIGSOURCE_MASK);
	 if (lval) bms = VD80_TRIGSOURCE_EXT;
	 else      bms = VD80_TRIGSOURCE_INT;
	 tval &= ~VD80_TRIGSOURCE_MASK;
	 tval |= bms;
	 SetReg(regs,VD80_TCR1,tval,mcon);
	 return SkelUserReturnOK;

      case Vd80NumSET_TRIGGER_EDGE: /* Vd80Edge 0 = Rising, 1 = Falling */

	 tval = (GetReg(regs,VD80_TCR1,mcon) & ~VD80_TRIGEDGE_MASK);
	 if (lval) bms = VD80_TRIGEDGE_FALLING;
	 else      bms = VD80_TRIGEDGE_RISING;
	 tval &= ~VD80_TRIGEDGE_FALLING;
	 tval |= bms;
	 SetReg(regs,VD80_TCR1,tval,mcon);
	 return SkelUserReturnOK;

      case Vd80NumSET_TRIGGER_TERMINATION: /* Vd80Termination 0 = None, 1 = 50Ohms */

	 tval = (GetReg(regs,VD80_TCR1,mcon) & ~VD80_TRIG_TERM_MASK);
	 if (lval) bms = VD80_TRIG_TERM_50OHM;
	 else      bms = VD80_TRIG_TERM_NONE;
	 tval &= ~VD80_TRIG_TERM_50OHM;
	 tval |= bms;
	 SetReg(regs,VD80_TCR1,tval,mcon);
	 return SkelUserReturnOK;

      case Vd80NumGET_TRIGGER: /* Vd80Trigger 0 = Internal, 1 = External, 2 = Illegal */

	 tval  = (GetReg(regs,VD80_TCR1,mcon) & VD80_TRIGSOURCE_MASK);
	 if      (tval == VD80_TRIGSOURCE_INT)    *lap = Vd80DrvrTriggerINTERNAL;
	 else if (tval == VD80_TRIGSOURCE_EXT)    *lap = Vd80DrvrTriggerEXTERNAL;
	 else if (tval == VD80_TRIGSOURCE_ANALOG) *lap = Vd80DrvrTriggerINTERNAL;
	 else                                     *lap = Vd80DrvrTRIGGERS;
	 return SkelUserReturnOK;

      case Vd80NumGET_TRIGGER_EDGE: /* Vd80Edge 0 = Rising 1 = Falling */

	 tval = (GetReg(regs,VD80_TCR1,mcon) & VD80_TRIGEDGE_MASK);
	 if      (tval == VD80_TRIGEDGE_RISING)  *lap = Vd80DrvrEdgeRISING;
	 else if (tval == VD80_TRIGEDGE_FALLING) *lap = Vd80DrvrEdgeFALLING;
	 else                                    *lap = Vd80DrvrEDGES;
	 return SkelUserReturnOK;

      case Vd80NumGET_TRIGGER_TERMINATION: /* Vd80Termination 0 = None 1 = 50_OHMS */

	 tval = (GetReg(regs,VD80_TCR1,mcon) & VD80_TRIG_TERM_MASK);
	 if      (tval == VD80_TRIG_TERM_NONE)  *lap = Vd80DrvrTerminationNONE;
	 else if (tval == VD80_TRIG_TERM_50OHM) *lap = Vd80DrvrTermination50OHM;
	 else                                   *lap = Vd80DrvrTERMINATIONS;
	 return SkelUserReturnOK;

      /* ============================ */

      case Vd80NumSET_ANALOGUE_TRIGGER:
	 atrg = (Vd80DrvrAnalogTrig *) arg;

	 for (i=0; i<VD80_CHANNELS; i++)
	    SetReg(regs,VD80_ATRIG_CHAN1+(i*4),0,mcon);

	 i = atrg->Channel;
	 if ((i<1) || (i>VD80_CHANNELS)) {
	    report_client(ccon, SkelDrvrDebugFlagWARNING, "Bad channel number");
	    return SkelUserReturnFAILED;
	 }
	 i--;

	 tval = (GetReg(regs,VD80_TCR1,mcon) & ~VD80_TRIGSOURCE_MASK);
	 tval &= ~VD80_TRIGSOURCE_MASK;
	 tval |= VD80_TRIGSOURCE_ANALOG;
	 SetReg(regs,VD80_TCR1,tval,mcon);

	 tval = ( (atrg->Level   & 0xFFF) << VD80_ATRIG_LEVEL_ABOVE_SHIFT
	      |   (atrg->Level   & 0xFFF) << VD80_ATRIG_LEVEL_BELOW_SHIFT
	      |   (atrg->Control & 0x007) |  VD80_ATRIG_CHANGE_EDGE
	      );

	 SetReg(regs,VD80_ATRIG_CHAN1+(i*4),tval,mcon);

	 return SkelUserReturnOK;

      case Vd80NumGET_ANALOGUE_TRIGGER:
	 atrg = (Vd80DrvrAnalogTrig *) arg;
	 i = atrg->Channel;
	 if ((i<1) || (i>VD80_CHANNELS)) {
	    report_client(ccon, SkelDrvrDebugFlagWARNING, "Bad channel number");
	    return SkelUserReturnFAILED;
	 }
	 i--;

	 tval = GetReg(regs,VD80_ATRIG_CHAN1+(i*4),mcon);

	 atrg->Control = tval & 0x007;
	 atrg->Level   = (tval >> VD80_ATRIG_LEVEL_ABOVE_SHIFT) & 0xFFF;

	 return SkelUserReturnOK;

      /* ============================ */

      case Vd80NumGET_STATE: /* Vd80State 1 = Idle, 2 = PRE-TRIGGER, 4 = POST-TRIGGER */

	 *lap = GetReg(regs,VD80_GSR,mcon) & VD80_STATE_MASK;
	 return SkelUserReturnOK;

      case Vd80NumSET_COMMAND: /* Vd80Command */

	 tval  = lval & VD80_COMMAND_MASK;
	 if (tval == VD80_COMMAND_READ) tval |= lval & VD80_OPERANT_MASK;
	 SetReg(regs,VD80_GCR1,tval,mcon);

	 if (tval == VD80_COMMAND_STOP)
		wait_for_idle(mcon, VD80_KLUDGE_DELAY);

	 return SkelUserReturnOK;

      case Vd80NumREAD_ADC: /* => Channel, <= 16 bit ADC value */

	 if ((lval >=1 ) && (lval <=16)) {
	    tval = (lval -1) >> 1;
	    tval = VD80_ADCR1 + (tval * 4);
	    tval = GetReg(regs,tval,mcon);
	    if (lval & 1) lval = tval & 0xFFFF;
	    else          lval = tval >> 16;
	    *lap = lval;
	    return SkelUserReturnOK;
	 }
      break;

      case Vd80NumSET_POSTSAMPLES: /* Set the number of post samples you want */

	 tval  = GetReg(regs,VD80_PTCR,mcon);
	 tval |= VD80_PSCNTCLRONSTART;
	 tval &= ~(VD80_PSCNTCLKONSMPL | VD80_PSREGCLRONREAD);
	 SetReg(regs,VD80_PTCR,tval,mcon);

	 if (lval) tval = (lval-1) / 32;
	 else      tval = 0;
	 SetReg(regs,VD80_TCR2,tval,mcon);

	 tval = GetReg(regs,VD80_TCR2,mcon);

	 return OK;

      case Vd80NumGET_POSTSAMPLES: /* Get the number of post samples you set */

	 tval = GetReg(regs,VD80_TCR2,mcon);
	 *lap = (tval + 1) * 32;
	 return OK;

      case Vd80NumGET_TRIGGER_CONFIG: /* Get Trig delay and min pre trig samples */

	 tcon = (Vd80DrvrTrigConfig *) arg;
	 tval = GetReg(regs,VD80_TCR1,mcon);
	 if (tval & VD80_TRIGOUT_DELAY)
	    tcon->TrigDelay = GetReg(regs,VD80_TCR3,mcon);
	 else
	    tcon->TrigDelay = 0;

	 tsmp   = GetReg(regs,VD80_PTCR,mcon);

	 tsmp  &= VD80_PRESAMPLESMIN_MASK;
	 tsmp >>= VD80_PRESAMPLESMIN_SHIFT;
	 tsmp  *= 32;
	 tcon->MinPreTrig = tsmp;

	 return OK;

      case Vd80NumSET_TRIGGER_CONFIG: /* Set Trig delay and min pre trig samples */

	 tcon = (Vd80DrvrTrigConfig *) arg;

	 tval = GetReg(regs,VD80_TCR1,mcon);
	 if (tcon->TrigDelay) {
	    tval |= VD80_TRIGOUT_DELAY;
	    SetReg(regs,VD80_TCR1,tval,mcon);
	    SetReg(regs,VD80_TCR3,tcon->TrigDelay,mcon);
	 } else {
	    tval &=~VD80_TRIGOUT_DELAY;
	    SetReg(regs,VD80_TCR1,tval,mcon);
	    SetReg(regs,VD80_TCR3,0,mcon);
	 }

	 tval = GetReg(regs,VD80_PTCR,mcon) & ~VD80_PRESAMPLESMIN_MASK;
	 if (tcon->MinPreTrig) {
	    tsmp   = tcon->MinPreTrig/32;
	    tsmp <<= VD80_PRESAMPLESMIN_SHIFT;
	    tsmp  &= VD80_PRESAMPLESMIN_MASK;
	    tval |= tsmp;
	 }
	 tval |= VD80_PSCNTCLRONSTART;
	 tval &= ~(VD80_PSCNTCLKONSMPL | VD80_PSREGCLRONREAD);
	 SetReg(regs,VD80_PTCR,tval,mcon);

	 return OK;

      case Vd80NumREAD_SAMPLE: /* => Vd80SampleBuf <= */

	 sbuf = (Vd80SampleBuf *) arg;

	 /* If the emulation is on, return an eight bit saw tooth function */

	 if (mcon->StandardStatus & SkelDrvrStandardStatusEMULATION) {
	    for (i=0; i<sbuf->BufSizeSamples; i++) {
	       sbuf->SampleBuf[i] = i & 0xFF;   /* Saw tooth */
	    }
	    sbuf->Samples = sbuf->BufSizeSamples;
	    return SkelUserReturnOK;
	 }

	 /* Check that the requested post samples have been set */

	 tval = GetReg(regs,VD80_TCR2,mcon);
	 if (tval == 0) {
	    report_client(ccon, SkelDrvrDebugFlagWARNING, "No postsamples have been set:No data to read");
	    return SkelUserReturnFAILED;
	 }

	 /* Force the module into the idle state */

	 tval = VD80_COMMAND_SUBSTOP;
	 SetReg(regs,VD80_GCR1,tval,mcon);
	 tval = VD80_COMMAND_STOP;
	 SetReg(regs,VD80_GCR1,tval,mcon);
	 wait_for_idle(mcon, VD80_KLUDGE_DELAY);

	 /* actpostrig is the number of actual post trigger samples */
	 /* shotlength is the total number of pre and post trigger samples */

	 actpostrig = (GetReg(regs,VD80_TSR,mcon) << VD80_ACTPOSTSAMPLES_SHIFT) & VD80_ACTPOSTSAMPLES_MASK;
	 shotlength = (GetReg(regs,VD80_SSR,mcon) << VD80_SHOTLEN_SHIFT) & VD80_SHOTLEN_MASK;

	 sbuf->PreTrigStat = GetReg(regs,VD80_PTSR,mcon); /* 100K ticks since start */

	 /* actpretrig is the actual number of pre trigger samples */

	 actpretrig =  shotlength - actpostrig;
	 if (sbuf->TrigPosition > actpretrig) { /* User asked for too many pre trigger samples */
	    sbuf->TrigPosition = actpretrig;    /* so give him what we have */
	 }

	 /* samplestrt is the position in buffer to start reading from */

	 samplestrt =  shotlength - (actpostrig + sbuf->TrigPosition); /* This has to be posative from above */

	 /* sbuf->samples is the total number we are going to read */

	 sbuf->Samples = sbuf->BufSizeSamples;
	 if (samplestrt + sbuf->BufSizeSamples > shotlength) { /* User asked for too many post samples */
	    sbuf->Samples = (shotlength - samplestrt);
	 }

	 /* Set up the readstart and readlength registers */

	 tval = samplestrt / 32;             /* Start Address divided by 32 */
	 tval = (tval << VD80_READSTART_SHIFT) & VD80_READSTART_MASK;
	 SetReg(regs,VD80_MCR1,tval,mcon);

	 sbuf->TrigPosition = actpretrig - (tval*32); /* The real actual trigger position in buffer */

	 if (sbuf->Samples < 32) tval = 0;                          /* Reads 32 samples */
	 else                    tval = (sbuf->Samples - 1) / 32;   /* Divide by 32 */
	 tval = (tval << VD80_READLEN_SHIFT) & VD80_READLEN_MASK;
	 SetReg(regs,VD80_MCR2,tval,mcon);

	 /* Start the read for the given channel */

	 tval = VD80_COMMAND_READ | (((sbuf->Channel -1)<<VD80_OPERANT_SHIFT) & VD80_OPERANT_MASK);
	 SetReg(regs,VD80_GCR1,tval,mcon);

	 remaining = sbuf->Samples; /* Samples remaining to be transfered by dma */
	 samples = 0;               /* Samples transfered so far */

	 psze = remaining;

	 while (remaining > 0) {

	    if (remaining > psze) tcnt = psze;
	    else                  tcnt = remaining;

	    BuildDmaReadDesc(mcon,&(sbuf->SampleBuf[samples]),tcnt*sizeof(short),&dma_desc);
	    if (vme_do_dma(&dma_desc)) {
	       report_module(mcon, SkelDrvrDebugFlagASSERTION, "SkelUserIoctl:Error vme_do_dma");
	       return SkelUserReturnFAILED;
	    }

	    remaining -= tcnt;
	    samples   += tcnt;
	 }

	 sbuf->Samples = samples; /* The actual number we successfully transfered */
	 return SkelUserReturnOK;
      break;

      default:
      break;
   }
   report_client(ccon, SkelDrvrDebugFlagWARNING, "SkelUserIoctls:Illegal IOCTL number");
   return SkelUserReturnFAILED;
}

/* =========================================================== */
/* Get the name of an IOCTL for Skel debug support             */

char *SkelUserGetIoctlName(int cmd) {

int i;

   cmd = _IOC_NR(cmd);
   if (cmd < Vd80NumLAST) {
      i = cmd - Vd80NumFIRST;
      if (i >= 0) return Vd80IoctlNames[cmd];
   }
   return Vd80IoctlNames[Vd80NumFIRST];
}

/* =========================================================== */
/* Initialize a Vd80 module, called from install               */

SkelUserReturn SkelUserModuleInit(SkelDrvrModuleContext *mcon) {

InsLibIntrDesc         *intrd = NULL;
InsLibModlDesc         *modld = NULL;
InsLibVmeModuleAddress *vmema = NULL;
InsLibVmeAddressSpace  *vmeas = NULL;

uint32_t *config = NULL;        /* Always use 32-bit accesses */

uint32_t  tval;
uint32_t  valu[4];
char *board_id = "VD80"; /* The character board_id in Vd80 configuration ROM */

char revis_id[VD80_CR_REV_ID_LEN +1]; /* The revis_id and terminator byte */

int i;
UserData *u;

   if (!mcon) return SkelUserReturnFAILED;

   modld = mcon->Modld;
   if (!modld) return SkelUserReturnFAILED;

   if (mcon->StandardStatus & SkelDrvrStandardStatusEMULATION) return SkelUserReturnOK;

   vmema = modld->ModuleAddress;
   if (!vmema) return SkelUserReturnFAILED;

   vmeas = vmema->VmeAddressSpace;
   while (vmeas) {

      switch (vmeas->AddressModifier) {

	 case VME_CR_CSR:    /* CRCSR Configuration space */

	    /* First check we're accessing a configuration ROM */

	    config = (uint32_t *) vmeas->Mapped;

	    valu[1] = (uint32_t) be32_to_cpu(ioread32(&config[VD80_CR_SIG1/4]));
	    valu[2] = (uint32_t) be32_to_cpu(ioread32(&config[VD80_CR_SIG2/4]));

	    if ( ((valu[1] & 0xFF) != 'C')
	    ||   ((valu[2] & 0xFF) != 'R') ) {
	       report_module(mcon, SkelDrvrDebugFlagWARNING, "Configuration Rom not found");
	       return SkelUserReturnFAILED;
	    }

	    /* Read the board ID and compare it to "VD80" */

	    for (i=0; i<strlen(board_id); i++) {
	       tval = be32_to_cpu(ioread32(&config[(VD80_CR_BOARD_ID/4)+i]));
	       if (board_id[i] != (char) (tval & 0xff)) {
		  report_module(mcon, SkelDrvrDebugFlagWARNING, "Bad Configuration Board ID");
		  return SkelUserReturnFAILED;
	       }
	    }

	    /* Read the board revision ID  */

	    for (i=0; i<VD80_CR_REV_ID_LEN; i++) {
	       tval = be32_to_cpu(ioread32(&config[(VD80_CR_REV_ID/4)+i]));
	       revis_id[i] = (char) (tval & 0xff);
	    }
	    revis_id[i] = '\0';
	    printk("VD80: Board Revision ID:%s ",revis_id);
	    if (strcmp("C1A9",revis_id) == 0)
	       printk("OK - Supported by this driver\n");
	    else if (strcmp("C2Aa",revis_id) == 0)
	       printk("OK - Supported by this driver\n");
	    else if (strcmp("C2Ab",revis_id) == 0)
	       printk("OK - Supported by this driver\n");
	    else if (strcmp("C2Ac",revis_id) == 0)
	       printk("OK - Supported by this driver\n");
	    else if (strcmp("C2Ad",revis_id) == 0)
	       printk("OK - Supported by this driver\n");
	    else if (strcmp("C2Ae",revis_id) == 0)
	       printk("OK - Supported by this driver\n");
	    else if (strcmp("C2Af",revis_id) == 0)
	       printk("OK - Supported by this driver\n");
	    else {
	       printk("ERROR - %s NOT SUPPORTED BY THIS DRIVER\n",revis_id);
	       return SkelUserReturnFAILED;
	    }

	    u = (UserData *) kmalloc(sizeof(UserData),GFP_KERNEL);  /* TODO: Leaks on uninstall */
	    if (u) strncpy(u->revis_id,revis_id,VD80_CR_REV_ID_LEN);
	    else
	       report_module(mcon, SkelDrvrDebugFlagWARNING, "No memory in SkelUserConfig:UserData");
	    mcon->UserData = u;

	    intrd = mcon->Modld->Isr;
	    iowrite32(cpu_to_be32((uint32_t) intrd->Vector), &(config[VD80_CSR_IRQ_VECTOR/4]));
	    iowrite32(cpu_to_be32((uint32_t) intrd->Level),  &(config[VD80_CSR_IRQ_LEVEL /4]));

	    if (Wa->Endian == InsLibEndianLITTLE)
	       iowrite32(cpu_to_be32((uint32_t) VD80_CSR_MBLT_LITTLE_ENDIAN), &(config[VD80_CSR_MBLT_ENDIAN /4]));
	    else
	       iowrite32(cpu_to_be32((uint32_t) VD80_CSR_MBLT_BIG_ENDIAN),    &(config[VD80_CSR_MBLT_ENDIAN /4]));

	    iowrite32(0, &(config[VD80_TCR3 /4])); // Clear trigger delay
	    iowrite32(0, &(config[VD80_PTCR /4])); // Clear min pre trigger samples
	 break;

	 case VME_A24_USER_DATA_SCT:

	    /* Configure ADER0 for A24 USER DATA access */

	    if (config) {

	       iowrite32(cpu_to_be32((uint32_t) (vmeas->BaseAddress >> 24) & 0xff), &(config[VD80_CSR_ADER0/4   ]));
	       iowrite32(cpu_to_be32((uint32_t) (vmeas->BaseAddress >> 16) & 0xff), &(config[VD80_CSR_ADER0/4 +1]));
	       iowrite32(cpu_to_be32((uint32_t) (vmeas->BaseAddress >>  8) & 0xff), &(config[VD80_CSR_ADER0/4 +2]));

	       iowrite32(cpu_to_be32((uint32_t) (VME_A24_USER_DATA_SCT * 4)), &(config[VD80_CSR_ADER0/4 +3]));
	       iowrite32(cpu_to_be32((uint32_t) (VME_A24_USER_MBLT     * 4)), &(config[VD80_CR_FUNC0_AM_MASK/4 +3]));
	       iowrite32(cpu_to_be32((uint32_t) (VME_A24_USER_MBLT     * 4)), &(config[VD80_CR_FUNC1_AM_MASK/4 +3]));

	       iowrite32(cpu_to_be32((uint32_t) (vmeas->BaseAddress >> 24) & 0xff), &(config[VD80_CSR_ADER1/4   ]));
	       iowrite32(cpu_to_be32((uint32_t) (vmeas->BaseAddress >> 16) & 0xff), &(config[VD80_CSR_ADER1/4 +1]));
	       iowrite32(cpu_to_be32((uint32_t) (vmeas->BaseAddress >>  8) & 0xff), &(config[VD80_CSR_ADER1/4 +2]));
	       iowrite32(cpu_to_be32((uint32_t) (VME_A24_USER_MBLT * 4)          ), &(config[VD80_CSR_ADER1/4 +3]));

	       iowrite32(cpu_to_be32((uint32_t) (0)                              ), &(config[VD80_CSR_FUNC_ACEN/4]));
	       iowrite32(cpu_to_be32((uint32_t) (VD80_CSR_ENABLE_MODULE)         ), &(config[VD80_CSR_BITSET/4   ]));

	    } else {
	       report_module(mcon, SkelDrvrDebugFlagWARNING, "Missing AM=2F can't config A24");
	       return SkelUserReturnFAILED;
	    }

	 break;

	 case VME_A24_USER_MBLT:    /* Nothing needs mapping for DMA */

	    /* I already set this up in VME_A24_USER_DATA_SCT, see just above */
	    /* This saves me from having to specify the AM in the XML file */

	 break;

	 default:
	    report_module(mcon, SkelDrvrDebugFlagASSERTION, "Unsupported AM");
	    return SkelUserReturnFAILED;
      }

      vmeas = vmeas->Next;
   }

   /* Enable the module */

   iowrite32(cpu_to_be32((uint32_t) VD80_CSR_ENABLE_MODULE),
		  &(config[VD80_CSR_BITSET/4]) );

   return SkelUserReturnOK;
}

void SkelUserModuleRelease(SkelDrvrModuleContext *mcon)
{
}

SkelUserReturn SkelUserClientInit(SkelDrvrClientContext *ccon)
{
	return SkelUserReturnNOT_IMPLEMENTED;
}

void SkelUserClientRelease(SkelDrvrClientContext *ccon)
{
}

struct skel_conf SkelConf; /* no special config for this driver */
