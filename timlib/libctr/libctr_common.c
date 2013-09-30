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
	uint32_t arg = 0;

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
	uint32_t arg = modnum;

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
	uint32_t arg = 0;

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
	CtrDrvrClientConnections ccon;
	int i, pid, flg=0;

	pid = getpid();
	if (ctr_get_client_connections(handle,pid,&ccon) < 0)
		return -1;

	if (!mask) {
		for (i=0; i<ccon.Size; i++)
			if (ioctl(h->fd,CtrIoctlDISCONNECT,&ccon.Connections[i]) < 0)
				return -1;
	} else {
		for (i=0; i<ccon.Size; i++) {
			if ((ccon.Connections[i].EqpNum   == mask)
			&&  (ccon.Connections[i].EqpClass == ctr_class)) {
				if (ioctl(h->fd,CtrIoctlDISCONNECT,&ccon.Connections[i]) < 0)
					return -1;
				flg = 1;
				break;
			}
		}
		if (!flg) {
			errno = ENOTCONN; /* The socket is not connected */
			return -1;
		}
	}
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
	ctr_interrupt->payload   = rbf.Frame.Long;
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
	struct ctr_handle_s *h = handle;

	CtrDrvrPtimBinding           ob;
	CtrDrvrAction                act;
	CtrDrvrTrigger              *trg;
	CtrDrvrCounterConfiguration *cnf;
	CtrDrvrTgmGroup             *grp;
	CtrDrvrCounterMaskBuf        cmsb;
	CtrDrvrCtimObjects           ctimo;

	uint32_t mod;
	int anm, i, ok;

	trg = &(act.Trigger);
	cnf = &(act.Config);
	grp = &(trg->Group);

	ob.EqpNum = ltim;
	if (ioctl(h->fd,CtrIoctlGET_PTIM_BINDING,&ob) < 0)
		return -1;

	mod = ob.ModuleIndex +1;
	if (ioctl(h->fd,CtrIoctlSET_MODULE,&mod) < 0)
		return -1;

	if (index >= ob.Size) {
		errno = ERANGE;
		return -1;
	}

	anm = ob.StartIndex + index + 1;
	act.TriggerNumber = anm;
	if (ioctl(h->fd,CtrIoctlGET_ACTION,&act) < 0)
		return -1;

	trg->Counter = ob.Counter;

	if (ctr_ccv_fields & CTR_CCV_ENABLE) {
		if (ctr_ccv->enable & CtrDrvrCounterOnZeroBUS)
			cnf->OnZero |= CtrDrvrCounterOnZeroBUS;
		else
			cnf->OnZero &= ~CtrDrvrCounterOnZeroBUS;

		if (ctr_ccv->enable & CtrDrvrCounterOnZeroOUT)
			cnf->OnZero |= CtrDrvrCounterOnZeroOUT;
		else
			cnf->OnZero &= ~CtrDrvrCounterOnZeroOUT;
	}

	if (ctr_ccv_fields & CTR_CCV_START)
		cnf->Start = ctr_ccv->start;

	if (ctr_ccv_fields & CTR_CCV_MODE)
		cnf->Mode = ctr_ccv->mode;

	if (ctr_ccv_fields & CTR_CCV_CLOCK)
		cnf->Clock = ctr_ccv->clock;

	if (ctr_ccv_fields & CTR_CCV_PULSE_WIDTH)
		cnf->PulsWidth = ctr_ccv->pulse_width;

	if (ctr_ccv_fields & CTR_CCV_DELAY)
		cnf->Delay = ctr_ccv->delay;

	if (ctr_ccv_fields & CTR_CCV_COUNTER_MASK) {
		cmsb.Counter = ob.Counter;
		if (ioctl(h->fd,CtrIoctlGET_OUT_MASK,&cmsb) < 0)
			return -1;
		cmsb.Mask = ctr_ccv->counter_mask;
		if (ioctl(h->fd,CtrIoctlSET_OUT_MASK,&cmsb) < 0)
			return -1;
	}

	if (ctr_ccv_fields & CTR_CCV_POLARITY) {
		cmsb.Counter = trg->Counter;
		if (ioctl(h->fd,CtrIoctlGET_OUT_MASK,&cmsb) < 0)
			return -1;
		cmsb.Polarity = ctr_ccv->polarity;
		if (ioctl(h->fd,CtrIoctlSET_OUT_MASK,&cmsb) < 0)
			return -1;
	}

	if (ctr_ccv_fields & CTR_CCV_CTIM) {
		if (ioctl(h->fd,CtrIoctlLIST_CTIM_OBJECTS,&ctimo) < 0)
			return -1;

		ok = 0;
		for (i=0; i<ctimo.Size; i++) {
			if (ctimo.Objects[i].EqpNum == ctr_ccv->ctim) {
				trg->Ctim = ctr_ccv->ctim;
				trg->Frame = ctimo.Objects[i].Frame;
				ok = 1;
				break;
			}
		}
		if (!ok) {
			errno = EADDRNOTAVAIL; /* Address not available */
			return -1;
		}
	}

	if (ctr_ccv_fields & CTR_CCV_PAYLOAD)
		trg->Frame.Struct.Value = ctr_ccv->payload;

	if (ctr_ccv_fields & CTR_CCV_CMP_METHOD)
		trg->TriggerCondition = ctr_ccv->cmp_method;

	if (ctr_ccv_fields & CTR_CCV_GRNUM)
		grp->GroupNumber = ctr_ccv->grnum;

	if (ctr_ccv_fields & CTR_CCV_GRVAL)
		grp->GroupValue = ctr_ccv->grval;

	if (ctr_ccv_fields & CTR_CCV_TGNUM)
		trg->Machine = ctr_ccv->tgnum;

	if (ioctl(h->fd,CtrIoctlSET_ACTION,&act) < 0)
		return -1;

	return 0;
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
	struct ctr_handle_s *h = handle;

	CtrDrvrPtimBinding           ob;
	CtrDrvrAction                act;
	CtrDrvrTrigger              *trg;
	CtrDrvrCounterConfiguration *cnf;
	CtrDrvrTgmGroup             *grp;
	CtrDrvrCounterMaskBuf        cmsb;

	uint32_t mod;
	int anm;

	trg = &(act.Trigger);
	cnf = &(act.Config);
	grp = &(trg->Group);

	ob.EqpNum = ltim;
	if (ioctl(h->fd,CtrIoctlGET_PTIM_BINDING,&ob) < 0)
		return -1;

	mod = ob.ModuleIndex +1;
	if (ioctl(h->fd,CtrIoctlSET_MODULE,&mod) < 0)
		return -1;

	if (index >= ob.Size) {
		errno = ERANGE;
		return -1;
	}

	anm = ob.StartIndex + index + 1;
	act.TriggerNumber = anm;
	if (ioctl(h->fd,CtrIoctlGET_ACTION,&act) < 0)
		return -1;

	ctr_ccv->enable      = cnf->OnZero;
	ctr_ccv->start       = cnf->Start;
	ctr_ccv->mode        = cnf->Mode;
	ctr_ccv->clock       = cnf->Clock;
	ctr_ccv->pulse_width = cnf->PulsWidth;
	ctr_ccv->delay       = cnf->Delay;

	cmsb.Counter = ob.Counter;
	if (ioctl(h->fd,CtrIoctlGET_OUT_MASK,&cmsb) < 0)
		return -1;

	ctr_ccv->counter_mask  = cmsb.Mask;
	ctr_ccv->polarity      = cmsb.Polarity;

	ctr_ccv->ctim          = trg->Ctim;
	ctr_ccv->payload       = trg->Frame.Struct.Value;
	ctr_ccv->cmp_method    = trg->TriggerCondition;

	ctr_ccv->grnum         = grp->GroupNumber;
	ctr_ccv->grval         = grp->GroupValue;
	ctr_ccv->tgnum         = trg->Machine;

	return 0;
}

