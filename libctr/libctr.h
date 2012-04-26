/**
 * Julian Lewis Fri-30th March 2012 BE/CO/HT
 * julian.lewis@cern.ch
 *
 * CTR timing library definitions.
 * This library is to be used exclusivlely by the new open CBCM timing library.
 * It is up to the implementers of the open CBCM timing library to provide any
 * abstraction layers and to hide this completely from user code.
 */

#ifndef LIBCTR
#define LIBCTR

#include <sys/time.h>
#include <ctrdrvr.h>
#include <ctrhard.h>

struct ctr_ccv_s {
	int enable;                         /* Enable = 1, Disable = 0 */
	CtrDrvrCounterStart start;          /* The counters start. */
	CtrDrvrCounterMode mode;            /* The counters operating mode. */
	CtrDrvrCounterClock clock;          /* Clock specification. */
	int pulse_width;                    /* Number of 40MHz ticks, 0 = as fast as possible. */
	int delay;                          /* 32 bit delay to load into counter. */
	CtrDrvrCounterMask counter_mask;    /* Output lemo connectors mask, invited signals */
	CtrDrvrPolarity polarity;           /* Polarity of output */
	int ctim;                           /* CTIM triggering event of this action */
	int payload;                        /* 16-Bit Payload */
	CtrDrvrTriggerCondition cmp_method; /* Payload compare method */
	int grnum;                          /* Telegram group number or zero */
	int grval;                          /* Telegram group value */
	int tgnum;                          /* Telegram number or zero */
};

typedef enum {
	CTR_CCV_ENABLE       = 0x0001,
	CTR_CCV_START        = 0x0002,
	CTR_CCV_MODE         = 0x0004,
	CTR_CCV_CLOCK        = 0x0008,
	CTR_CCV_PULSE_WIDTH  = 0x0010,
	CTR_CCV_DELAY        = 0x0020,
	CTR_CCV_COUNTER_MASK = 0x0040,
	CTR_CCV_POLARITY     = 0x0080,
	CTR_CCV_CTIM         = 0x0100,
	CTR_CCV_PAYLOAD      = 0x0200,
	CTR_CCV_CMP_METHOD   = 0x0400,
	CTR_CCV_GRNUM        = 0x0800,
	CTR_CCV_GRVAL        = 0x1000,
	CTR_CCV_TGNUM        = 0x2000,
} ctr_ccv_fields_t;

struct ctr_interrupt_s {
	CtrDrvrConnectionClass ctr_class; /** CTR interrupt class */
	int equip;                        /** LTIM id, hardware mask, CTIM id */
	int payload;                      /** 16-Bit payload of the event if CTIM */
	int modnum;                       /** Module number of interrupting device */
	CtrDrvrCTime onzero;              /** Time of end of action */
	CtrDrvrCTime trigger;             /** Trigger time of action */
	CtrDrvrCTime start;               /** Counter start time */
};

struct ctr_module_address_s {
	CtrDrvrDevice device_type;        /** Which kind of device PCI/VME */
	void *memory_map;                 /** Main FPGA address (VME A24/BAR2) */
	void *jtag_address;               /** JTAG IO address (VME D16/BAR0) */
	unsigned int specific[4];         /** If VME vec-level. If PCI pci_slot-module-vid-did */
};

/**
 * @brief Convert the CTR driver time to standard unix time
 * @param ctime points to the CtrDrvrTime value  to be converted
 * @param utime points to the unix timeval struct where conversion will be stored
 * @return Always returns zero
 */
int ctr_ctime_to_unix(CtrDrvrTime *ctime, struct timeval *utime);

/**
 * @brief Convert the standard unix time to CTR driver time
 * @param utime points to the unix timeval to be converted
 * @param ctime points to the CtrDrvrTime value where conversion will be stored
 * @return Always returns zero
 */
int ctr_unix_to_ctime(struct timeval *utime, CtrDrvrTime *ctime);

