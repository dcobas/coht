/**
 * @file lib/cvorb.c
 *
 * @brief CVORB library
 *
 * Copyright (c) 2012
 * @author Samuel Iglesias Gonsalvez <siglesia@cern.ch>
 * @author Michel Arruat <michel.arruat@cern.ch>
 *
 * @section license_sec License
 * Released under the GPL v2.
 */

#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <byteswap.h>

#include <libcvorb.h>
#include <cvorbdev.h>
#include "libinternal.h"

#define LIBCVORB_VERSION        "1.0"
#define USE_DMA                 0
/** useful macro to check input parameter against a valid range */
#define WITHIN_RANGE(min,arg,max) ( ((max) >= (arg)) && ((min) <= (arg)) )

/*
 * @brief: set the loglevel and read once constant attributes
 */
static void __cvorb_initialize(void)
{
        char *value;

        value = getenv("LIBCVORB_LOGLEVEL");
        if (value) {
                __cvorb_loglevel = strtol(value, NULL, 0);
                LIBCVORB_DEBUG(3, "Setting loglevel to %d\n", __cvorb_loglevel);
        }
}

/**
 * @brief Open a CVORB device handle
 * @param lun   - Logical Unit address of the device to open from 0 to n-1 (So call LUN).
 *
 * @return Pointer to the opened file handle on success, NULL on failure
 */
cvorb_t *cvorb_open(unsigned int lun)
{
        static int __cvorb_init=0;
        cvorb_t *dev = NULL;
        char path[CVORB_PATH_SIZE];
        char devname[32];
        int len;

        if (!__cvorb_init) {
                __cvorb_initialize();
                __cvorb_init = 1;
        }
        LIBCVORB_DEBUG(4, "opening device %d\n", lun);

        /* check if the device exist by looking sysfs. */
        if (!cvorbdev_get_sysfs_path(lun, path, CVORB_PATH_SIZE)) {
                __cvorb_libc_error(__func__);
                __cvorb_lib_error(__func__, LIBCVORB_ENODEV);
                return NULL;
        }
        /* Create a cvorb device handler */
        dev = calloc(1, sizeof(*dev));
        if (dev == NULL) {
                __cvorb_libc_error(__func__);
                return NULL;
        }
        /* keep lun and the sysfs path for this card */
        dev->lun = lun;
        strncpy(dev->sysfs_path, path, CVORB_PATH_SIZE);

        /* Read once static data from the Hardware:  hw_version and pcb_id */
        len = snprintf(path, CVORB_PATH_SIZE, "%s/%s", dev->sysfs_path, "hw_version");
        path[len] = '\0';
        if(cvorbdev_get_attr_char(path, dev->hw_version, CVORB_HW_VER_SIZE))
                goto out_free;
        len = snprintf(path, CVORB_PATH_SIZE, "%s/%s", dev->sysfs_path, "pcb_id");
        path[len] = '\0';
        if(cvorbdev_get_attr_uint64(path, &dev->pcb_id))
                goto out_free;

        /* Create a file descriptor */
        cvorbdev_get_devname(lun, devname, sizeof(devname));
        len = snprintf(path, CVORB_PATH_SIZE, "/dev/%s", devname);
        path[len] = '\0';
        dev->fd = open(path, O_RDWR, 0);
        if (dev->fd < 0)
                goto out_free;

        LIBCVORB_DEBUG(4, "opened device %d on handle %p, fd %d\n", lun, dev, dev->fd);
        return dev;

out_free:
        if (dev)
                free(dev);
        __cvorb_libc_error(__func__);
        return NULL;
}

/**
 * @brief Close a CVORB device handle
 * @param device        - CVORB device handle
 *
 * @return 0 on success, -1 on failure
 */
int cvorb_close(cvorb_t *device)
{
        int ret;

        LIBCVORB_DEBUG(4, "handle %p\n", device);
        ret = close(device->fd);
        if (ret < 0)
                __cvorb_libc_error(__func__);

        free(device);
        return ret;
}

/**
 * @brief Get the hardware version of the board
 * @param device           - CVORB device handle
 * @param[out] hw_version  - output buffer allocated by the caller used to return the hw_version string
 * @param size             - hw_version buffer size
 *
 * @return 0 on success, -1 on failure
 */