/**
 * @brief Create an empty LTIM object on the current module
 * @param A handle that was allocated in open
 * @param ltim number to create
 * @param channel number for ltim
 * @param size of ltim action array (PLS lines)
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_create_ltim(void *handle, int ltim, int ch, int size)
{
	struct ctr_handle_s *h = handle;
	CtrDrvrPtimBinding ptim;
	int mod;

	mod = ctr_get_module(handle);
	if (mod < 0)
		return -1;

	ptim.EqpNum = ltim;
	ptim.ModuleIndex = mod -1;
	ptim.Counter = ch;
	ptim.Size = size;
	ptim.StartIndex = 0;

	if (ioctl(h->fd,CtrIoctlCREATE_PTIM_OBJECT,&ptim) < 0)
		return -1;
	return 0;
}

/**
 * @brief Destroy an LTIM object on the current module
 * @param A handle that was allocated in open
 * @param ltim number to destroy
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_destroy_ltim(void *handle, int ltim)
{
	struct ctr_handle_s *h = handle;
	CtrDrvrPtimBinding ptim;

	ptim.EqpNum = ltim;
	ptim.ModuleIndex = 0;
	ptim.Counter = 0;
	ptim.Size = 0;
	ptim.StartIndex = 0;

	if (ioctl(h->fd,CtrIoctlGET_PTIM_BINDING,&ptim) < 0)
		return -1;

	if (ioctl(h->fd,CtrIoctlDESTROY_PTIM_OBJECT,&ptim) < 0)
		return -1;

	return 0;
}

/**
 * @brief get a telegram
 * @param index into the array of telegrams 0..7
 * @param telegram point to a short array of at least size 32
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_telegram(void *handle, int index, short *telegram)
{
	struct ctr_handle_s *h = handle;
	CtrDrvrTgmBuf tgmb;
	int i;

	tgmb.Machine = index;
	if (ioctl(h->fd,CtrIoctlREAD_TELEGRAM,&tgmb) < 0)
		return -1;

	for (i=0; i<32; i++)
		telegram[i] = tgmb.Telegram[i];

	return 0;
}

/**
 * @brief Get time
 * @param A handle that was allocated in open
 * @param ctr_time point to where time will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_time(void *handle, CtrDrvrCTime *ctr_time)
{
	struct ctr_handle_s *h = handle;
	CtrDrvrCTime ct;

	if (ioctl(h->fd,CtrIoctlGET_UTC,&ct) < 0)
		return -1;

	*ctr_time = ct;
	return 0;
}

/**
 * @brief Set the time on the current module
 * @param A handle that was allocated in open
 * @param ctr_time the time to be set
 * @return Zero means success else -1 is returned on error, see errno
 *
 * Note this time will be overwritten within 1 second if the
 * current module is enabled and connected to the timing network.
 */
