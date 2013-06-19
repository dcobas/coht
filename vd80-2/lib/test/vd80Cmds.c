/* ========================================================== */
/* Test program for vd80 static library                       */
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
		configpath = "./vd80test.config";
		gpath = fopen(configpath, "r");
		if (gpath == NULL) {
			configpath = "/usr/local/vd80/vd80test.config";
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

static int vd80 = 0;
static int devn = 0;
static int mcnt = 0;
static struct vd80_mod_addr_s moad;

int do_open(int dev)
{
	vd80_err_t err;
	uint32_t count = 0;

	if ((mcnt > 0) && (dev > mcnt)) {
		fprintf(stderr,"Device:%d number out of range [0..%d]\n",
			dev, mcnt);
		return 0;
	}

	vd80CloseHandle(vd80);
	vd80 = vd80OpenHandle(dev);

	if (vd80 <= 0) {
		fprintf(stderr,"Cannot open that device:%d\n",dev);
		perror("vd80OpenHandle");
		return 0;
	}

	err = vd80GetModuleAddress(vd80, &moad);
	if (err != VD80_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",vd80ErrToStr(err));
		perror("vd80GetModuleAddress");
	}

	err = vd80GetModuleCount(vd80, &count);
	if (err != VD80_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",vd80ErrToStr(err));
		perror("vd80GetModuleCount");
	} else {
		mcnt = count;
		devn = dev;
	}

	return vd80;
}

/* ========================================================== */

int show_dev(int flag)
{
	if (vd80 > 0) {
	       if (flag) {
			printf("Device:/dev/vd80.%d Lun:%d Vec:0x%X VmeBase:0x%X Slt:%d Open\n",
			       devn, moad.lun, moad.vec, moad.vmeb, moad.slot);

			printf("There are:%d installed modules\n",mcnt);

		}
		return 1;
	}

	if (vd80 <= 0)
		fprintf(stderr,"There is no current open VD80 device driver connection\n");

	return 0;
}

/* ========================================================== */

int get_lun()
{
	if (vd80 > 0)
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

	vd80_debug_level_t deb;
	vd80_err_t err;

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
		err = vd80SetDebug(vd80, deb);
		if (err != VD80_LIB_ERR_SUCCESS) {
			fprintf(stderr,"%s\n",vd80ErrToStr(err));
			perror("vd80SetDebug");
			return arg;
		}
	}

	err = vd80GetDebug(vd80, &deb);
	if (err != VD80_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",vd80ErrToStr(err));
		perror("vd80GetDebug");
		return arg;
	}

	if      (deb == VD80_DEBUG_OFF)
		printf("Deb:OFF\n");
	else if (deb == VD80_DEBUG_IOCTL)
		printf("Deb:IOCTL\n");
	else if (deb == VD80_DEBUG_BUF)
		printf("Deb:BUFFERS\n");
	else
		printf("Deb:%d\n", deb);

	return arg;
}

/* ========================================================== */

int reset(int arg)
{
	vd80_err_t err;

	if (show_help(arg,"Resets the module and re-initializes it")) {
		arg++;
		return arg;
	}

	arg++;
	if (!show_dev(0))
		return arg;

	err = vd80ResetMod(vd80);
	if (err != VD80_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",vd80ErrToStr(err));
		perror("vd80ResetMod");
	}

	printf("Reset OK\n");
	return arg;
}

/* ========================================================== */

static uint32_t chn = 1;

int get_adc_value(int arg)
{
	ArgVal *v;
	AtomType at;

	vd80_err_t err;
	int adc = 0;

	if (show_help(arg,"Reads the ADC value for a given channel:[1..16]")) {
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
		if ((v->Number>=1) && (v->Number<=VD80_LIB_CHANNELS))
			chn = v->Number;
	}

	err = vd80GetAdcValue(vd80, chn, &adc);
	if (err != VD80_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",vd80ErrToStr(err));
		perror("vd80GetAdcValue");
	}
	printf("Chn:%d ADC:%d (0x%X)\n",chn,adc,adc);
	return arg;
}

/* ========================================================== */

int get_state(int arg)
{
	vd80_err_t err;
	vd80_state_t state;
	char *cp = NULL;

	if (show_help(arg,"Reads the VD80 current state")) {
		arg++;
		return arg;
	}

	arg++;
	if (!show_dev(0))
		return arg;

	err = vd80GetState(vd80, &state);
	if (err != VD80_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",vd80ErrToStr(err));
		perror("vd80GetState");
	}

	if      (state == VD80_STATE_IDLE)        cp = "Idle";
	else if (state == VD80_STATE_PRETRIGGER)  cp = "PreTrigger";
	else if (state == VD80_STATE_POSTTRIGGER) cp = "PostTrigger";
	else                                      cp = "Unknown";

	printf("State:%s 0x%X\n",cp,state);
	return arg;
}

