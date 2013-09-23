#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* #define Tg8Conv      //select the Tg8 convertor */

#define BUFFERED 1      /*1=buffered I/o */
#define local static

 typedef char * string;
 typedef unsigned char  byte;
 typedef unsigned short word;

#define UndefinedNameType 255 /*name is referred only - not declared yet */
#define KeyWordType       0

 typedef struct { /*Predfined symbols */
  char *name;   /* symbol name */
  short type;   /* its type */
  int   val;    /* its value */
 } SymDef;

 typedef struct symbol { /*Symbol table format */
  short Type;           /*type of symbol: SYM|NUM |TYPE|STRU|UNIO|ENU |keyword */
  byte  VarType,        /*variable type: char ... or my specific */
	VarPtrLevel;    /*         level of pointer: >=0 */
  int   VarSpec;        /*         specific attributes (mode of access, etc) */
  int   VarDim;         /*         dimension: >=1 */
  int   Val;            /*its value */
  void *Ext;            /*    and extended value (detailed descriptor, etc) */
  struct symbol *TypeDef; /*Type definition for variable (NULL for type itself) */
  struct symbol *Block;   /*Block descriptor */
  struct symbol *Next,    /*Next entry into symbol table */
		*Prev;
  char *Name;             /*Name pointer */
 } symbol;

 local symbol *symtab =NULL,    /*symbol table begin address */
	      *symtop =NULL,    /*symbol table top (last) address */
	      *nameptr=NULL,    /*pointer to the last selected name */
	      *idnptr =NULL,    /*pointer to the last selected identifier */
	      *blkptr =NULL;    /*pointer to the last selected  block */

 local int  nline  =1,   /*current line number */
	    compfl =1,   /*compilation enable flag for conditional blocks */
	    complv =0,   /*condition compilation level (nested IFs) */
	    complev=0,   /*last compiled conditional block level (0 or more) */
	    inclev =0;   /*level of nested include-files */

 local short error =0,   /*error flag */
	     errors=0;   /*total number of errors */

#define MAXLEVEL 8              /*Max. level of nesting include-files */

#if BUFFERED==0
#define GetString(s,l) fgets(s,l,inpf)
#define GetChar()      {int c=fgetc(inpf); if (c=='\n') nline++; return c;}
#define UnGetChar(c)   ungetc(c,inpf)           /*put character back to stream */
#define GetHexNum(p)   fscanf(inpf,"%x",p)      /*read integer number in hexadecimal format */
#define GetOctalNum(p) fscanf(inpf,"%o",p)      /*                       octal              */
#define GetNum(p)      fscanf(inpf,"%d",p)      /*                       decimal            */

#else                   /*buffered i/o */
 local int f_line, f_err;
 local char f_string[256],f_hist[256], *f_pos, f_char;
 char *GetString(char *s,int l);

#endif /*-BUFFERED*/

#define  SkipBlanks()  {while ((c=GetChar())==' ' || c=='\t');}

 local FILE *inpf;                      /*Current compilated input file pointer */
 local struct FilePos {
   FILE *File;          /*Input file pointers for any included files */
   int   Pos;           /*Position in file (line number) */
} inpfiles [MAXLEVEL+1];

 local char symb[128];          /*Symbol or line is saved here */
 local char NoName[]="\0\0";    /*Empty name */
 local char dyname[]="a?\0";    /*Dynamical name building for a blocks: a? b? ... */
 local char *progname;          /*Program name */

/*--------- Prototypes and external functions ----------*/
symbol *LookUnique(), *LookUp(), *Insert(), *FindByVal(), *FindByName(),
       *MoreSymbols(),*MoreNames();
void Error(char *fmt, ...);
FILE *InitParser(int argc,char *argv[]);

#define TopLevelBlock 0

/* --yacc.h-- */
