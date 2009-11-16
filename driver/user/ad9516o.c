/**
 * @file ad9516.c
 *
 * @brief provide control for the AD9516-O clock generator
 *
 * Copyright (c) 2009 CERN
 * @author Emilio G. Cota <emilio.garcia.cota@cern.ch>
 *
 * Copyright (c) 2009 CERN
 * @author Yury Georgievskiy <yury.georgievskiy@cern.ch>
 *
 * @section license_sec License
 * Released under the GPL v2. (and only v2, not any later version)
 */
#include <cdcm/cdcm.h>
#include <cdcm/cdcmBoth.h>
#include <cdcm/cdcmIo.h>
#include <ad9516o.h>

/*
 * define this if you want to debug the SPI interface with the AD9516
 */
#define DEBUG_SPI

#define AD9516_CALIB_DIVIDER	2
#define AD9516_SPI_SLEEP_US	5

static struct ad9516o ad9516o; /* device state */

/*
 * Note regarding locking: each module has a mutex to manage the access
 * to the CLKCTL register. Since you usually want to set-up a few parameters
 * of the AD9516 at once, the (single) read and write functions have no locked/
 * unlocked primitives. They're just unlocked but bear in mind that they're
 * meant to be called with the mutex held. Therefore, define functions with a
 * set of particular commands to do whatever you want to do, and protect *that*
 * set of operations properly.
 */

/**
 * @brief check if the data is up to date
 *
 * @param rval - CLKCTL register value
 * @param addr - address on the clockgen to be checked
 *
 * @return 1 - if OK
 * @return 0 - if not OK (data not up to date, or address mismatch)
 */
static int clkgen_ok(uint32_t rval, unsigned int addr)
{
	uint32_t readaddr = (rval & CLKCTL_ADDR) >> 8;

	if (rval & CLKCTL_UP2DATE && readaddr == addr)
		return 1;
	return 0;
}

/**
 * @brief read an address within the AD9516
 *
 * @param addr - address on the clockgen to read from
 *
 * @return value of the CLKCTL register
 */
static uint32_t clkgen_rval(unsigned int addr)
{
	uint32_t val;
	int i;

	cdcm_iowrite32be(AD9516_OP_READ | (addr << 8), ad9516o.ba);

	/* try a few times, although normally 5us should be enough */
	for (i = 0; i < 3; i++) {
		/* delay to allow the serial transfer to happen */
		usec_sleep(AD9516_SPI_SLEEP_US);
		val = cdcm_ioread32be(ad9516o.ba);
		if (clkgen_ok(val, addr))
			goto out;
	}
out:
	return val;
}

static uint32_t clkgen_read(unsigned int addr)
{
	uint32_t rval = clkgen_rval(addr);

	return rval & 0xff;
}

/**
 * @brief debug a write-to-clockgen operation
 *
 * @param addr - address on the clockgen that has (hopefully) been written to
 * @param data - data that has been written to addr
 *
 * If we cannot read back the data we've written, then print a debug message
 */
#ifdef DEBUG_SPI
static void debug_clkgen_write(unsigned int addr, uint8_t data)
{
	uint32_t rval = clkgen_rval(addr);

	if (!clkgen_ok(rval, addr)) {
		unsigned raddr = (rval & CLKCTL_ADDR) >> 8;
		unsigned rdata = rval & CLKCTL_DATA;

		kkprintf("[AD9516_INFO] SPI to AD9516-O Error: (expected/read) addr: 0x%0x"
			 "/0x%x, data=0x%0x/0x%x", addr, raddr, data, rdata);
	}
}
#else
static void debug_clkgen_write(unsigned int addr, uint8_t data)
{
}
#endif /* DEBUG_SPI */

/**
 * @brief write to Clock Generator register
 *
 * @param addr - address of the AD9516-O to write to (10 bits)
 * @param data - data to be written (8 bits)
 */
static void clkgen_write(unsigned int addr, uint8_t data)
{
	uint32_t cmd = AD9516_OP_WRITE | (addr << 8) | data;

	cdcm_iowrite32be(cmd, ad9516o.ba);
	usec_sleep(AD9516_SPI_SLEEP_US);

	debug_clkgen_write(addr, data);
}

