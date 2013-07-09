/* ========================================================== */
/* Test program for icv196 static library                     */
/* ========================================================== */

char *configpath = NULL;

/* ========================================================== */

static char path[128];

char *get_file(char *name)
{
	FILE *gpath = NULL;
	char txt[128];
	int i, j;

	if (configpath) {
		gpath = fopen(configpath, "r");
		if (gpath == NULL) {
			configpath = NULL;
		}
	}

	if (configpath == NULL) {
		configpath = "./icv196test.config";
		gpath = fopen(configpath, "r");
		if (gpath == NULL) {
			configpath = "/usr/local/icv196/icv196test.config";
			gpath = fopen(configpath, "r");
			if (gpath == NULL) {
				configpath = NULL;
				sprintf(path, "./%s", name);
				return path;
			}
		}
	}

	bzero((void *) path, 128);

	while (1) {
		if (fgets(txt, 128, gpath) == NULL)
			break;
		if (strncmp(name, txt, strlen(name)) == 0) {
			for (i = strlen(name); i < strlen(txt); i++) {
				if (txt[i] != ' ')
					break;
			}
			j = 0;
			while ((txt[i] != ' ') && (txt[i] != 0)) {
				path[j] = txt[i];
				j++;
				i++;
			}
			strcat(path, name);
			fclose(gpath);
			return path;
		}
	}
	fclose(gpath);
	return NULL;
}

/* ========================================================== */
/* Convert time to string.                                    */
/* Result is pointer to static string representing the time.  */
/*    the format is: Thu-18/Jan/2001 08:25:14.967             */
/*                   day-dd/mon/yyyy hh:mm:ss.ddd             */

#define TBLEN 128

char *time_to_str(time_t utc)
{

	static char tbuf[TBLEN];

	char tmp[TBLEN];
	char *yr, *ti, *md, *mn, *dy;

	bzero((void *) tbuf, TBLEN);
	bzero((void *) tmp, TBLEN);

	if (utc) {

		ctime_r(&utc, tmp);
		tmp[3] = 0;
		dy = &(tmp[0]);
		tmp[7] = 0;
		mn = &(tmp[4]);
		tmp[10] = 0;
		md = &(tmp[8]);
		if (md[0] == ' ')
			md[0] = '0';
		tmp[19] = 0;
		ti = &(tmp[11]);
		tmp[24] = 0;
		yr = &(tmp[20]);

		sprintf(tbuf, "%s-%s/%s/%s %s", dy, md, mn, yr, ti);

	} else {
		sprintf(tbuf, "--- Zero ---");
	}
	return (tbuf);
}

/* ========================================================== */

static int icv196 = 0;
static int devn = 0;
static int mcnt = 0;
static struct icv196_mod_addr_s moad;

int do_open(int dev)
{
	icv196_err_t err;
	uint32_t count = 0;

	if ((mcnt > 0) && (dev > mcnt)) {
		fprintf(stderr,"Device:%d number out of range [0..%d]\n",
			dev, mcnt);
		return 0;
	}

	icv196Close(icv196);
	icv196 = icv196Open(dev);

	if (icv196 <= 0) {
		fprintf(stderr,"Cannot open that device:%d\n",dev);
		perror("icv196Open");
		return 0;
	}

	err = icv196GetModuleAddress(icv196, &moad);
	if (err != ICV196_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",icv196ErrToStr(err));
		perror("icv196GetModuleAddress");
	}

	err = icv196GetDeviceCount(icv196, &count);
	if (err != ICV196_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",icv196ErrToStr(err));
		perror("icv196GetDeviceCount");
	} else {
		mcnt = count;
		devn = dev;
	}

	return icv196;
}

/* ========================================================== */

int show_dev(int flag)
{
	if (icv196 > 0) {
	       if (flag) {
			printf("Device:/dev/icv196.%d Lun:%d Vec:0x%X VmeBase:0x%X Open\n",
			       devn, moad.lun, moad.vec, moad.vmeb);

			printf("There are:%d installed modules\n",mcnt);

		}
		return 1;
	}

	if (icv196 <= 0)
		fprintf(stderr,"There is no current open ICV196 device driver connection\n");

	return 0;
}