int ctr_set_time(void *handle, CtrDrvrTime ctr_time)
{
	struct ctr_handle_s *h = handle;

	if (ioctl(h->fd,CtrIoctlSET_UTC,&ctr_time) < 0)
		return -1;
	return 0;
}

/**
 * @brief Get cable ID
 * @param A handle that was allocated in open
 * @param cable_id points to where id will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_cable_id(void *handle, int *cable_id)
{
	struct ctr_handle_s *h = handle;
	uint32_t cid;

	if (ioctl(h->fd,CtrIoctlGET_CABLE_ID,&cid) < 0)
		return -1;

	*cable_id = (int) cid;
	return 0;
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
	struct ctr_handle_s *h = handle;
	uint32_t cid = cable_id;

	if (ioctl(h->fd,CtrIoctlSET_CABLE_ID,&cid) < 0)
		return -1;
	return 0;
}

/**
 * @brief Get driver and firmware version
 * @param A handle that was allocated in open
 * @param version points to where version will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_version(void *handle, CtrDrvrVersion *version)
{
	struct ctr_handle_s *h = handle;
	CtrDrvrVersion ver;

	if (ioctl(h->fd,CtrIoctlGET_VERSION,&ver) < 0)
		return -1;

	*version = ver;
	return 0;
}

/**
 * @brief List LTIM objects
 * @param A handle that was allocated in open
 * @param Place where the list will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_list_ltim_objects(void *handle, CtrDrvrPtimObjects *ltims)
{
	struct ctr_handle_s *h = handle;
	CtrDrvrPtimObjects ptimo;

	if (ioctl(h->fd,CtrIoctlLIST_PTIM_OBJECTS,&ptimo) < 0)
		return -1;

	*ltims = ptimo;
	return 0;
}

/**
 * @brief List CTIM objects
 * @param A handle that was allocated in open
 * @param Place where the list will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_list_ctim_objects(void *handle, CtrDrvrCtimObjects *ctims)
{
	struct ctr_handle_s *h = handle;
	CtrDrvrCtimObjects ctimo;

	if (ioctl(h->fd,CtrIoctlLIST_CTIM_OBJECTS,&ctimo) < 0)
		return -1;
	*ctims = ctimo;
	return 0;
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
	struct ctr_handle_s *h = handle;
	CtrDrvrCtimBinding cb;

	cb.EqpNum     = ctim;
	cb.Frame.Long = mask;

	if (ioctl(h->fd,CtrIoctlCREATE_CTIM_OBJECT,&cb) < 0)
		return -1;
	return 0;
}

/**
 * @brief Destroy a CTIM
 * @param A handle that was allocated in open
 * @param ctim event Id to destroy
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_destroy_ctim(void *handle, int ctim)
{
	struct ctr_handle_s *h = handle;
	CtrDrvrCtimBinding cb;

	cb.EqpNum     = ctim;
	cb.Frame.Long = 0;

	if (ioctl(h->fd,CtrIoctlDESTROY_CTIM_OBJECT,&cb) < 0)
		return -1;
	return 0;
}

/**
 * @brief Get the size of your queue for a given handle
 * @param A handle that was allocated in open
 * @return Queue size or -1 on error
 */