/* ========================================================== */

int get_version(int arg)
{
	vd80_err_t err;
	struct vd80_version_s ver;

	if (show_help(arg,"Displays driver date and VD80 firmware version in Hex")) {
		arg++;
		return arg;
	}

	arg++;
	if (!show_dev(0))
		return arg;

	err = vd80GetVersion(vd80, &ver);
	if (err != VD80_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",vd80ErrToStr(err));
		perror("vd80GetVersion");
	}

	printf("Driver compiled:%s\n",time_to_str(ver.drvrver));
	printf("VD80 Firmware version:0x%04X\n",ver.vhdlver);

	return arg;
}

/* ========================================================== */

int get_set_clk(int arg)
{
	ArgVal *v;
	AtomType at;

	vd80_err_t err;
	struct vd80_input_s clk;
	char *edgp, *trmp, *srcp;

	if (show_help(arg,"Get/Set clock: Edge, Termination, Source")) {
		printf("\n");
		printf("Where: Edge        in [0=Rising   1=Falling]\n");
		printf("       Termination in [0=None     1=50 Ohms]\n");
		printf("       Source      in [0=Internal 1=External]\n");
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

		if ((v->Number != 0) && (v->Number != 1)) {
			fprintf(stderr,"Bad clock edge value:%d not in [0..1]\n",v->Number);
			return arg;
		}
		clk.edge = v->Number;

		v = &(vals[arg]);
		at = v->Type;
		if (at == Numeric) {
			arg++;

			if ((v->Number != 0) && (v->Number != 1)) {
				fprintf(stderr,"Bad clock termination value:%d not in [0..1]\n",v->Number);
				return arg;
			}
			clk.termination = v->Number;

			v = &(vals[arg]);
			at = v->Type;
			if (at == Numeric) {
				arg++;

				if ((v->Number != 0) && (v->Number != 1)) {
					fprintf(stderr,"Bad clock source value:%d not in [0..1]\n",v->Number);
					return arg;
				}
				clk.source = v->Number;

				err = vd80SetClock(vd80, &clk);
				if (err != VD80_LIB_ERR_SUCCESS) {
					fprintf(stderr,"%s\n",vd80ErrToStr(err));
					 perror("vd80SetClock");
				}
			}
		}
	}

	err = vd80GetClock(vd80, &clk);
	if (err != VD80_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",vd80ErrToStr(err));
		perror("vd80GetClock");
		return arg;
	}

	if      (clk.edge == 0) edgp = "Rising";
	else if (clk.edge == 1) edgp = "Falling";
	else                    edgp = "Illegal value";

	if      (clk.termination == 0) trmp = "None";
	else if (clk.termination == 1) trmp = "50 Ohms";
	else                           trmp = "Illegal value";

	if      (clk.source == 0) srcp = "Internal";
	else if (clk.source == 1) srcp = "External";
	else                      srcp = "Illegal value";

	printf("Clock:\n");
	printf("   Edge        %d=%s\n",clk.edge       ,edgp);
	printf("   Termination %d=%s\n",clk.termination,trmp);
	printf("   Source      %d=%s\n",clk.source     ,srcp);

	return arg;
}

/* ========================================================== */

#define INT_CLKFRQ 200000

int get_set_clk_div(int arg)
{
	ArgVal *v;
	AtomType at;

	vd80_err_t err;
	uint32_t div = 0;

	if (show_help(arg,"Get/Set clock divisor: 0=No divide[20KHz], 1=divide by 2 etc")) {
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
		div = v->Number;

		err = vd80SetClockDivide(vd80, div);
		if (err != VD80_LIB_ERR_SUCCESS) {
			fprintf(stderr,"%s\n",vd80ErrToStr(err));
			perror("vd80SetClockDivide");
		}
	}

	err = vd80GetClockDivide(vd80, &div);
	if (err != VD80_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",vd80ErrToStr(err));
		perror("vd80GetClockDivide");
		return arg;
	}

	printf("Clock divide:%d = %d Hz\n",div,(INT_CLKFRQ/(div+1)));

	return arg;
}

/* ========================================================== */