/**
 * As this library runs exclusivley on Linux I use standard kernel coding
 * style and error reporting where possible. It is available both as a shared
 * object and as a static link. It exports the ctrdrvr public definitions
 * which follow the old OSF-Motif coding style.
 * In all cases if the return value is -1, then errno contains the error number.
 * The errno variable is per thread and so this mechanism is thread safe.
 * A return of zero or a posative value means success.
 * Error numbers are defined in errno.h and there are standard Linux
 * facilities for treating them. See err(3), error(3), perror(3), strerror(3)
 */

#define CTR_ERROR (-1)

/**
 * @brief Get a handle to be used in subsequent library calls
 * @param Version string or NULL for the latest
 * @return The handle to be used in subsequent calls or -1
 *
 * The ctr_open call returns a pointer to an opeaque structure
 * defined within the library internal implementation. Clients
 * never see what is behind the void pointer.
 *
 * If a version string is specified and shared objects are in use,
 * then the specified version will be loaded, else a NULL or empty
 * string points to the installed version. Version strings consits
 * of two integers seperated by a point eg "3.1" or "1.0" these
 * numbers are the major and minor version numbers.
 *
 * Implementation hint:
 * NEVER hard code the version number into the source!! Its part
 * of the environment, suggest CTR_LIB_VERSION environment variable.
 * If it's not defined, use the default NULL string.
 *
 * The returned handle is -1 on error otherwise its a valid handle.
 * On error use the standard Linux error functions for details.
 *
 * Each time ctr_open is called a new handle is allocated, due to the
 * current ctr driver implementation there can never be more than
 * 16 open handles at any one time (this limitation should be removed).
 *
 *  void *my_handle;
 *  my_handle = ctr_open(NULL);
 *  if ((int) my_handle == CTR_ERROR)
 *          perror("ctr_open error");
 *
 */
void *ctr_open(char *version);

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
 * @brief Set the current working module number
 * @param A handle that was allocated in open
 * @param modnum module number 1..n  (n = module_count)
 * @return Zero means success else -1 is returned on error, see errno
 *
 * A client owns the handle he opened and should use it exclusivley
 * never giving it to another thread. In this case it is thread safe
 * to call set_module for your handle. All subsequent calls will work
 * using the set module number.
 */
int ctr_set_module(void *handle, int modnum);

/**
 * @brief Get the current working module number
 * @param A handle that was allocated in open
 * @return module number 1..n or -1 on error
 */
int ctr_get_module(void *handle);

/**
 * @brief Get the device type handled by the driver CTRV, CTRP, CTRI, CTRE
 * @param A handle that was allocated in open
 * @param Pointer to where the device type will be stored
 * @return Zero means success else -1 is returned on error, see errno
 *
 * Different device types implement different features.
 * In any case where the device type is important, say setting the P2 byte, then
 * the routine will check and return an error if its not supported.
 */
int ctr_get_type(void *handle, CtrDrvrHardwareType *type);

/**
 * @brief Get the addresses of a module
 * @param A handle that was allocated in open
 * @param Pointer to where the module address will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_module_address(void *handle, struct ctr_module_address_s *module_address);

/**
 * @brief Connect to a ctr interrupt
 * @param A handle that was allocated in open
 * @param ctr_class see CtrDrvrConnectionClass, the class of timing to connect
 * @param equip is class specific: hardware mask, ctim number or ltim number
 * @return Zero means success else -1 is returned on error, see errno
 *
 * In the case of connecting to a ctim event you create the ctim first and 
 * pass this id in parameter equip. To connect to an LTIM you must use the
 * module number on which the LTIM exists.
 *
 *  Connect to the PPS hardware event on module 2
 *
 *  CtrDrvrConnectionClass ctr_class = CtrDrvrConnectionClassHARD;
 *  CtrDrvrInterruptMask hmask       = CtrDrvrInterruptMaskPPS;
 *  int modnum                       = 2;
 *
 *  if (ctr_set_module(handle,modnum) < 0) ...
 *  if (ctr_connect(handle,ctr_class,(int) hmask) < 0) ...
 */