/* ========================================================== */

int get_lun()
{
	if (icv196 > 0)
		return moad.lun;
	return 0;
}

/* ========================================================== */

int show_help(int arg, char *extra)
{
	ArgVal *v;
	AtomType at;
	int cid = 0;

	v = &(vals[arg]);
	at = v->Type;

	if (at == Alpha) {
		cid = v->CId;

		arg++;
		v = &(vals[arg]);
		at = v->Type;
	}

	if (at == Operator) {
		if ((cid) && (v->OId == OprNOOP)) {

			printf("\n");
			printf("%s - %s - %s\n",
			       cmds[cid].Name,
			       cmds[cid].Help,
			       cmds[cid].Optns);

			if (extra)
				printf("%s - %s\n",cmds[cid].Name,extra);

			return 1;
		}
	}
	return 0;
}

/* ========================================================== */

int get_set_dev(int arg)
{
	ArgVal *v;
	AtomType at;

	int dev;

	if (show_help(arg,"in range:[0..installed devices]")) {
		arg++;
		return arg;
	}

	arg++;
	v = &(vals[arg]);
	at = v->Type;
	if (at == Numeric) {
		arg++;
		dev = v->Number;
		do_open(dev);
	}

	show_dev(1);
	return arg;
}

/* ========================================================== */

int get_set_debug(int arg)
{
	ArgVal *v;
	AtomType at;

	icv196_debug_level_t deb;
	icv196_err_t err;

	if (show_help(arg,"0=>OFF, 1=>IOCTL, 2=>BUFFERS")) {
		arg++;
		return arg;
	}

	arg++;
	if (!show_dev(0))
		return arg;

	v = &(vals[arg]);
	at = v->Type;
	if (at == Numeric) {
		arg++;
		deb = v->Number;
		err = icv196SetDebug(icv196, deb);
		if (err != ICV196_LIB_ERR_SUCCESS) {
			fprintf(stderr,"%s\n",icv196ErrToStr(err));
			perror("icv196SetDebug");
			return arg;
		}
	}

	err = icv196GetDebug(icv196, &deb);
	if (err != ICV196_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",icv196ErrToStr(err));
		perror("icv196GetDebug");
		return arg;
	}

	if      (deb == ICV196_DEBUG_OFF)
		printf("Deb:OFF\n");
	else if (deb == ICV196_DEBUG_IOCTL)
		printf("Deb:IOCTL\n");
	else if (deb == ICV196_DEBUG_BUF)
		printf("Deb:BUFFERS\n");
	else
		printf("Deb:%d\n", deb);

	return arg;
}

/* ========================================================== */

int reset(int arg)
{
	icv196_err_t err;

	if (show_help(arg,"Resets the module and re-initializes it")) {
		arg++;
		return arg;
	}

	arg++;
	if (!show_dev(0))
		return arg;

	err = icv196Reset(icv196);
	if (err != ICV196_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",icv196ErrToStr(err));
		perror("icv196Reset");
	}

	printf("Reset OK\n");
	return arg;
}

/* ========================================================== */

int get_version(int arg)
{
	icv196_err_t err;
	struct icv196_version_s ver;

	if (show_help(arg,"Displays driver date and ICV196 firmware version in Hex")) {
		arg++;
		return arg;
	}

	arg++;
	if (!show_dev(0))
		return arg;

	err = icv196GetVersion(icv196, &ver);
	if (err != ICV196_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",icv196ErrToStr(err));
		perror("icv196GetVersion");
	}

	printf("Driver compiled:%s\n",time_to_str(ver.drvrver));
	printf("ICV196 Firmware version:0x%04X\n",ver.vhdlver);

	return arg;
}

/* ========================================================== */

static uint32_t imask = 0;

