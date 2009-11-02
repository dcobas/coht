#ifndef _CVORB_REG_ID_H_INCLUDE_
#define _CVORB_REG_ID_H_INCLUDE_

#include "dg/ServiceRegId.h"

/* CVORB module registers ID. Used by DAL */
typedef enum _tag_CVORB_rid {
  INT_SRC_ID = LAST_SRV_REG, /* Interrupt sources (RWMODE r) */
  INT_EN_ID, /* Interrupt enable mask (RWMODE rw) */
  INT_L_ID, /* Interrupt level (RWMODE rw) */
  INT_V_ID, /* Interrupt vector (RWMODE rw) */
  VHDL_V_ID, /* VHDL version (RWMODE rc) */
  PCB_SN_H_ID, /* PCB serial number (MSBs) (RWMODE r) */
  PCB_SN_L_ID, /* PCB serial number (LSBs) (RWMODE r) */
  TEMP_ID, /* Temperature (RWMODE r) */
  ADC_ID, /* ADC (RWMODE r) */
  SOFT_PULSE_ID, /* Soft pulses (RWMODE w) */
  EXT_CLK_FREQ_ID, /* External clock frequency (RWMODE r) */
  CLK_GEN_CNTL_ID, /* Clock generator control (RWMODE rw) */
  MOD_STAT_ID, /* Module status (RWMODE r) */
  MOD_CFG_ID, /* Module config (RWMODE rw) */
  DAC_VAL_ID, /* DAC value (RWMODE rw) */
  SRAM_SA_ID, /* SRAM start address (RWMODE rw) */
  SRAM_DATA_ID, /* SRAM data (RWMODE rw) */
  WAVE_L_ID, /* Waveform length (RWMODE rw) */
  WAVE_SA_ID, /* Waveform start address (RWMODE rw) */
  REC_CYC_ID, /* Recurrent cycles (RWMODE rw) */
  DAC_CNTL_ID, /* DAC control (RWMODE rw) */
  CH_STAT_ID, /* Channel status (RWMODE r) */
  CH_CFG_ID, /* Channel config (RWMODE rw) */
  FUNC_SEL_ID, /* Function selection (RWMODE rw) */
  FCT_EM_H_ID, /* Fct enable mask (63 to 32) (RWMODE rw) */
  FCT_EM_L_ID, /* Fct enable mask (31 to 0) (RWMODE rw) */
  SLOPE_ID, /* Slope (RWMODE rw) */
  CH_REC_CYC_ID, /* Recurrent cycles (RWMODE rw) */
} CVORB_rid_t;

#endif /* _CVORB_REG_ID_H_INCLUDE_ */
