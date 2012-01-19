#ifndef _AD9516_LIB_H_
#define _AD9516_LIB_H_

#include <ad9516.h>

extern int ad9516_fill_pll_conf(unsigned int freq, struct ad9516_pll *pll);
extern int ad9516_fill_pll_conf_ext_clk(unsigned int freq, struct ad9516_pll *pll, unsigned int ext_clk_freq);
#endif /* _AD9516_LIB_H_ */
