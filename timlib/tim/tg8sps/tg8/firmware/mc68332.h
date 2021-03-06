/***************************************+++++++*****************/
/* MC68332 micro processor (MCP) specific hardware definitions */
/* J.Lewis Mon 17Th Oct 1994                                   */
/* Updated by V.Kovaltsov April 1996                           */
/* Vladimir Kovaltsov for the SL Timing, February, 1997	       */
/***************************************************************/

#ifndef MCP_I
#define MCP_I

/***************************************************************/
/* MC68332 & MC68332BCC features                               */
/***************************************************************/

/*	MC68332 features are as follows:
	-------
	- CPU MC68010 upward compatible + New instructions for
	  controller operation
	- 16 MHz
	- Low power operation: 60mW maximum, 500uW in standby mode
	- Intelligent 16 bits timer:
		- 16 independent programmable channels and Pins
		- any channel can perform any time function (Input
		  capture, output compare, pulse width comparison, ...)
		- 2 timer count registers with 2-bit programmable prescalers
		- selectable channel priority levels
	- Two serial I/O subsystems:
		- Enhanced M68HC11-type Serial communication interface (SCI)
		  UART with parity
		- Enhanced M68HC11-type Serial Peripheral interface (SPI)
		  with I/O queue (SQI)
	- 2Kb on-board Standby RAM (2 cycles access time)
	- Up to 12 signal for memory and peripheral I/O select
	- System failure protection:
		- M68HC11-type Computer Operating Properly (COP) watchdog
		- M68HC11-type Periodic Interrupt timer
		- M68000 Family Spurious interrupt, Halt and Bus timeouts
	- Up to 32 discrete I/O pins

	M68332BCC (Board Card Circuite) features are as follows:
	---------
	- Internal Clock 32.768 KHz
	- External Clock 25 to 50 KHz
	- 64Kb RAM 85ns
	- 128Kb EPROM 200ns
	- RS-232C level converter chip
*/

/****************************************************************/
/* M68332 Memory Map definitions                                */
/****************************************************************/
/* ==ERAM==  ==PROM==  ==SIM==  ==SBR==  ==QSM==  ==TPU==       */
/*    64Kb     128Kb              2Kb                     size  */
/*  0000      60000     FFA00    FFB00    FFC00    FFE00  start */
/* 10000      80000     FF?00    FF?00    FF?00    FF?00  end   */
/****************************************************************/

#define McpERAM_START  0x000000   /* Start 64Kb on-board RAM */
#define McpBUG_END     0x003000   /* End of RAM area used by 332Bug   */
				  /* as it documented in the corres-  */
				  /* ponding manual. In reality the   */
				  /* addresses near 0x4000 ??? are touched */
				  /* by the 332Bug RESET routine. One */
				  /* may use the memory starting from */
				  /* 4020 without fear.               */
#define McpERAM_END    0x010000   /* End of 64Kb on-board RAM */

#define McpPROM_START  0x060000   /* Start of 128Kb on-board EPROM */
#define McpPROM_END    0x080000   /* End of 128Kb on-board EPROM */

#define McpSIM_BASE    0xfffffa00 /* Systems Integration Module (SIM) regs */
#define McpSBR_BASE    0xfffffb00 /* On-chip stand-by RAM ctrl regs */
#define McpQSM_BASE    0xfffffc00 /* QSM regs */
#define McpTPU_BASE    0xfffffe00 /* Time Processor Unit regs */
#define McpTPU_ACT     0xffffff00 /* Address space of the TPU memory map (256 bytes) */

/****************************************************************/
/* SIM: System Intergration module - Base address YFFA00        */
/****************************************************************/

#if CONVERTOR
#define McpSim_spare McpSIM_BASE
#define McpQsm_spare McpQSM_BASE
#define McpSbr_spare McpSBR_BASE
#define McpTpu_spare McpTPU_BASE
#else
#define McpSim_spare 0
#define McpQsm_spare 0
#define McpSbr_spare 0
#define McpTpu_spare 0
#endif