int ctr_get_queue_size(void *handle)
{
	struct ctr_handle_s *h = handle;
	uint32_t qs;

	if (ioctl(h->fd,CtrIoctlGET_QUEUE_SIZE,&qs) < 0)
		return -1;
	return (int) qs;
}

/**
 * @brief Turn your queue on or off
 * @param A handle that was allocated in open
 * @param flag 1=>queuing is off, 0=>queuing is on
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_set_queue_flag(void *handle, int flag)
{
	struct ctr_handle_s *h = handle;
	uint32_t qf;

	if (flag) qf = 0;
	else      qf = 1;
	if (ioctl(h->fd,CtrIoctlSET_QUEUE_FLAG,&qf) < 0)
		return -1;
	return 0;
}

/**
 * @brief Get the current queue flag setting
 * @param A handle that was allocated in open
 * @return The queue flag 0..1 (QOFF..QON)  else -1 on error
 */
int ctr_get_queue_flag(void *handle)
{
	struct ctr_handle_s *h = handle;
	uint32_t qf;

	if (ioctl(h->fd,CtrIoctlGET_QUEUE_FLAG,&qf) < 0)
		return -1;
	if (qf) return 0;

	return 1;
}

/**
 * @brief Enable/Disable timing reception on current module
 * @param A handle that was allocated in open
 * @param Enable flag (1=enabled 0=disabled)
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_set_enable(void *handle, int flag)
{
	struct ctr_handle_s *h = handle;
	uint32_t en = flag;

	if (ioctl(h->fd,CtrIoctlENABLE,&en) < 0)
		return -1;
	return 0;
}

/**
 * @brief Get the Enable/Disable flag value
 * @param A handle that was allocated in open
 * @return The enable/Disable flag value or -1 on error
 */
int ctr_get_enable(void *handle)
{
	struct ctr_handle_s *h = handle;
	uint32_t st;

	if (ioctl(h->fd,CtrIoctlGET_STATUS,&st) < 0)
		return -1;

	if (st & CtrDrvrStatusENABLED)
		return 1;
	return 0;
}

/**
 * @brief Set the CTR timing input delay value
 * @param A handle that was allocated in open
 * @param The new delay value in 40MHz (25ns) Ticks (24-Bit)
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_set_input_delay(void *handle, int delay)
{
	struct ctr_handle_s *h = handle;
	uint32_t dly = delay;

	if (ioctl(h->fd,CtrIoctlSET_INPUT_DELAY,&dly) < 0)
		return -1;
	return 0;
}

/**
 * @brief Get the CTR timing input delay value
 * @param A handle that was allocated in open
 * @return The input delay value in 40MHz ticks value or -1 on error
 */
