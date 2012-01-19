#include <math.h>
#include <stdio.h>

#include <ad9516.h>

/*
 * given a desired output frequency and constant f_vco, check if just playing
 * with the output dividers can suffice. If it does, then return 0. Otherwise,
 * return -1, keeping the best dividers found.
 */
static int ad9516_get_dividers(int freq, struct ad9516_pll *pll)
{
	double diff;
	double mindiff = 1e10;
	int dvco, d1, d2;
	int i, j, k;
	double f_vco;

	f_vco = ((double)pll->input_freq/pll->r)*((double)pll->p * pll->b + pll->a);

	/*
	 * Note: this loop is sub-optimal, since sometimes combinations
	 * that lead to the same result are tried.
	 * For more info on this, check the octave (.m) files in /scripts.
	 */
	for (i = 2; i <= 6; i++) {
		for (j = 1; j <= 32; j++) {
			for (k = j; k <= 32; k++) {
				diff = f_vco / i / j / k - freq;
				if (fabs(diff) <= (double)AD9516_MAXDIFF)
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
static void __ad9516_calc_pll_params(int freq, struct ad9516_pll *pll, unsigned int ext_clk_freq, int ext_clk)
{
	double n, nr;
	double clk = AD9516_OSCILLATOR_FREQ;
	/*
	 * What's the required N / R ?
	 * N / R = Dividers * freq / fref, where Dividers = dvco * d1 * d1
	 * and fref = AD9516_OSCILLATOR_FREQ
	 */
	if (ext_clk)
		clk = ext_clk_freq;

	nr = (double)pll->dvco * pll->d1 * pll->d2 * freq / clk;

	n = nr * pll->r;
	/*
	 * No need to touch R or P; let's just re-calculate B and A
	 */
	pll->b = ((int)n) / pll->p;
	pll->a = ((int)n) % pll->p;

}


static void ad9516_calc_pll_params(int freq, struct ad9516_pll *pll)
{
    __ad9516_calc_pll_params(freq, pll, 0, 0);
}

static void ad9516_calc_pll_params_ext_clk(int freq, struct ad9516_pll *pll, unsigned int ext_clk_freq)
{
    pll->input_freq = ext_clk_freq;
    __ad9516_calc_pll_params(freq, pll, ext_clk_freq, 1);
}

static int ad9516_check_pll(struct ad9516_pll *pll, unsigned int ext_clk_freq, int ext_clk)
{
	int err = 0;
	double clk = AD9516_OSCILLATOR_FREQ;
	double fvco; 	

	if (ext_clk)
		clk = ext_clk_freq;
	
	fvco = clk * (pll->p * pll->b + pll->a) / pll->r;

	if (pll->a > pll->b) {
		err = -1;
	}

	if (pll->a > 63) {
		err = -1;
	}

	/* valid values are powers of two in 2..32 range) */
	if (pll->p < 2 || pll->p > 32 || (pll->p & (pll->p -1))) {
		err = -1;
	}

	if (pll->b <= 0 || pll->b > 8191) {
		err = -1;
	}

	if (pll->r <= 0 || pll->r > 16383) {
		err = -1;
	}

	if (fvco < 2.55e9 || fvco > 2.95e9) {
		err = -1;
	}

	return err;
}

int ad9516_get_fvco(unsigned int freq, struct ad9516_pll * pll, unsigned int ext_clk_freq)
{
	long int f_vco = 0;
	int a = 0;
	int b = 1;
	int p[5] = {32, 16, 8, 4, 2};
	int r;
	int i;
	double alpha, beta;

	/*
	 * Using r = 100 the maximum is 7.5e9 Hz that is beyond the maximum unsigned int value.
	 * Just in case, the comparation is maintained.
	 */
	if (ext_clk_freq < 1e6 || ext_clk_freq > 7.5e9)
		return -1;
	
	alpha = AD9516_VCO_FREQ_MIN / ext_clk_freq;
	beta = AD9516_VCO_FREQ_MAX / ext_clk_freq;
	
	// To be sure for getting valid values, the constant is 2.0 instead of 1.0
	//r = ceil(2.0/(beta - alpha));
	//if (r <= 0)
	//	return -1;

	/*
	 * Use of this constant value (r = 100) to allow the PLL to lock with the external clock.
	 * The frequency used by the client is 10 MHz. This constant allows to give
	 * proper values for the rest of parameters.
	 */
	r = 100;

	f_vco = ceil(r*alpha);

	if (f_vco < r*alpha || f_vco > r*beta)
		return -1;
	
	// Search the values of p, a, b.
	for (i = 0; i < 5; i++) {
		b = f_vco / p[i];
		a = f_vco % p[i];

		if (b <= 0 || b >= 8192)
			continue;
		if (a == 0 || (b > a && b > 3))
			goto found;

	}

	// Not found proper parameters, give error
	return -1;

found :
	pll->a = a;
	pll->b = b;
	pll->p = p[i]; 
	pll->r = r;
	pll->dvco = 1;
	pll->d1 = 1;
	pll->d2 = 1;
	pll->external = 0;

	return 0;
} 

int __ad9516_fill_pll_conf(unsigned int freq, struct ad9516_pll *pll, unsigned int ext_clk_freq, int ext_clk)
{
	if (!freq) {
		pll->a = 0;
		pll->b = 4375;
		pll->p = 16;
		pll->r = 1000;
		pll->dvco = 1;
		pll->d1 = 1;
		pll->d2 = 1;
		pll->external = 1;
		return 0;
	}

	/* initial sanity check */
	if (freq < AD9516_MINFREQ){
		return -1;
	}

	/*
	 * Algorithm for adjusting the PLL
	 */

	if (!ext_clk) {
		/*
	 	 * 1. set f_vco = 2.8 GHz
	 	 *	N = (p * b) + a -> N = 70000
	 	 *	f_vco = f_ref * N / R = 40e6 * 70000 = 2.8e9 Hz
	 	 */
		pll->a = 0;
		pll->b = 4375;
		pll->p = 16;
		pll->r = 1000;
		pll->external = 0;
	} else {
		// Search for proper parameter values.
		if (ad9516_get_fvco(freq, pll, ext_clk_freq)) {
			return -1;
		}
	}

	/*
	 * 2. check if playing with the dividers we can achieve the required
	 *    output frequency (@freq) without changing the f_vco above.
	 *    If a exact match is not possible, then return the closest result.
	 */
	if (ad9516_get_dividers(freq, pll)) {
		/*
		 * 2.1 Playing with the dividers is not enough: tune the VCO
		 *     by adjusting the relation N/R.
		 */
		if (ext_clk)
			ad9516_calc_pll_params_ext_clk(freq, pll, ext_clk_freq);
		else
			ad9516_calc_pll_params(freq, pll);
	}

	/* 3. Check the calculated PLL configuration is feasible */
	return ad9516_check_pll(pll, ext_clk_freq, ext_clk);
		
}

int ad9516_fill_pll_conf(unsigned int freq, struct ad9516_pll *pll)
{
	pll->ext_clk_pll = 0;
	pll->external = 0;
	pll->input_freq = AD9516_OSCILLATOR_FREQ;
	return __ad9516_fill_pll_conf(freq, pll, 0, 0);
}

int ad9516_fill_pll_conf_ext_clk(unsigned int freq, struct ad9516_pll *pll, unsigned int ext_clk_freq)
{
	pll->ext_clk_pll = 1;
	pll->external = 0; 	/* As we use the internal PLL this should be zero */
	pll->input_freq = ext_clk_freq;
	return __ad9516_fill_pll_conf(freq, pll, ext_clk_freq, 1);
}