int connect(int arg)
{
	ArgVal *v;
	AtomType at;

	icv196_err_t err;
	uint32_t msk;

	if (show_help(arg,"Connect/Disconnect from interrupt: Mask")) {
		printf("\n");
		printf("Where: Mask: in [0..0xFFFF]\n");
		arg++;
		return arg;
	}

	arg++;
	if (!show_dev(0))
		return arg;

	v = &(vals[arg]);
	at = v->Type;
	if (at == Numeric) {
		arg++;

		msk = v->Number & 0xFFFF;
		if ((v->Number) && (!msk)) {
			fprintf(stderr,"Bad mask value:0x%X Not in [0..0x%X]\n",v->Number,0xFFFF);
			return arg;
		}

		err = icv196Connect(icv196, msk);
		if (err != ICV196_LIB_ERR_SUCCESS) {
			fprintf(stderr,"%s\n",icv196ErrToStr(err));
			perror("icv196Connect");
		}

		imask = msk;
	}
	err = icv196GetConnect(icv196,&msk);
	if (err != ICV196_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",icv196ErrToStr(err));
		perror("icv196GetConnect");
		return arg;
	}
	imask = msk;

	printf("Connections:0x%X",imask);
	if (imask == 0)
		printf(" :None");
	printf("\n");

	return arg;
}

/* ========================================================== */

int get_set_timeout(int arg)
{
	ArgVal *v;
	AtomType at;

	icv196_err_t err;
	uint32_t tmout;

	if (show_help(arg,"Set/Clear timeout: Timeout")) {
		printf("\n");
		printf("Where: Timout: in [0=None, N=Milliseconds]\n");
		arg++;
		return arg;
	}

	arg++;
	if (!show_dev(0))
		return arg;

	v = &(vals[arg]);
	at = v->Type;
	if (at == Numeric) {
		arg++;

		tmout = v->Number;
		err = icv196SetTimeout(icv196, tmout);
		if (err != ICV196_LIB_ERR_SUCCESS) {
			fprintf(stderr,"%s\n",icv196ErrToStr(err));
			perror("icv196SetTimeout");
		}
	}

	err = icv196GetTimeout(icv196, &tmout);
	if (err != ICV196_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",icv196ErrToStr(err));
		perror("icv196GetTimeout");
		return arg;
	}

	if (tmout == 0)
		printf("Timeout: Non timeout, wait forever\n");
	else
		printf("Timeout: %d Milliseconds\n",tmout);

	return arg;
}

/* ========================================================== */

int wait_interrupt(int arg)
{
	icv196_err_t err;
	struct icv196_int_buf_s intr;
	uint32_t msk;

	if (show_help(arg,"Wait for an Interrupt")) {
		arg++;
		return arg;
	}

	arg++;
	if (!show_dev(0))
		return arg;

	err = icv196GetConnect(icv196,&msk);
	if (err != ICV196_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",icv196ErrToStr(err));
		perror("icv196GetConnect");
		return arg;
	}
	if (msk) {

		err = icv196Wait(icv196,&intr);
		if (err != ICV196_LIB_ERR_SUCCESS) {
			fprintf(stderr,"%s\n",icv196ErrToStr(err));
			perror("icv196Wait");
			bzero(&intr,sizeof(struct icv196_int_buf_s));
		}
		msk = intr.src;

		printf("Source:0x%X",msk);

		if (!msk)
			printf(" :Timeout\n");

		else
			printf(" - Count:%d Dev:%d\n",intr.cnt,intr.dev);
	} else
		printf("No connection\n");

	return arg;
}

/* ========================================================== */

int ms_sleep(int arg)
{
	ArgVal *v;
	AtomType at;

	unsigned int usec;

	if (show_help(arg,"Sleep for: Milliseconds")) {
		arg++;
		return arg;
	}

	arg++;

	usec = 1000;

	v = &(vals[arg]);
	at = v->Type;
	if (at == Numeric) {
		arg++;
		usec = v->Number * 1000;
	}

	usleep(usec);
	return arg;
}

/* ========================================================== */

