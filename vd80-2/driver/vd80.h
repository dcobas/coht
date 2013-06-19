/**
 * =========================================================================================
 *
 * VD80 (ADC Sampler module) Device driver.
 * Definitions file
 * Julian Lewis BE/CO/HT Thu 14th Mar 2013
 *
 * =========================================================================================
 */

#ifndef VD80
#define VD80

#define DEBUG 0
#define TIMEOUT 1000

/*
 * =========================================================================================
 * Debug level
 */

typedef enum {
	VD80_DEBUG_OFF,
	VD80_DEBUG_IOCTL,
	VD80_DEBUG_BUF
} vd80_debug_level_t;

/**
 * =========================================================================================
 * Parameter for module address supplied as insmod parameters
 */

struct vd80_mod_addr_s {
	uint32_t lun;     /* Logical unit number */
	uint32_t vec;     /* Interrupt vector */
	uint32_t vmeb;    /* VD80 VME base address */
	uint32_t slot;    /* VME Slot number */
};

/**
 * =========================================================================================
 * Get compliation dates of the driver and library
 */

struct vd80_version_s {
	uint32_t drvrver; /* Driver  version (UTC compilation time) */
	uint32_t vhdlver; /* VD80 firmware revision ID */
};

/*
 * =========================================================================================
 * Parameter for raw IO
 */

struct vd80_riob_s {
	uint32_t boffs; /** Byte offset in map */
	uint32_t bsize; /** The number of bytes to read */
	void *buffer;   /** Pointer to user data area */
};

/*
 * =========================================================================================
 * Whats read back from waiting on interrupts
 */

struct vd80_int_buf_s {
	uint32_t src; /** Interrupt mask */
	uint32_t cnt; /** Interrupt count */
	uint32_t lun; /** Interrupt logical unit number */
};

/**
 * =========================================================================================
 */

/* WO is a value that you must supply */
/* RO is a value returned to you by the driver */
/* RW is WO followed by RO */

/* The driver will try to supply the requested data, in any case */
/* it will adjust the values of TrigPosition and Samples as best it can */
/* You should have set up the number of post-samples SET_POSTSAMPLES */
/* prior to calling READ_SAMPLE. */

struct vd80_sample_buf_s {
	uint32_t channel;          /* WO Channel to read from */
	uint32_t buf_size_samples; /* WO Target buffer size (In samples) */
	uint32_t trig_position;    /* RW Position of trigger in buf */
	uint32_t samples_read;     /* RO On return: The number of samples read */
	uint32_t pre_trig_stat;    /* RO Pre-Trigger Status Register */
	int16_t *sample_buf;       /* RO Buffer where 16Bit signed samples will be stored */
};

/* ========================================================================================= */

typedef enum {
	VD80_DIVIDE_MODE_DIVIDE,
	VD80_DIVIDE_MODE_SUBSAMPLE,
	VD80_DIVIDE_MODES
} vd80_divide_mode_t;

typedef enum {
	VD80_EDGE_RISING,
	VD80_EDGE_FALLING,
	VD80_EDGES
} vd80_edge_t;

typedef enum {
	VD80_TERMINATION_NONE,
	VD80_TERMINATION_50OHM,
	VD80_TERMINATIONS
} vd80_termination_t;

typedef enum {
	VD80_SOURCE_INTERNAL,
	VD80_SOURCE_EXTERNAL,
	VD80_SOURCES
} vd80_source_t;

typedef enum {
	VD80_STATE_IDLE        = 0x1,
	VD80_STATE_PRETRIGGER  = 0x2,
	VD80_STATE_POSTTRIGGER = 0x4,
	VD80_STATES            = 3
} vd80_state_t;

typedef enum {
	VD80_COMMAND_STOP     = 0xE,
	VD80_COMMAND_START    = 0xB,
	VD80_COMMAND_SUBSTOP  = 0x8,
	VD80_COMMAND_SUBSTART = 0x9,
	VD80_COMMAND_TRIGGER  = 0x2,
	VD80_COMMAND_READ     = 0x4,
	VD80_COMMAND_SINGLE   = 0x1,
	VD80_COMMAND_CLEAR    = 0xC,
	VD80_COMMANDS         = 8
} vd80_command_t;

typedef enum {
	VD80_ANALOGUE_TRIG_DISABLED   = 0x0,
	VD80_ANALOGUE_TRIG_BELOW      = 0x3,
	VD80_ANALOGUE_TRIG_ABOVE      = 0x4,
} vd80_analogue_trig_t;

/* ========================================================================================= */

struct vd80_analogue_trig_s {
	uint32_t             channel; /* Channel to define the trigger on */
	vd80_analogue_trig_t control; /* Control options for trigger */
	int16_t              level;   /* Trigger level */
};

/* ========================================================================================= */

struct vd80_trig_config_s {
	uint32_t trig_delay;   /* Time to wait after trig in sample intervals */
	uint32_t min_pre_trig; /* Mininimum number of pretrig samples */
};