int cvorb_get_hw_version(cvorb_t *device, char *hw_version, unsigned int size) {
        LIBCVORB_DEBUG(4, "handle %p\n", device);
        if (size < CVORB_HW_VER_SIZE)
        {
                __cvorb_lib_error(__func__, -ENOBUFS);
                return -1;
        }
        strncpy(hw_version, device->hw_version, CVORB_HW_VER_SIZE);
        return 0;
}

/**
 * @brief Get the pcb_id of the board
 * @param device        - CVORB device handle
 * @param[out] pcb_id   - output parameter used to return the pcb_id value
 *
 * @return 0 on success (cannot failed)
 */
int cvorb_get_pcb_id(cvorb_t *device, uint64_t *pcb_id) {
        *pcb_id = device->pcb_id;
        return 0;
}

/**
 * @brief Get the temperature of the board
 * @param device        - CVORB device handle
 * @param[out] temp     - output parameter used to return the temperature value
 *
 * @return 0 on success, -1 on failure
 */
int cvorb_get_temperature(cvorb_t *device, int *temp) {
        int ret, val, len;
        char attr_path[CVORB_PATH_SIZE];

        LIBCVORB_DEBUG(4, "handle %p\n", device);
        len = snprintf(attr_path, CVORB_PATH_SIZE, "%s/%s", device->sysfs_path, "temperature");
        attr_path[len] = '\0';
        ret = cvorbdev_get_attr_int32(attr_path, &val);
        if (ret < 0)
        {
                __cvorb_libc_error(__func__);
                return -1;
        }
        *temp = val;
        return 0;
}

/**
 * @brief Set the input pulse polarity for the given submodule of the board
 * @param device        - CVORB device handle
 * @param submodule     - desired submodule (0 or 1)
 * @param polarity      - desired polarity
 *
 * @return 0 on success, -1 on failure
 */
int cvorb_sm_set_input_polarity(cvorb_t *device, unsigned int submodule, enum cvorb_input_polarity polarity) {
        int ret, len;
        char attr_path[CVORB_PATH_SIZE];

        LIBCVORB_DEBUG(4, "handle %p\n", device);
        if (submodule!=0 && submodule!=1)
        {
                __cvorb_lib_error(__func__, -EINVAL);
                return -1;
        }
        len = snprintf(attr_path, CVORB_PATH_SIZE, "%s/submodule.%d/%s", device->sysfs_path, submodule, "input_polarity");
        attr_path[len] = '\0';
        ret = cvorbdev_set_attr_uint32(attr_path, polarity);
        if (ret < 0) {
                __cvorb_libc_error(__func__);
                return -1;
        }
        return 0;
}

/**
 * @brief Get the input pulse polarity for the given submodule of the board
 * @param device        - CVORB device handle
 * @param submodule     - desired submodule (0 or 1)
 * @param[out] polarity - output parameter used to return the current polarity value
 *
 * @return 0 on success, -1 on failure
 */
int cvorb_sm_get_input_polarity(cvorb_t *device, unsigned int submodule, enum cvorb_input_polarity *polarity) {
        int ret, val, len;
        char attr_path[CVORB_PATH_SIZE];

        LIBCVORB_DEBUG(4, "handle %p\n", device);
        if (submodule!=0 && submodule!=1)
        {
                __cvorb_lib_error(__func__, -EINVAL);
                return -1;
        }
        len = snprintf(attr_path, CVORB_PATH_SIZE, "%s/submodule.%d/%s", device->sysfs_path, submodule, "input_polarity");
        attr_path[len] = '\0';
        ret = cvorbdev_get_attr_int32(attr_path, &val);
        if (ret < 0) {
                __cvorb_libc_error(__func__);
                return -1;
        }
        *polarity = val;
        return 0;
}

/**
 * @brief Get the status for the given submodule of the board
 * @param device        - CVORB device handle
 * @param submodule     - desired submodule (0 or 1)
 * @param[out] status   - output parameter used to return the current status
 *
 * @return 0 on success, -1 on failure
 */
