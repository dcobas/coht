#ifndef _TEST_H_INCLUDE_
#define _TEST_H_INCLUDE_

#ifdef __linux__
#include <curses.h>
#include <term.h>
#else
extern int  tgetent(char*, char*);
extern char *tgetstr(char*, char**);
#endif

#ifndef OK
#define OK 0	/* return code */
#endif

#include "dg/dal.h"
#include "CvorbUserDefinedAccess.h"
#include "CvorbRegId.h"

#define WHITE_CLR	"\033[1;37m"
#define END_CLR		"\033[m"

#define BLOCK00_NUM_REGISTERS 18
#define BLOCK01_NUM_REGISTERS 7

#define EXTRANEOUS_NUM_REGISTERS 0
#define SERVICE_NUM_REGISTERS    6
#define WRITEONLY_NUM_REGISTERS  1

/* CvorbTestUser.c */
int UserDefinedMenu(HANDLE, int);

/* CvorbTest.c */
extern char *ClearScreen;
extern const char *g_drvrVersInfo;
extern int g_drvrVers;
int         screenHeight();
int         screenWidth();
int ServiceRegsOps(HANDLE);
int Extraneous(HANDLE);
int WORegsMenu(HANDLE);
int Block00(HANDLE);
int Block01(HANDLE);

#endif /* _TEST_H_INCLUDE_ */
