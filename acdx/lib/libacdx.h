/* ================================================ */
/* Simply library interface to the AC Dipole system */
/* Julian Lewis AB/CO/HT Fri 19th Sept 2008         */
/* ================================================ */

typedef enum {
   AcdxLibOK,
   AcdxLibFAIL
 } AcdxLibCompletion;


AcdxLibCompletion AcdxLoadFunction(unsigned int freq,         /* Frequency in Hertz */
				   unsigned int ampl);        /* Amplitude in Milli-Volts */

AcdxLibCompletion AcdxArm();      /* Arm */

AcdxLibCompletion AcdxUnArm();    /* Set Arm bit to 0 */

int AcdxIsArmed();                /* Returns 1 if ARMED else 0 */

int AcdxIsBusy();                 /* Returns 1 if BUSY else 0 */

AcdxLibCompletion  AcdxAmpOn();   /* Set AmpOn status bit to ON */

AcdxLibCompletion  AcdxAmpOff();  /* Set AmpOn status bit to OFF */

int AcdxAmpIsOn();                /* Returns value of AmpOn status bit */
