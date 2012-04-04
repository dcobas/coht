/**
 * Julian Lewis Fri-30th March 2012 BE/CO/HT
 * julian.lewis@cern.ch
 *
 * CTR timing common code used between CTRV and CTRP/I
 * This library is to be used exclusivlely by the new open CBCM timing library.
 * It is up to the implementers of the open CBCM timing library to provide any
 * abstraction layers and to hide this completely from user code.
 */

#ifndef LIBCTR_COMMON
#define LIBCTR_COMMON

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <errno.h>        /* Error numbers */
#include <sys/file.h>
#include <a.out.h>
#include <ctype.h>

#include <libctr.h>
#include <libctrP.h>

/**
 * @brief Get the number of installed CTR modules
 * @param A handle that was allocated in open
 * @return The installed module count or -1 on error
 */
int ctr_get_module_count(void *handle)
{
	struct ctr_handle_s *h = handle;
	unsigned long arg = 0;

	if (ioctl(h->fd,CtrIoctlGET_MODULE_COUNT,&arg) < 0)
		return -1;
	return (int) arg;
}

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
int ctr_set_module(void *handle, int modnum)
{
	struct ctr_handle_s *h = handle;
	unsigned long arg = modnum;

	if (ioctl(h->fd,CtrIoctlSET_MODULE,&arg) < 0)
		return -1;
	return 0;
}

/**
 * @brief Get the current working module number
 * @param A handle that was allocated in open
 * @return module number 1..n or -1 on error
 */
int ctr_get_module(void *handle)
{
	struct ctr_handle_s *h = handle;
	unsigned long arg = 0;

	if (ioctl(h->fd,CtrIoctlGET_MODULE,&arg) < 0)
		return -1;
	return arg;
}

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
int ctr_get_type(void *handle, CtrDrvrHardwareType *type)
{
	struct ctr_handle_s *h = handle;
	CtrDrvrVersion version;

	if (ioctl(h->fd,CtrIoctlGET_VERSION,&version) < 0)
		return -1;
	*type = version.HardwareType;
	return 0;
}

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
int ctr_connect(void *handle, CtrDrvrConnectionClass ctr_class, int equip)
{
	struct ctr_handle_s *h = handle;
	CtrDrvrConnection con;
	int mod;

	mod = ctr_get_module(handle);
	if (mod < 0)
		return -1;

	con.Module = mod;
	con.EqpNum = equip;
	con.EqpClass = ctr_class;
	if (ioctl(h->fd,CtrIoctlCONNECT,&con) < 0)
		return -1;
	return 0;
}

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
int ctr_connect_payload(void *handle, int ctim, int payload)
{
	struct ctr_handle_s *h = handle;
	CtrDrvrAction act;
	CtrDrvrCtimObjects ctimo;
	CtrDrvrTrigger *trg;
	int i ,j, mod;

	trg = &(act.Trigger);
	mod = ctr_get_module(handle);
	if (mod < 0)
		return -1;

	if (ioctl(h->fd,CtrIoctlLIST_CTIM_OBJECTS,&ctimo) < 0)
		return -1;

	for (i=0; i<ctimo.Size; i++) {
		if (ctimo.Objects[i].EqpNum == ctim) {
			if (ctr_connect(handle,CtrDrvrConnectionClassCTIM,ctim) < 0)
				return -1;

			for (j=1; j<=CtrDrvrRamTableSIZE; j++) {
				act.TriggerNumber = j;
				if (ioctl(h->fd,CtrIoctlGET_ACTION,&act) < 0)
					return -1;

				if (act.EqpClass == CtrDrvrConnectionClassCTIM) {
					if (trg->Ctim == ctim) {
						trg->Frame.Long = ((ctimo.Objects[i].Frame.Long & 0xFFFF0000)
								|  (payload & 0x0000FFFF));

						if (ioctl(h->fd,CtrIoctlSET_ACTION,&act) < 0)
							return -1;
						else
							return 0;
					}
				}
			}
		}
	}
	errno = ENODEV; /* No such device */
	return -1;
}

/**
 * @brief Disconnect from an interrupt on current module
 * @param A handle that was allocated in open
 * @param ctr_class the calss of timing to disconnect
 * @param mask the class specific, hardware mask, ctim or ltim number
 * @return Zero means success else -1 is returned on error, see errno
 *
 * The client code must remember what it is connected to in order to disconnect.
 */
int ctr_disconnect(void *handle, CtrDrvrConnectionClass ctr_class, int mask)
{
	struct ctr_handle_s *h = handle;
	CtrDrvrConnection con;
	int mod;

	mod = ctr_get_module(handle);
	if (mod < 0)
		return -1;

	con.Module = mod;
	con.EqpNum = mask;
	con.EqpClass = ctr_class;
	if (ioctl(h->fd,CtrIoctlDISCONNECT,&con) < 0)
		return -1;
	return 0;
}

