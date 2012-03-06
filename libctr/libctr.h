/**
 * Proposal for a ctr module library.
 * Julian Lewis Mon 5th March 2012 BE/CO/HT
 */

#include <sys/time.h>

/**
 * General remarks:
 * As this library runs exclusivley on Linux I use standard kernel coding
 * style and error reporting. It is available both as a shared object and
 * as a static link - which should be discouraged unless there is some strong
 * reason why so libraries can't be used.
 *
 * This include refers to no other timing includes or binaries and makes no use
 * of any other timing facilities such as tgm and tgv.
 * As a result of not using tgv in particular event frames are used directly
 * rather than CTIM numbers.
 *
 * Logical unit numbers are unique values asociated with a given module instance.
 * A logical unit number can have any integer value, eg -49,731,0,-64 ....
 * In our control system they are defined in Oracle in the crate description.
 * Module numbers run from 1 to the number of installed modules, each installed
 * module has a corresponding logical unit number (lun). Removing a module from
 * a crate changes the numbers of sucsessive modules but not their luns. Luns bind
 * to either the vme addresses or to PCI bus, slot via /etc/crateconfig.
 * The CTR driver works only with luns, however until TimLib is suppressed we must
 * support the lun and module number.
 *
 * This library uses the Linux standard error codes and the errno variable.
 * In all cases if the return value is -1, then errno contains the error number.
 * The errno variable is per thread and so this mechanism is thread safe.
 * A return of zero or a posative value means success.
 * Error numbers are defined in errno.h and there are standard Linux
 * facilities for treating them. See err(3), error(3), perror(3), strerror(3)
 * Correspondance between current timlib error codes and standard errors is
 * doccumented where needed.
 */

#define CTR_ERROR (-1)

/**
 * @brief Get a handle to be used in subsequent library calls
 * @return The handle to be used in subsequent calls or -1
 *
 * The ctr_open call returns a pointer to an opeaque structure
 * defined within the library internal implementation. Clients
 * never see what is in the void pointer.
 *
 * The returned handle is -1 on error otherwise its a valid handle.
 * On error use the standard Linux error functions for details.
 *
 * Each time ctr_open is called a new handle is allocated, there is
 * no fixed limit to how many open handles can be allocated, however
 * memory gets allocated for each call.
 *
 *  void *my_handle;
 *  my_handle = ctr_open();
 *  if ((int) my_handle == CTR_ERROR)
 *          perror("ctr_open error");
 *
 */

void *ctr_open();

/**
 * @brief Close a handle and free up resources
 * @param A handle that was allocated in open
 * @return Zero means success else -1 is returned on error, see errno
 *
 * This routine disconnects from all interrupts, frees up memory and
 * closes the ctr driver. It should be called once for each ctr_open.
 */

int ctr_close(void *handle);

/**
 * @brief Get the number of installed CTR modules
 * @param A handle that was allocated in open
 * @return The installed module count or -1 on error
 */

int ctr_get_module_count(void *handle);

/**
 * @brief Get the corresponding logical unit number for a given module number
 * @param modnum is the module number 1..module_count
 * @param pointer to integer where the lun value will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */

int ctr_get_logical_unit(int modnum, int *lun);

/**
 * @brief Get the corresponding module number for a given logical unit number
 * @param lun of the module whose modnum you want
 * @param modnum pointer to integer where the module number will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */

int ctr_get_module_number(int lun, int *modnum);

/**
 * @brief Connect to a ctr interrupt
 * @param A handle that was allocated in open
 * @param ctr_class see CTR_CLASS, the class of timing to connect
 * @param mask see CTR_MASK, the class specific, hardware mask, event frame or ptim number
 * @param lun is the logical unit number of the module.
 * @return Zero means success else -1 is returned on error, see errno
 *
 * In the case of connecting to a ctim event you must specify a payload in the
 * masks lsw with the ctim number in the msw. The payload wild card is 0xFFFF.
 *
 *  Connect to all CPS.RPLS events
 *
 *  int mask = 0x34100FFFF;
 *  int lun;
 *
 *  if (ctr_get_logical_unit(1,&lun) == CTR_ERROR) {
 *          perror(ctr_get_logical_unit);
 *          exit 1;
 *  }
 *  if (ctr_connect(handle,CTR_CLASS_EVENT_FRAME,mask,lun) == CTR_ERROR ...
 */

#define CTR_CLASS_HARDWARE 0
#define CTR_CLASS_EVENT_FRAME 1
#define CTR_CLASS_LTIM 2

#define CTR_MASK_CTIM          = 0x01      /* Any connected CTIM interrupt */
#define CTR_MASK_COUNTER_1     = 0x02      /* Any counter 1 output */
#define CTR_MASK_COUNTER_2     = 0x04      /* Any counter 2 output */
#define CTR_MASK_COUNTER_3     = 0x08      /* Any counter 3 output */
#define CTR_MASK_COUNTER_4     = 0x10      /* Any counter 4 output */
#define CTR_MASK_COUNTER_5     = 0x20      /* Any counter 5 output */
#define CTR_MASK_COUNTER_6     = 0x40      /* Any counter 6 output */
#define CTR_MASK_COUNTER_7     = 0x80      /* Any counter 7 output */
#define CTR_MASK_COUNTER_8     = 0x100     /* Any counter 8 output */
#define CTR_MASK_PLL_ITERATION = 0x200     /* Phase locked loop iteration */
#define CTR_MASK_GMT_EVENT_IN  = 0x400     /* Any incomming timing frame */
#define CTR_MASK_1HZ           = 0x800     /* Tne one second UTC time pulse */
#define CTR_MASK_1KHZ          = 0x1000    /* One Kilo Hertz */
#define CTR_MASK_MATCHED       = 0x2000    /* Incomming event trigger matched */