int ctr_connect(void *handle, CtrDrvrConnectionClass ctr_class, int equip);

/**
 * @brief Connect to a ctr interrupt with a given payload
 * @param A handle that was allocated in open
 * @param ctim you want to connect to.
 * @param payload that must match the CTIM event (equality)
 * @return Zero means success else -1 is returned on error, see errno
 *
 * In the case of connecting to a ctim event you create the ctim first and 
 * pass this id in parameter here
 *
 *  Connect to the millisecond CTIM at C100 on module 1
 *
 *  int ctim    = 911; # (0x0100FFFF) Millisecond C-Event with wildcard
 *  int payload = 100; # C-time to be woken up at i.e. C100
 *  int modnum  = 1;   # Module 1
 *
 *  if (ctr_set_module(handle,modnum) < 0) ...
 *  if (ctr_connect_payload(handle,ctim,payload) < 0) ...
 */
int ctr_connect_payload(void *handle, int ctim, int payload);

/**
 * @brief Disconnect from an interrupt on current module
 * @param A handle that was allocated in open
 * @param ctr_class the calss of timing to disconnect
 * @param mask the class specific, hardware mask, ctim or ltim number
 * @return Zero means success else -1 is returned on error, see errno
 *
 * The client code must remember what it is connected to in order to disconnect.
 */
int ctr_disconnect(void *handle, CtrDrvrConnectionClass ctr_class, int mask);

/**
 * @brief Wait for an interrupt
 * @param A handle that was allocated in open
 * @param Pointer to an interrupt structure
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_wait(void *handle, struct ctr_interrupt_s *ctr_interrupt);

/**
 * @brief Set a CCV
 * @param A handle that was allocated in open
 * @param ltim number to be set
 * @param index into ptim action array 0..size-1
 * @param ctr_ccv are the values to be set
 * @param ctr_ccv_fields to be set from ctr_ccv
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_set_ccv(void *handle, int ltim, int index, struct ctr_ccv_s *ctr_ccv, ctr_ccv_fields_t ctr_ccv_fields);

/**
 * @brief get an ltim action setting
 * @param A handle that was allocated in open
 * @param ltim number to get
 * @param index into ltim action array 0..size-1
 * @param ctr_ccv points to where the values will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_ccv(void *handle, int ltim, int index, struct ctr_ccv_s *ctr_ccv);

/**
 * @brief Create an empty LTIM object on the current module
 * @param A handle that was allocated in open
 * @param ltim number to create
 * @param channel number for ltim
 * @param size of ltim action array (PLS lines)
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_create_ltim(void *handle, int ltim, int ch, int size);

/**
 * @brief get a telegram
 * @param index into the array of telegrams 0..7
 * @param telegram point to a short array of at least size 32
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_telegram(void *handle, int index, short *telegram);

/**
 * @brief Get time
 * @param A handle that was allocated in open
 * @param ctr_time point to where time will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_time(void *handle, CtrDrvrCTime *ctr_time);

/**
 * @brief Set the time on the current module
 * @param A handle that was allocated in open
 * @param ctr_time the time to be set
 * @return Zero means success else -1 is returned on error, see errno
 *
 * Note this time will be overwritten within 1 second if the
 * current module is enabled and connected to the timing network.
 */
int ctr_set_time(void *handle, CtrDrvrTime ctr_time);

/**
 * @brief Get cable ID
 * @param A handle that was allocated in open
 * @param cable_id points to where id will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_cable_id(void *handle, int *cable_id);

/**
 * @brief Set the cable ID of a module
 * @param A handle that was allocated in open
 * @param The cable ID to set
 * @return Zero means success else -1 is returned on error, see errno
 *
 * Note this cable ID will be overwritten within 1 second if the
 * current module is enabled and connected to the timing network.
 */
int ctr_set_cable_id(void *handle, int cable_id);

