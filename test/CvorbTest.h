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

#define BLOCK00_NUM_REGISTERS 21
#define BLOCK01_NUM_REGISTERS 1
#define BLOCK02_NUM_REGISTERS 1
#define BLOCK03_NUM_REGISTERS 1
#define BLOCK04_NUM_REGISTERS 1
#define BLOCK05_NUM_REGISTERS 1
#define BLOCK06_NUM_REGISTERS 1
#define BLOCK07_NUM_REGISTERS 1
#define BLOCK08_NUM_REGISTERS 1
#define BLOCK09_NUM_REGISTERS 7
#define BLOCK10_NUM_REGISTERS 1

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
int Block02(HANDLE);
int Block03(HANDLE);
int Block04(HANDLE);
int Block05(HANDLE);
int Block06(HANDLE);
int Block07(HANDLE);
int Block08(HANDLE);
int Block09(HANDLE);
int Block10(HANDLE);

#endif /* _TEST_H_INCLUDE_ */
