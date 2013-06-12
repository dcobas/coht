/* ****************************************************************************** */
/* VMEIO test program, calls driver                                               */
/* Julian Lewis AB/CO/HT Julian.Lewis@cern.ch Oct 2010                            */
/* ****************************************************************************** */

#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>   /* ctime */

#include "vmeio_support.h"

#include "Cmds.h"

#ifndef COMPILE_TIME
#define COMPILE_TIME 0
#endif

static char *editor = "e";

/* ============================= */

char *defaultconfigpath = "/usr/local/drivers/vmeio/vmeiotest.config";

char *configpath = NULL;
char localconfigpath[128];  /* After a CD */

static char *drvr_name = DRIVER_NAME;
static int  reg     =  0;
static int  lun     =  0;
static int  dma     =  0;
static int  dmaswap =  0;
static int  win     =  1;
static int  dwd     =  4;
static int  debug   =  0;
static int  tmo     =  1000;
struct vmeio_get_window_s winpars;

static void *vmeio[DRV_MAX_DEVICES];

static char *mem   = NULL;
static int memlen  = 0;

struct regdesc {
   char name[32];
   char flags[4];
   int  offset;
   int  size;
   int  window;
   int  depth;
   struct regdesc *next;
};

char dev_name[32];
static struct regdesc *regs = NULL;
int reg_cnt = 0;

/* ============================= */

static char path[128];

char *GetFile(char *name) {
FILE *gpath = NULL;
char txt[128];
int i, j;

   if (configpath) {
      gpath = fopen(configpath,"r");
      if (gpath == NULL) {
	 configpath = NULL;
      }
   }

   if (configpath == NULL) {
      configpath = "./vmeiotest.config";
      gpath = fopen(configpath,"r");
      if (gpath == NULL) {
	 configpath = "/dsc/local/data/vmeiotest.config";
	 gpath = fopen(configpath,"r");
	 if (gpath == NULL) {
	    configpath = defaultconfigpath;
	    gpath = fopen(configpath,"r");
	    if (gpath == NULL) {
	       configpath = NULL;
	       sprintf(path,"./%s",name);
	       return path;
	    }
	 }
      }
   }

   bzero((void *) path,128);

   while (1) {
      if (fgets(txt,128,gpath) == NULL) break;
      if (strncmp(name,txt,strlen(name)) == 0) {
	 for (i=strlen(name); i<strlen(txt); i++) {
	    if (txt[i] != ' ') break;
	 }
	 j= 0;
	 while ((txt[i] != ' ') && (txt[i] != 0)) {
	    path[j] = txt[i];
	    j++; i++;
	 }
	 strcat(path,name);
	 fclose(gpath);
	 return path;
      }
   }

   fclose(gpath);
   sprintf(path,"./%s",name);
   return path;
}

/* ============================= */

struct regdesc *new_reg() {

struct regdesc *reg;

   reg = malloc(sizeof(struct regdesc));
   if (reg == NULL) return NULL;

   reg->next = regs;
   regs = reg;
   return regs;
}

/* ============================= */

struct regdesc *get_reg_by_offset(struct regdesc *start, int offset) {

struct regdesc *reg;

   reg = start;
   while (reg) {
      if ((reg->offset == offset)
      &&  (reg->window == win)) {
	 return reg;
      }
      reg = reg->next;
   }
   return NULL;
}

/* ============================= */

struct regdesc *get_reg_by_name(struct regdesc *start, char *name) {

struct regdesc *reg;

   reg = start;
   while (reg) {
      if ((strcmp(reg->name,name) == 0)
      &&  (reg->window == win)) {
	 return reg;
      }
      reg = reg->next;
   }
   return NULL;
}

/* ============================= */

void print_regs(int window) {
struct regdesc *reg;

   reg = regs;
   while (reg) {
      if (reg->window == window) {
	 if (reg->depth != 1) {
	    printf("%16s - %2s Wn:%d Of:0x%04X Sz:%d Dp:%d\n",
		   reg->name,
		   reg->flags,
		   reg->window,
		   reg->offset,
		   reg->size,
		   reg->depth);
	 } else {
	    printf("%16s - %2s Wn:%d Of:0x%04X Sz:%d\n",
		   reg->name,
		   reg->flags,
		   reg->window,
		   reg->offset,
		   reg->size);
	 }
      }
      reg = reg->next;
   }
}