static void clkgen_or(unsigned int addr, uint8_t data)
{
	uint32_t oldval = clkgen_read(addr) & 0xff;

	clkgen_write(addr, oldval | data);
}

static void clkgen_and(unsigned int addr, uint8_t data)
{
	uint32_t oldval = clkgen_read(addr) & 0xff;

	clkgen_write(addr, oldval & data);
}

/**
 * @brief sleep until the calibration of the VCO has finished
 *
 * @param r - configured parameter (divider) R of the AD9516's PLL
 * @param freqref - reference frequence (REFIN), in Hz
 *
 * The following math is derived from pg.40, 'VCO Calibration' of the datasheet
 *
 * fcal_clock = freqref / (R * cal_divider) [cycles/s]
 * 	--> hence fcal_clock / 1000 = fcal_clock_ms [cycles/ms]
 *
 * 4400 calibration cycles are needed: how much is that in ms?
 * 	--> 4400 [cycles] / fcal_clock [cycles/ms]
 *
 * re-arranging the operations to avoid overflow:
 * 	--> calibration_time [ms] = 4400 * R * cal_divider / (fcal_clock_ms)
 *
 * @return 0 - if the callibration was successful
 * @return -1 - if the callibration didn't happen in time
 */
static int calib_vco_sleep(int r, int freqref)
{
	int interval;

	/* first convert the reference to cycles/ms, rounding up */
	freqref = freqref / 1000 + !!(freqref % 1000);

	/* calculate the calibration time interval in ms, rounding up */
	interval = 4400 * r * AD9516_CALIB_DIVIDER;
	interval = interval / freqref + !!(interval % freqref);

	/* convert to tens of ms, rounding up */
	interval = interval / 10 + !!(interval % 10);

	/* sleep for 'interval' tens of ms */
	tswait(NULL, SEM_SIGIGNORE, interval);

	/* check for completion */
	if (!(clkgen_read(AD9516_PLLREADBACK) & 0x40))
			return -1;
	return 0;
}

static int clkgen_read_dvco(void)
{
	int dvco = clkgen_read(AD9516_VCO_DIVIDER);
	int inputclks = clkgen_read(AD9516_INPUT_CLKS);
	int retval;

	/* is it bypassed? */
	if (inputclks & 0x1)
		return 1;

	retval = (dvco & 0x7) + 2;

	if (retval >= 7) {
		kkprintf("[AD9516_INFO] Suspicious dvco value: 0x%x", retval);
		retval = 1;
	}

	return retval;
}

/**
 * @brief get the value of a divider
 *
 * @param offset - offset within the AD9516 memory map
 * @param nr - number of the divider (1 or 2)
 *
 * M - Low cycles divider
 * N - High cycles divider
 *
 * @return value of the divider
 */
static int clkgen_read_d(int offset, int nr)
{
	int val;
	int bypass = clkgen_read(offset + 3);
	int m, n;

	/* sanity check */
	if (!WITHIN_RANGE(1, nr, 2)) {
		kkprintf("[AD9516_WARN] Invalid divider nr: %d. Setting to 1", nr);
		nr = 1;
	}

	offset += nr - 1;
	val = clkgen_read(offset);

	m = (val & 0xf0) >> 4;
	n = val & 0x0f;

	bypass &= 1 << (4 +(--nr));

	if (bypass)
		return 1;

	return m + n + 2;
}

static int clkgen_read_a(void)
{
	return clkgen_read(AD9516_ACOUNT) & 0x3f;
}

static int clkgen_read_b(void)
{
	return clkgen_read(AD9516_BCOUNT_LSB) |
	     ((clkgen_read(AD9516_BCOUNT_MSB) & 0x1f) << 8);
}

static int clkgen_read_r(void)
{
	return clkgen_read(AD9516_RCOUNT_LSB) |
	     ((clkgen_read(AD9516_RCOUNT_MSB) & 0x3f) << 8);
}

static int clkgen_read_p(void)
{
	int prescaler = clkgen_read(AD9516_PLL1) & 0x7;
	int ret = 1;

	if (prescaler == 5)
		ret = 16;
	else if (prescaler == 6)
		ret = 32;
	else
		kkprintf("[AD9516_INFO] Prescaler P not 16 nor 32");

	return ret;
}

