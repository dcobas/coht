/**
 * =========================================================================================
 * PRIVATE driver structures, info, module and client contexts etc.
 *
 * Julian Lewis BE/CO/HT Thu 14th Mar 2013
 * =========================================================================================
 */

/* This = vd80drvrP.h : PRIVATE driver structures, not for normal clients.  */

#ifndef VD80DRVR_P
#define VD80DRVR_P

#include <vd80.h>
#include <vd80hard.h>

/* ============================================= */
/* Declare the structures in the VD80 memory map */
/* ============================================= */

/* ----------------------------- */
/* Interrupt registers (IRQ)     */

struct vd80_irq_reg_s {
	uint32_t src;     /* Interrupt source register */
	uint32_t enable;  /* Interrupt Enable          */
	uint32_t vec;     /* Interrupt vector          */
	uint32_t lvl;     /* Interrupt level           */
};

#ifdef __KERNEL__

/* --------------------------------- */
/* Module side connection structures */

struct vd80_map_s {
	struct vd80_mod_addr_s   mad;               /* insmod parameters */
	uint32_t                *vad;               /* The mapped address or NULL if window is not mapped */
	struct vme_berr_handler *bus_error_handler; /* The bus error handler called for bus errors in window */
};

struct vd80_device_s {
	struct vd80_map_s map;
	struct mutex mutex;
	uint32_t timeout;
	uint32_t revid;         /** VD80 Firmware revision ID */
	uint32_t dev_idx;       /** Device index 0..devices */
	uint32_t deb_lvl;       /** Debug level */
};

#define MAX_DEVICES 8

struct working_area_s {
	uint32_t devcnt;                        /** Number of intstalled devices */
	struct vd80_device_s devs[MAX_DEVICES]; /** Devices */
	wait_queue_head_t queue;
	struct vd80_int_buf_s ibuf;
};

#endif
#endif