/* ============================= */

void read_regs(char *name) {

int i, j;
char fn[32], txt[128], c, *cp;
FILE *nf = NULL;
struct regdesc *reg;

   if (regs) return;
   bzero((void *) fn, 32);
   for (i=0; i<31; i++) {
      if (i<strlen(name)) fn[i]=toupper(name[i]);
      else break;
   }

   strcat(fn,".regs");
   nf = fopen(GetFile(fn),"r");
   if (nf == NULL) {
      printf("%s:not available\n",fn);
      return;
   }

   while (1) {
      if (fgets(txt,128,nf) == NULL) break;
      cp = "NAME";
      if (strncmp(cp, txt, strlen(cp)) == 0) {
	 cp = &txt[strlen(cp)];
	 bzero((void *) dev_name, 32);
	 for (i=0, j=0; i<31; i++) {
	    c = *cp++;
	    if ((c == '\n') ||  (c == 0)) break;
	    if (isspace(c)) continue;
	    dev_name[j++] = c;
	 }
      }
      if (strncmp("$", txt, 1) == 0) {
	 reg = new_reg();

	 sscanf(txt,"$ %s %s 0x%X %d %d %d",
		reg->name,
		reg->flags,
		&reg->offset,
		&reg->size,
		&reg->window,
		&reg->depth);
	 reg_cnt++;
      }
   }
   print_regs(1);
   print_regs(2);
   printf("Device:%s %d:reg descriptions read\n",dev_name, reg_cnt);
}

/* ============================= */

static char *TimeToStr(int t) {

static char tbuf[128];

char tmp[128];
char *yr, *ti, *md, *mn, *dy;

   bzero((void *) tbuf, 128);
   bzero((void *) tmp, 128);

   if (t) {
      ctime_r ((time_t *) &t, tmp);  /* Day Mon DD HH:MM:SS YYYY\n\0 */

      tmp[3] = 0;
      dy = &(tmp[0]);
      tmp[7] = 0;
      mn = &(tmp[4]);
      tmp[10] = 0;
      md = &(tmp[8]);
      if (md[0] == ' ') md[0] = '0';
      tmp[19] = 0;
      ti = &(tmp[11]);
      tmp[24] = 0;
      yr = &(tmp[20]);

      sprintf (tbuf, "%s-%s/%s/%s %s"  , dy, md, mn, yr, ti);
    } else
      sprintf (tbuf, "--- Zero ---");
    return (tbuf);
}

/* ============================= */
/* News                          */

int News(int arg) {

char sys[128], npt[128];

   arg++;

   if (GetFile("vmeio_news")) {
      strcpy(npt,path);
      sprintf(sys,"%s %s",GetFile(editor),npt);
      printf("\n%s\n",sys);
      system(sys);
      printf("\n");
   }
   return(arg);
}

/* ============================= */
/* Batch mode controls YesNo     */

static int yesno=1;
int Batch(int arg) {

ArgVal   *v;
AtomType  at;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Numeric) {
      yesno = v->Number;
      arg++;
   }
   return arg;
}

/* ============================= */

static int YesNo(char *question, char *name) {
int yn, c;

   if (yesno == 0) return 1;

   printf("%s: %s\n",question,name);
   printf("Continue (Y/N):"); yn = getchar(); c = getchar();
   if ((yn != (int) 'y') && (yn != 'Y')) return 0;
   return 1;
}

/* ============================= */

int ChangeEditor(int arg) {
static int eflg = 0;

   arg++;
   if (eflg++ > 4) eflg = 1;

   if      (eflg == 1) editor = "e";
   else if (eflg == 2) editor = "emacs";
   else if (eflg == 3) editor = "nedit";
   else if (eflg == 4) editor = "vi";

   printf("Editor: %s\n",GetFile(editor));
   return arg;
}

/* ============================= */

static int DefaultFile = 0;