/**
 * @brief Wait for an interrupt
 * @param A handle that was allocated in open
 * @param Pointer to an interrupt structure
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_wait(void *handle, struct ctr_interrupt_s *ctr_interrupt)
{
	struct ctr_handle_s *h = handle;
	CtrDrvrReadBuf rbf;

	if (read(h->fd,&rbf,sizeof(CtrDrvrReadBuf)) <= 0) {
		errno = ETIME; /* Timer expired */
		return -1;
	}

	ctr_interrupt->ctr_class = rbf.Connection.EqpClass;
	ctr_interrupt->equip     = rbf.Connection.EqpNum;
	ctr_interrupt->payload   = rbf.Frame.Long & 0xFFFF;
	ctr_interrupt->modnum    = rbf.Connection.Module;
	ctr_interrupt->onzero    = rbf.OnZeroTime;
	ctr_interrupt->trigger   = rbf.TriggerTime;
	ctr_interrupt->start     = rbf.OnZeroTime;

	return 0;
}

/**
 * @brief Set a CCV
 * @param A handle that was allocated in open
 * @param ltim number to be set
 * @param index into ptim action array 0..size-1
 * @param ctr_ccv are the values to be set
 * @param ctr_ccv_fields to be set from ctr_ccv
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_set_ccv(void *handle, int ltim, int index, struct ctr_ccv_s *ctr_ccv, ctr_ccv_fields_t ctr_ccv_fields)
{
	return -1;
}

/**
 * @brief get an ltim action setting
 * @param A handle that was allocated in open
 * @param ltim number to get
 * @param index into ltim action array 0..size-1
 * @param ctr_ccv points to where the values will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_ccv(void *handle, int ltim, int index, struct ctr_ccv_s *ctr_ccv)
{
	return -1;
}

/**
 * @brief Create an empty LTIM object on the current module
 * @param A handle that was allocated in open
 * @param ltim number to create
 * @param size of ltim action array (PLS lines)
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_create_ltim(void *handle, int ltim, int size)
{
	return -1;
}

/**
 * @brief get a telegram
 * @param index into the array of telegrams 0..7
 * @param telegram point to a short array of at least size 32
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_telegram(void *handle, int index, short *telegram)
{
	return -1;
}

/**
 * @brief Get time
 * @param A handle that was allocated in open
 * @param ctr_time point to where time will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_time(void *handle, CtrDrvrTime *ctr_time)
{
	return -1;
}

/**
 * @brief Set the time on the current module
 * @param A handle that was allocated in open
 * @param The time to set
 * @return Zero means success else -1 is returned on error, see errno
 *
 * Note this time will be overwritten within 1 second if the
 * current module is enabled and connected to the timing network.
 */
int ctr_set_time(void *handle, CtrDrvrTime *ctr_time)
{
	return -1;
}

/**
 * @brief Get cable ID
 * @param A handle that was allocated in open
 * @param cable_id points to where id will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_cable_id(void *handle, int *cable_id)
{
	return -1;
}

/**
 * @brief Set the cable ID of a module
 * @param A handle that was allocated in open
 * @param The cable ID to set
 * @return Zero means success else -1 is returned on error, see errno
 *
 * Note this cable ID will be overwritten within 1 second if the
 * current module is enabled and connected to the timing network.
 */
int ctr_set_cable_id(void *handle, int cable_id)
{
	return -1;
}

/**
 * @brief Get firmware version
 * @param A handle that was allocated in open
 * @param version points to where version will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_fw_version(void *handle, int *version)
{
	return -1;
}

/**
 * @brief Get driver version
 * @param A handle that was allocated in open
 * @param version points to where version will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_dvr_version(void *handle, int *version)
{
	return -1;
}

/**
 * @brief Associate a CTIM number to a Frame
 * @param A handle that was allocated in open
 * @param ctim event Id to create
 * @param mask event frame, like 0x2438FFFF (if there is a payload, set FFFF at the end)
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_create_ctim(void *handle, int ctim, int mask)
{
	return -1;
}

/**
 * @brief Destroy a CTIM
 * @param A handle that was allocated in open
 * @param ctim event Id to destroy
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_destroy_ctim(void *handle, int ctim)
{
	return -1;
}

/**
 * @brief Get the size of your queue for a given handle
 * @param A handle that was allocated in open
 * @return Queue size or -1 on error
 */
int ctr_get_queue_size(void *handle)
{
	return -1;
}