int cvorb_sm_get_status(cvorb_t *device, unsigned int submodule, unsigned int *status) {
        unsigned int val;
        int ret, len;
        char attr_path[CVORB_PATH_SIZE];

        LIBCVORB_DEBUG(4, "handle %p\n", device);
        if (submodule!=0 && submodule!=1)
        {
                __cvorb_lib_error(__func__, -EINVAL);
                return -1;
        }
        len = snprintf(attr_path, CVORB_PATH_SIZE, "%s/submodule.%d/%s", device->sysfs_path, submodule, "status");
        attr_path[len] = '\0';
        ret = cvorbdev_get_attr_uint32(attr_path, &val);
        if (ret < 0) {
                __cvorb_libc_error(__func__);
                return -1;
        }
        *status = val;
        return 0;
}

/**
 * @brief Allows to send a software trigger like a start, stop, event_start and event_stop for the given submodule of the board
 * @param device        - CVORB device handle
 * @param submodule     - desired submodule (0 or 1)
 * @param polarity      - desired software trigger
 *
 * @return 0 on success, -1 on failure
 */
int cvorb_sm_set_trigger(cvorb_t *device, unsigned int submodule, enum cvorb_trigger trigger) {
        int ret, len;
        char attr_path[CVORB_PATH_SIZE];
        char attr_name[32];

        LIBCVORB_DEBUG(4, "handle %p\n", device);
        if (submodule!=0 && submodule!=1)
        {
                __cvorb_lib_error(__func__, -EINVAL);
                return -1;
        }
        switch (trigger) {
                case CVORB_START:
                        strncpy(attr_name, "start", 32);
                        break;
                case CVORB_STOP:
                        strncpy(attr_name, "stop", 32);
                        break;
                case CVORB_EVT_START:
                        strncpy(attr_name, "event_start", 32);
                        break;
                case CVORB_EVT_STOP:
                        strncpy(attr_name, "event_stop", 32);
                        break;
                default:
                        __cvorb_lib_error(__func__, -EINVAL);
                        return -1;
        }
        len = snprintf(attr_path, CVORB_PATH_SIZE, "%s/submodule.%d/%s", device->sysfs_path, submodule, attr_name);
        attr_path[len] = '\0';
        ret = cvorbdev_set_attr_uint32(attr_path, 1);
        if (ret < 0) {
                __cvorb_libc_error(__func__);
                return -1;
        }
        return 0;
}

static int cvorb_get_ch_attribute_path(cvorb_t *dev, unsigned int chnr, char* attr_name, char* attr_path)
{
        int len, hw_chnr, smnr;

        /* check input parameters */
        if (!WITHIN_RANGE(1, chnr, CVORB_MAX_CH_NR))
        {
                __cvorb_lib_error(__func__, -EINVAL);
                return -1;
        }
        /* channel number in the hardware is in the range [0,7]*/
        hw_chnr = (chnr>8) ? chnr-9 : chnr-1;
        /* submodule number*/
        smnr = (chnr>8) ? 1 : 0;
        len = snprintf(attr_path, CVORB_PATH_SIZE, "%s/submodule.%d/channel.%d/%s",
                        dev->sysfs_path, smnr, hw_chnr, attr_name);
        attr_path[len] = '\0';
        return 0;
}

/**
 * @brief Get the status of the given channel.
 * @param device        - CVORB device handle
 * @param submodule     - desired channel in the range [1,16]
 * @param[out] status   - output parameter used to return the current status
 *
 * @return 0 on success, -1 on failure
 */
int cvorb_ch_get_status(cvorb_t *dev, unsigned int chnr, unsigned int *status) {
        unsigned int val;
        int ret;
        char attr_path[CVORB_PATH_SIZE];

        LIBCVORB_DEBUG(4, "handle %p\n", dev);
        ret = cvorb_get_ch_attribute_path(dev, chnr, "status", attr_path);
        if (ret < 0)
                return -1;
        ret = cvorbdev_get_attr_uint32(attr_path, &val);
        if (ret < 0) {
                __cvorb_libc_error(__func__);
                return -1;
        }
        *status = val;
        return 0;
}

