#ifndef _CVORB_USER_DEFINED_SIM_H_INCLUDE_
#define _CVORB_USER_DEFINED_SIM_H_INCLUDE_

#include "CvorbSim.h"

struct sel; /* preliminary structure declaration to supress warnings during
	       user code compilation */

/* user-defined statics data table for CVORB module */
struct CVORBUserStatics_t {
  /*
    +=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=+
    |                     insert your new members here                        |
    | NOTE! All that will be placed here should be managed by you, and you    |
    | alone!!! All the pointers-related memory allocation and deallocation    |
    | should be done by you. It will NOT be done automatically! If the        |
    | pointer is declared - it should be initialized properly by allocating   |
    | a new memory (using sysbrk) or by assigning it a correct and valid      |
    | address. Don't forget to free allocated memory (if any) in the          |
    | uninstallation procedure! Have fun (-;                                  |
    +=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=+
  */
};



/*
  +-------------------------------+
  | INSERT USER-DEFINED CODE HERE |
  +-------------------------------+
*/


int CvorbUserOpen(int*, register CVORBStatics_t*, int, struct file*);
int CvorbUserClose(int*, register CVORBStatics_t*, struct file*);
int CvorbUserRead(int*, register CVORBStatics_t*, struct file*, char*, int);
int CvorbUserWrite(int*, register CVORBStatics_t*, struct file*, char*, int);
int CvorbUserSelect(int*, register CVORBStatics_t*, struct file*, int, struct sel*);
int CvorbUserIoctl(int*, register CVORBStatics_t*, struct file*, int, int, char*);
char* CvorbUserInst(int*, register DevInfo_t*, register CVORBStatics_t*);
int CvorbUserUnInst(int*, CVORBStatics_t*);

#endif /* _CVORB_USER_DEFINED_SIM_H_INCLUDE_ */
