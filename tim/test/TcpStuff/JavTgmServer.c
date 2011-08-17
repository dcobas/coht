/* ============================================================= */
/* Send packets to a Socket                                      */
/* Do it with streams in server client form                      */
/* Julian Lewis AB/CO/HT 23rd Feb 2006 Julian.Lewis@CERN.ch      */
/* For Email the Subject line should contain the string "nospam" */
/* ============================================================= */

#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/time.h>
#include <errno.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <signal.h>
#include <wait.h>
#include <unistd.h>

#include <err/err.h>            /* Error handling */
#include <dtm/dtm.h>
#include <tgv/tgv.h>
#include <tgm/tgm.h>

#include "JavTgm.h"

#define APP_NAME "JavTgmServer"

static int ssock = 0;
static int errors = 0;
static int debug = 0;
static unsigned short port = 0;
static TgmMachine mch = TgmMACHINE_NONE;

extern char *gbOptions[OPTIONS];

extern int _TgmSemIsBlocked (const char *sname);
extern int _TgmSemDoBlocked (char *sname);
extern CfgGetTgmXXX *_TgmCfgGetTgmXXX (void);
extern tConfig *GetConfiguration (char **);

extern pid_t getpgid (pid_t pid);

/* ============================================================= */
/* Open a UDP port for JavDtmServer data                         */
/* ============================================================= */

int OpenPort (unsigned short port)
{

    struct sockaddr_in sin;

    int s, on, off, val;

    if (!ssock) {

        s = socket (PF_INET, SOCK_STREAM, 0);
        if (s < 0) {
            ErrLog (ErrWARNING, APP_NAME ":OpenPort:Socket:Error:Cant open sockets (err=%s)", strerror (errno));
            return FAIL;
        }
        ssock = s;
        on = 1;
        off = 0;
        val = 5;
        setsockopt (ssock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (int));
        setsockopt (ssock, SOL_TCP, TCP_DEFER_ACCEPT, &off, sizeof (int));
        setsockopt (ssock, SOL_TCP, TCP_QUICKACK, &on, sizeof (int));
        setsockopt (ssock, SOL_TCP, TCP_KEEPCNT, &on, sizeof (int));
        setsockopt (ssock, SOL_TCP, TCP_KEEPIDLE, &val, sizeof (int));
        setsockopt (ssock, SOL_TCP, TCP_KEEPINTVL, &val, sizeof (int));
        setsockopt (ssock, SOL_TCP, TCP_LINGER2, &val, sizeof (int));
        setsockopt (ssock, SOL_TCP, TCP_NODELAY, &on, sizeof (int));
        setsockopt (ssock, SOL_TCP, TCP_SYNCNT, &val, sizeof (int));

        bzero ((void *) &sin, SinSIZE);
        sin.sin_family = PF_INET;
        sin.sin_port = htons (port);
        sin.sin_addr.s_addr = htonl (INADDR_ANY);
        if (bind (s, (struct sockaddr *) &sin, SinSIZE) < 0) {
            ErrLog (ErrWARNING, APP_NAME ":OpenPort:Bind:Error:Cant bind port:%d (Err=%s)", port, strerror (errno));
            close (s);
            ssock = 0;
            return FAIL;
        }
        if (listen (s, BACK_LOG) < 0) {
            ErrLog (ErrWARNING, APP_NAME ":Listen:Error:Port:%d (Err=%s)", port, strerror (errno));
            close (s);
            ssock = 0;
            return FAIL;
        }
    }
    return ssock;
}

/* ============================================================= */
/* Convert a packet to a string                                  */
/* ============================================================= */

char *HisBufToStr (TgmHistoryBuffer * hb)
{

    static char txt[JTS_PACKET_STRING_SIZE];

    EvtEvent *e = (EvtEvent *) hb;
    unsigned long *p, msk;
    int i;
    char tmp[32];

    bzero ((void *) txt, JTS_PACKET_STRING_SIZE);
    p = (unsigned long *) e;
    for (i = 0; i < EvtFIELDS; i++) {
        msk = 1 << i;
        if (e->FieldsMask & msk) {
            sprintf (tmp, "%lx ", p[i]);
            strcat (txt, tmp);
        }
        if (msk == EvtLAST_MASK)
            break;
    }

    p = (unsigned long *) e->Telegram;
    for (i = 0; i < 32; i++) {
        msk = 1 << i;
        if (e->SetGroups & msk) {
            sprintf (tmp, "%lx ", p[i]);
            strcat (txt, tmp);
        }
        else
            break;
    }

    strcat (txt, "\n");

    return txt;
}

/* ============================================================= */
/* Errlog handler                                                */
/* ============================================================= */

static int ok = 1;

static Boolean error_handler (class, text)
     ErrClass class;
     char *text;
{

    errors++;
    fprintf (stderr, "Errors:%d %s\n", errors, text);
    if (errors > MAX_ERRORS)
        ok = 0;
    return True;
}

/* ============================================================= */
/* Catch broken pipe signals when client dies  or times out      */
/* ============================================================= */

void sigpipehand (int signo, siginfo_t * info, void *context)
{
    ok = 0;
}

