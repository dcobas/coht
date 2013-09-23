/*******************************************/
/* Useful macro-definitions                */
/* to manipulate with the Control Word.    */
/* V. Kovaltsov for SL Timing, April, 1997 */
/*******************************************/

/* INT/OUT field */

#define Tg8CW_INT_Get(cw)   (((cw)>>Tg8CW_INT_BITN) & Tg8CW_INT_BITM)
#define Tg8CW_INT_Set(cw,v) ((cw) |= ((v)<<Tg8CW_INT_BITN))
#define Tg8CW_INT_Clr(cw)   ((cw) &= (~(Tg8CW_INT_BITM<<Tg8CW_INT_BITN)))

/* COUNTER number field */

#define Tg8CW_CNT_Get(cw)   (((cw)>>Tg8CW_CNT_BITN) & Tg8CW_CNT_BITM) /*1-7,0*/
#define Tg8CW_CNT_Set(cw,v) ((cw) |= ((v)<<Tg8CW_CNT_BITN))
#define Tg8CW_CNT_Clr(cw)   ((cw) &= (~(Tg8CW_CNT_BITM<<Tg8CW_CNT_BITN)))

/* START mode field */

#define Tg8CW_START_Get(cw)   (((cw)>>Tg8CW_START_BITN) & Tg8CW_START_BITM)
#define Tg8CW_START_Set(cw,v) ((cw) |= ((v)<<Tg8CW_START_BITN))
#define Tg8CW_START_Clr(cw)   ((cw) &= (~(Tg8CW_START_BITM<<Tg8CW_START_BITN)))

/* CLOCK type filed */

#define Tg8CW_CLOCK_Get(cw)   (((cw)>>Tg8CW_CLOCK_BITN) & Tg8CW_CLOCK_BITM)
#define Tg8CW_CLOCK_Set(cw,v) ((cw) |= ((v)<<Tg8CW_CLOCK_BITN))
#define Tg8CW_CLOCK_Clr(cw)   ((cw) &= (~(Tg8CW_CLOCK_BITM<<Tg8CW_CLOCK_BITN)))

/* STATE field */

#define Tg8CW_STATE_Get(cw)   (((cw)>>Tg8CW_STATE_BITN) & Tg8CW_STATE_BITM)
#define Tg8CW_STATE_Set(cw,v) ((cw) |= ((v)<<Tg8CW_STATE_BITN))
#define Tg8CW_STATE_Clr(cw)   ((cw) &= (~(Tg8CW_STATE_BITM<<Tg8CW_STATE_BITN)))

/* PPM LINE STATE field */

#define Tg8CW_PPML_Get(cw)   (((cw)>>Tg8CW_PPML_BITN) & Tg8CW_PPML_BITM)
#define Tg8CW_PPML_Set(cw,v) ((cw) |= ((v)<<Tg8CW_PPML_BITN))
#define Tg8CW_PPML_Clr(cw)   ((cw) &= (~(Tg8CW_PPML_BITM<<Tg8CW_PPML_BITN)))

#define Tg8CW_MASK ((Tg8CW_INT_BITM<<Tg8CW_INT_BITN)|(Tg8CW_CNT_BITM<<Tg8CW_CNT_BITN)| \
		    (Tg8CW_START_BITM<<Tg8CW_START_BITN)|(Tg8CW_CLOCK_BITM<<Tg8CW_CLOCK_BITN)| \
		    (Tg8CW_STATE_BITM<<Tg8CW_STATE_BITN))

/*eof*/