typedef struct {     /************************************/
	char    SimSpare_bytes [McpSim_spare];
	short   SimConf;           /* 00 Module configuration          */
	short   SimTest;           /* 02 Module test                   */
	short   SimSync;           /* 04 Clock synthesizer Control     */
	short   SimReset;          /* 06 Reset Status Register         */
	short   SimTestE;          /* 08 module test E                 */
	short   SimU0a;            /* 0A                               */
	short   SimU0c;            /* 0C                               */
	short   SimU0e;            /* 0E                               */
	short   SimDataE0;         /* 10 Port E Data 0                 */
	short   SimDataE1;         /* 12 Port E Data 1                 */
	short   SimDirE;           /* 14 Port E Data direction         */
	short   SimPinE;           /* 16 Port E Pin Assignment         */
	short   SimDataF0;         /* 18 Port F Data 0                 */
	short   SimDataF1;         /* 1A Port F Data 1                 */
	short   SimDirF;           /* 1C Port F Data direction         */
	short   SimPinF;           /* 1E Port F Pin Assignment         */
	short   SimProtect;        /* 20 System Protection Control     */
	short   SimPic;            /* 22 Periodic Interrupt Control    */
	short   SimPit;            /* 24 Periodic Interrupt Timing     */
	short   SimServ;           /* 26 Software service              */
	short   SimU28;            /* 28                               */
	short   SimU2a;            /* 2A                               */
	short   SimU2c;            /* 2C                               */
	short   SimU2e;            /* 2E                               */
	short   SimTestMrA;        /* 30 Test module master A          */
	short   SimTestMrB;        /* 32 Test module master B          */
	char    SimTestScA,TstScB; /* 34 Test module shift count A & B */
	short   SimTestRep;        /* 36 Test module Repeat. counter   */
	short   SimTestCtl;        /* 38 Test module controle          */
	short   SimTestDis;        /* 3A Test module distributed reg   */
	short   SimU3c;            /* 3C                               */
	short   SimU3e;            /* 3E                               */
	short   SimDataC;          /* 40 Port C data                   */
	short   SimU42;            /* 42                               */
	int     SimCSpin;          /* 44 chip-select pin assignment    */
	int     SimCSboot;         /* 48 chip-select boot              */
	int     SimCS0;            /* 4C chip-select 0                 */
	int     SimCS1;            /* 50 chip-select 1                 */
	int     SimCS2;            /* 54 chip-select 2                 */
	int     SimCS3;            /* 58 chip-select 3                 */
	int     SimCS4;            /* 5C chip-select 4                 */
	int     SimCS5;            /* 60 chip-select 5                 */
	int     SimCS6;            /* 64 chip-select 6                 */
	int     SimCS7;            /* 68 chip-select 7                 */
	int     SimCS8;            /* 6C chip-select 8                 */
	int     SimCS9;            /* 70 chip-select 9                 */
	int     SimCS10;           /* 74 chip-select 10                */
	int     SimU78;            /* 78                               */
	int     SimU7c;            /* 7C                               */
} McpSim;                       /************************************/

/****************************************************************/
/* Standby RAM (with TPU emulation) Base address YFFB00         */
/****************************************************************/

typedef struct {     /************************************/
	char    SbrSpare_bytes [McpSbr_spare];
	short   SbrConf;        /* 00 Configuration register       */
	short   SbrTest;        /* 02 Test register                */
	short   SbrBaSt;        /* 04 Base address/Status reg.     */
} McpSbr;                       /************************************/

/****************************************************************/
/* QSM: Queued Serial module - Base address YFFC00              */
/****************************************************************/

