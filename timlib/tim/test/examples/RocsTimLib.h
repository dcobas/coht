/* ========================================== */
/* Replace Tg8 functionality on ROCS by a CTR */
/* Julian Thu 21/Feb/07                       */

typedef enum {
   RocsTimLibErrorFAILED = 0,
   RocsTimLibErrorOK     = 1,
   RocsTimLibERRORS
 } RocsTimLibError;

/* ==================================================== */
/* Explicit initialize of the library.                  */
/* Attaches to the SPS telegram and initializes         */
/* settings and TimLib (with old style events allowed!) */
/* We need a standard AB/CO Front end and get_tgm_tim   */
/* must be running.                                     */

RocsTimLibError RocsTimLibInit();

/* ================================================================ */
/* Emulate setting up a TG8 action accross TimLib.                  */
/* The "data" word is bound to the "config" and will be returned by */
/* the wait routine should the config produce an interrupt.         */
/* If "config->uControl" is zero, we connect to a cable event, else */
/* I create a PTIM ID "next_ptim" to emulate the TG8 action.        */

RocsTimLibError RocsTimLibSet(Tg8User *config, unsigned short data);

/* ===================================================== */
/* Wait for a PTIM or a CTIM arriving from settings made */

RocsTimLibError RocsTimLibWait(unsigned long      *frame,    /* Full 32-bit event frame and payload */
			       unsigned short     *scnum,    /* Super-cycle number */
			       unsigned long      *msinsc,   /* Current milli-second in Super-cycle */
			       unsigned long      *sclenms,  /* Super-cycle length in milli-seconds */
			       unsigned short     *data,     /* Setting data */
			       unsigned long long *cystamp); /* Cycle stamp in nano-seconds */

