#ifndef _CVORB_GET_SET_REG_H_INCLUDE_
#define _CVORB_GET_SET_REG_H_INCLUDE_

/*
  These functions are used to deliver register values directly to the user
  space.

  API is the following:
  1 param -- statics table

  2 param -- ioctl argument in predefined format:
             Massive of 3 elements, each is 4 bytes long.
             [0] - user-space address
             [1] - number of elements to r/w
             [2] - element index, starting from zero

             In case of service registers -- ioctl arguments can vary.
             Their amount depends on specific ioctl number.
             See service routines (those are with __SRV__ subword)
             for more details on parameter amount.

             For example, if this is a repetitive r/w request
             (ioctl number is SRV__REP_REG_RW) then we should have 4 arguments,
             that are packed as follows:

             [0] -- ioctl number
             [1] -- user-space address
             [2] -- number of elements to r/w
             [3] -- element index, starting from zero

  3 param -- check r/w bounds (1 - yes, 0 - no)
             valid only in case of Lynx
  4 param -- repeatedly read register (1 - yes, 0 - no)


  Bear in mind, that r/w operation results goes diretly to the user space.
  If you want to operate on the HW registers inside the driver -- use
  low-level port operation functions from port_ops_[linux/lynx].h like:
  __inb      -- read a byte from a port
  __inw      -- read a word from a port
  __in       -- lread a long from a port
  __outb     -- write a byte to a port
  __outw     -- write a word to a port
  __outl     -- write a long to a port
  __rep_inb  -- read multiple bytes from a port into a buffer
  __rep_inw  -- read multiple words from a port into a buffer
  __rep_inl  -- read multiple longs from a port into a buffer
  __rep_outb -- write multiple bytes to a port from a buffer
  __rep_outw -- write multiple words to a port from a buffer
  __rep_outl -- write multiple longs to a port from a buffer

  These functions are used to r/w HW registers inside the driver.
  Never access registers directly. Use this function to do this.
*/

/* Service register operations */
int get___SRV__DEBUG_FLAG(register CVORBStatics_t*, char*, int, int);
int set___SRV__DEBUG_FLAG(register CVORBStatics_t*, char*, int, int);
int get___SRV__DEVINFO_T(register CVORBStatics_t*, char*, int, int);
int get___SRV__DRVR_VERS(register CVORBStatics_t*, char*, int, int);
int get___SRV__DAL_CONSISTENT(register CVORBStatics_t*, char*, int, int);

/* Interrupt sources */
int get_INT_SRC(register CVORBStatics_t*, char*, int, int);

/* Interrupt enable mask */
int get_INT_EN(register CVORBStatics_t*, char*, int, int);
int set_INT_EN(register CVORBStatics_t*, char*, int, int);

/* Interrupt level */
int get_INT_L(register CVORBStatics_t*, char*, int, int);
int set_INT_L(register CVORBStatics_t*, char*, int, int);

/* Interrupt vector */
int get_INT_V(register CVORBStatics_t*, char*, int, int);
int set_INT_V(register CVORBStatics_t*, char*, int, int);

/* VHDL version */
int get_VHDL_V(register CVORBStatics_t*, char*, int, int);

/* PCB serial number (MSBs) */
int get_PCB_SN_H(register CVORBStatics_t*, char*, int, int);

/* PCB serial number (LSBs) */
int get_PCB_SN_L(register CVORBStatics_t*, char*, int, int);

/* Temperature */
int get_TEMP(register CVORBStatics_t*, char*, int, int);

/* ADC */
int get_ADC(register CVORBStatics_t*, char*, int, int);

/* Soft pulses */
int get_wo_SOFT_PULSE(register CVORBStatics_t*, char*, int, int);
int set_SOFT_PULSE(register CVORBStatics_t*, char*, int, int);

/* External clock frequency */
int get_EXT_CLK_FREQ(register CVORBStatics_t*, char*, int, int);

/* Clock generator control */
int get_CLK_GEN_CNTL(register CVORBStatics_t*, char*, int, int);
int set_CLK_GEN_CNTL(register CVORBStatics_t*, char*, int, int);

/* Module status */
int get_MOD_STAT(register CVORBStatics_t*, char*, int, int);