/**
 * @brief Enable the given channel.
 * @param device        - CVORB device handle
 * @param submodule     - desired channel in the range [1,16]
 *
 * @return 0 on success, -1 on failure
 */
int cvorb_ch_enable(cvorb_t *dev, unsigned int chnr) {
        int ret;
        char attr_path[CVORB_PATH_SIZE];

        LIBCVORB_DEBUG(4, "handle %p\n", dev);
        ret = cvorb_get_ch_attribute_path(dev, chnr, "enable", attr_path);
        if (ret < 0)
                return -1;
        ret = cvorbdev_set_attr_uint32(attr_path, 1);
        if (ret < 0) {
                __cvorb_libc_error(__func__);
                return -1;
        }
        return 0;
}

/**
 * @brief Disable the given channel.
 * @param device        - CVORB device handle
 * @param submodule     - desired channel in the range [1,16]
 *
 * @return 0 on success, -1 on failure
 */
int cvorb_ch_disable(cvorb_t *dev, unsigned int chnr) {
        int ret;
        char attr_path[CVORB_PATH_SIZE];

        LIBCVORB_DEBUG(4, "handle %p\n", dev);
        ret = cvorb_get_ch_attribute_path(dev, chnr, "enable", attr_path);
        if (ret < 0)
                return -1;
        ret = cvorbdev_set_attr_uint32(attr_path, 0);
        if (ret < 0) {
                __cvorb_libc_error(__func__);
                return -1;
        }
        return 0;
}

/**
 * @brief Get the repeat count defining the number of time a function will be generated for the given channel. By default this value is 1.
 * @param device        - CVORB device handle
 * @param chnr          - desired channel in the range [1,16]
 * @param[out] count    - output parameter used to return the current repeat count
 *
 * @return 0 on success, -1 on failure
 */
int cvorb_ch_get_repeat_count(cvorb_t *dev, unsigned int chnr, unsigned int *count) {
        unsigned int val;
        int ret;
        char attr_path[CVORB_PATH_SIZE];

        LIBCVORB_DEBUG(4, "handle %p\n", dev);
        ret = cvorb_get_ch_attribute_path(dev, chnr, "repeat_count", attr_path);
        if (ret < 0)
                return -1;
        ret = cvorbdev_get_attr_uint32(attr_path, &val);
        if (ret < 0) {
                __cvorb_libc_error(__func__);
                return -1;
        }
        *count = val;
        return 0;
}

/**
 * @brief Set the repeat count defining the number of time a function will be generated for the given channel. By default this value is 1.
 * @param device        - CVORB device handle
 * @param chnr          - desired channel in the range [1,16]
 * @param count         - the new repeat count
 *
 * @return 0 on success, -1 on failure
 */
int cvorb_ch_set_repeat_count(cvorb_t *dev, unsigned int chnr, unsigned int count) {
        int ret;
        char attr_path[CVORB_PATH_SIZE];

        LIBCVORB_DEBUG(4, "handle %p\n", dev);
        ret = cvorb_get_ch_attribute_path(dev, chnr, "repeat_count", attr_path);
        if (ret < 0)
                return -1;
        ret = cvorbdev_set_attr_uint32(attr_path, count);
        if (ret < 0) {
                __cvorb_libc_error(__func__);
                return -1;
        }
        return 0;
}

/**
 * @brief Get the function enable mask for the given channel
 * @param device        - CVORB device handle
 * @param chnr          - desired channel in the range [1,16]
 * @param[out] enbl_mask    - output parameter used to return the function enable mask
 *
 * @return 0 on success, -1 on failure
 */
int cvorb_ch_get_enable_fcn_mask( cvorb_t *dev, unsigned int chnr, uint64_t *enbl_mask)
{
        char attr_path[CVORB_PATH_SIZE];
        int ret;
        uint64_t val;
        ret = cvorb_get_ch_attribute_path(dev, chnr, "enable_fcn_mask", attr_path);
        if (ret < 0)
                return ret;
        ret = cvorbdev_get_attr_uint64(attr_path, &val);
        if (ret < 0) {
                __cvorb_libc_error(__func__);
        }
        *enbl_mask = val;
        return ret;
}