int GetPath(int arg,char *defnam) {
ArgVal   *v;
AtomType  at;

char fname[128], *cp;
int n, earg;

   for (earg=arg; earg<pcnt; earg++) {
      v = &(vals[earg]);
      if ((v->Type == Close)
      ||  (v->Type == Terminator)) break;
   }

   v = &(vals[arg]);
   at = v->Type;
   if ((v->Type == Close)
   ||  (v->Type == Terminator)) {

      DefaultFile = 1;
      GetFile(defnam);

   } else {

      DefaultFile = 0;
      bzero((void *) fname, 128);

      n = 0;
      cp = &(cmdbuf[v->Pos]);
      do {
	 at = atomhash[(int) (*cp)];
	 if ((at != Seperator)
	 &&  (at != Close)
	 &&  (at != Terminator))
	    fname[n++] = *cp;
	 fname[n] = 0;
	 cp++;
      } while ((at != Close) && (at != Terminator));
      strcpy(path,fname);
   }
   return earg;
}

/* ============================= */

int ChangeDirectory(int arg) {
ArgVal   *v;
AtomType  at;
char txt[128], fname[128], *cp;
int n, earg;

   arg++;
   for (earg=arg; earg<pcnt; earg++) {
      v = &(vals[earg]);
      if ((v->Type == Close)
      ||  (v->Type == Terminator)) break;
   }

   v = &(vals[arg]);
   at = v->Type;
   if ((v->Type != Close)
   &&  (v->Type != Terminator)) {

      bzero((void *) fname, 128);

      n = 0;
      cp = &(cmdbuf[v->Pos]);
      do {
	 at = atomhash[(int) (*cp)];
	 if ((at != Seperator)
	 &&  (at != Close)
	 &&  (at != Terminator))
	    fname[n++] = *cp;
	 fname[n] = 0;
	 cp++;
      } while ((at != Close) && (at != Terminator));

      strcpy(localconfigpath,fname);
      strcat(localconfigpath,"/vmeiotest.config");
      if (YesNo("Change vmeiotest config to:",localconfigpath))
	 configpath = localconfigpath;
      else
	 configpath = NULL;
   }

   cp = GetFile(editor);
   sprintf(txt,"%s %s",cp,configpath);
   printf("\n%s\n",txt);
   system(txt);
   printf("\n");
   return(arg);
}

/* ============================= */

unsigned long get_mem(int address) {

int i;
unsigned long data = 0;

   i = address/dwd;
   if      (dwd == 8) data = ((long  *) mem)[i];
   else if (dwd == 4) data = ((int   *) mem)[i];
   else if (dwd == 2) data = ((short *) mem)[i];
   else               data = ((char  *) mem)[i];
   return data;
}

/* ============================= */

void set_mem(int address, unsigned long data) {

int i;

   i = address/dwd;
   if      (dwd == 8) ((long  *) mem)[i] = (long)  data;
   else if (dwd == 4) ((int   *) mem)[i] = (int)   data;
   else if (dwd == 2) ((short *) mem)[i] = (short) data;
   else               ((char  *) mem)[i] = (char)  data;
   return;
}

/* ============================= */

char *get_name(int offset) {

struct regdesc *reg;
static char ntxt[128];
char atxt[64];

   bzero((void *) ntxt, 128);

   reg = get_reg_by_offset(regs,offset);
   while (reg) {
      if (reg->depth != 1) sprintf(atxt,"%s:%s:[0x%X]:",reg->name,reg->flags,reg->depth);
      else                 sprintf(atxt,"%s:%s:",reg->name,reg->flags);
      strcat(ntxt,atxt);
      reg = get_reg_by_offset(reg->next,offset);
   }
   return(ntxt);
}

/* ============================= */

int get_offset(char *name) {

struct regdesc *reg;

   reg = get_reg_by_name(regs,name);
   if (reg) return reg->offset;
   return 0;
}

/* ============================= */

