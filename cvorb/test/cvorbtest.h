#ifndef _CVORBTEST_H_
#define _CVORBTEST_H_

/*
 * Channel status contains a state machine encoded in four bits in the status register.
 * channel_state_machine gives the meaning of the various states.
 */
#define CVORB_CH_STATE_SHIFT 0x4
#define CVORB_CH_STATE_MASK 0xF
static const int CVORB_CH_MAX_STATES = 11;
static const char *channel_state_machine[] = {
        "Idle",
        "Load internal RAM",
        "Wait start event",
        "Init counters",
        "Enable function generation",
        "Wait stop event",
        "Stop event",
        "Internal stop",
        "Load next vector",
        "Reload internal RAM(after internal stop)",
        "Wait event start(internal stop)",
        "Write current vector(internal stop)"
};

/*
static const int SUBMODULE_STATUS_BITS_NR = 4;
static const char *submodule_status_bit_meaning[] = {
        "SubModule is ready\n",
        "SubModule SRAM access in progress\n",
        "SubModule is busy generating a waveform (OR between all channels)\n",
        "SubModule paused (OR between all channels)\n"
};
*/

#endif /* _CVORBTEST_H_ */