int get_set_trg(int arg)
{
	ArgVal *v;
	AtomType at;

	vd80_err_t err;
	struct vd80_input_s trg;
	char *edgp, *trmp, *srcp;

	if (show_help(arg,"Get/Set trigger: Edge, Termination, Source")) {
		printf("\n");
		printf("Where: Edge        in [0=Rising   1=Falling]\n");
		printf("       Termination in [0=None     1=50 Ohms]\n");
		printf("       Source      in [0=Internal 1=External]\n");
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

		if ((v->Number != 0) && (v->Number != 1)) {
			fprintf(stderr,"Bad trigger edge value:%d not in [0..1]\n",v->Number);
			return arg;
		}
		trg.edge = v->Number;

		v = &(vals[arg]);
		at = v->Type;
		if (at == Numeric) {
			arg++;

			if ((v->Number != 0) && (v->Number != 1)) {
				fprintf(stderr,"Bad trigger termination value:%d not in [0..1]\n",v->Number);
				return arg;
			}
			trg.termination = v->Number;

			v = &(vals[arg]);
			at = v->Type;
			if (at == Numeric) {
				arg++;

				if ((v->Number != 0) && (v->Number != 1)) {
					fprintf(stderr,"Bad trigger source value:%d not in [0..1]\n",v->Number);
					return arg;
				}
				trg.source = v->Number;

				err = vd80SetTrigger(vd80, &trg);
				if (err != VD80_LIB_ERR_SUCCESS) {
					fprintf(stderr,"%s\n",vd80ErrToStr(err));
					 perror("vd80SetTrigger");
				}
			}
		}
	}

	err = vd80GetTrigger(vd80, &trg);
	if (err != VD80_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",vd80ErrToStr(err));
		perror("vd80GetTrigger");
		return arg;
	}

	if      (trg.edge == 0) edgp = "Rising";
	else if (trg.edge == 1) edgp = "Falling";
	else                    edgp = "Illegal value";

	if      (trg.termination == 0) trmp = "None";
	else if (trg.termination == 1) trmp = "50 Ohms";
	else                           trmp = "Illegal value";

	if      (trg.source == 0) srcp = "Internal";
	else if (trg.source == 1) srcp = "External";
	else                      srcp = "Illegal value";

	printf("Trigger:\n");
	printf("   Edge        %d=%s\n",trg.edge       ,edgp);
	printf("   Termination %d=%s\n",trg.termination,trmp);
	printf("   Source      %d=%s\n",trg.source     ,srcp);

	return arg;
}

/* ========================================================== */

int get_set_analog_trg(int arg)
{
	ArgVal *v;
	AtomType at;

	int neg_lev = 0;
	char *cp;
	vd80_err_t err;
	struct vd80_analogue_trig_s atrg;

	if (show_help(arg,"Get/Set analogue trigger: Channel, Control, Level")) {
		arg++;
		printf("\n");
		printf("Where: Channel in [1..16]\n");
		printf("       Control in [0=Off,3=Below,4=Above]\n");
		printf("       Level   in [int16 (signed short)]\n");
		return arg;
	}

	arg++;
	if (!show_dev(0))
		return arg;

	atrg.channel = chn;
	atrg.control = VD80_ANALOGUE_TRIG_DISABLED;
	atrg.level   = 0;

	v = &(vals[arg]);
	at = v->Type;
	if (at == Numeric) {
		arg++;
		if ((v->Number < 1) || (v->Number >VD80_LIB_CHANNELS)) {
			printf("Bad channel number:%d\n",v->Number);
			return arg;
		}
		chn = v->Number;
		atrg.channel = chn;

		v = &(vals[arg]);
		at = v->Type;
		if (at == Numeric) {
			arg++;

			if ((v->Number != VD80_ANALOGUE_TRIG_DISABLED)
			&&  (v->Number != VD80_ANALOGUE_TRIG_BELOW)
			&&  (v->Number != VD80_ANALOGUE_TRIG_ABOVE)) {
				printf("Bad trigger control:%d Not in [%d,%d,%d]\n",
					v->Number,
					VD80_ANALOGUE_TRIG_DISABLED,
					VD80_ANALOGUE_TRIG_BELOW,
					VD80_ANALOGUE_TRIG_ABOVE);
				return arg;
			}
			atrg.control = v->Number;

			v = &(vals[arg]);
			at = v->Type;

			if (at == Operator) {
				arg++;

				if (v->OId == OprMI)
					neg_lev = -1;

				v = &(vals[arg]);
				at = v->Type;
			}

			v = &(vals[arg]);
			at = v->Type;
			if (at == Numeric) {
				arg++;

				atrg.level = (0x7FFF & v->Number);
				if (neg_lev)
					atrg.level = -atrg.level;
			}
		}

		err = vd80SetTriggerLevel(vd80, &atrg);
		if (err != VD80_LIB_ERR_SUCCESS) {
			fprintf(stderr,"%s\n",vd80ErrToStr(err));
			perror("vd80SetTriggerLevel");
		}
	}

	err = vd80GetTriggerLevel(vd80, &atrg);
	if (err != VD80_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",vd80ErrToStr(err));
		perror("vd80SetTriggerLevel");
		return arg;
	}

	if      (atrg.control == VD80_ANALOGUE_TRIG_DISABLED) cp = "Disabled";
	else if (atrg.control == VD80_ANALOGUE_TRIG_BELOW)    cp = "Below";
	else if (atrg.control == VD80_ANALOGUE_TRIG_ABOVE)    cp = "Above";
	else                                                  cp = "Illegal value";

	printf("Analogue Trigger:\n");
	printf("   Channel: %d\n",atrg.channel);
	printf("   Control: %d=%s\n",atrg.control,cp);
	printf("   Level:   %d\n",atrg.level);

	return arg;
}