/* ============================================================= */
/* Catch Sig Child to clean up zombies                           */
/* ============================================================= */

void sigchildhand (int signo, siginfo_t * info, void *context)
{
    waitpid (-1, NULL, WNOHANG);
}

/* ============================================================= */
/* Catch Sig Child to clean up zombies                           */
/* ============================================================= */

void sighuphand (int signo, siginfo_t * info, void *context)
{
    exit (kill (0, SIGHUP));
}

/* ============================================================= */
/* Main: Proccess arguments and send events for ever             */
/* ============================================================= */

int main (int argc, char *argv[])
{


    struct sockaddr_in client;
    struct sigaction action, old;
    pid_t child;
    char bSemName[256];

    TgmHistoryBuffer *hb;
    int len, i, clsock, on;
    char machine[TgmNAME_SIZE], *cp, *ep;

    if (argc <= 1) {
        printf (APP_NAME ": Parameters are ...\n\n"
                "   -p  <PORT>        ;Sets the target host port number\n"
                "   -m  <TGM MACHINE> ;Sets which machines events are sent\n" "   -d                ;Set Debug printing on\n");
        exit (1);
    }

    ErrSetHandler ((ErrHandler) error_handler);

    /* Process command line arguments */

    for (i = 1; i < argc; i++) {

        cp = NULL;

        if (strncmp (argv[i], gbOptions[PORT], strlen (gbOptions[PORT])) == 0) {
            i++;
            cp = argv[i];
            if (cp)
                port = (unsigned short) strtoul (cp, &ep, 0);
            continue;
        }

        if (strncmp (argv[i], gbOptions[MACHINE], strlen (gbOptions[MACHINE])) == 0) {
            i++;
            cp = argv[i];
            if (cp) {
                strncpy (machine, cp, TgmNAME_SIZE);
                mch = TgmGetMachineId (machine);
            }
            continue;
        }

        if (strncmp (argv[i], gbOptions[DEBUG], strlen (gbOptions[DEBUG])) == 0) {
            debug = 1;
            continue;
        }

        ErrLog (ErrFATAL, APP_NAME ": No such option: %s", argv[i]);
        exit (1);
    }

    if (mch == TgmMACHINE_NONE) {
        ErrLog (ErrFATAL, APP_NAME ":No Tgm machine specified");
        exit (1);
    }

    bzero ((void *) &action, sizeof (action));
    action.sa_handler = (void (*)(int)) sigchildhand;
    sigaction (SIGCHLD, &action, &old);

    bzero ((void *) &action, sizeof (action));
    action.sa_handler = (void (*)(int)) sighuphand;
    sigaction (SIGHUP, &action, &old);

    if (OpenPort (port)) {

        printf (APP_NAME ": Port:%d TgmMachine:%s. Up and Running OK\n", (int) port, machine);

        while (ok) {

            len = sizeof (client);
            clsock = accept (ssock, (struct sockaddr *) &client, &len);
            if (clsock == -1) {
                if (errno == EINTR)
                    continue;
                ErrLog (ErrWARNING, APP_NAME ":Main:Accept:Error: %s", strerror (errno));
                close (ssock);
                exit (1);
            }

            on = 1;
            setsockopt (clsock, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof (int));

            child = fork ();
            if (child == 0) {
                if (debug)
                    printf (APP_NAME ":Serve:IpAddress:%s Port:%d TgmMachine:%s\n",
                            inet_ntoa (client.sin_addr), (int) port, machine);

                bzero ((void *) &action, sizeof (action));
                action.sa_handler = (void (*)(int)) sigpipehand;
                sigaction (SIGPIPE, &action, &old);

                bzero ((void *) &action, sizeof (action));
                action.sa_handler = (void (*)(int)) sigpipehand;
                sigaction (SIGALRM, &action, &old);

                if (TgmAttach (mch, TgmTELEGRAM) != TgmSUCCESS) {
                    ErrLog (ErrFATAL, APP_NAME ":Fatal:Cant attach to:%s telegram", machine);
                    close (clsock);
                    exit (1);
                }
                sprintf (bSemName, "%s.JavaServer", _TgmCfgGetTgmXXX ()->semName);

                do {
                    TgmMachine eMch;

                    if (_TgmSemIsBlocked (bSemName) == 0)
                        exit (kill (0, SIGHUP));

                    if ((hb = TgmHisWaitForAnyEvent (&eMch)) != NULL) {
                        if (eMch == mch) {
                            cp = HisBufToStr (hb);
                            alarm (5);
                            len = write (clsock, cp, strlen (cp));
                            if (len <= 0)
                                ok = 0;
                        }
                    }
                } while (ok);
                if (debug)
                    printf (APP_NAME ":Terminate:Errors:%d IpAddress:%s Port:%d TgmMachine:%s\n",
                            errors, inet_ntoa (client.sin_addr), (int) port, machine);
                close (clsock);
                exit (0);
            }
            setpgid (child, getpgid (0));
            waitpid (-1, NULL, WNOHANG);
        }
    }
    printf (APP_NAME ":Main:Exit:Errors:%d\n", errors);
    exit (0);
}
