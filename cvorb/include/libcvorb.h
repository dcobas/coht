#ifndef _LIBCVORB_H_
#define _LIBCVORB_H_

#include <stdint.h>

#include <cvorb.h>

/**
 * Most of the client are written in C++
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @cond INTERNAL: makes doxygen ignore the following definition */
#define CVORB_PATH_SIZE  256
struct __cvorb_t {
	int fd;
        int lun;
        char sysfs_path[CVORB_PATH_SIZE];
        char hw_version[CVORB_HW_VER_SIZE];
        uint64_t pcb_id;
};
/** @endcond */

/**
 * @brief opaque cvorb module handle
 */
typedef struct __cvorb_t cvorb_t;

/* Open/close device Handle*/
cvorb_t *cvorb_open(unsigned int lun);
int cvorb_close(cvorb_t *device);
/* Board */
int cvorb_get_hw_version(cvorb_t *device, char *hw_version, unsigned int size);
int cvorb_get_pcb_id(cvorb_t *device, uint64_t *pcb_id);
int cvorb_get_temperature(cvorb_t *device, int *temp);
/* Submodule */
int cvorb_sm_set_input_polarity(cvorb_t *device, unsigned int submodule, enum cvorb_input_polarity polarity);
int cvorb_sm_get_input_polarity(cvorb_t *device, unsigned int submodule, enum cvorb_input_polarity *polarity);
int cvorb_sm_get_status(cvorb_t *device, unsigned int submodule, unsigned int *status);
int cvorb_sm_set_trigger(cvorb_t *device, unsigned int submodule, enum cvorb_trigger trigger);
int cvorb_sm_enable_optical_output(cvorb_t *device, unsigned int submodule);
int cvorb_sm_disable_optical_output(cvorb_t *device, unsigned int submodule);
int cvorb_sm_set_optical_output(cvorb_t *device, unsigned int submodule, unsigned int chnr);
int cvorb_sm_get_optical_output(cvorb_t *device, unsigned int submodule, unsigned int *chnr);
/* Channel */
int cvorb_ch_get_status(cvorb_t *device, unsigned int channelnr, unsigned int *status);
int cvorb_ch_enable(cvorb_t *dev, unsigned int channelnr);
int cvorb_ch_disable(cvorb_t *dev, unsigned int channelnr);
int cvorb_ch_get_repeat_count(cvorb_t *device, unsigned int channelnr, unsigned int *count);
int cvorb_ch_set_repeat_count(cvorb_t *device, unsigned int channelnr, unsigned int count);
int cvorb_ch_set_fcn( cvorb_t *dev, unsigned int channelnr, unsigned int functionnr, struct cvorb_vector_fcn *value, unsigned int size);
int cvorb_ch_get_fcn( cvorb_t *dev, unsigned int channelnr, unsigned int functionnr, struct cvorb_vector_fcn *value, unsigned int max_size, unsigned int *size);
int cvorb_ch_enable_fcn( cvorb_t *dev, unsigned int channelnr, unsigned int functionnr);
int cvorb_ch_disable_fcn( cvorb_t *dev, unsigned int channelnr, unsigned int functionnr);
int cvorb_ch_select_fcn( cvorb_t *dev, unsigned int channelnr, unsigned int functionnr);
int cvorb_ch_get_enable_fcn_mask( cvorb_t *dev, unsigned int channelnr, uint64_t *enbl_mask);
/* Utility */
int cvorb_get_submodule_nr(int channel_nr, int *submodule_nr);
int cvorb_loglevel(int loglevel);
int cvorb_errno(void);
char *cvorb_strerror(int errnum);
void cvorb_perror(const char *string);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _LIBCVORB_H_ */