/**
 * Utility function in charge to compute hw vector in terms of ss: step size and nos: number of steps
 * in order to reach the end point starting from the start point
 * return 0 on success, -1 in case of failure
 */
static int compute_hw_vector(struct cvorb_vector_fcn *start_pt,  struct cvorb_vector_fcn *end_pt, uint16_t *ss, uint16_t *nos)
{
        double lldt;
        uint64_t lldt_int, max_vector_time;

        if (end_pt->t == 0) {
                /* internal stop */
                *ss = (uint16_t)-1;
                *nos = (uint16_t)0;
                return 0;
        }
        if (end_pt->t <= start_pt->t) {
                /* illegal vector table */
                __cvorb_lib_error(__func__, -LIBCVORB_EINVTR);
                return -1;
        }
        /* compute delta time between "end" and "start" in us to not loose precision */
        lldt = (end_pt->t - start_pt->t)*1000;
        /* Handle the MAX time between two vectors
           (2hrs 58min 56sec 926ms 725us)
           Vector should be split in two vectors.
           (see 4.2 Vector definition in CVORB Tech Guide) (1<<16)-1)*((1<<15)-1)*5 CVORB_MAX_VECTOR_TIME */
        /* HW function MAX possible time between 2 vectors = 2h58mn57s */
        max_vector_time = (uint64_t)(CVORB_MAX_STEPS*CVORB_MAX_STEP_SIZE)*CVORB_MIN_STEP_SIZE;
        if (lldt > max_vector_time) {
                __cvorb_lib_error(__func__, -LIBCVORB_EVTRTOOLONG);
                return -1;
        }
        /* first -- round to the nearest multiple of
           MIN_STEP (5us) size */
        lldt_int = CVORB_MIN_STEP_SIZE * (uint64_t)(lldt / CVORB_MIN_STEP_SIZE + 0.5);

        /* get minimum step size needed to hit dt range */
        *ss = ((lldt_int-1)/CVORB_MAX_TIME_MIN_STEP) + 1;

        /* Round delta t if needed.
           Ensure step size (in us) is a factor of dt,
           i.e. no remainder */
        lldt_int -= lldt_int % (*ss * CVORB_MIN_STEP_SIZE);

        /* compute number of steps */
        *nos = lldt_int/(*ss * CVORB_MIN_STEP_SIZE);

        return 0;
}

/* compute hw function size in 32 bits word */
static int get_hw_fcn_size32(unsigned int n_vector)
{
        unsigned int sz;
        /* A function is encoded in the HW as a table of 2 bytes word and is organized this way:  */
        /* Header: 3 words                                                                        */
        /* Vectors: 3 words * nb_of_vectors                                                       */
        /*           (vector is encoded using 3 words:amplitude, nr of steps, step size)          */
        /* Footer: 6 words (12 bytes)                                                             */
        /* compute HW fcn size in 32 bits word */
        sz = CVORB_FCT_HEADER_SZ + (n_vector*CVORB_VCT_SZ);
        if (sz & 1)
                ++sz; /*should be even number of 16 bits words (in order to be 32 bits word aligned)*/
        sz += CVORB_FOOTER_SZ;
        /* convert in 32 bits words which is the SRAM unit*/
        sz >>= 1;
        return sz;
}

/**
 * @brief Load Channel Function in device SRAM.
 *
 * @param dev      - CVORB device handle
 * @param chnr     - desired channel [1-16]
 * @param fcnnr    - desired function number to set [1-64]
 * @param value    - function to load \n
 *                      MAX allowed vector amount in this array -- is
 *                      @ref CVORB_MAX_VECTOR \n
 *                      Element[0] -- should be @b always [t0 ==0, V0] \n
 *                      Actual element amount passed by the user in @b n_vector parameter
 *                      can be smaller.
 * @param n_vector - how many data elements(vectors) were passed by the user
 *                      (including value[0] -- which is @b always [t0==0 V0])
 *
 * See struct cvorb_vector_fcn description for more details on the data types
 * Channel function can hold up to @ref CVORB_MAX_VECTOR.
 * Element[0] should @b ALWAYS hold start time == 0 and
 * initial physical value. \n
 *
 * @b NOTE on rounding down time between two vectors. \n
 *
 * MIN step possible between two vectors is 5us.
 * Every 327.675ms -- step granularity is lowered by 5us.
 * I.e the finest time step resolution possible only if dt is less
 * then 327.675ms.
 * For example, in 2nd dt range [327.680 - 655.350]ms -- step will be 10us.
 * It means that dt requested should be multipe of 10.
 * I.e. if you'll request let's say dt to be 400.355ms -- it will be rounded
 * down to 400.350ms.
 *
 * In 3rd time range -- dt must be multiple of 15 and so on...
 *
 * If it is not the case, (i.e. dt is not a multiple of current step size)
 * -- time between vectors will be rounded down to be
 * divisible by step size of the current range.
 * Step size will always be a factor of dt.
 *
 * @return 0 on success, -1 on failure
 */
