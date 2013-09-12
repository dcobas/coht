#ifndef _CVORG_LIBINTERNAL_H_
#define _CVORG_LIBINTERNAL_H_

#include "libcvorg.h"
#include "cvorg_errno.h"

extern int __cvorg_loglevel;
extern int __cvorg_errno;
extern int __cvorg_init;

void __cvorg_libc_error(const char *string);
void __cvorg_internal_error(int error_number);

#define LIBCVORG_DEBUG(level, format, args...) \
	do { \
		if (__cvorg_loglevel >= (level)) {	\
			fprintf(stderr, "%s: " format,	\
				__func__, ##args);	\
		}					\
	} while (0)

struct cvorg_calib {
	int *devicenr_list; /* device number list used to match device number in cvorg_open */
	uint32_t **dac_calib_list; /* array of pointer to dac calib values for each board */
};

static const int32_t cvorg_gains[] = {
	[0]	= -22,
	[1]	= -16,
	[2]	= -10,
	[3]	= -4,
	[4]	= 2,
	[5]	= 8,
	[6]	= 14,
	[7]	= 20,
};
#define CVORG_NR_GAINS	(sizeof(cvorg_gains) / sizeof(cvorg_gains[0]))
#define CVORG_CALIB_FILE "/usr/local/cvorg/cvorgCalibration.csv"

#endif /* _CVORG_LIBINTERNAL_H_ */