/* Module config */
int get_MOD_CFG(register CVORBStatics_t*, char*, int, int);
int set_MOD_CFG(register CVORBStatics_t*, char*, int, int);

/* DAC value */
int get_DAC_VAL(register CVORBStatics_t*, char*, int, int);
int set_DAC_VAL(register CVORBStatics_t*, char*, int, int);

/* SRAM start address */
int get_SRAM_SA(register CVORBStatics_t*, char*, int, int);
int set_SRAM_SA(register CVORBStatics_t*, char*, int, int);

/* SRAM data */
int get_SRAM_DATA(register CVORBStatics_t*, char*, int, int);
int set_SRAM_DATA(register CVORBStatics_t*, char*, int, int);

/* Waveform length */
int get_WAVE_L(register CVORBStatics_t*, char*, int, int);
int set_WAVE_L(register CVORBStatics_t*, char*, int, int);

/* Waveform start address */
int get_WAVE_SA(register CVORBStatics_t*, char*, int, int);
int set_WAVE_SA(register CVORBStatics_t*, char*, int, int);

/* Recurrent cycles */
int get_REC_CYC(register CVORBStatics_t*, char*, int, int);
int set_REC_CYC(register CVORBStatics_t*, char*, int, int);

/* DAC control */
int get_DAC_CNTL(register CVORBStatics_t*, char*, int, int);
int set_DAC_CNTL(register CVORBStatics_t*, char*, int, int);

/* All registers of channel 1 */
int get_CH1(register CVORBStatics_t*, char*, int, int);
int set_CH1(register CVORBStatics_t*, char*, int, int);

/* All registers of channel 2 */
int get_CH2(register CVORBStatics_t*, char*, int, int);
int set_CH2(register CVORBStatics_t*, char*, int, int);

/* All registers of channel 3 */
int get_CH3(register CVORBStatics_t*, char*, int, int);
int set_CH3(register CVORBStatics_t*, char*, int, int);

/* All registers of channel 4 */
int get_CH4(register CVORBStatics_t*, char*, int, int);
int set_CH4(register CVORBStatics_t*, char*, int, int);

/* All registers of channel 5 */
int get_CH5(register CVORBStatics_t*, char*, int, int);
int set_CH5(register CVORBStatics_t*, char*, int, int);

/* All registers of channel 6 */
int get_CH6(register CVORBStatics_t*, char*, int, int);
int set_CH6(register CVORBStatics_t*, char*, int, int);

/* All registers of channel 7 */
int get_CH7(register CVORBStatics_t*, char*, int, int);
int set_CH7(register CVORBStatics_t*, char*, int, int);

/* All registers of channel 8 */
int get_CH8(register CVORBStatics_t*, char*, int, int);
int set_CH8(register CVORBStatics_t*, char*, int, int);

/* Channel status */
int get_CH_STAT(register CVORBStatics_t*, char*, int, int);

/* Channel config */
int get_CH_CFG(register CVORBStatics_t*, char*, int, int);
int set_CH_CFG(register CVORBStatics_t*, char*, int, int);

/* Function selection */
int get_FUNC_SEL(register CVORBStatics_t*, char*, int, int);
int set_FUNC_SEL(register CVORBStatics_t*, char*, int, int);

/* Fct enable mask (63 to 32) */
int get_FCT_EM_H(register CVORBStatics_t*, char*, int, int);
int set_FCT_EM_H(register CVORBStatics_t*, char*, int, int);

/* Fct enable mask (31 to 0) */
int get_FCT_EM_L(register CVORBStatics_t*, char*, int, int);
int set_FCT_EM_L(register CVORBStatics_t*, char*, int, int);

/* Slope */
int get_SLOPE(register CVORBStatics_t*, char*, int, int);
int set_SLOPE(register CVORBStatics_t*, char*, int, int);

/* Recurrent cycles */
int get_CH_REC_CYC(register CVORBStatics_t*, char*, int, int);
int set_CH_REC_CYC(register CVORBStatics_t*, char*, int, int);

/* All channels to access in one chunk */
int get_ALL_CH(register CVORBStatics_t*, char*, int, int);
int set_ALL_CH(register CVORBStatics_t*, char*, int, int);

#endif /* _CVORB_GET_SET_REG_H_INCLUDE_ */
