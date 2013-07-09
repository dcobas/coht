/**
 * =========================================================================================
 * PRIVATE driver structures, info, module and client contexts etc.
 *
 * Julian Lewis BE/CO/HT Thu 14th Mar 2013
 * =========================================================================================
 */

/* This = icv196drvrP.h : PRIVATE driver structures, not for normal clients.  */

#ifndef ICV196DRVR_P
#define ICV196DRVR_P

#include <icv196.h>
#include <Z8536CIO.h>

/* ============================================= */
/* Declare the structures in the ICV196 memory map */
/* ============================================= */

/* ----------------------------- */
/* Interrupt registers (IRQ)     */

struct icv196_irq_reg_s {
	uint32_t src;     /* Interrupt source register */
	uint32_t enable;  /* Interrupt Enable          */
	uint32_t vec;     /* Interrupt vector          */
	uint32_t lvl;     /* Interrupt level           */
};

#ifdef __KERNEL__

/* --------------------------------- */
/* Module side connection structures */

struct icv196_map_s {
	struct icv196_mod_addr_s  mad;               /* insmod parameters */
	uint8_t                  *vad;               /* The mapped address or NULL if window is not mapped */
	struct vme_berr_handler  *bus_error_handler; /* The bus error handler called for bus errors in window */
};

struct icv196_device_s {
	struct icv196_map_s map;
	struct mutex mutex;
	uint32_t timeout;
	uint32_t revid;         /** ICV196 Firmware revision ID */
	uint32_t dev_idx;       /** Device index 0..devices */
	uint32_t deb_lvl;       /** Debug level */
	uint32_t out_msk;       /** Group IO direction mask */
	uint32_t int_msk;       /** Interrupt mask */
};

#define MAX_DEVICES 8

struct working_area_s {
	uint32_t devcnt;                          /** Number of intstalled devices */
	struct icv196_device_s devs[MAX_DEVICES]; /** Devices */
	wait_queue_head_t queue;
	struct icv196_int_buf_s ibuf;
};

#endif
#endif