int cvorb_ch_set_fcn( cvorb_t *dev, unsigned int chnr, unsigned int fcnnr, struct cvorb_vector_fcn *value, unsigned int n_vector) {
        struct cvorb_hw_fcn fcn;
        unsigned int sz32, i, res=0, n_vtr;
        uint16_t *vp, ss, nos;
        int ioctl_call = 1; /* set_fcn is implemented via sysfs to be symetric with get_function*/
#if __CVORB_DEBUG__
        uint16_t *lp;
#endif
        int ret, len;
        char attr_path[CVORB_PATH_SIZE];

        /* check input parameters */
        if ( (!WITHIN_RANGE(1, chnr, CVORB_MAX_CH_NR)) ||
             (!WITHIN_RANGE(1, fcnnr, CVORB_MAX_FCT_NR)) ||
             (n_vector > CVORB_MAX_VECTOR) )
        {
                __cvorb_lib_error(__func__, -EINVAL);
                return -1;
        }
        /* the real number of vectors should exclude the first element */
        /* which just specifies the initial amplitude */
        n_vtr = n_vector -1;
        sz32 = get_hw_fcn_size32(n_vtr);
        fcn.hw_fcn = calloc(sz32, sizeof(uint32_t));
        fcn.hw_fcn_size32 = sz32;
        fcn.n_vector = n_vtr;
        /* channel number in the hardware is in the range [0,7]*/
        fcn.channr = (chnr>8) ? chnr-9 : chnr-1;
        fcn.submodulenr = (chnr>8) ? 1 : 0;
        /* function number in the HW is in the range [0,63] */
        fcn.fcnnr = fcnnr-1;

        vp = (uint16_t *)fcn.hw_fcn;
        vp[0] = n_vtr;
        vp[1] = (uint16_t)0;
        vp[2] = value[0].v; // set V0
        for (i = 1, vp +=3; i <= n_vtr; ++i, vp += 3) {
                res = compute_hw_vector(&value[i-1], &value[i], &ss, &nos);
                if (res)
                        goto error;
                vp[0] = nos;
                vp[1] = ss;;
                vp[2] = value[i].v;
        }

#if __CVORB_DEBUG__
        printf("cvorb_ch_set_fcn hw-function:\n");
        vp = (uint16_t *)fcn.hw_fcn;
        printf("N of vectors[0]: %d | not used[1]:%d | V0[2]: %d\n", vp[0], vp[1], vp[2]);
        for(i=1, vp +=3; i<=vp[0]; ++i, vp+=3) {
                printf ("nos: %d | ss: %d | amp.: %d\n", vp[0], vp[1], vp[2]);
        }
        printf("footer:\n");
        /* address of the last 16 bits word */
        lp = ((uint16_t *)fcn.hw_fcn + (sz32*2))-1;
        for (; vp<=lp; ++vp)
                printf("%d,", *vp);
        printf("\n");
#endif
        /* call the driver */
        if (ioctl_call) /* via ioctl */
        {
                if (ioctl(dev->fd, CVORB_IOCTL_SET_FCT, &fcn)) {
                        __cvorb_libc_error(__func__);
                        res = -1;
                }
        }
        else /* via sysfs: driver implements set function via sysfs but by default ioctl is used */
        {
                len = snprintf(attr_path, CVORB_PATH_SIZE, "%s/submodule.%d/channel.%d/%s",
                                dev->sysfs_path, fcn.submodulenr, fcn.channr, "load_fcn");
                attr_path[len] = '\0';
                ret = cvorbdev_set_attr_bin(attr_path, (void *)&fcn, sizeof(struct cvorb_hw_fcn));
                if (ret < 0) {
                        __cvorb_libc_error(__func__);
                        res = -1;
                }
        }

error:
        /* free allocated buffer */
        free (fcn.hw_fcn);
        return res;
}

 /**
 * @brief Get channel function from the board (SRAM)
 * @param dev           - CVORB device handle
 * @param chnr          - desired channel number [1,16]
 * @param fcnnr         - desired function number [1,64]
 * @param[out] buff     - buffer allocated by the caller
 * @param n_vector      - buffer capacity in terms of number of vector elements including the first element which should be @b always [t0 ==0, V0].\n
 *                              NB: if the real number of vectors of the function is unknown \n
 *                              the caller should allocate a buffer of CVORB_MAX_VECTOR size
 * @param[out] fcn_sz   - actual function vector count
 * @return 0 on success, -1 on failure
 */
