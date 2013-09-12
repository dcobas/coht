#ifndef _CVORG_ERRNO_H_
#define _CVORG_ERRNO_H_

#include "errno.h"

#define LIBCVORG_ERROR_BASE	0x3000
#define LIBCVORG_EINVAL		((LIBCVORG_ERROR_BASE)+1)
#define LIBCVORG_ENODEV		((LIBCVORG_ERROR_BASE)+2)
#define LIBCVORG_ENOTSUPP	((LIBCVORG_ERROR_BASE)+3)
#define LIBCVORG_NOCALIB	((LIBCVORG_ERROR_BASE)+4)

extern void __cvorg_lib_error(int err);
#endif /* _CVORG_ERRNO_H_ */