/* ========================================================== */

int get_set_config_trg(int arg)
{
	ArgVal *v;
	AtomType at;

	vd80_err_t err;
	struct vd80_trig_config_s ctrg;

	if (show_help(arg,"Get/Set pre-trigger configuration: Delay, MinPreTrig")) {
		printf("Pre Trigger Configuration:\n");
		printf("   Delay: Number of sample clock ticks to wait after trigger\n");
		printf("   MinPreTrig: Minimum number of samples before trigger\n");
		arg++;
		return arg;
	}

	arg++;
	if (!show_dev(0))
		return arg;

	ctrg.trig_delay   = 0;
	ctrg.min_pre_trig = 0;

	v = &(vals[arg]);
	at = v->Type;
	if (at == Numeric) {
		arg++;

		ctrg.trig_delay = v->Number;

		v = &(vals[arg]);
		at = v->Type;

		if (at == Numeric) {
			arg++;

			ctrg.min_pre_trig = v->Number;

			err = vd80SetTriggerConfig(vd80, &ctrg);
			if (err != VD80_LIB_ERR_SUCCESS) {
				fprintf(stderr,"%s\n",vd80ErrToStr(err));
				perror("vd80SetTriggerConfig");
			}
		}
	}

	err = vd80GetTriggerConfig(vd80, &ctrg);
	if (err != VD80_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",vd80ErrToStr(err));
		perror("vd80GetTriggerConfig");
		return arg;
	}

	printf("Pre Trigger Configuration:\n");
	printf("   Delay     : %d Ticks\n",  ctrg.trig_delay);
	printf("   MinPreTrig: %d Samples\n",ctrg.min_pre_trig);

	return arg;
}

/* ========================================================== */

#define MIN_SIZE 1024
#define MIN_POST 32

static struct vd80_buffer_s buffer = { NULL, 0, 0, 0, 0, 0};

static uint32_t bsze = 0;
static uint32_t post = 0;

int get_set_buffer_size(int arg)
{
	ArgVal *v;
	AtomType at;

	vd80_err_t err;

	if (show_help(arg,"Get/Set Buffer size: Samples, MinPostSamples")) {
		printf("\n");
		printf("Where: Samples:       The total buffer size\n");
		printf("       MinPostSamples:The minimum number of post trigger samples \n");
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

		if (v->Number < MIN_SIZE)
			v->Number = MIN_SIZE;
		bsze = (v->Number/32)*32;

		v = &(vals[arg]);
		at = v->Type;

		if (at == Numeric) {
			arg++;

			post = (v->Number/32)*32;
			if (post < MIN_POST)
				post = MIN_POST;

			if (post > bsze)
				bsze = post + 32;
		}
	}

	if (bsze > buffer.bsze) {
		if (buffer.addr)
			free(buffer.addr);
		buffer.addr = malloc(bsze * sizeof(int16_t));
		if (buffer.addr == NULL) {
			fprintf(stderr,"Can't allocat sample buffer of size:%d samples\n",bsze);
			perror("malloc");
			return arg;
		}
	}

	if ((buffer.bsze != bsze) || (buffer.post != post)) {

		buffer.bsze = bsze;
		buffer.post = post;

		err = vd80SetPostSamples(vd80, post);
		if (err != VD80_LIB_ERR_SUCCESS) {
			fprintf(stderr,"%s\n",vd80ErrToStr(err));
			perror("vd80SetBufferSize");
		}
	}

	err = vd80GetPostSamples(vd80, &post);
	if (err != VD80_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",vd80ErrToStr(err));
		perror("vd80SetBufferSize");
		return arg;
	}

	printf("Current buffer: Size:%d samples Post:%d samples\n",bsze,post);

	return arg;
}

/* ========================================================== */

int strt_acquisition(int arg)
{
	vd80_err_t err;

	if (show_help(arg,"Start an acquisition cycle")) {
		arg++;
		return arg;
	}

	arg++;
	if (!show_dev(0))
		return arg;

	err = vd80StrtAcquisition(vd80);
	if (err != VD80_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",vd80ErrToStr(err));
		perror("vd80StartAcquisition");
	}

	printf("Started\n");
	return arg;
}

/* ========================================================== */