/**
 * @brief Get driver and firmware version
 * @param A handle that was allocated in open
 * @param version points to where version will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_version(void *handle, CtrDrvrVersion *version);

/**
 * @brief Associate a CTIM number to a Frame
 * @param A handle that was allocated in open
 * @param ctim event Id to create
 * @param mask event frame, like 0x2438FFFF (if there is a payload, set FFFF at the end)
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_create_ctim(void *handle, int ctim, int mask);

/**
 * @brief Destroy a CTIM
 * @param A handle that was allocated in open
 * @param ctim event Id to destroy
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_destroy_ctim(void *handle, int ctim);

/**
 * @brief Get the size of your queue for a given handle
 * @param A handle that was allocated in open
 * @return Queue size or -1 on error
 */
int ctr_get_queue_size(void *handle);

/**
 * @brief Turn your queue on or off
 * @param A handle that was allocated in open
 * @param flag 1=>queuing is off, 0=>queuing is on
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_set_queue_flag(void *handle, int flag);

/**
 * @brief Get the current queue flag setting
 * @param A handle that was allocated in open
 * @return The queue flag 0..1 (QOFF..QON)  else -1 on error
 */
int ctr_get_queue_flag(void *handle);

/**
 * @brief Enable/Disable timing reception on current module
 * @param A handle that was allocated in open
 * @param Enable flag (1=enabled 0=disabled)
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_set_enable(void *handle, int flag);

/**
 * @brief Get the Enable/Disable flag value
 * @param A handle that was allocated in open
 * @return The enable/Disable flag value or -1 on error
 */
int ctr_get_enable(void *handle);

/**
 * @brief Set the CTR timing input delay value
 * @param A handle that was allocated in open
 * @param The new delay value in 40MHz (25ns) Ticks (24-Bit)
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_set_input_delay(void *handle, int delay);

/**
 * @brief Get the CTR timing input delay value
 * @param A handle that was allocated in open
 * @return The input delay value in 40MHz ticks value or -1 on error
 */
int ctr_get_input_delay(void *handle);

/**
 * @brief Set the CTR driver debug print out level
 * @param A handle that was allocated in open
 * @param The level to be set 0=None ..7 Up to level 7
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_set_debug_level(void *handle, int level);

/**
 * @brief Get the CTR driver debug print out level
 * @param A handle that was allocated in open
 * @return The debug level 0..7 (0=Off) else -1 for error
 */
int ctr_get_debug_level(void *handle);

/**
 * @brief Set your timeout in milliseconds
 * @param A handle that was allocated in open
 * @param The timeout im milliseconds, zero means no timeout
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_set_timeout(void *handle, int timeout);

/**
 * @brief Get your timeout in milliseconds
 * @param A handle that was allocated in open
 * @return The timeout in millisecond else -1 for error
 */
int ctr_get_timeout(void *handle);

/**
 * @brief Get the CTR module status
 * @param A handle that was allocated in open
 * @param Pointer to where the status will be stored of type CtrDrvrStatus
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_status(void *handle, CtrDrvrStatus *stat);

/**
 * @brief Set up a counter remotley from user code rather than from a CTIM
 * @param A handle that was allocated in open
 * @param remote flag 0=normal, 1=remote control by user
 * @param ch is the channel 1..3 for ctri, 1..4 for ctrp, 1..8 for ctrv.
 * @param rcmd is the command see CtrDrvrRemote
 * @param ctr_ccv are the values to be set
 * @param ctr_ccv_fields to be set from ctr_ccv
 * @return Zero means success else -1 is returned on error, see errno
 *
 * Set a counter under full remote control (IE under DSC tasks control)
 * This feature permits you to do what you like with counters even if
 * there is no timing cable attached. With this you can drive stepper
 * motors, wire scanners or whatever. No PTIM or CTIM is involved, the
 * configuration is loaded directly by the application. Note that when
 * the argument remflg is set to 1, the counter can not be written to
 * by incomming triggers so all PTIM objects using the counter stop
 * overwriting the counter configuration and are effectivley disabled.
 * Setting the remflg 0 permits PTIM triggers to write to the counter
 * configuration, the write block is removed. Also note that in some
 * cases it is useful to perform remote actions, such as remoteSTOP,
 * even if the remflg is set to zero. The remflg simply blocks PTIM
 * overwrites, the counter configuration can still be accessed !
 */
