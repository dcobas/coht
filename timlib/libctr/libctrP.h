/**
 * Julian Lewis Fri-30th March 2012 BE/CO/HT
 * julian.lewis@cern.ch
 *
 * CTR timing library definitions (PRIVATE).
 * This library is to be used exclusivlely by the new open CBCM timing library.
 * It is up to the implementers of the open CBCM timing library to provide any
 * abstraction layers and to hide this completely from user code.
 */


#ifndef LIBCTRP
#define LIBCTRP

#include <libctr.h>

#define CTR_PATH_SIZE 64

typedef enum {
	CTR_INDEX_get_module_count,
	CTR_INDEX_set_module,
	CTR_INDEX_get_module,
	CTR_INDEX_get_type,
	CTR_INDEX_get_module_address,
	CTR_INDEX_connect,
	CTR_INDEX_connect_payload,
	CTR_INDEX_disconnect,
	CTR_INDEX_wait,
	CTR_INDEX_set_ccv,
	CTR_INDEX_get_ccv,
	CTR_INDEX_create_ltim,
	CTR_INDEX_destroy_ltim,
	CTR_INDEX_get_telegram,
	CTR_INDEX_get_time,
	CTR_INDEX_set_time,
	CTR_INDEX_get_cable_id,
	CTR_INDEX_set_cable_id,
	CTR_INDEX_get_version,
	CTR_INDEX_create_ctim,
	CTR_INDEX_destroy_ctim,
	CTR_INDEX_get_queue_size,
	CTR_INDEX_set_queue_flag,
	CTR_INDEX_get_queue_flag,
	CTR_INDEX_set_enable,
	CTR_INDEX_get_enable,
	CTR_INDEX_set_input_delay,
	CTR_INDEX_get_input_delay,
	CTR_INDEX_set_debug_level,
	CTR_INDEX_get_debug_level,
	CTR_INDEX_set_timeout,
	CTR_INDEX_get_timeout,
	CTR_INDEX_get_status,
	CTR_INDEX_set_remote,
	CTR_INDEX_get_remote,
	CTR_INDEX_set_pll_lock_method,
	CTR_INDEX_get_pll_lock_method,
	CTR_INDEX_get_io_status,
	CTR_INDEX_get_stats,
	CTR_INDEX_memory_test,
	CTR_INDEX_get_client_pids,
	CTR_INDEX_get_client_connections,
	CTR_INDEX_simulate_interrupt,
	CTR_INDEX_set_p2_output_byte,
	CTR_INDEX_get_p2_output_byte,
	CTR_INDEX_list_ltim_objects,
	CTR_INDEX_list_ctim_objects,

	CTR_INDEX_LAST
} ctr_index_t;