typedef struct {    /************************************/
	char    QsmSpare_bytes [McpQsm_spare];
	short   QsmConf;           /* c00 QMCR  Configuration register */
	short   QsmTest;           /* c02 QTEST Test register          */
	char    QsmIntLev,         /* c04 QILR Interrupt Level         */
		QsmIntVec;         /* c05 QIVR           Vector        */
	short   QsmU06;            /* c06                              */
	short   QsmSciC0;          /* c08 SCI Control reg 0            */
	short   QsmSciC1;          /* c0A SCI Control reg 1            */
	short   QsmSciS;           /* c0C SCI Status reg               */
	short   QsmSciD;           /* c0E SCI Data reg                 */
	short   QsmU10;            /* c10                              */
	short   QsmU12;            /* c12                              */
	char    QsmU14,            /* c14                              */
		QsmPd;             /* c15 QPDR Port Data register      */
	char    QsmPin,            /* c16 QPAR Pin Assignment          */
		QsmDir;            /* c17 QDDR Data direction          */
	short   QsmSpiC0;          /* c18 QSPI Control reg 0           */
	short   QsmSpiC1;          /* c1A QSPI Control reg 1           */
	short   QsmSpiC2;          /* c1C QSPI Control reg 2           */
	char    QsmSpiC3,          /* c1E QSPI Control reg 3           */
		QsmSpiS;           /* c1F QSPI status                  */
	char    QsmU20[0xE0];      /* c20  ??kov. [was 0xc0 !]         */
	unsigned
	short   QsmRecv_ram[16],   /* d00 QSPI Receive data            */
		QsmTran_ram[16];   /* d20 QSPI Transmit data           */
	unsigned
	char    QsmComd_ram[16];   /* d40 QSPI Control                 */
} McpQsm;                       /************************************/

/****************************************************************/
/* TPU: Time Processing Unit module - Base address YFFE00       */
/*      Address space of the TPU memory map occupies 256 bytes  */
/*      starting at address YFFF00                              */
/****************************************************************/

typedef struct {     /************************************/
	char    TpuSpare_bytes [McpTpu_spare];
	short   TpuConf;           /* 00 Module configuration register */
	short   TpuCR;             /* 02 Configuration reg.            */
	short   TpuDsc;            /* 04 Development support control   */
	short   TpuDss;            /* 06 Development support status    */
	short   TpuIc;             /* 08 Interrupt Configuration Reg.  */
	short   TpuIe;             /* 0A Interrupt Enable Reg.         */
	short   TpuCfs0;           /* 0C Channel Function Select Reg.0 */
	short   TpuCfs1;           /* 0E                             1 */
	short   TpuCfs2;           /* 10                             2 */
	short   TpuCfs3;           /* 12                             3 */
	short   TpuHs0;            /* 14 Host Sequence Reg. 0          */
	short   TpuHs1;            /* 16                    1          */
	short   TpuHsr0;           /* 18 Host Service Request Reg.0    */
	short   TpuHsr1;           /* 1A                          1    */
	short   TpuCp0;            /* 1C Channel Priority Reg.0        */
	short   TpuCp1;            /* 1E                      1        */
	short   TpuIs;             /* 20 Interrupt Status Reg.         */
	short   TpuLink;           /* 22 Link Register                 */
	short   TpuSgl;            /* 24 Service Grant Latch Reg.      */
	short   TpuDcn;            /* 26 Decode Channel Number Reg.    */
} McpTpu;                          /************************************/

/****************************************************************/
/* Chip Selection                                               */
/****************************************************************/

/* Define fields in chip select base address and options registers */

#define McpCS_BA_ADDRESS_MASK  0xFFF80000
#define McpCS_BA_ADDRESS_SHIFT 8

/* Block size BLKSZ */

#define McpCS_BA_BLKSZ_2K   0x00000000
#define McpCS_BA_BLKSZ_8K   0x00010000
#define McpCS_BA_BLKSZ_16K  0x00020000
#define McpCS_BA_BLKSZ_64K  0x00030000
#define McpCS_BA_BLKSZ_128K 0x00040000
#define McpCS_BA_BLKSZ_256K 0x00050000
#define McpCS_BA_BLKSZ_512K 0x00060000
#define McpCS_BA_BLKSZ_1M   0x00070000