int trig_acquisition(int arg)
{
	vd80_err_t err;

	if (show_help(arg,"Trigger an acquisition cycle")) {
		arg++;
		return arg;
	}

	arg++;
	if (!show_dev(0))
		return arg;

	err = vd80TrigAcquisition(vd80);
	if (err != VD80_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",vd80ErrToStr(err));
		perror("vd80TrigAcquisition");
	}

	printf("Triggered\n");
	return arg;
}

/* ========================================================== */

int stop_acquisition(int arg)
{
	vd80_err_t err;

	if (show_help(arg,"Stop an acquisition cycle")) {
		arg++;
		return arg;
	}

	arg++;
	if (!show_dev(0))
		return arg;

	err = vd80StopAcquisition(vd80);
	if (err != VD80_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",vd80ErrToStr(err));
		perror("vd80StopAcquisition");
	}

	printf("Stopped\n");
	return arg;
}

/* ========================================================== */

static vd80_intr_mask_t imask = 0;

int connect(int arg)
{
	ArgVal *v;
	AtomType at;

	vd80_err_t err;
	uint32_t msk;

	if (show_help(arg,"Connect/Disconnect from interrupt: Mask")) {
		printf("\n");
		printf("Where: Mask: in [0=Clear,1=Trigger,2=Acquisition,4=Error]\n");
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

		msk = v->Number & VD80_LIB_INTR_MASK;
		if ((v->Number) && (!msk)) {
			fprintf(stderr,"Bad mask value:0x%X Not in [0..0x%X]\n",v->Number,VD80_LIB_INTR_MASK);
			return arg;
		}

		err = vd80Connect(vd80, msk);
		if (err != VD80_LIB_ERR_SUCCESS) {
			fprintf(stderr,"%s\n",vd80ErrToStr(err));
			perror("vd80Connect");
		}

		imask = msk;
	}
	err = vd80GetConnect(vd80,&msk);
	if (err != VD80_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",vd80ErrToStr(err));
		perror("vd80GetConnect");
		return arg;
	}
	imask = msk;

	printf("Connections:0x%X",imask);
	if (imask & VD80_LIB_INTR_MASK_TRIGGER)
		printf(" :Trigger");
	if (imask & VD80_LIB_INTR_MASK_ACQUISITION)
		printf(" :Acquisition");
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

	vd80_err_t err;
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
		err = vd80SetTimeout(vd80, tmout);
		if (err != VD80_LIB_ERR_SUCCESS) {
			fprintf(stderr,"%s\n",vd80ErrToStr(err));
			perror("vd80SetTimeout");
		}
	}

	err = vd80GetTimeout(vd80, &tmout);
	if (err != VD80_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",vd80ErrToStr(err));
		perror("vd80GetTimeout");
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
	vd80_err_t err;
	struct vd80_int_buf_s intr;
	uint32_t msk;

	if (show_help(arg,"Wait for an Interrupt")) {
		arg++;
		return arg;
	}

	arg++;
	if (!show_dev(0))
		return arg;

	err = vd80Wait(vd80,&intr);
	if (err != VD80_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",vd80ErrToStr(err));
		perror("vd80Wait");
	}
	msk = intr.src;

	printf("Source:0x%X",msk);

	if (!msk)
		printf(" :Timeout\n");

	else {

		if (msk & VD80_LIB_INTR_MASK_TRIGGER)
			printf(" :Trigger");
		if (msk & VD80_LIB_INTR_MASK_ACQUISITION)
			printf(" :Acquisition");

		printf(" - Count:%d Lun:%d\n",intr.cnt,intr.lun);
	}
	return arg;
}

/* ========================================================== */

int read_file(int arg)
{
	ArgVal *v;
	AtomType at;

	char *cp, *ep, bname[TBLEN], txt[TBLEN];
	FILE *bfile = NULL;
	uint32_t lun;
	int16_t *sp, datum;
	int i, j;

	lun = moad.lun;

	if (show_help(arg,"Read file: Channel")) {
		printf("\n");
		printf("   The file VD80SAMP.<lun>.<chn> is read\n");
		printf("   Current Lun=%02d Chn=%02d\n",lun,chn);
		printf("\n");
		printf("Where: Channel in [1..16]\n");
		arg++;
		return arg;
	}

	arg++;

	if (bsze == 0) {
		fprintf(stderr,"No buffer allocated yet\n");
		return arg;
	}

	v = &(vals[arg]);
	at = v->Type;
	if (at == Numeric) {
		arg++;
		if ((v->Number>=1) && (v->Number<=VD80_LIB_CHANNELS))
			chn = v->Number;
		else {
			fprintf(stderr,"Bad channel number:%d Not in [1..16]\n",(int) v->Number);
			return arg;
		}
	}

	get_file("VD80SAMP");
	sprintf(bname,"%s.%02d.%02d",path,lun,chn);
	printf("Read file:%s\n",bname);

	bfile = fopen(bname, "r");
	if (bfile) {
		buffer.asze = 0;
		buffer.tpos = 0;
		buffer.post = 0;
		buffer.ptsr = 0;

		while (fgets(txt, TBLEN, bfile) != NULL) {

			cp = txt;
			i = strtol(cp, &ep, 0);
			if (cp == ep)
				break;

			cp = ep;
			datum = strtol(cp, &ep, 0);
			if (cp == ep)
				break;

			buffer.asze = i + 1;
			if (buffer.asze >= buffer.bsze)
				break;

			if (buffer.tpos)
				buffer.post++;
			j = 0;
			while ((ep[j] != '\n') && (*ep != 0)) {
				if (ep[j++] == '#')
					buffer.tpos = buffer.asze;
			}

			sp = (int16_t *) buffer.addr;
			sp[i] = (int16_t) (datum & 0xFFFF);
		}
		fclose(bfile);

		printf("Read file:Read:%d samples from:%s\n",buffer.asze,bname);
		printf("Read file:Trig:%d\n",buffer.tpos);
		printf("Read file:Post:%d\n",buffer.post);

	} else {
		fprintf(stderr,"Error: Could not open the file: %s for read\n",bname);
		perror("fopen");
	}
	return arg;
}