int cvorb_ch_get_fcn( cvorb_t *dev, unsigned int chnr, unsigned int fcnnr, struct cvorb_vector_fcn *buff, unsigned int n_vector, unsigned int *fcn_sz) {
        struct cvorb_hw_fcn fcn;
        unsigned int sz32, i, res=0, n_vtr, len;
        uint16_t *vp;
        char attr_path[CVORB_PATH_SIZE];

        /* check input parameters */
        if ( (!WITHIN_RANGE(1, chnr, CVORB_MAX_CH_NR)) ||
             (!WITHIN_RANGE(1, fcnnr, CVORB_MAX_FCT_NR)) )
        {
                __cvorb_lib_error(__func__, -EINVAL);
                return -1;
        }
        /* the real number of vectors should exclude the first element */
        /* which just specifies the initial amplitude */
        n_vtr = n_vector -1;
        sz32 = get_hw_fcn_size32(n_vtr);
        fcn.hw_fcn = calloc(sz32, sizeof(uint32_t));
        fcn.hw_fcn_size32 = sz32;
        /* channel number in the hardware is in the range [0,7]*/
        fcn.channr = (chnr>8) ? chnr-9 : chnr-1;
        fcn.submodulenr = (chnr>8) ? 1 : 0;
        /* The driver check if the resident function in the HW has not more vectors than the client buffer capacity */
        fcn.n_vector = n_vtr;
        /* function index in the HW is in the range [0-63] */
        fcn.fcnnr = fcnnr-1;

        /* call the driver : read function cannot be implemented via sysfs because */
        /* reading a function requires an input parameter to specify submod,ch,etc... */
        if (ioctl(dev->fd, CVORB_IOCTL_GET_FCT, &fcn)) {
            __cvorb_libc_error(__func__);
            res = -1;
            goto error;
        }

#if __CVORB_DEBUG__
        printf("cvorb_ch_get_fcn hw-function:\n");
        vp = (uint16_t *)fcn.hw_fcn;
        printf("N of vectors: %d | V0: %d\n", vp[0], vp[2]);
        for(i=1, vp +=3; i<=fcn.n_vector; ++i, vp+=3) {
                printf ("nos: %d | ss: %d | amp.: %d\n", vp[0], vp[1], vp[2]);
        }
#endif
        *fcn_sz = fcn.n_vector+1;
        /* Convert HW function table into user function vector table */
        vp = (uint16_t *)fcn.hw_fcn;
        buff[0].t = 0;
        buff[0].v = vp[2];
        for (i = 1, vp +=3; i <= fcn.n_vector; ++i, vp += 3) {
                if (vp[1] == (uint16_t)-1)
                        buff[i].t = 0;
                else
                        buff[i].t = ((vp[0]*vp[1]*CVORB_MIN_STEP_SIZE)/1000) + buff[i-1].t;
                buff[i].v = vp[2];
        }

error:
        /* free allocated buffer */
        free (fcn.hw_fcn);
        return res;
}