/**
 * =========================================================================================
 * IOCTL definitions
 */

typedef enum {

	vd80SET_SW_DEBUG,            /** Set driver debug mode */
	vd80GET_SW_DEBUG,            /** Get driver debug mode */
	vd80GET_VERSION,             /** Get version date */
	vd80SET_TIMEOUT,             /** Set the read timeout value */
	vd80GET_TIMEOUT,             /** Get the read timeout value */
	vd80RESET,                   /** Reset the module re-establish connections */
	vd80CONNECT,                 /** Connect to and object interrupt */
	vd80GET_CONNECT,             /** Get connections */

	vd80GET_MODULE_COUNT,        /** Get the number of installed modules */
	vd80GET_MODULE_ADDRESS,      /** Get the VME module base address */

	vd80SET_CLOCK,               /** Set to a Vd80 clock (Clock) */
	vd80SET_CLOCK_DIVIDE_MODE,   /** Sub sample or clock (DivideMode) */
	vd80SET_CLOCK_DIVISOR,       /** A 16 bit integer so the lowest frequency is */
	vd80SET_CLOCK_EDGE,          /** Set to a Vd80 edge (Edge) */
	vd80SET_CLOCK_TERMINATION,   /** Set to a Vd80 termination (Termination) */
	vd80GET_CLOCK,               /** (Clock) */
	vd80GET_CLOCK_DIVIDE_MODE,   /** (DivideMode) */
	vd80GET_CLOCK_DIVISOR,       /** Diviser -1 -> 0 = 20KHz */
	vd80GET_CLOCK_EDGE,          /** (Edge) */
	vd80GET_CLOCK_TERMINATION,   /** (Termination) */

	vd80SET_TRIGGER,             /** (Trigger) */
	vd80SET_TRIGGER_EDGE,        /** (Edge) */
	vd80SET_TRIGGER_TERMINATION, /** (Termination) */
	vd80GET_TRIGGER,             /** (Trigger) */
	vd80GET_TRIGGER_EDGE,        /** (Edge) */
	vd80GET_TRIGGER_TERMINATION, /** (Termination) */
	vd80SET_ANALOGUE_TRIGGER,    /** (AnalogTrig) */
	vd80GET_ANALOGUE_TRIGGER,    /** (AnalogTrig) */

	vd80GET_STATE,               /** Returns a Vd80 state (State) */
	vd80SET_COMMAND,             /** Set to a Vd80 command (Command) */
	vd80READ_ADC,                /** 16Bit ADC vaule is signed and extended to int32_t */
	vd80READ_SAMPLE,             /** Get acquired sample buffer */
	vd80GET_POSTSAMPLES,         /** Get the number of post samples you set */
	vd80SET_POSTSAMPLES,         /** Set the number of post samples you want */
	vd80GET_TRIGGER_CONFIG,      /** Get Trig delay and min pre trig samples */
	vd80SET_TRIGGER_CONFIG,      /** Set Trig delay and min pre trig samples */
	vd80RAW_READ,                /** Raw direct read from module address space */
	vd80RAW_WRITE,               /** Ditto for direct write */

	vd80LAST                     /** For range checking (LAST - FIRST) */

} vd80_ioctl_function_t;

/*
 * =========================================================================================
 * Set up the IOCTL numbers
 */

#define MAGIC 'V'

#define VIO(nr)      _IO(MAGIC,nr)
#define VIOR(nr,sz)  _IOR(MAGIC,nr,sz)
#define VIOW(nr,sz)  _IOW(MAGIC,nr,sz)
#define VIOWR(nr,sz) _IOWR(MAGIC,nr,sz)

/** TODO: Put in the correct structures when its not uint23_t */

#define VD80_SET_SW_DEBUG            VIOWR(vd80SET_SW_DEBUG            ,uint32_t)                    /** Set driver debug mode */
#define VD80_GET_SW_DEBUG            VIOWR(vd80GET_SW_DEBUG            ,uint32_t)                    /** Get driver debug mode */
#define VD80_GET_VERSION             VIOWR(vd80GET_VERSION             ,struct vd80_version_s)       /** Get version date */
#define VD80_SET_TIMEOUT             VIOWR(vd80SET_TIMEOUT             ,uint32_t)                    /** Set the read timeout value */
#define VD80_GET_TIMEOUT             VIOWR(vd80GET_TIMEOUT             ,uint32_t)                    /** Get the read timeout value */
#define VD80_RESET                   VIOWR(vd80RESET                   ,uint32_t)                    /** Reset the module re-establish connections */
#define VD80_CONNECT                 VIOWR(vd80CONNECT                 ,uint32_t)                    /** Connect to interrupt */
#define VD80_GET_CONNECT             VIOWR(vd80GET_CONNECT             ,uint32_t)                    /** Get connections */