/* ========================================================== */

int write_file(int arg)
{
	ArgVal *v;
	AtomType at;

	char bname[TBLEN];
	FILE *bfile = NULL;
	uint32_t lun;
	int16_t *sp, datum;
	int i;

	lun = moad.lun;

	if (show_help(arg,"Write file: Channel")) {
		printf("\n");
		printf("   The file VD80SAMP.<lun>.<chn> is written\n");
		printf("   Current Lun=%02d Chn=%02d\n",lun,chn);
		printf("\n");
		printf("Where: Channel in [1..16]\n");
		arg++;
		return arg;
	}

	arg++;
	if (!show_dev(0))
		return arg;

	if (bsze == 0) {
		fprintf(stderr,"No buffer allocated yet\n");
		return arg;
	}

	v = &(vals[arg]);
	at = v->Type;
	if (at == Numeric) {
		arg++;
		if ((v->Number>=1) && (v->Number<=VD80_LIB_CHANNELS))
			chn = v->Number;
		else {
			fprintf(stderr,"Bad channel number:%d Not in [1..16]\n",(int) v->Number);
			return arg;
		}
	}

	get_file("VD80SAMP");
	sprintf(bname,"%s.%02d.%02d",path,lun,chn);
	printf("Write file:%s\n",bname);

	bfile = fopen(bname, "w");
	if (bfile) {
		sp = (short *) buffer.addr;
		for (i=0; i<buffer.asze; i++) {
			datum = (int) sp[i];
			fprintf(bfile, "%3d %d", i, datum);
			if (i == buffer.tpos)
				fprintf(bfile, " %d  # Trigger", datum);
			fprintf(bfile, "\n");
		}
		fclose(bfile);

		printf("Write file:Writ:%d samples from:%s\n",buffer.asze,bname);
		printf("Write file:Trig:%d\n",buffer.tpos);
		printf("Write file:Post:%d\n",buffer.post);

	} else {
		fprintf(stderr,"Error: Could not open the file: %s for write\n",bname);
		perror("fopen");
	}
	return arg;
}

/* ========================================================== */

int plot_samp(int arg)
{
	ArgVal *v;
	AtomType at;

	FILE *pltfile = NULL;
	char bname[128];
	char pltcmd[128];
	uint32_t lun;

	lun = moad.lun;

	if (show_help(arg,"Plot file: Channel")) {
		printf("\n");
		printf("   The file VD80SAMP.<lun>.<chn> GNU plot file\n");
		printf("   Current Lun=%02d Chn=%02d\n",lun,chn);
		printf("\n");
		printf("Where: Channel in [1..16]\n");
		arg++;
		return arg;
	}

	arg++;

	v = &(vals[arg]);
	at = v->Type;
	if (at == Numeric) {
		arg++;
		if ((v->Number>=1) && (v->Number<=VD80_LIB_CHANNELS))
			chn = v->Number;
		else {
			fprintf(stderr,"Bad channel number:%d Not in [1..16]\n",(int) v->Number);
			return arg;
		}
	}

	get_file("VD80SAMP");
	sprintf(bname, "%s.%02d.%02d",path,lun,chn);

	get_file("VD80.plot");
	pltfile = fopen(path, "w");
	if (pltfile) {
		fprintf(pltfile, "plot \"%s\" with line, \"%s\" using 1:3 with boxes\npause 10\n",
			bname, bname);
		fclose(pltfile);
		sprintf(pltcmd, "set term = xterm; gnuplot %s", path);
		printf("Plot command:%s\n", pltcmd);
		system(pltcmd);
	}
	return arg;
}