void EditBuffer(int address) {

unsigned long addr, data;
char c, *cp, str[128];
int n, radix, nadr;

   if ((mem == NULL) || (memlen == 0)) {
      printf("No allocated memory to edit\n");
      return;
   }

   printf("EditMemory: [?]Help [/]Open [CR]Next [.]Exit [x]Hex [d]Dec\n\n");

   addr = address;
   radix = 16;
   c = '\n';

   while (1) {

      if (radix == 16) {
	 if (c=='\n') printf("%24s 0x%04x: 0x%08x ", get_name(addr), (int) addr, (int) get_mem(addr));
      } else {
	 if (c=='\n') printf("%24s %04d: %5d ",      get_name(addr), (int) addr, (int) get_mem(addr));
      }

      c = (char) getchar();

      if (c == '/') {
	 bzero((void *) str, 128); n = 0;
	 while ((c != '\n') && (n < 128)) c = str[n++] = (char) getchar();
	 nadr = strtoul(str,&cp,radix);
	 if (cp != str) addr = nadr;
      }

      else if (c == 'x')  { radix = 16; if (addr) addr--; continue; }
      else if (c == 'd')  { radix = 10; if (addr) addr--; continue; }
      else if (c == '.')  { c = getchar(); printf("\n"); break; }
      else if (c == 'q')  { c = getchar(); printf("\n"); break; }
      else if (c == '\n') { addr += dwd; if (addr >= memlen) {addr = 0; printf("\n----\n");} }
      else if (c == '?')  { printf("[?]Help [/]Open [CR]Next [.]Exit [x]Hex [d]Dec\n"); }

      else {
	 bzero((void *) str, 128); n = 0;
	 str[n++] = c;
	 while ((c != '\n') && (n < 128)) c = str[n++] = (char) getchar();
	 data = strtoul(str,&cp,radix);
	 if (cp != str) set_mem(addr,data);
      }
   }
}

/* ============================= */

void get_window_parameters(int tlun) {

   if (cvora_get_window(vmeio[tlun],&winpars)) {
      if (win == 2) dwd = winpars.dwd2;
      else          dwd = winpars.dwd1;
   }
}

/* ============================= */

int EditMem(int arg) { /* Address */
ArgVal   *v;
AtomType  at;
int i, len;
char val;

   arg++;
   len = 0;
   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      len = v->Number;
      v = &(vals[arg]);
      at = v->Type;
      if (len > memlen) {
	 if (mem) free(mem);
	 mem = malloc(len);
	 if (mem == NULL) {
	    printf("Can't allocate that much memory\n");
	    return arg;
	 }
	 memlen = len;
	 printf("Allocated %d byte buffer\n",memlen);
      }
   }

   if (at == Numeric) {
      arg++;
      val = (char) v->Number;
      for (i=0; i<len; i++) mem[i] = val;
      printf("Set %d bytes in buffer to %d\n",len,(int) val);
      len = 0;
   }

   printf("Buffer size is:%d bytes\n",memlen);
   EditBuffer(len);
   return arg;
}

/* ============================= */

int EditRegs(int arg) { /* Register number */
ArgVal   *v;
AtomType  at;

int reg_val;
char *cp;

   arg++;
   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      reg = v->Number;
      cp = get_name(reg*dwd);
   } else if (at == Alpha) {
     cp = v->Text;
     reg = get_offset(cp)/dwd;
   } else goto rdrg;

   arg++;
   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      reg_val = v->Number;
      if (cvora_write_reg(vmeio[lun],reg,&reg_val) == 0) {
	 printf("Can't write reg:%d %s\n",reg,cp);
	 return arg;
      }
   }

rdrg:
   cp = get_name(reg*dwd);

   if (cvora_read_reg(vmeio[lun],reg,&reg_val) == 0) {
      printf("Can't read reg:%d %s\n",reg,cp);
      return arg;
   }
   printf("reg:%d %s = 0x%X (%d) OK\n",reg,cp,reg_val,reg_val);
   return arg;
}

/* ============================= */

int EditDrvName(int arg) {

   arg++;
   printf("Not implemented yet!\n");
   return arg;
}

/* ============================= */