static void __get_pll_conf(struct pll *pll)
{
	/* get the PLL parameters */
	pll->a = clkgen_read_a();
	pll->b = clkgen_read_b();
	pll->r = clkgen_read_r();
	pll->p = clkgen_read_p();

	/* get non-pll dividers */
	pll->dvco = clkgen_read_dvco();
	pll->d1 = clkgen_read_d(AD9516_CMOSDIV3_1, 1);
	pll->d2 = clkgen_read_d(AD9516_CMOSDIV3_1, 2);
}


void init_ad9516(void *ba)
{
	cdcm_mutex_init(&ad9516o.clkgen_lock);
	cdcm_mutex_init(&ad9516o.pll_lock);
	ad9516o.ba = ba;
}

void get_pll_conf(struct pll *pll)
{
	cdcm_mutex_lock(&ad9516o.clkgen_lock);
	__get_pll_conf(pll);
	cdcm_mutex_unlock(&ad9516o.clkgen_lock);
}

/*
 * make the 'bypassed?' test easier: it will just check for !val.
 */
static void check_pll_dividers(struct pll *pll)
{
	if (pll->r == 1)
		pll->r = 0;
	if (pll->dvco == 1)
		pll->dvco = 0;

	if (pll->d1 == 1)
		pll->d1 = 0;
	if (pll->d2 == 1)
		pll->d2 = 0;

	if (!pll->d1 && !pll->d2)
		return; /* the checks below don't apply */
	/*
	 * pg. 45 of the datasheet: if only one divider is bypassed,
	 * it must be d2.
	 */
	if (!pll->d1 && pll->d2) {
		pll->d1 = pll->d2;
		pll->d2 = 0;
		return; /* do not clash with the check below */
	}

	/*
	 * pg. 45: if only one divider has an even divide by, it must be d2.
	 */
	if (pll->d1 % 2 && !(pll->d2 % 2)) {
		int tmp = pll->d1;

		pll->d1 = pll->d2;
		pll->d2 = tmp;
	}
}

static void clkgen_write_a(int a)
{
	clkgen_write(AD9516_ACOUNT, a & 0x3f);
}

static void clkgen_write_b(int b)
{
	clkgen_write(AD9516_BCOUNT_LSB, b & 0xff);
	clkgen_write(AD9516_BCOUNT_MSB, (b >> 8) & 0x3f);
}

static void clkgen_write_r(int r)
{
	clkgen_write(AD9516_RCOUNT_LSB, r & 0xff);
	clkgen_write(AD9516_RCOUNT_MSB, (r >> 8) & 0x3f);
}

static void clkgen_write_p(int p)
{
	int prescaler = p == 16 ? 0x5 : 0x6; /* see the module's docs */

	/* reset P */
	clkgen_and(AD9516_PLL1, ~0x07);

	/* write new P value */
	clkgen_or(AD9516_PLL1, prescaler);
}

static void clkgen_write_dvco(struct pll *pll)
{
	if (!pll->dvco) {
		if (pll->external) { /* bypassing is OK */
			clkgen_or(AD9516_INPUT_CLKS, 0x1);
			return;
		} else {
			kkprintf("[AD9516_WARN] Cannot bypass the VCO divider: setting to 2");
			pll->dvco = 2;
		}
	}

	if (!WITHIN_RANGE(2, pll->dvco, 6)) {
		kkprintf("[AD9516_WARN] Invalid d_vco: %d ([2-6]). Setting to 2", pll->dvco);
		pll->dvco = 2;
	}

	/* Note: the (-2) should be there, check the datasheet */
	clkgen_write(AD9516_VCO_DIVIDER, (pll->dvco - 2) & 0x7);
	/* no bypass */
	clkgen_and(AD9516_INPUT_CLKS, ~0x1);
}

/**
 * @brief write to memory the values of a CMOS divider
 *
 * @param divider - 3 or 4
 * @param nr - 1 or 2
 * @param d - value of the divider to be written
 *
 * M - Low cycles divider
 * N - High cycles divider
 * More info in the datasheet: pg 44 'Channel Dividers - LVDS/CMOS Outputs'
 */