/* ========================================================== */

int read_samp(int arg)
{
	ArgVal *v;
	AtomType at;

	vd80_err_t err;

	if (show_help(arg,"Read samples: Channel")) {
		printf("   Reads the vd80 hardware buffer for channel\n");
		printf("\n");
		printf("Where: Channel in [1..16]\n");
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
		if ((v->Number>=1) && (v->Number<=VD80_LIB_CHANNELS))
			chn = v->Number;
		else {
			fprintf(stderr,"Bad channel number:%d Not in [1..16]\n",(int) v->Number);
			return arg;
		}
	}

	if (!bsze) {
		fprintf(stderr,"No buffer allocated yet\n");
		return arg;
	}

	err = vd80GetBuffer(vd80,chn,&buffer);
	if (err != VD80_LIB_ERR_SUCCESS) {
		fprintf(stderr,"%s\n",vd80ErrToStr(err));
		perror("vd80GetBuffer");
		return arg;
	}

	printf("Buffer samples:\n");
	printf("   Actual buffer size  :%d\n",buffer.asze);
	printf("   Trigger position    :%d\n",buffer.tpos);
	printf("   Post trigger samples:%d\n",buffer.post);

	return arg;
}

/* ========================================================== */

int print_samp(int arg)
{
	ArgVal *v;
	AtomType at;
	int i, j, ix, start, count;
	int16_t *sp;
	char hex[32];

	if (show_help(arg,"Print samples: Start, Count")) {
		arg++;
		return arg;
	}

	arg++;

	start = buffer.tpos;
	v = &(vals[arg]);
	at = v->Type;
	if (at == Numeric) {
		arg++;
		start = v->Number;
	}

	count = buffer.post;
	v = &(vals[arg]);
	at = v->Type;
	if (at == Numeric) {
		arg++;
		count = v->Number;
	}

	if (!bsze) {
		fprintf(stderr,"No buffer allocated yet\n");
		return arg;
	}

	printf("Print samples:\n");
	printf("   Actual buffer size  :%d\n",buffer.asze);
	printf("   Trigger position    :%d\n",buffer.tpos);
	printf("   Post trigger samples:%d\n",buffer.post);

	printf("\n   --- Start:%d Count:%d ---\n",start,count);
	sp = (short *) buffer.addr;
	for (i=start, j=0; i<(start+count); i++, j++) {

		if (i>buffer.bsze)
			break;

		if (i== buffer.tpos) {
			sprintf(hex,"trig");
			ix = 0;
		} else if (i>buffer.asze) {
			sprintf(hex,"----");
			ix = 0;
		} else {
			sprintf(hex,"%04X",sp[i]);
			ix = strlen(hex) -4;
		}

		printf("%6d(%04d):%6d [%s] ",i,j,sp[i],&hex[ix]);

		if (((i + 1) % 4) == 0)
			printf("\n");
	}
	printf("\n   --- End ---\n");
	return arg;
}
/* ========================================================== */