#define REG_NAME_LEN 16

struct reg_name_s {
	char name[REG_NAME_LEN];
	uint32_t boffs;
	uint32_t regnum;
};

#define REGS 21

struct reg_name_s names[REGS] = {

	{"CLEAR",   0x00, 0},

	{"CH00-07", 0x03, 1},
	{"CH08-15", 0x02, 2},
	{"CH16-23", 0x05, 3},
	{"CH24-31", 0x04, 4},
	{"CH32-39", 0x07, 5},
	{"CH40-47", 0x06, 6},
	{"CH48-55", 0x09, 7},
	{"CH56-63", 0x08, 8},
	{"CH64-71", 0x0B, 9},
	{"CH72-79", 0x0A, 10},
	{"CH80-87", 0x0D, 11},
	{"CH88-95", 0x0C, 12},

	{"DIR07-00",0x0F, 13},
	{"DIR11-08",0x0E, 14},

	{"NOT-USED",0x80, 15},

	{"PORT-C",  0x81, 16},
	{"PORT-B",  0x83, 17},
	{"PORT-A",  0x85, 18},

	{"CTRL",    0x87, 19},

	{"CS-NIT",  0xC1, 20},

};

#define EREGLEN 32

void edit_regs(uint32_t regn)
{
	struct icv196_riob_s rio;
	struct reg_name_s *rnm;
	icv196_err_t err;
	uint8_t regvi[MAX_BYTES], regvo[MAX_BYTES], i;
	char txt[EREGLEN] , *ep;

	while (1) {

		if (regn>=REGS) {
			regn = 0;
			printf("\n---\n");
		}
		rnm = &names[regn];

		rio.boffs  = rnm->boffs;
		rio.bsize  = 1;
		rio.buffer = regvi;

		err = icv196RawIo(icv196,&rio,ICV196_IO_DIR_READ);
		if (err != ICV196_LIB_ERR_SUCCESS) {
			fprintf(stderr,"%s\n",icv196ErrToStr(err));
			perror("icv196RawIo");
			break;
		}

		printf("%15s:[0x%02X](%02d):0x%02x/",rnm->name,rnm->boffs,rnm->regnum,regvi[0]);
		fflush(stdout);

		if (fgets(txt,EREGLEN,stdin) == NULL) {
			perror("fgets");
			return;
		}

		if (txt[0] == '.') {
			printf("\n");
			break;
		}

		if (txt[0] == '/') {
			ep = &txt[1];
			i = strtoul(&txt[1],&ep,10);
			if (ep != &txt[1])
				regn = i;
			continue;
		}

		if ((strlen(txt) == 0) || (txt[0] == '\n')) {
			regn++;
			continue;
		}

		ep = txt;
		regvo[0] = strtoul(txt,&ep,16);
		if (ep != txt) {
			rio.boffs = rnm->boffs;
			rio.bsize = 1;
			rio.buffer = regvo;

			err = icv196RawIo(icv196,&rio,ICV196_IO_DIR_WRITE);
			if (err != ICV196_LIB_ERR_SUCCESS) {
				fprintf(stderr,"%s\n",icv196ErrToStr(err));
				perror("icv196RawIo");
				break;
			}
			continue;
		}
	}
}

/* ========================================================== */

int raw_io(int arg)
{
	ArgVal *v;
	AtomType at;

	uint32_t regn;

	if (show_help(arg,"Raw IO 8-Bit Register editor: RegNum")) {
		printf("\n");
		printf("Where: RegNum: in [0..%d]\n",REGS-1);
		arg++;
		return arg;
	}

	arg++;
	if (!show_dev(0))
		return arg;

	regn = 0;
	v = &(vals[arg]);
	at = v->Type;
	if (at == Numeric) {
		arg++;

		if (v->Number>=REGS)
			regn = v->Number;

		else {
			fprintf(stderr,"Bad register number:%d Not in [0..%d]\n",(int) v->Number, REGS-1);
			return arg;
		}
	}

	edit_regs(regn);

	return arg;
}