static void clkgen_write_d(int divider, int nr, int d)
{
	unsigned int m, n;
	unsigned int divider_addr, bypass_addr, bypass_mask;

	/* sanity checks */
	if (!WITHIN_RANGE(3, divider, 4)) {
		kkprintf("[AD9516_INFO] Invalid divider: %d [3-4]. Ignoring.", divider);
		return;
	}
	if (!WITHIN_RANGE(1, nr, 2)) {
		kkprintf("[AD9516_INFO] Invalid divider nr: %d [1-2]. Ignoring.", nr);
		return;
	}

	/* get the correct offsets within the AD9516 memory map */
	if (divider == 3) {
		bypass_addr = AD9516_CMOSDIV3_BYPASS;
		if (nr == 1) {
			divider_addr = AD9516_CMOSDIV3_1;
			bypass_mask = 0x10;
		} else {
			divider_addr = AD9516_CMOSDIV3_2;
			bypass_mask = 0x20;
		}
	} else {
		bypass_addr = AD9516_CMOSDIV4_BYPASS;
		if (nr == 1) {
			divider_addr = AD9516_CMOSDIV4_1;
			bypass_mask = 0x10;
		} else {
			divider_addr = AD9516_CMOSDIV4_2;
			bypass_mask = 0x20;
		}
	}

	/* bypass? */
	if (!d) {
		clkgen_or(bypass_addr, bypass_mask);
		return;
	}

	d -= 2; /* D = M + N + 2 */
	m = d / 2 + d % 2; /* pg. 45: An odd D must be set as M = N + 1 */
	n = d / 2;
	clkgen_write(divider_addr, ((m & 0xf) << 4) | (n & 0xf));
	/* clear the bypass bit */
	clkgen_and(bypass_addr, ~bypass_mask);
}

static int pllcmp(struct pll *pll, struct pll *pll2)
{
	if (	pll->a != pll2->a || pll->b != pll2->b ||
		pll->p != pll2->p || pll->r != pll2->r) {
		return 1;
	}
	return 0;
}

static int __put_pll_conf(struct pll *pll)
{
	int ret = 0;

	check_pll_dividers(pll);

	if (pll->external)
		clkgen_write(AD9516_INPUT_CLKS, ~0x2);
	else
		clkgen_write(AD9516_INPUT_CLKS, 0x2);

	clkgen_write_a(pll->a);
	clkgen_write_b(pll->b);
	clkgen_write_r(pll->r);
	clkgen_write_p(pll->p);

	clkgen_write_dvco(pll);

	clkgen_write_d(3, 1, pll->d1);
	clkgen_write_d(3, 2, pll->d2);
	clkgen_write_d(4, 1, pll->d1);
	clkgen_write_d(4, 2, pll->d2);

	clkgen_write(AD9516_PLL3,		0x00);
	clkgen_write(AD9516_UPDATE_ALL,		0x01);
	clkgen_write(AD9516_PLL3,		0x01);
	clkgen_write(AD9516_UPDATE_ALL,		0x01);

	/* perform the update in the module context */
	cdcm_mutex_lock(&ad9516o.pll_lock);

	/* if the PLL configuration has changed, VCO calibration is needed */
	if (pllcmp(&ad9516o.pll, pll) && !pll->external) {
		if (calib_vco_sleep(pll->r, OSCILLATOR_FREQ)) {
			kkprintf("[AD9516_DBG] VCO calibration failed [PLL: A=%d B=%d P=%d "
				"R=%d d1=%d d2=%d dvco=%d]", pll->a, pll->b,
				pll->p, pll->r, pll->d1, pll->d2, pll->dvco);
			ret = -1;
		}
	}

	/* update the module's PLL description */
	memcpy(&ad9516o.pll, pll, sizeof(struct pll));

	cdcm_mutex_unlock(&ad9516o.pll_lock);

	return ret;
}

int put_pll_conf(struct pll *pll)
{
	int retval;

	cdcm_mutex_lock(&ad9516o.clkgen_lock);
	retval = __put_pll_conf(pll);
	cdcm_mutex_unlock(&ad9516o.clkgen_lock);

	return retval;
}

