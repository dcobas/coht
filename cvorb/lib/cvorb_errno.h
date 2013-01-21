#ifndef _CVORB_ERRNO_H_
#define _CVORB_ERRNO_H_

#include <errno.h>

#define LIBCVORB_ERROR_BASE	0x3000
#define LIBCVORB_EINVTR		    ((LIBCVORB_ERROR_BASE)+1) /* Invalid vector*/
#define LIBCVORB_EVTRTOOLONG    ((LIBCVORB_ERROR_BASE)+2) /* Vector duration exceed HW limits */
#define LIBCVORB_ENODEV		    ((LIBCVORB_ERROR_BASE)+3) /* no such device */
#define LIBCVORB_ENOBUFS	    ((LIBCVORB_ERROR_BASE)+4) /* no User buffer space available */ 
#define LIBCVORB_ENOTSUPP	    ((LIBCVORB_ERROR_BASE)+5) /* Feature not supported*/

#endif /* _CVORB_ERRNO_H_ */