int ctr_get_input_delay(void *handle)
{
	struct ctr_handle_s *h = handle;
	uint32_t dly;

	if (ioctl(h->fd,CtrIoctlGET_INPUT_DELAY,&dly) < 0)
		return -1;
	return (int) dly;
}

/**
 * @brief Set the CTR driver debug print out level
 * @param A handle that was allocated in open
 * @param The level to be set 0=None ..7 Up to level 7
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_set_debug_level(void *handle, int level)
{
	struct ctr_handle_s *h = handle;
	uint32_t deb = level;

	if (ioctl(h->fd,CtrIoctlSET_SW_DEBUG,&deb) < 0)
		return -1;
	return 0;
}

/**
 * @brief Get the CTR driver debug print out level
 * @param A handle that was allocated in open
 * @return The debug level 0..7 (0=Off) else -1 for error
 */
int ctr_get_debug_level(void *handle)
{
	struct ctr_handle_s *h = handle;
	uint32_t deb;

	if (ioctl(h->fd,CtrIoctlGET_SW_DEBUG,&deb) < 0)
		return -1;
	return (int) deb;
}

/**
 * @brief Set your timeout in milliseconds
 * @param A handle that was allocated in open
 * @param The timeout im milliseconds, zero means no timeout
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_set_timeout(void *handle, int timeout)
{
	struct ctr_handle_s *h = handle;
	uint32_t tmo = timeout;

	if (ioctl(h->fd,CtrIoctlSET_TIMEOUT,&tmo) < 0)
		return -1;
	return 0;
}

/**
 * @brief Get your timeout in milliseconds
 * @param A handle that was allocated in open
 * @return The timeout in millisecond else -1 for error
 */
int ctr_get_timeout(void *handle)
{
	struct ctr_handle_s *h = handle;
	uint32_t tmo;

	if (ioctl(h->fd,CtrIoctlGET_TIMEOUT,&tmo) < 0)
		return -1;
	return (int) tmo;
}

