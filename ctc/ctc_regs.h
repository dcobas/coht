#ifndef _CTC_REGS_H
#define _CTC_REGS_H

#ifdef __cplusplus
extern "C" {
#endif

/* struct encore_reginfo definition needed */

extern struct encore_reginfo ctc_registers[];
extern int ctc_nregs;

enum ctc_register_id {
	STATUS,
	CNTR_ENABLE,
	confChan,
	clock1Delay,
	clock2Delay,
	outputCntr,
	cntr1CurVal,
	cntr2CurVal,
	channel_1,
	channel_2,
	channel_3,
	channel_4,
	channel_5,
	channel_6,
	channel_7,
	channel_8,
	ALL_CHANNELS,

};

#ifdef __cplusplus
}
#endif
#endif /* _CTC_REGS_H */