int OpenLun(int arg) {
ArgVal   *v;
AtomType  at;

int tlun;

   arg++;

   tlun = lun;
   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      if ((v->Number >= 0) && (v->Number < DRV_MAX_DEVICES)) {
	 tlun = v->Number;
	 if (vmeio[tlun]) cvora_close(vmeio[tlun]);
	 vmeio[tlun] = cvora_open(tlun);
	 get_window_parameters(tlun);
      }
   }

   printf("Open:%s.%d",drvr_name,tlun);
   if (vmeio[tlun] != NULL) {
      lun = tlun;
      printf(":OK\n");
   } else {
      printf(":ERROR (See dmesg)\n");
      perror(drvr_name);
   }
   return arg;
}

/* ============================= */

int EditLun(int arg) {
ArgVal   *v;
AtomType  at;

int tlun;

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      if ((v->Number >= 0) && (v->Number < DRV_MAX_DEVICES)) {
	 tlun = v->Number;
	 if (vmeio[tlun]) lun = tlun;
	 else printf("lun:%d is not open\n",tlun);
      }
   }
   get_window_parameters(lun);
   printf("lun:%d selected\n",lun);
   return arg;
}

/* ============================= */

int EditDmaFlag(int arg) {
ArgVal   *v;
AtomType  at;

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      if (v->Number) dma = 1;
      else           dma = 0;
      v = &(vals[arg]);
      at = v->Type;
      if (at == Numeric) {
	 arg++;
	 if (v->Number) dmaswap = 1;
	 else           dmaswap = 0;
      }
   }
   if (!cvora_set_params(vmeio[lun],win,dma,dmaswap))
      printf("set_params:Error\n");

   printf("dma:DMA:");
   if (dma) printf("ON  ");
   else     printf("OFF ");
   printf("SWAP:");
   if (dmaswap) printf("ON  ");
   else         printf("OFF ");
   printf("\n");

   return arg;
}

/* ============================= */

int EditWindowNumber(int arg) {
ArgVal   *v;
AtomType  at;

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      if ((v->Number == 1) || (v->Number == 2)) {
	 win = v->Number;
      } else printf("Bad window number:%d Must be [1..2]\n",v->Number);
   }
   printf("Wn:%d selected\n",win);
   print_regs(win);
   return arg;
}

/* ============================= */

int EditDebug(int arg) {
ArgVal   *v;
AtomType  at;

int tdebug;

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      if (cvora_set_debug(vmeio[lun],&v->Number)) debug = v->Number;
      else printf("Error from cvora_set_debug, level:%d\n",v->Number);
   }
   if (cvora_get_debug(vmeio[lun],&tdebug)) debug = tdebug;
   else printf("Error:cvora_get_debug\n");
   printf("debug:lun:%d:level:%d:",lun,debug);
   if (debug) printf("ON\n");
   else       printf("OFF\n");
   return arg;
}

/* ============================= */

int EditTimeout(int arg) {
ArgVal   *v;
AtomType  at;

int ttmo;

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      if (cvora_set_timeout(vmeio[lun],&v->Number)) tmo = v->Number;
      else printf("Error:cvora_set_debug:level:%d\n",v->Number);
   }
   if (cvora_get_timeout(vmeio[lun],&ttmo)) tmo = ttmo;
   else printf("Error:cvora_get_timeout\n");
   printf("timeout:lun:%d:milliseconds:%d:",lun,tmo);
   if (tmo) printf("SET\n");
   else     printf("NOT_SET, wait forever\n");
   return arg;
}

/* ============================= */

int EditOffset(int arg) {
ArgVal   *v;
AtomType  at;

int offset;
int reg;

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      reg = v->Number;
      offset = reg * dwd;
      cvora_set_offset(vmeio[lun],&offset);
   }

   cvora_get_offset(vmeio[lun],&offset);
   reg = offset/dwd;

   printf("BlockOffset:Register:%d RealOffset:0x%X\n",reg,offset);
   return arg;
}

/* ============================= */

int DoInterrupt(int arg) {
ArgVal   *v;
AtomType  at;

int mask;

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      if (cvora_do_interrupt(vmeio[lun],&v->Number)) {
	 mask = v->Number;
	 printf("Interrupt:0x%X sent OK\n",mask);
	 return arg;
      } else {
	 printf("Error:cvora_do_interrupt:mask:0x%X\n",v->Number);
	 return arg;
      }
   }
   printf("I need a mask value to send\n");
   return arg;
}