#define VD80_GET_MODULE_COUNT        VIOWR(vd80GET_MODULE_COUNT        ,uint32_t)                    /** Get the number of installed modules */
#define VD80_GET_MODULE_ADDRESS      VIOWR(vd80GET_MODULE_ADDRESS      ,struct vd80_mod_addr_s)      /** Get the VME module base address */

#define VD80_SET_CLOCK               VIOWR(vd80SET_CLOCK               ,uint32_t)                    /** Set to a Vd80 clock (Clock) */
#define VD80_SET_CLOCK_DIVIDE_MODE   VIOWR(vd80SET_CLOCK_DIVIDE_MODE   ,uint32_t)                    /** Sub sample or clock (DivideMode) */
#define VD80_SET_CLOCK_DIVISOR       VIOWR(vd80SET_CLOCK_DIVISOR       ,uint32_t)                    /** A 16 bit integer so the lowest frequency is */
#define VD80_SET_CLOCK_EDGE          VIOWR(vd80SET_CLOCK_EDGE          ,uint32_t)                    /** Set to a Vd80 edge (Edge) */
#define VD80_SET_CLOCK_TERMINATION   VIOWR(vd80SET_CLOCK_TERMINATION   ,uint32_t)                    /** Set to a Vd80 termination (Termination) */
#define VD80_GET_CLOCK               VIOWR(vd80GET_CLOCK               ,uint32_t)                    /** (Clock) */
#define VD80_GET_CLOCK_DIVIDE_MODE   VIOWR(vd80GET_CLOCK_DIVIDE_MODE   ,uint32_t)                    /** (DivideMode) */
#define VD80_GET_CLOCK_DIVISOR       VIOWR(vd80GET_CLOCK_DIVISOR       ,uint32_t)                    /** Diviser -1 -> 0 = 20KHz */
#define VD80_GET_CLOCK_EDGE          VIOWR(vd80GET_CLOCK_EDGE          ,uint32_t)                    /** (Edge) */
#define VD80_GET_CLOCK_TERMINATION   VIOWR(vd80GET_CLOCK_TERMINATION   ,uint32_t)                    /** (Termination) */

#define VD80_SET_TRIGGER             VIOWR(vd80SET_TRIGGER             ,uint32_t)                    /** (Trigger) */
#define VD80_SET_TRIGGER_EDGE        VIOWR(vd80SET_TRIGGER_EDGE        ,uint32_t)                    /** (Edge) */
#define VD80_SET_TRIGGER_TERMINATION VIOWR(vd80SET_TRIGGER_TERMINATION ,uint32_t)                    /** (Termination) */
#define VD80_GET_TRIGGER             VIOWR(vd80GET_TRIGGER             ,uint32_t)                    /** (Trigger) */
#define VD80_GET_TRIGGER_EDGE        VIOWR(vd80GET_TRIGGER_EDGE        ,uint32_t)                    /** (Edge) */
#define VD80_GET_TRIGGER_TERMINATION VIOWR(vd80GET_TRIGGER_TERMINATION ,uint32_t)                    /** (Termination) */
#define VD80_SET_ANALOGUE_TRIGGER    VIOWR(vd80SET_ANALOGUE_TRIGGER    ,struct vd80_analogue_trig_s) /** (AnalogTrig) */
#define VD80_GET_ANALOGUE_TRIGGER    VIOWR(vd80GET_ANALOGUE_TRIGGER    ,struct vd80_analogue_trig_s) /** (AnalogTrig) */

#define VD80_GET_STATE               VIOWR(vd80GET_STATE               ,uint32_t)                    /** Returns a Vd80 state (State) */
#define VD80_SET_COMMAND             VIOWR(vd80SET_COMMAND             ,uint32_t)                    /** Set to a Vd80 command (Command) */
#define VD80_READ_ADC                VIOWR(vd80READ_ADC                ,int32_t)                     /** 16Bit ADC vaule is signed and extended to int32_t */
#define VD80_READ_SAMPLE             VIOWR(vd80READ_SAMPLE             ,struct vd80_sample_buf_s)    /** Get acquired sample buffer */
#define VD80_GET_POSTSAMPLES         VIOWR(vd80GET_POSTSAMPLES         ,uint32_t)                    /** Get the number of post samples you set */
#define VD80_SET_POSTSAMPLES         VIOWR(vd80SET_POSTSAMPLES         ,uint32_t)                    /** Set the number of post samples you want */
#define VD80_GET_TRIGGER_CONFIG      VIOWR(vd80GET_TRIGGER_CONFIG      ,struct vd80_trig_config_s)   /** Get Trig delay and min pre trig samples */
#define VD80_SET_TRIGGER_CONFIG      VIOWR(vd80SET_TRIGGER_CONFIG      ,struct vd80_trig_config_s)   /** Set Trig delay and min pre trig samples */
#define VD80_RAW_READ                VIOWR(vd80RAW_READ                ,struct vd80_riob_s)          /** Raw direct read from module address space */
#define VD80_RAW_WRITE               VIOWR(vd80RAW_WRITE               ,struct vd80_riob_s)          /** Ditto for direct write */

#endif