int swap_buf(int arg)
{
	ArgVal *v;
	AtomType at;
	int j, ifl, wfl, bfl, csze, isze;


	uint32_t *ip, i;
	int16_t *sp, w;
	char *cp, c;

	if (show_help(arg,"Swap words/bytes: Int_flag, Words_flag, Bytes_flag")) {
		arg++;
		return arg;
	}

	arg++;

	ifl = 1;
	v = &(vals[arg]);
	at = v->Type;
	if (at == Numeric) {
		arg++;
		ifl = v->Number;
	}

	wfl = 0;
	v = &(vals[arg]);
	at = v->Type;
	if (at == Numeric) {
		arg++;
		wfl = v->Number;
	}

	bfl = 0;
	v = &(vals[arg]);
	at = v->Type;
	if (at == Numeric) {
		arg++;
		bfl = v->Number;
	}

	if (!bsze) {
		fprintf(stderr,"No buffer allocated yet\n");
		return arg;
	}

	printf("Swap:");

	if (ifl) {
		printf("Integers ");
		ip = (uint32_t *) buffer.addr;
		isze = bsze >> 1;
		for (j=0; j<isze; j+=2) {
			i       = ip[j];
			ip[j]   = ip[j+1];
			ip[j+1] = i;
		}
	}

	if (wfl) {
		printf("Words ");
		sp = buffer.addr;
		for (j=0; j<bsze; j+=2) {
			w       = sp[j];
			sp[j]   = sp[j+1];
			sp[j+1] = w;
		}
	}

	if (bfl) {
		printf("Bytes ");
		cp = (char *) buffer.addr;
		csze = bsze << 1;
		for (j=0; j<csze; j+=2) {
			c       = cp[j];
			cp[j]   = cp[j+1];
			cp[j+1] = c;
		}
	}

	printf("Done:%d\n",bsze);

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

#define REGS 56

struct reg_name_s names[REGS] = {

	{"FIFO",           0x00, 0},
	{"GCR1",           0x10, 1},
	{"GCR2",           0x14, 2},
	{"GSR",            0x18, 3},
	{"CCR",            0x1c, 4},

	{"TCR1",           0x20, 5},
	{"TCR2",           0x24, 6},
	{"TSR",            0x28, 7},
	{"SSR",            0x2c, 8},

	{"ADCR1",          0x30, 9},
	{"ADCR2",          0x34, 10},
	{"ADCR3",          0x38, 11},
	{"ADCR4",          0x3c, 12},

	{"ADCR5",          0x40, 13},
	{"ADCR6",          0x44, 14},
	{"ADCR7",          0x48, 15},
	{"ADCR8",          0x4c, 16},

	{"MCR1",           0x50, 17},
	{"MCR2",           0x54, 18},
	{"PTSR",           0x58, 19},
	{"PTCR",           0x5c, 20},

	{"CALR",           0x60, 21},

	{"TCR3",           0x64, 22},

	{"SUBSAMPCR",      0x70, 23},

	{"SUBSAMP_CHAN1",  0x80, 24},
	{"SUBSAMP_CHAN2",  0x84, 25},
	{"SUBSAMP_CHAN3",  0x88, 26},
	{"SUBSAMP_CHAN4",  0x8c, 27},
	{"SUBSAMP_CHAN5",  0x90, 28},
	{"SUBSAMP_CHAN6",  0x94, 29},
	{"SUBSAMP_CHAN7",  0x98, 30},
	{"SUBSAMP_CHAN8",  0x9c, 31},
	{"SUBSAMP_CHAN9",  0xa0, 32},
	{"SUBSAMP_CHAN10", 0xa4, 33},
	{"SUBSAMP_CHAN11", 0xa8, 34},
	{"SUBSAMP_CHAN12", 0xac, 35},
	{"SUBSAMP_CHAN13", 0xb0, 36},
	{"SUBSAMP_CHAN14", 0xb4, 37},
	{"SUBSAMP_CHAN15", 0xb8, 38},
	{"SUBSAMP_CHAN16", 0xbc, 39},

	{"ATRIG_CHAN1",    0xc0, 40},
	{"ATRIG_CHAN2",    0xc4, 41},
	{"ATRIG_CHAN3",    0xc8, 42},
	{"ATRIG_CHAN4",    0xcc, 43},
	{"ATRIG_CHAN5",    0xd0, 44},
	{"ATRIG_CHAN6",    0xd4, 45},
	{"ATRIG_CHAN7",    0xd8, 46},
	{"ATRIG_CHAN8",    0xdc, 47},
	{"ATRIG_CHAN9",    0xe0, 48},
	{"ATRIG_CHAN10",   0xe4, 49},
	{"ATRIG_CHAN11",   0xe8, 50},
	{"ATRIG_CHAN12",   0xec, 51},
	{"ATRIG_CHAN13",   0xf0, 52},
	{"ATRIG_CHAN14",   0xf4, 53},
	{"ATRIG_CHAN15",   0xf8, 54},
	{"ATRIG_CHAN16",   0xfc, 55}
};

#define EREGLEN 32

void edit_regs(uint32_t regn)
{
	struct vd80_riob_s rio;
	struct reg_name_s *rnm;
	vd80_err_t err;
	uint32_t regvi, regvo, i;
	char txt[EREGLEN] , *ep;

	while (1) {

		if (regn>=REGS) {
			regn = 0;
			printf("\n---\n");
		}
		rnm = &names[regn];

		rio.boffs = rnm->boffs;
		rio.bsize = sizeof(uint32_t);
		rio.buffer = &regvi;

		err = vd80RawIo(vd80,&rio,VD80_IO_DIR_READ);
		if (err != VD80_LIB_ERR_SUCCESS) {
			fprintf(stderr,"%s\n",vd80ErrToStr(err));
			perror("vd80RawIo");
			break;
		}

		printf("%15s:[0x%02X](%02d):0x%08x/",rnm->name,rnm->boffs,rnm->regnum,regvi);
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
		regvo = strtoul(txt,&ep,16);
		if (ep != txt) {
			rio.boffs = rnm->boffs;
			rio.bsize = sizeof(uint32_t);
			rio.buffer = &regvo;

			err = vd80RawIo(vd80,&rio,VD80_IO_DIR_WRITE);
			if (err != VD80_LIB_ERR_SUCCESS) {
				fprintf(stderr,"%s\n",vd80ErrToStr(err));
				perror("vd80RawIo");
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

	if (show_help(arg,"Raw IO 32-Bit Register editor: RegNum")) {
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