int ctr_set_remote(void *handle,
		   int remote_flag,
		   CtrDrvrCounter ch,
		   CtrDrvrRemote rcmd,
		   struct ctr_ccv_s *ctr_ccv,
		   ctr_ccv_fields_t ctr_ccv_fields);

/**
 * @brief Get the remote counter flag and config
 * @param A handle that was allocated in open
 * @param ch is the channel 1..3 for ctri, 1..4 for ctrp, 1..8 for ctrv.
 * @param ctr_ccv are the values of the counter
 * @return The remote flag 0=normal, 1=remote or -1 on error
 */
int ctr_get_remote(void *handle, CtrDrvrCounter ch, struct ctr_ccv_s *ctr_ccv);

/**
 * @brief Choose PLL locking method, brutal or slow
 * @param A handle that was allocated in open
 * @param The lock flag 0=Brutal 1= slow
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_set_pll_lock_method(void *handle, int lock_method);

/**
 * @brief Get Pll locking method
 * @param A handle that was allocated in open
 * @return The lock flag (0=Brutal 1=Slow) or -1 on error
 */
int ctr_get_pll_lock_method(void *handle);

/**
 * @brief Read the io status
 * @param A handle that was allocated in open
 * @param Pointer to where the iostatus will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_io_status(void *handle, CtrDrvrIoStatus *io_stat);

/**
 * @brief Get module statistics
 * @param A handle that was allocated in open
 * @param Pointer to where the statistics will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_stats(void *handle, CtrDrvrModuleStats *stats);

/**
 * @brief Perform a memory test
 * @param A handle that was allocated in open
 * @param points to where a bad address will be stored
 * @param points to the data written
 * @param points to the data read back
 * @return Zero success (no mem error) else -1 errno is set 0 (mem error)
 *
 * The Module must have been disabled for this test to run
 * This routine will return -1 with errno set zero if there is a memory error
 * in this case the address where the error happened, the write and read data
 * are available to see what went wrong.
 */
int ctr_memory_test(void *handle, int *address, int *wpat, int *rpat);

/**
 * @brief Get the list of all driver clients
 * @param A handle that was allocated in open
 * @param Pointer to the client list
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_client_pids(void *handle, CtrDrvrClientList *client_pids);

/**
 * @brief Get a clients connections
 * @param A handle that was allocated in open
 * @param Pid of the client whose connections you want
 * @param Pointer to where clients connections will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_client_connections(void *handle, int pid, CtrDrvrClientConnections *connections);

/**
 * @brief simulate an interrupt
 * @param A handle that was allocated in open
 * @param Class of interrupt to simulate
 * @param Class value
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_simulate_interrupt(void *handle, CtrDrvrConnectionClass ctr_class, int equip);

/**
 * @brief Select the P2 output byte number for current module
 * @param A handle that was allocated in open
 * @param The output byte number or zero
 * @return Zero means success else -1 is returned on error, see errno
 *
 * OutputByte: In the VME version of the CTR, the eight counter outputs
 * can be placed on one byte of the P2 connector. If this value is zero
 * the CTR does not drive the P2 connector, a value between 1..8 selects
 * the byte number in the 64bit P2 VME bus.
 */
int ctr_set_p2_output_byte(void *handle, int p2byte);

/**
 * @brief Get the P2 output byte number
 * @param A handle that was allocated in open
 * @return The output byte number or -1 on error
 *
 * If a value of 0 is returned, no output byte is set
 */
int ctr_get_p2_output_byte(void *handle);

#endif
