/**
 * @file ad9516o.c
 *
 * @brief ad9516 chip library
 *
 * Adopted from CVORB test program, written by
 * Emilio G. Cota <emilio.garcia.cota@cern.ch>
 *
 * @author Copyright (C) 2009 CERN. Yury GEORGIEVSKIY <ygeorgie@cern.ch>
 *
 * @date Created on 16/11/2009
 *
 * @section license_sec License
 *          Released under the GPL
 */
#include <sys/ioctl.h>
#include <stdint.h>
#include <math.h>
#include <CvorbUserDefinedAccess.h>
#include <CvorbDrvrDefs.h>
#include <ad9516o-drvr.h>

#define TEST_MAXDIFF            0
#define TEST_VCO_FREQ           2800000000UL

/*
 * given a desired output frequency and constant f_vco, check if just playing
 * with the output dividers can suffice. If it does, then return 0. Otherwise,
 * return -1, keeping the best dividers found.
 */
static int ad9516o_get_dividers(int freq, struct pll *pll)
{
        double diff;
        double mindiff = 1e10;
        int dvco, d1, d2;
        int i, j, k;

        /*
         * Note: this loop is sub-optimal, since sometimes combinations
         * that lead to the same result are tried.
         * For more info on this, check the octave (.m) files in /scripts.
         */
        for (i = 2; i <= 6; i++) {
                for (j = 1; j <= 32; j++) {
                        for (k = j; k <= 32; k++) {
                                diff = (double)TEST_VCO_FREQ / i / j / k - freq;
                                if (fabs(diff) <= (double)TEST_MAXDIFF)
                                        goto found;
                                if (fabs(diff) < fabs(mindiff)) {
                                        mindiff = diff;
                                        dvco = i;
                                        d1 = j;
                                        d2 = k;
                                }
                        }
                }
        }
        pll->dvco = dvco;
        pll->d1 = d1;
        pll->d2 = d2;
        return -1;
 found:
        pll->dvco = i;
        pll->d1 = j;
        pll->d2 = k;
        return 0;
}

/*
 * Here we consider the dividers fixed, and play with N, R to calculate
 * a satisfactory f_vco, given a certain required output frequency @freq
 */
static void ad9516o_calc_pll_params(int freq, struct pll *pll)
{
        double n, nr;
        /*
         * What's the required N / R ?
         * N / R = Dividers * freq / fref, where Dividers = dvco * d1 * d1
         * and fref = CVORG_OSCILLATOR_FREQ
         */
        nr = (double)pll->dvco * pll->d1 * pll->d2 * freq / OSCILLATOR_FREQ;

        n = nr * pll->r;
        /*
         * No need to touch R or P; let's just re-calculate B and A
         */
        pll->b = ((int)n) / pll->p;
        pll->a = ((int)n) % pll->p;
}

int ad9516o_check_pll(struct pll *pll)
{
        int err = 0;
        double fvco = (double)OSCILLATOR_FREQ *
                (pll->p * pll->b + pll->a) / pll->r;

        if (pll->a > pll->b) {
                printf("Warning: pll->a > pll->b\n");
                err = -1;
        }

        if (pll->a > 63) {
                printf("Warning: pll->a out of range\n");
                err = -1;
        }

        if (pll->p != 16 && pll->p != 32) {
                printf("Warning: pll-> p != {16,32}\n");
                err = -1;
        }

        if (pll->b <= 0 || pll->b > 8191) {
                printf("Warning: b out of range\n");
                err = -1;
        }

        if (pll->r <= 0 || pll->r > 16383) {
                printf("Warning: r out of range\n");
                err = -1;
        }

        if (fvco < 2.55e9 || fvco > 2.95e9) {
                printf("Warning: fvco (%g) out of range\n", fvco);
                err = -1;
        }

        return err;
}

static int ad9516o_fill_pll_conf(int freq, struct pll *pll)
{
	/*
         * PLL defaults: fvco = 2.7GHz, f_output = 100MHz, for the given
         * fref = 40MHz
         */
        if (!freq) {
		*pll = (struct pll) {
			.a              = 0,
			.b              = 4375,
			.p              = 16,
			.r              = 1000,
			.dvco           = 2,
			.d1             = 14,
			.d2             = 1,
			.force          = 0,
			.external       = 0
		};
                return 0;
        }

        /* initial sanity check */
        if (freq < 416000) {
                printf("The minimum sampling freq is 416KHz\n");
                return -1;
        }

        /*
         * Algorithm for adjusting the PLL
         * 1. set f_vco = 2.8 GHz
         *      N = (p * b) + a -> N = 70000
         *      f_vco = f_ref * N / R = 40e6 * 70000 = 2.8e9 Hz
         */
        pll->a = 0;
        pll->b = 4375;
        pll->p = 16;
        pll->r = 1000;
        pll->external = 0;

        /*
         * 2. check if playing with the dividers we can achieve the required
         *    output frequency (@freq) without changing the f_vco above.
         *    If a exact match is not possible, then return the closest result.
         */
        if (ad9516o_get_dividers(freq, pll)) {
                /*
                 * 2.1 Playing with the dividers is not enough: tune the VCO
                 *     by adjusting the relation N/R.
                 */
                ad9516o_calc_pll_params(freq, pll);
        }

        /* 3. Check the calculated PLL configuration is feasible */
        return ad9516o_check_pll(pll);
}

static double ad9516o_print_pll(struct pll *pll)
{
        unsigned long n = pll->p * pll->b + pll->a;
        double fvco, fout;

        fvco = (double)OSCILLATOR_FREQ * n / pll->r;

        printf("debug: fref=%g, n=%lu, r=%d\n", (double)OSCILLATOR_FREQ,
	       n, pll->r);

        printf("PLL params: [P=%d, B=%d, A=%d, R = %d]\n"
	       "\tdvco=%d, d1=%d, d2=%d\n",
	       pll->p, pll->b, pll->a, pll->r, pll->dvco, pll->d1, pll->d2);

        printf("f_vco = f_ref * N / R, where N = PxB + A\n"
                "\tf_ref = %g Hz\n"
                "\tN = %lu\n"
	       "\tf_vco = %g Hz\n",
	       (double)OSCILLATOR_FREQ, n, fvco);

	fout  = fvco / pll->dvco / pll->d1 / pll->d2;
        printf("f_out: %g Hz\n", fout);
	return fout;
}

/**
 * @brief Enable onboard generator
 *
 * @param fd   -- open driver node
 * @param iocn -- ioctl number to call
 * @param freq -- desired frequency
 *                0 -- use default (that is 100MHz)
 *
 * @return   0 -- if OK
 * @return < 0 -- if FAILED
 */
int ad9516o_on(int fd, int iocn, int freq)
{
	struct pll pll;
	ad9516o_fill_pll_conf(freq, &pll);
	return ioctl(fd, iocn, &pll);
}

/**
 * @brief Disable onboard generator. TODO.
 *
 * @param fd   -- open driver node
 * @param iocn -- ioctl number to call
 *
 * @return   0 -- if OK
 * @return < 0 -- if FAILED
 */
int ad9516o_off(int fd, int iocn)
{
	return -1;
}

/**
 * @brief Current Clock generator frequency.
 *
 * @param fd  -- open driver node
 * @param str -- human-readable form goes here, if not NULL
 *               Should be freen by the caller afterwards
 *
 * @return   0 -- if OK
 * @return < 0 -- if FAILED
 */
double ad9516o_get_freq(int fd, char **str)
{
	struct pll pll = { 0 };

	ioctl(fd, AD9516_GET_PLL, &pll);
	return ad9516o_print_pll(&pll);
}