/* ========================================================== */

int get_set_out(int arg)
{
	ArgVal *v;
	AtomType at;

	icv196_err_t err;
	uint32_t omsk;

	if (show_help(arg,"Get/Set byte output mask: OMsk")) {
		printf("\n");
		printf("Where: OMsk: in [0..0xFFF]\n");
		arg++;
		return arg;
	}

	arg++;
	if (!show_dev(0))
		return arg;

	v = &(vals[arg]);
	at = v->Type;
	if (at == Numeric) {
		arg++;
		omsk = v->Number & 0xFFF;

		err = icv196SetGroupsAsOutput(icv196,omsk);
		if (err != ICV196_LIB_ERR_SUCCESS) {
			fprintf(stderr,"%s\n",icv196ErrToStr(err));
			perror("icv196SetGroupsAsOutput");
		}
	}

	err = icv196GetGroupsAsOutput(icv196,&omsk);
	if (err != ICV196_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",icv196ErrToStr(err));
		perror("icv196GetGroupsAsOutput");
	}

	printf("OMsk: 0x%03X\n",omsk);
	return arg;
}

/* ========================================================== */

int get_set_byte(int arg)
{
	ArgVal *v;
	AtomType at;

	icv196_err_t err;
	struct icv196_digiob_s giob;
	uint32_t i, msk, bnum, bval, omsk;

	if (show_help(arg,"Get/Set byte: BNum BVal")) {
		printf("\n");
		printf("Where: BNum: in [0..11]\n");
		printf("       BVal: in [0..0xFF]\n");
		arg++;
		return arg;
	}

	arg++;
	if (!show_dev(0))
		return arg;

	v = &(vals[arg]);
	at = v->Type;
	if (at == Numeric) {
		arg++;
		bnum = v->Number;
		if (bnum >= MAX_BYTES) {
			fprintf(stderr,"Illegal byte number:%d not in: [0..11]\n", bnum);
			return arg;
		}

		v = &(vals[arg]);
		at = v->Type;
		if (at == Numeric) {
			bval = v->Number;
			if (bval > 0xFF) {
				fprintf(stderr,"Illegal byte value:%d not in: [0..255]\n", bval);
				return arg;
			}

			giob.msk = 1 << bnum;
			giob.val[bnum] = bval;

			err = icv196SetGroups(icv196,&giob);
			if (err != ICV196_LIB_ERR_SUCCESS) {
				fprintf(stderr,"%s\n",icv196ErrToStr(err));
				perror("icv196SetGroups");
			}
		}
	}

	giob.msk = 0xFFF;

	err = icv196GetGroups(icv196,&giob);
	if (err != ICV196_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",icv196ErrToStr(err));
		perror("icv196GetGroups");
		return arg;
	}

	err = icv196GetGroupsAsOutput(icv196,&omsk);
	if (err != ICV196_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",icv196ErrToStr(err));
		perror("icv196GetGroupsAsOutput");
	}


	printf("Byte values are:\n");
	for (i=0; i<MAX_BYTES; i++) {
		printf("%02d: %02X - %03d", i, giob.val[i], giob.val[i]);

		msk = 1 << i;
		if (msk & omsk) printf(" -> Output");
		else            printf(" <- Input");
		printf("\n");
	}
	return arg;
}

/* ========================================================== */

int roll_bits(int arg)
{
	int i, j;
	struct icv196_digiob_s giob;
	icv196_err_t err;

	arg++;

	printf("Rolling bit ...");
	fflush(stdout);
	for (i=0; i<MAX_BYTES; i++) {
		for (j=0; j<8; j++) {
			giob.val[i] = 1<<j;
			giob.msk = 1<<i;

			err = icv196SetGroups(icv196,&giob);
			if (err != ICV196_LIB_ERR_SUCCESS) {
				fprintf(stderr,"%s\n",icv196ErrToStr(err));
				perror("icv196SetGroups");
			}

			usleep(100000);
		}
	}
	printf(" Done\n");
	return arg;
}
