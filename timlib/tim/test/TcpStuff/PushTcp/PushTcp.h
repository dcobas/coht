/* ================================================ */
/*                                                  */
/* PushTcp Server                                   */
/* Send stdin to a client via a stream              */
/*                                                  */
/* Julian Lewis AB/CO/HT CERN 28th Feb 2006         */
/* EMAIL: Julian.Lewis@CERN.ch                      */
/*        Place the word "nospam" in the subject    */
/*                                                  */
/* ================================================ */

#ifndef PUSH_TCP
#define PUSH_TCP

/* =========================================== */
/* Now the usual program definitions and types */

#define BACK_LOG 8
#define FAIL 0
#define DEFAULT_PORT 1356
#define LOOPBACK "127.0.0.1"

#define SinSIZE (sizeof(struct sockaddr_in))

typedef enum {
   PORT,
   SERVER,
   DEBUG,
   REMOTE,
   OPTIONS
 } Options;

#endif