int check_pll(struct pll *pll)
{
	int err = 0;

	if (pll->a > pll->b)
		err = 1;
	if (pll->a > 63)
		err = 1;
	if (pll->p != 16 && pll->p != 32)
		err = 1;
	if (pll->b <= 0 || pll->b > 8191)
		err = 1;
	if (pll->r <= 0 || pll->r > 16383)
		err = 1;

	return err;
}

/**
 * @brief write default configuration to the AD9516-O Clock Generator
 *
 * Note. the module's clkgen_lock mutex must be held by the caller
 */
static int __clkgen_default_config(void)
{
	/*
	 * PLL defaults: fvco = 2.7GHz, f_output = 100MHz, for the given
	 * fref = 40MHz
	 */
	struct pll pll = {
		.a		= 0,
		.b		= 4375,
		.p		= 16,
		.r		= 1000,
		.dvco		= 2,
		.d1		= 14,
		.d2		= 1,
		.force		= 0,
		.external	= 0,
	};

	/*
	 * PFD polarity 0, CP current 4.8 mA, CP mode: Normal,
	 * PLL Operating mode: normal
	 */
	clkgen_write(AD9516_PDF_CP,	0x7c);

	/* CP Normal, No reset counters, No B bypass, P = 16 @ DM mode */
	clkgen_write(AD9516_PLL1,	0x05);

	/* STATUS: N divider output, Antibackslash pulse width: 2.9ns */
	clkgen_write(AD9516_PLL2,	0x04);

	/* Do nothing on !SYNC, PLL Divider delays: 0 ps */
	clkgen_write(AD9516_PLL4,	0x00);

	/*
	 * REF{1,2} frequency threshold: valid if freq is above the higher
	 * freq threshold, LD pin: Digital lock detech (high = lock)
	 */
	clkgen_write(AD9516_PLL5,	0x00);

	/*
	 * enable VCO freq monitor, disable REF2 freqmon, enable REF1 freqmon,
	 * REFMON: status of VCO frequency (active high)
	 */
	clkgen_write(AD9516_PLL6,	0xab);

	/*
	 * enable switchover deglitch, select REF1 for PLL, manual reference
	 * switchover, return to REF1 automatically when REF1 status is good
	 * again, REF2 power on, REF1 power on, differential referente mode
	 */
	clkgen_write(AD9516_PLL7,	0x07);

	/*
	 * PLL status register enable, disable LD pin comparator, holdover
	 * disabled, automatic holdover mode
	 */
	clkgen_write(AD9516_PLL8,	0x00);

	/*
	 * PFD cycles to determine lock: 5, high range lock detect window,
	 * normal lock detect operation, VCO calibration divider: 2
	 */
	clkgen_write(AD9516_PLL3,	0x00);

	/* power down the LVPECL outputs, they're not used */
	clkgen_write(AD9516_LVPECL_OUT0,	0x0b);
	clkgen_write(AD9516_LVPECL_OUT1,	0x0b);
	clkgen_write(AD9516_LVPECL_OUT2,	0x0b);
	clkgen_write(AD9516_LVPECL_OUT3,	0x0b);
	clkgen_write(AD9516_LVPECL_OUT4,	0x0b);
	clkgen_write(AD9516_LVPECL_OUT5,	0x0b);

	/*
	 * Configure CMOS outputs
	 *
	 * Non-inverting, turn off CMOS B output, LVDS logic level,
	 * current level: 3.5 mA (100 m-ohm), power on
	 */
	clkgen_write(AD9516_LVCMOS_OUT6,	0x42);
	clkgen_write(AD9516_LVCMOS_OUT7,	0x42);
	/*
	 * Non-inverting, turn off CMOS B output, CMOS logic level,
	 * current level: 1.75 mA (100 m-ohm), power on
	 */
	clkgen_write(AD9516_LVCMOS_OUT8,	0x48);
	clkgen_write(AD9516_LVCMOS_OUT9,	0x48);

	/* and configure the PLL */
	return __put_pll_conf(&pll);
}

/**
 * @brief write default configuration to the AD9516-O Clock Generator
 */
int clkgen_default_config(void)
{
	int retval;

	cdcm_mutex_lock(&ad9516o.clkgen_lock);
	retval = __clkgen_default_config();
	cdcm_mutex_unlock(&ad9516o.clkgen_lock);

	return retval;
}