int ctr_connect(void *handle, int ctr_class, int mask, int lun);

/**
 * @brief Disconnect from an interrupt
 * @param A handle that was allocated in open
 * @param ctr_class see CTR_CLASS, the calss of timing to connect
 * @param mask see CTR_MASK, the class specific, hardware mask, event_frame or ptim number
 * @param lun is a valid logical unit number.
 * @return Zero means success else -1 is returned on error, see errno
 *
 * The client code must remember what it is connected to in order to disconnect.
 */

int ctr_disconnect(void *handle, int ctr_class, int mask, int lun);

/**
 * @brief Wait for an interrupt
 * @param A handle that was allocated in open
 * @param Pointer to an interrupt structure
 * @return Zero means success else -1 is returned on error, see errno
 */

struct ctr_time_s {
	struct timeval time;    /** Standard Linux time value */
	int ctrain;             /** Corresponding ctrain value */
};

struct ctr_interrupt_s {
	int ctr_class;          /** CTR interrupt class */
	int mask;               /** Corresponding mask */
	int lun;                /** Lun of interrupting device */
	ctr_time_s end;         /** Time of end of action */
	ctr_time_s trigger;     /** Trigger time of action */
	ctr_time_s start;       /** Counter start time */
};

int ctr_wait(void *handle, struct ctr_interrupt_s *ctr_interrupt);

/**
 * @brief Set a CCV
 * @param A handle that was allocated in open
 * @param lun is the logical unit number
 * @param ptim number to be set
 * @param index into ptim action array 0..size-1
 * @param ctr_ccv are the values to be set
 * @param ctr_ccv_fields to be set from ctr_ccv
 * @return Zero means success else -1 is returned on error, see errno
 */

typedef struct {
	int enable;      /* Enable = 1, Disable = 0 */
	int start;       /* The counters start. */
	int mode;        /* The counters operating mode. */
	int clock;       /* Clock specification. */
	int pulse_width; /* Number of 40MHz ticks, 0 = as fast as possible. */
	int delay;       /* 32 bit delay to load into counter. */
	int output_mask; /* Output lemo connectors mask */
	int polarity;    /* Polarity of output */
	int event_frame; /* Triggers counter load */
	int cmp_method   /* Payload compare method 0=Equality, 1=And */
	int counter;     /* Counter number 1..8 (0 is a special hardware counter) */
} ctr_ccv_s;

typedef enum {
	CTR_CCV_ENABLE      = 0x001,
	CTR_CCV_START       = 0x002,
	CTR_CCV_CLOCK       = 0x004,
	CTR_CCV_PULSE_WIDTH = 0x008,
	CTR_CCV_DELAY       = 0x010,
	CTR_CCV_OUTPUT_MASK = 0x020,
	CTR_CCV_POLARITY    = 0x040,
	CTR_CCV_EVENT_FRAME = 0x080,
	CTR_CCV_CMP_METHOD  = 0x100,
	CTR_CCV_COUNTER     = 0x200
} ctr_ccv_fields_t;

int ctr_set_ccv(void *handle, int lun, int ptim, int index, struct ctr_ccv_s *ctr_ccv, ctr_ccv_fields_t ctr_ccv_fields);

/**
 * @brief get a ptim action setting
 * @param A handle that was allocated in open
 * @param lun is the logical unit number
 * @param ptim number to get
 * @param index into ptim action array 0..size-1
 * @param ctr_ccv points to where the values will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */

int ctr_get_ccv(void *handle, int lun, int ptim, int index, struct ctr_ccv_s *ctr_ccv);

/**
 * @brief Create an empty PTIM object
 * @param A handle that was allocated in open
 * @param lun is the logical unit number
 * @param ptim number to create
 * @param size of ptim action array
 * @return Zero means success else -1 is returned on error, see errno
 */

int ctr_create_ptim(void *handle, int lun, int ptim, int size);

/**
 * @brief get a telegram
 * @param lun is the logical unit number
 * @param index into the array of telegrams 0..7
 * @param telegram point to a short array of at least size 32
 * @return Zero means success else -1 is returned on error, see errno
 */

int ctr_get_telegram(void *handle, int lun, int index, short *telegram);

/**
 * @brief Get time
 * @param A handle that was allocated in open
 * @param lun is the logical unit number
 * @param ctr_time point to where time will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */

int ctr_get_time(void *handle, int lun, ctr_time_s *ctr_time);

/**
 * @brief Get cable ID
 * @param A handle that was allocated in open
 * @param lun is the logical unit number
 * @param cid points to where id will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */

int ctr_get_cid(void *handle, int lun, int *cid);

/**
 * @brief Get firmware version
 * @param A handle that was allocated in open
 * @param lun is the logical unit number
 * @param version points to where version will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */

int ctr_get_version(void *handle, int lun, int *version);
