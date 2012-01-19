#ifndef _AD9516_H_
#define _AD9516_H_

#define AD9516_VCO_FREQ		2.8e9L
#define AD9516_VCO_FREQ_MIN	2.55e9L
#define AD9516_VCO_FREQ_MAX	2.95e9L
#define AD9516_MAXDIFF		0
#define AD9516_OSCILLATOR_FREQ	40e6L

#define AD9516_MINFREQ		416000

/**
 * @brief PLL configuration structure
 * @a - [0-63]
 * @b - 13 bits [0-8191]
 * @p - 16 or 32
 * @r - 14 bits [0-16383]
 * @dvco - [1,6]
 * @d1 - [1,32]
 * @d2 - [1,32]
 * @external - set to 0 to use the internal PLL; 1 to use EXTCLK
 * @ext_clk_pll - set to 0 to use to use the internal oscillator in the PLL; 1 to use EXTCLK in the PLL.
 */
struct ad9516_pll {
	int a;
	int b;
	int p;
	int r;
	int dvco;
	int d1;
	int d2;
	int external;
	int ext_clk_pll;
	int input_freq;
};

#endif /* _AD9516_H_ */