/**
 * @brief Turn your queue on or off
 * @param A handle that was allocated in open
 * @param flag 1=>queuing is off, 0=>queuing is on
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_set_queue_flag(void *handle, int flag)
{
	return -1;
}

/**
 * @brief Get the current queue flag setting
 * @param A handle that was allocated in open
 * @return The queue flag 0..1 (QOFF..QON)  else -1 on error
 */
int ctr_get_queue_flag(void *handle)
{
	return -1;
}

/**
 * @brief Enable/Disable timing reception on current module
 * @param A handle that was allocated in open
 * @param Enable flag (1=enabled 0=disabled)
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_set_enable(void *handle, int flag)
{
	return -1;
}

/**
 * @brief Get the Enable/Disable flag value
 * @param A handle that was allocated in open
 * @return The enable/Disable flag value or -1 on error
 */
int ctr_get_enable(void *handle)
{
	return -1;
}

/**
 * @brief Set the CTR timing input delay value
 * @param A handle that was allocated in open
 * @param The new delay value in 40MHz (25ns) Ticks (24-Bit)
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_set_input_delay(void *handle, int delay)
{
	return -1;
}

/**
 * @brief Get the CTR timing input delay value
 * @param A handle that was allocated in open
 * @return The input delay value in 40MHz ticks value or -1 on error
 */
int ctr_get_input_delay(void *handle)
{
	return -1;
}

/**
 * @brief Set the CTR driver debug print out level
 * @param A handle that was allocated in open
 * @param The level to be set 0=None ..7 Up to level 7
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_set_debug_level(void *handle, int level)
{
	return -1;
}

/**
 * @brief Get the CTR driver debug print out level
 * @param A handle that was allocated in open
 * @return The debug level 0..7 (0=Off) else -1 for error
 */
int ctr_get_debug_level(void *handle)
{
	return -1;
}

/**
 * @brief Set your timeout in milliseconds
 * @param A handle that was allocated in open
 * @param The timeout im milliseconds, zero means no timeout
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_set_timeout(void *handle, int timeout)
{
	return -1;
}

/**
 * @brief Get your timeout in milliseconds
 * @param A handle that was allocated in open
 * @return The timeout in millisecond else -1 for error
 */
int ctr_get_timeout(void *handle)
{
	return -1;
}

/**
 * @brief Get the CTR module status
 * @param A handle that was allocated in open
 * @param Pointer to where the status will be stored of type CtrDrvrStatus
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_status(void *handle, CtrDrvrStatus *stat)
{
	return -1;
}

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
		   ctr_ccv_fields_t ctr_ccv_fields)

{
	return -1;
}

/**
 * @brief Get the remote counter flag and config
 * @param A handle that was allocated in open
 * @param ch is the channel 1..3 for ctri, 1..4 for ctrp, 1..8 for ctrv.
 * @param ctr_ccv are the values of the counter
 * @return The remote flag 0=normal, 1=remote or -1 on error
 */
int ctr_get_remote(void *handle, CtrDrvrCounter ch, struct ctr_ccv_s *ctr_ccv)
{
	return -1;
}

/**
 * @brief Choose PLL locking method, brutal or slow
 * @param A handle that was allocated in open
 * @param The lock flag 0=Brutal 1= slow
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_set_pll_lock_method(void *handle, int lock_method)
{
	return -1;
}

/**
 * @brief Get Pll locking method
 * @param A handle that was allocated in open
 * @return The lock flag or -1 on error
 */
int ctr_get_pll_lock_method(void *handle)
{
	return -1;
}

/**
 * @brief Read the io status
 * @param A handle that was allocated in open
 * @param Pointer to where the iostatus will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_io_status(void *handle, CtrDrvrIoStatus *io_stat)
{
	return -1;
}

/**
 * @brief Get module statistics
 * @param A handle that was allocated in open
 * @param Pointer to where the statistics will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_stats(void *handle, CtrDrvrModuleStats *stats)
{
	return -1;
}

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
int ctr_memory_test(void *handle, int *address, int *wpat, int *rpat)
{
	return -1;
}

/**
 * @brief Get the list of all driver clients
 * @param A handle that was allocated in open
 * @param Pointer to the client list
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_client_pids(void *handle, CtrDrvrClientList *client_pids)
{
	return -1;
}

/**
 * @brief Get a clients connections
 * @param A handle that was allocated in open
 * @param Pointer to where clients connections will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_client_connections(void *handle, CtrDrvrClientConnections *connections)
{
	return -1;
}

/**
 * @brief simulate an interrupt
 * @param A handle that was allocated in open
 * @param Class of interrupt to simulate
 * @param Class value
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_simulate_interrupt(void *handle, CtrDrvrConnectionClass ctr_class, int equip)
{
	return -1;
}

#endif