static int cvorb_get_fcn_attribute_path(cvorb_t *dev, unsigned int chnr, unsigned int fcnnr,
                char* attr_name, char* attr_path)
{
        int len, hw_chnr, smnr;

        /* check input parameters */
        if ( (!WITHIN_RANGE(1, chnr, CVORB_MAX_CH_NR)) ||
             (!WITHIN_RANGE(1, fcnnr, CVORB_MAX_FCT_NR)) )
        {
                __cvorb_lib_error(__func__, -EINVAL);
                return -1;
        }
        LIBCVORB_DEBUG(4, "handle %p\n", dev);
        /* channel number in the hardware is in the range [0,7]*/
        hw_chnr = (chnr>8) ? chnr-9 : chnr-1;
        smnr = (chnr>8) ? 1 : 0;

        len = snprintf(attr_path, CVORB_PATH_SIZE, "%s/submodule.%d/channel.%d/%s",
                        dev->sysfs_path, smnr, hw_chnr, attr_name);
        attr_path[len] = '\0';
        return 0;
}

/**
 * @brief Enable a function for the given channel.
 * @param dev           - CVORB device handle
 * @param chnr          - desired channel in the range [1,16]
 * @param fcnr          - desired function in the range [1,64]
 *
 * @return 0 on success, -1 on failure
 */
int cvorb_ch_enable_fcn( cvorb_t *dev, unsigned int chnr, unsigned int fcnnr)
{
        char attr_path[CVORB_PATH_SIZE];
        int ret;
        ret = cvorb_get_fcn_attribute_path(dev, chnr, fcnnr, "enable_fcn", attr_path);
        if (ret < 0)
                return ret;
        /*hw_fcn number is in teh range [0,63] */
        ret = cvorbdev_set_attr_uint32(attr_path, fcnnr-1);
        if (ret < 0) {
                __cvorb_libc_error(__func__);
        }
        return ret;
}

/**
 * @brief Disable a function for the given channel.
 * @param dev           - CVORB device handle
 * @param chnr          - desired channel in the range [1,16]
 * @param fcnr          - desired function in the range [1,64]
 *
 * @return 0 on success, -1 on failure
 */
int cvorb_ch_disable_fcn( cvorb_t *dev, unsigned int chnr, unsigned int fcnnr)
{
        char attr_path[CVORB_PATH_SIZE];
        int ret;
        ret = cvorb_get_fcn_attribute_path(dev, chnr, fcnnr, "disable_fcn", attr_path);
        if (ret < 0)
                return ret;
        /*hw_fcn number is in teh range [0,63] */
        ret = cvorbdev_set_attr_uint32(attr_path, fcnnr-1);
        if (ret < 0) {
                __cvorb_libc_error(__func__);
        }
        return ret;
}

/**
 * @brief Select a function for the given channel.
 * @param dev           - CVORB device handle
 * @param chnr          - desired channel in the range [1,16]
 * @param fcnr          - desired function in the range [1,64]
 *
 * @return 0 on success, -1 on failure
 */
int cvorb_ch_select_fcn( cvorb_t *dev, unsigned int chnr, unsigned int fcnnr)
{
        char attr_path[CVORB_PATH_SIZE];
        int ret;
        ret = cvorb_get_fcn_attribute_path(dev, chnr, fcnnr, "select_fcn", attr_path);
        if (ret < 0)
                return ret;
        /*hw_fcn number is in teh range [0,63] */
        ret = cvorbdev_set_attr_uint32(attr_path, fcnnr-1);
        if (ret < 0) {
                __cvorb_libc_error(__func__);
        }
        return ret;
}

/**
 * @brief Get the submodule number corresponding to the channel
 * @param ch_nr         - desired channel number in the range [1,16]
 * @param sm_nr[out]    - output parameter used to return the submodule number.
 *
 * @return 0 on success, -1 on failure
 */
int cvorb_get_submodule_nr(int ch_nr, int *sm_nr)
{
        /* check input parameters */
        if ( !WITHIN_RANGE(1, ch_nr, CVORB_MAX_CH_NR))
        {
                __cvorb_lib_error(__func__, -EINVAL);
                return -1;
        }
        *sm_nr = (ch_nr>8) ? 1 : 0;
        return 0;
}