/**
 * @brief Get the CTR module status
 * @param A handle that was allocated in open
 * @param Pointer to where the status will be stored of type CtrDrvrStatus
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_status(void *handle, CtrDrvrStatus *stat)
{
	struct ctr_handle_s *h = handle;
	CtrDrvrStatus st;

	if (ioctl(h->fd,CtrIoctlGET_STATUS,&st) < 0)
		return -1;
	*stat = st;
	return 0;
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
	struct ctr_handle_s *h = handle;
	CtrDrvrCounterConfiguration *cnf;
	CtrDrvrCounterConfigurationBuf cnfb;
	CtrdrvrRemoteCommandBuf crmb;
	CtrDrvrCounterMaskBuf cmsb;

	cnf = &cnfb.Config;

	crmb.Counter = ch;
	crmb.Remote = remote_flag;

	if (ioctl(h->fd,CtrIoctlSET_REMOTE,&crmb) < 0)
		return -1;

	cnfb.Counter = ch;
	if (ioctl(h->fd,CtrIoctlGET_CONFIG,&cnfb) < 0)
		return -1;

	if ((ctr_ccv) && (ctr_ccv_fields)) {

		if (ctr_ccv_fields & CTR_CCV_ENABLE) {
			if (ctr_ccv->enable & CtrDrvrCounterOnZeroBUS)
				cnf->OnZero |= CtrDrvrCounterOnZeroBUS;
			else
				cnf->OnZero &= ~CtrDrvrCounterOnZeroBUS;

			if (ctr_ccv->enable & CtrDrvrCounterOnZeroOUT)
				cnf->OnZero |= CtrDrvrCounterOnZeroOUT;
			else
				cnf->OnZero &= ~CtrDrvrCounterOnZeroOUT;
		}

		if (ctr_ccv_fields & CTR_CCV_START)
			cnf->Start = ctr_ccv->start;

		if (ctr_ccv_fields & CTR_CCV_MODE)
			cnf->Mode = ctr_ccv->mode;

		if (ctr_ccv_fields & CTR_CCV_CLOCK)
			cnf->Clock = ctr_ccv->clock;

		if (ctr_ccv_fields & CTR_CCV_PULSE_WIDTH)
			cnf->PulsWidth = ctr_ccv->pulse_width;

		if (ctr_ccv_fields & CTR_CCV_DELAY)
			cnf->Delay = ctr_ccv->delay;

		if (ctr_ccv_fields & CTR_CCV_COUNTER_MASK) {
			cmsb.Counter = ch;
			if (ioctl(h->fd,CtrIoctlGET_OUT_MASK,&cmsb) < 0)
				return -1;
			cmsb.Mask = ctr_ccv->counter_mask;
			if (ioctl(h->fd,CtrIoctlSET_OUT_MASK,&cmsb) < 0)
				return -1;
		}

		if (ctr_ccv_fields & CTR_CCV_POLARITY) {
			cmsb.Counter = ch;
			if (ioctl(h->fd,CtrIoctlGET_OUT_MASK,&cmsb) < 0)
				return -1;
			cmsb.Polarity = ctr_ccv->polarity;
			if (ioctl(h->fd,CtrIoctlSET_OUT_MASK,&cmsb) < 0)
				return -1;
		}

		if (ioctl(h->fd,CtrIoctlSET_CONFIG,&cnfb) < 0)
			return -1;

		crmb.Remote = CtrDrvrRemoteLOAD;
		if (ioctl(h->fd,CtrIoctlREMOTE,&crmb) < 0)
			return -1;
	}

	if (rcmd) {
		crmb.Remote = rcmd;
		if (ioctl(h->fd,CtrIoctlREMOTE,&crmb) < 0)
			return -1;
	}
	return 0;
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
	struct ctr_handle_s *h = handle;
	CtrdrvrRemoteCommandBuf crmb;
	CtrDrvrCounterConfigurationBuf cnfb;
	CtrDrvrCounterConfiguration *cnf;
	CtrDrvrCounterMaskBuf cmsb;

	crmb.Counter = ch;
	crmb.Remote = 0;

	if (ioctl(h->fd,CtrIoctlGET_REMOTE,&crmb) < 0)
		return -1;

	if (ctr_ccv) {
		cnf = &cnfb.Config;

		cnfb.Counter = ch;
		if (ioctl(h->fd,CtrIoctlGET_CONFIG,&cnfb) < 0)
			return -1;

		cmsb.Counter = ch;
		if (ioctl(h->fd,CtrIoctlGET_OUT_MASK,&cmsb) < 0)
			return -1;

		bzero((void *) ctr_ccv, sizeof(struct ctr_ccv_s));

		ctr_ccv->enable      = cnf->OnZero;
		ctr_ccv->start       = cnf->Start;
		ctr_ccv->mode        = cnf->Mode;
		ctr_ccv->clock       = cnf->Clock;
		ctr_ccv->pulse_width = cnf->PulsWidth;
		ctr_ccv->delay       = cnf->Delay;

		ctr_ccv->counter_mask = cmsb.Mask;
		ctr_ccv->polarity     = cmsb.Polarity;
	}
	return (int) crmb.Remote;
}

/**
 * @brief Choose PLL locking method, brutal or slow
 * @param A handle that was allocated in open
 * @param The lock flag 0=Brutal 1= slow
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_set_pll_lock_method(void *handle, int lock_method)
{
	struct ctr_handle_s *h = handle;
	uint32_t lkf = lock_method;

	if (ioctl(h->fd,CtrIoctlSET_BRUTAL_PLL,&lkf) < 0)
		return -1;
	return 0;
}

/**
 * @brief Get Pll locking method
 * @param A handle that was allocated in open
 * @return The lock flag (0=Brutal 1=Slow) or -1 on error
 */
int ctr_get_pll_lock_method(void *handle)
{
	struct ctr_handle_s *h = handle;
	uint32_t stat;

	if (ioctl(h->fd,CtrIoctlGET_IO_STATUS,&stat) < 0)
		return -1;

	if (CtrDrvrIOStatusUtcPllEnabled & stat)   /* Slow lock */
		return 0;

	return 1;                                  /* Brutal lock */
}