/* Asynchronous/Synchronous MODE */

#define McpCS_OP_MODE_SYNC  0x8000
#define McpCS_OP_MODE_ASYNC 0x0000

/* Upper/Lower BYTE */

#define McpCS_OP_BYTE_DISABLE 0x0000
#define McpCS_OP_BYTE_LOWER   0x2000
#define McpCS_OP_BYTE_UPPER   0x4000
#define McpCS_OP_BYTE_BOTH    0x6000

/* Read/Write bar RW */

#define McpCS_OP_RW_RO 0x0800
#define McpCS_OP_RW_WO 0x1000
#define McpCS_OP_RW_RW 0x1800

/* Address/Data strobe STRB */

#define McpCS_OP_STRB_ADDRESS 0x0000
#define McpCS_OP_STRB_DATA    0x0400

/* Data strobe acknowledge DSACK Bar */

#define McpCS_OP_DSACK_NOWAIT   0x0000
#define McpCS_OP_DSACK_WAIT_1   0x0040
#define McpCS_OP_DSACK_WAIT_2   0x0080
#define McpCS_OP_DSACK_WAIT_3   0x00C0
#define McpCS_OP_DSACK_WAIT_4   0x0100
#define McpCS_OP_DSACK_WAIT_5   0x0140
#define McpCS_OP_DSACK_WAIT_6   0x0180
#define McpCS_OP_DSACK_WAIT_7   0x01C0
#define McpCS_OP_DSACK_WAIT_8   0x0200
#define McpCS_OP_DSACK_WAIT_9   0x0240
#define McpCS_OP_DSACK_WAIT_10  0x0280
#define McpCS_OP_DSACK_WAIT_11  0x02C0
#define McpCS_OP_DSACK_WAIT_12  0x0300
#define McpCS_OP_DSACK_WAIT_13  0x0340
#define McpCS_OP_DSACK_FAST     0x0380
#define McpCS_OP_DSACK_EXTERNAL 0x03C0

/* Address space SPACE */

#define McpCS_OP_SPACE_CPU   0x0000
#define McpCS_OP_SPACE_USER  0x0010
#define McpCS_OP_SPACE_SUPER 0x0020
#define McpCS_OP_SPACE_BOTH  0x0030

/* Interrupt priority level IPL */

#define McpCS_OP_IPL_ANY 0x0000
#define McpCS_OP_IPL_1   0x0002
#define McpCS_OP_IPL_2   0x0004
#define McpCS_OP_IPL_3   0x0006
#define McpCS_OP_IPL_4   0x0008
#define McpCS_OP_IPL_5   0x000A
#define McpCS_OP_IPL_6   0x000C
#define McpCS_OP_IPL_7   0x000E

/* Auto vector AVEC */

#define McpCS_OP_AVEC_EXTERNAL 0x0000
#define McpCS_OP_AVEC_ENABLED  0x0001

/* Some usefull 68332 fixed Interrupt vector offsets */

#define McpVECTOR_BUS_ERROR           0x008
#define McpVECTOR_ADDRESS_ERROR       0x00C
#define McpVECTOR_ILLEGAL_INSTRUCTION 0x010
#define McpVECTOR_ZERO_DIVISION       0x014
#define McpVECTOR_PRIVILEGE_VIOLATION 0x020
#define McpVECTOR_SPURIOUS_INTERRUPT  0x060

/* Autovector offsets */

#define McpVECTOR_AUTO_1              0x064
#define McpVECTOR_AUTO_2              0x068
#define McpVECTOR_AUTO_3              0x06C
#define McpVECTOR_AUTO_4              0x070
#define McpVECTOR_AUTO_5              0x074
#define McpVECTOR_AUTO_6              0x078
#define McpVECTOR_AUTO_7              0x07C

/*?for what? #define McpVECTOR_TRAP_BASE 0x080 */

#define McpTRAP_VECTOR_BASE 32 /* Trap vector base address (like for the Tg8) */

#endif

/*EOF --mc68332.h-- */
