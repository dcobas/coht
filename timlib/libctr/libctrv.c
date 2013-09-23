/**
 * Julian Lewis Fri-30th March 2012 BE/CO/HT
 * julian.lewis@cern.ch
 *
 * CTR timing library stub.
 * This library is to be used exclusivlely by the new open CBCM timing library.
 * It is up to the implementers of the open CBCM timing library to provide any
 * abstraction layers and to hide this completely from user code.
 */

#define CTR_VME

#include "libctr.h"
#include "libctr_common.c"

/**
 * @brief Get the addresses of a module
 * @param A handle that was allocated in open
 * @param Pointer to where the module address will be stored
 * @return Zero means success else -1 is returned on error, see errno
 */
int ctr_get_module_address(void *handle, struct ctr_module_address_s *module_address)
{
	struct ctr_handle_s *h = handle;
	CtrDrvrModuleAddress moad;

	if (ioctl(h->fd,CtrIoctlGET_MODULE_DESCRIPTOR,&moad) < 0)
		return -1;

	module_address->device_type  = CtrDrvrDeviceCTRV;
	module_address->memory_map   = (uintptr_t) moad.VMEAddress;
	module_address->jtag_address = (uintptr_t) moad.JTGAddress;
	module_address->specific[0]  = moad.InterruptVector;
	module_address->specific[1]  = moad.InterruptLevel;

	return 0;
}

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
int ctr_set_p2_output_byte(void *handle, int p2byte)
{
	struct ctr_handle_s *h = handle;
	unsigned long obyte;

	obyte = p2byte;
	if (ioctl(h->fd,CtrIoctlSET_OUTPUT_BYTE,&obyte) < 0)
		return -1;
	return 0;
}

/**
 * @brief Get the P2 output byte number
 * @param A handle that was allocated in open
 * @return The output byte number or -1 on error
 *
 * If a value of 0 is returned, no output byte is set
 */
int ctr_get_p2_output_byte(void *handle)
{
	struct ctr_handle_s *h = handle;
	unsigned long obyte;

	if (ioctl(h->fd,CtrIoctlGET_OUTPUT_BYTE,&obyte) < 0)
		return -1;

	return (int) obyte;
}
