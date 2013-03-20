/* ================================================ */
/*                                                  */
/* CNGS Synchronization software to warn Gran Sasso */
/* when the SPS extracts beam towards the Neutrino  */
/* target.                                          */
/*                                                  */
/* Julian Lewis AB/CO/HT CERN 28th Feb 2006         */
/* EMAIL: Julian.Lewis@CERN.ch                      */
/*        Place the word "nospam" in the subject    */
/*                                                  */
/* ================================================ */

#ifndef CNGS
#define CNGS

#include <stdint.h>

/* ============================================= */
/* Configuration defaults for network and timing */

/* SPS Extraction event currently 80ms forwarning */

#define SEX_FW_80FO_CT 331
#define CngsFW_EXTRACTION_NS 80000000
#define CngsCTIM SEX_FW_80FO_CT

/* Sync event occurs each basic period */

#define SX_SBP_CT 320
#define CngsSYNC SX_SBP_CT

/* Nano seconds in a millisecond and in one second */

#define NS_IN_MS 1000000
#define NS_IN_SEC 1000000000

/* Target IP address at Gran Sasso */

#define OPERA "192.84.135.95"
#define CngsTARGET OPERA

/* CERN network ID, packets are rejected if not from CERN */

#define CngsSOURCE_NETWORK "137.138"
#define CngsPORT 1234
#define CngsMAGIG_NUMBER 0xCEA7FADE

/* =========================================== */
/* Now the usual program definitions and types */

#define CngsOK 1
#define CngsFAIL 0
#define CngsMAX_ERRORS 20

typedef struct {
   time_t   Second;    /* UTC Second */
   uint32_t Nano;      /* Nano second in the second */
   uint32_t CTrain;    /* Machine millisecond in cycle */
   uint32_t Machine;   /* Machine (4 = SPS) */
 } CngsTime;

typedef struct {
   uint32_t MagicNumber;       /* Must be set correctly */
   CngsTime BasicPeriod;            /* Basic 1.2 second period */
   CngsTime StartCycle;             /* Start time of SPS cycle */
   CngsTime EndCycle;               /* End time of SPS cycle */
   CngsTime Extraction;             /* Next warning extraction */
   CngsTime SendTime;               /* Local time packet was sent */
   uint32_t SequenceNumber;    /* Packet sequence number  */
   uint8_t  CycleName[32];          /* Name of cycle */
 } CngsPacket;

#endif
