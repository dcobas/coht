#ifndef _CVORB_LIBINTERNAL_H_
#define _CVORB_LIBINTERNAL_H_

#include <libcvorb.h>
#include "cvorb_errno.h"

extern int __cvorb_loglevel;
extern int __cvorb_errno;

void __cvorb_internal_error(int error_number);
void __cvorb_param_error(int err);
void __cvorb_libc_error(const char *string);
void __cvorb_lib_error(const char* string, int err);

#define LIBCVORB_DEBUG(level, format, args...) \
	do { \
		if (__cvorb_loglevel >= (level)) {	\
			fprintf(stderr, "%s: " format,	\
				__func__, ##args);	\
		}					\
	} while (0)

#endif /* _CVORB_LIBINTERNAL_H_ */