struct ctr_api_s {
	int   (*ctr_get_module_count)(void *handle);
	int   (*ctr_set_module)(void *handle, int modnum);
	int   (*ctr_get_module)(void *handle);
	int   (*ctr_get_type)(void *handle, CtrDrvrHardwareType *type);
	int   (*ctr_get_module_address)(void *handle, struct ctr_module_address_s *module_address);
	int   (*ctr_connect)(void *handle, CtrDrvrConnectionClass ctr_class, int equip);
	int   (*ctr_connect_payload)(void *handle, int ctim, int payload);
	int   (*ctr_disconnect)(void *handle, CtrDrvrConnectionClass ctr_class, int mask);
	int   (*ctr_wait)(void *handle, struct ctr_interrupt_s *ctr_interrupt);
	int   (*ctr_set_ccv)(void *handle, int ltim, int index, struct ctr_ccv_s *ctr_ccv, ctr_ccv_fields_t ctr_ccv_fields);
	int   (*ctr_get_ccv)(void *handle, int ltim, int index, struct ctr_ccv_s *ctr_ccv);
	int   (*ctr_create_ltim)(void *handle, int ltim, int ch, int size);
	int   (*ctr_destroy_ltim)(void *handle, int ltim);
	int   (*ctr_get_telegram)(void *handle, int index, short *telegram);
	int   (*ctr_get_time)(void *handle, CtrDrvrCTime *ctr_time);
	int   (*ctr_set_time)(void *handle, CtrDrvrTime ctr_time);
	int   (*ctr_get_cable_id)(void *handle, int *cable_id);
	int   (*ctr_set_cable_id)(void *handle, int cable_id);
	int   (*ctr_get_version)(void *handle, CtrDrvrVersion *version);
	int   (*ctr_create_ctim)(void *handle, int ctim, int mask);
	int   (*ctr_destroy_ctim)(void *handle, int ctim);
	int   (*ctr_get_queue_size)(void *handle);
	int   (*ctr_set_queue_flag)(void *handle, int flag);
	int   (*ctr_get_queue_flag)(void *handle);
	int   (*ctr_set_enable)(void *handle, int flag);
	int   (*ctr_get_enable)(void *handle);
	int   (*ctr_set_input_delay)(void *handle, int delay);
	int   (*ctr_get_input_delay)(void *handle);
	int   (*ctr_set_debug_level)(void *handle, int level);
	int   (*ctr_get_debug_level)(void *handle);
	int   (*ctr_set_timeout)(void *handle, int timeout);
	int   (*ctr_get_timeout)(void *handle);
	int   (*ctr_get_status)(void *handle, CtrDrvrStatus *stat);
	int   (*ctr_set_remote)(void *handle, int remote_flag, CtrDrvrCounter ch, CtrDrvrRemote rcmd, struct ctr_ccv_s *ctr_ccv, ctr_ccv_fields_t ctr_ccv_fields);
	int   (*ctr_get_remote)(void *handle, CtrDrvrCounter ch, struct ctr_ccv_s *ctr_ccv);
	int   (*ctr_set_pll_lock_method)(void *handle, int lock_method);
	int   (*ctr_get_pll_lock_method)(void *handle);
	int   (*ctr_get_io_status)(void *handle, CtrDrvrIoStatus *io_stat);
	int   (*ctr_get_stats)(void *handle, CtrDrvrModuleStats *stats);
	int   (*ctr_memory_test)(void *handle, int *address, int *wpat, int *rpat);
	int   (*ctr_get_client_pids)(void *handle, CtrDrvrClientList *client_pids);
	int   (*ctr_get_client_connections)(void *handle, int pid, CtrDrvrClientConnections *connections);
	int   (*ctr_simulate_interrupt)(void *handle, CtrDrvrConnectionClass ctr_class, int equip);
	int   (*ctr_set_p2_output_byte)(void *handle, int p2byte);
	int   (*ctr_get_p2_output_byte)(void *handle);
	int   (*ctr_list_ltim_objects)(void *handle, CtrDrvrPtimObjects *ltims);
	int   (*ctr_list_ctim_objects)(void *handle, CtrDrvrCtimObjects *ctims);
};

char *ctr_api_names[CTR_INDEX_LAST] = {
	"ctr_get_module_count",
	"ctr_set_module",
	"ctr_get_module",
	"ctr_get_type",
	"ctr_get_module_address",
	"ctr_connect",
	"ctr_connect_payload",
	"ctr_disconnect",
	"ctr_wait",
	"ctr_set_ccv",
	"ctr_get_ccv",
	"ctr_create_ltim",
	"ctr_destroy_ltim",
	"ctr_get_telegram",
	"ctr_get_time",
	"ctr_set_time",
	"ctr_get_cable_id",
	"ctr_set_cable_id",
	"ctr_get_version",
	"ctr_create_ctim",
	"ctr_destroy_ctim",
	"ctr_get_queue_size",
	"ctr_set_queue_flag",
	"ctr_get_queue_flag",
	"ctr_set_enable",
	"ctr_get_enable",
	"ctr_set_input_delay",
	"ctr_get_input_delay",
	"ctr_set_debug_level",
	"ctr_get_debug_level",
	"ctr_set_timeout",
	"ctr_get_timeout",
	"ctr_get_status",
	"ctr_set_remote",
	"ctr_get_remote",
	"ctr_set_pll_lock_method",
	"ctr_get_pll_lock_method",
	"ctr_get_io_status",
	"ctr_get_stats",
	"ctr_memory_test",
	"ctr_get_client_pids",
	"ctr_get_client_connections",
	"ctr_simulate_interrupt",
	"ctr_set_p2_output_byte",
	"ctr_get_p2_output_byte",
	"ctr_list_ltim_objects",
	"ctr_list_ctim_objects" };

struct ctr_handle_s {
	int fd;                 /** CTR driver file descriptor */
	void *dll_handle;       /** The Dll handle */
	struct ctr_api_s api;   /** The api function pointers */
};

#endif