/**
 * @brief Read the io status
 * @param A handle that was allocated in open
 * @param Pointer to where the iostatus will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_io_status(void *handle, CtrDrvrIoStatus *io_stat)
{
	struct ctr_handle_s *h = handle;
	uint32_t stat;

	if (ioctl(h->fd,CtrIoctlGET_IO_STATUS,&stat) < 0)
		return -1;

	*io_stat = (CtrDrvrIoStatus) stat;
	return 0;
}

/**
 * @brief Get module statistics
 * @param A handle that was allocated in open
 * @param Pointer to where the statistics will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_stats(void *handle, CtrDrvrModuleStats *stats)
{
	struct ctr_handle_s *h = handle;
	uint32_t stat;
	CtrDrvrModuleStats mstat;

	if (ioctl(h->fd,CtrIoctlGET_IO_STATUS,&stat) < 0)
		return -1;

	if (stat & CtrDrvrIOStatusExtendedMemory) {
		if (ioctl(h->fd,CtrIoctlGET_MODULE_STATS,&mstat) < 0)
			return -1;

		*stats = mstat;
		return 0;
	}
	errno = ENOSYS; /* Function not supported. (On this hardware version) */
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

#define RAMAD 389
#define RAMSZ 12288

	struct ctr_handle_s *h = handle;
	uint32_t stat;
	unsigned int i, data, addr;
	CtrDrvrRawIoBlock iob;

	if (ioctl(h->fd,CtrIoctlGET_STATUS,&stat) < 0)
		return -1;

	if (stat & CtrDrvrStatusENABLED) {
		errno = EBUSY;      /* Device or resource busy. Must disable first */
		return -1;
	}

	iob.UserArray = &data;
	iob.Size = 1;

	for (i=0; i<RAMSZ; i++) {
		addr = i + RAMAD;
		iob.Offset = addr;

		data = addr;
		if (ioctl(h->fd,CtrIoctlRAW_WRITE,&iob) < 0)
			return -1;

		data = 0;
		if (ioctl(h->fd,CtrIoctlRAW_READ,&iob) < 0)
			return -1;

		if (data != addr) {
			errno = 0;      /* I am using "0" to say memory error */
			*address = addr;
			*wpat = addr;
			*rpat = data;
			return -1;
		}
	}
	return 0;
}

/**
 * @brief Get the list of all driver clients
 * @param A handle that was allocated in open
 * @param Pointer to the client list
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_client_pids(void *handle, CtrDrvrClientList *client_pids)
{
	struct ctr_handle_s *h = handle;
	CtrDrvrClientList clist;

	if (ioctl(h->fd,CtrIoctlGET_CLIENT_LIST,&clist) < 0)
		return -1;

	*client_pids = clist;
	return 0;
}

/**
 * @brief Get a clients connections
 * @param A handle that was allocated in open
 * @param Pid of the client whose connections you want
 * @param Pointer to where clients connections will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_client_connections(void *handle, int pid, CtrDrvrClientConnections *connections)
{
	struct ctr_handle_s *h = handle;
	CtrDrvrClientConnections cons;

	bzero((void *) &cons, sizeof(CtrDrvrClientConnections));
	cons.Pid = pid;
	if (ioctl(h->fd,CtrIoctlGET_CLIENT_CONNECTIONS,&cons) < 0)
		return -1;

	*connections = cons;
	return 0;
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
	struct ctr_handle_s *h = handle;
	CtrDrvrConnection con;
	CtrDrvrWriteBuf wbf;
	int cc, mod;

	mod = ctr_get_module(handle);
	if (mod < 0)
		return -1;

	con.Module = mod;
	con.EqpNum = equip;
	con.EqpClass = ctr_class;

	wbf.TriggerNumber = 0;
	wbf.Connection = con;
	wbf.Payload = 0;

	cc = write(h->fd,&wbf,sizeof(CtrDrvrReadBuf));
	if (cc > 0)
		return 0;
	return -1;
}

#endif