/* ============================= */

int WaitLun(int arg) {
struct vmeio_read_buf_s event;

   arg++;

   if (!cvora_wait(vmeio[lun],&event)) {
      printf("wait:ERROR:lun:%d\n",lun);
      perror(DRIVER_NAME);
      return arg;
   }

   printf("wait:lun:%d:mask:0x%X:icount:%d:OK\n",event.logical_unit,event.interrupt_mask,event.interrupt_count);
   return arg;
}

/* ============================= */

int DisplayDevicePars(int arg) {

   arg++;
   get_window_parameters(lun);

   printf("lun:%02d lvl:%d vec:0x%02X\n",winpars.lun,winpars.lvl,winpars.vec);
   printf("vme1:0x%06X amd1:0x%02X win1:0x%04X\n",winpars.vme1, winpars.amd1, winpars.win1);
   printf("vme2:0x%06x amd2:0x%02X win2:0x%04X\n",winpars.vme2, winpars.amd2, winpars.win2);
   printf("nmap:%d isrc:%d\n",winpars.nmap,winpars.isrc);

   return arg;
}

/* ============================= */

int RawRead(int arg) {  /* Start, Item count */
ArgVal   *v;
AtomType  at;
struct vmeio_riob_s iob;
int items, start, len;

   arg++;

   start = 0;
   items = 1;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      start = v->Number;

      arg++;
      v = &(vals[arg]);
      at = v->Type;
      if (at == Numeric) {
	 items = v->Number;

	 arg++;
      }
   }

   if (memlen < items * dwd) {
      if (mem) free(mem);
      memlen = 0;
      len = items * dwd;
      mem = malloc(len);
      if (!mem) {
	 printf("Error:NoMemory\n");
	 return arg;
      }
      memlen = len;
   }

   iob.winum  = win;
   iob.offset = start*dwd;
   iob.bsize  = items*dwd;
   iob.buffer = mem;

   if (dma) {
      if (cvora_dma(vmeio[lun],&iob,0)) printf("Read:cvora_dma:[Ad:0x%X,Sz:0x%X]-OK\n",iob.offset,iob.bsize);
      else                        printf("Read:cvora_dma:Error(See dmesg)\n");
   } else {
      if (cvora_raw(vmeio[lun],&iob,0)) printf("Read:cvora_raw:[Ad:0x%X,Sz:0x%X]-OK\n",iob.offset,iob.bsize);
      else                        printf("Read:cvora_raw:Error(See dmesg)\n");
   }
   return arg;
}

/* ============================= */

int RawWrite(int arg) {
ArgVal   *v;
AtomType  at;
struct vmeio_riob_s iob;
int items, start;

   arg++;

   start = 0;
   items = 1;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      start = v->Number;

      arg++;
      v = &(vals[arg]);
      at = v->Type;
      if (at == Numeric) {
	 items = v->Number;

	 arg++;
      }
   }

   if (memlen < items * dwd) {
      printf("Error:NoBufferAllocated\n");
      return arg;
   }

   iob.winum  = win;
   iob.offset = start*dwd;
   iob.bsize  = items*dwd;
   iob.buffer = mem;

   if (dma) {
      if (cvora_dma(vmeio[lun],&iob,1)) printf("Write:cvora_dma:[Ad:0x%X,Sz:0x%X]-OK\n",iob.offset,iob.bsize);
      else                        printf("Write:cvora_dma:Error(See dmesg)\n");
   } else {
      if (cvora_raw(vmeio[lun],&iob,1)) printf("Write:cvora_raw:[Ad:0x%X,Sz:0x%X]-OK\n",iob.offset,iob.bsize);
      else                        printf("Write:cvora_raw:Error(See demsg)\n");
   }
   return arg;
}

/* ============================= */

int GetVersion(int arg) {
struct vmeio_version_s ver;

   if (cvora_get_version(vmeio[lun],&ver)) {
      printf("Driver:  %s - %d\n",TimeToStr(ver.driver),  ver.driver);
      printf("Library: %s - %d\n",TimeToStr(ver.library), ver.library);
   } else
      printf("GetVersion:Error\n");

   arg++;
   return arg;
}
