/* Copyright (C) 1989 by Matthew Merzbacher.
   All Rights Reserved.
   Bug is provided as is, without express or implied warranty.  In no event
   shall Matthew Merzbacher become liable for any loss or damages, consequential
   or otherwise, arising from the use or possession of this software.
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <fcntl.h>
#ifdef _AIX
#include <sys/select.h>
#endif

#define MARK "<MARK>"
#define MARKLEN 6

/*
 * Brief Description:
 * Bug is used to tap a stream socket.  It is useful for tracing communication 
 * between a server and its clients.  The communication between server and
 * client is maintained while transcripts of all communication lines are taken.
 * It has six required (and no optional) arguments.  
 *
 * Usage:
 * bug <in port> <out port> <machine> <client file> <server file>
 *
 * <in port>     - the port (on the machine on which bug runs) to which to
 *				   clients will connect (the pseudo-port)
 * <out port>    - the true server port
 * <machine>     - the server machine
 * <client file> - transcript file of what each client says
 * <server file> - transcript file of what the server responds
 *
 * Example: 
 * Suppose a server listens on port 3000 of machine "xyz".  The user must 
 * artificially make the clients talk to some other port (on any machine -
 * it could be the server machine, the client machine, or a third machine).
 * That port is called the pseudo-port.  Bug runs on the machine of the
 * pseudo-port.  To run it, type:
 * 
 * bug 2999 3000 xyz client server
 *
 * This assumes that the pseudo-port is 2999.  The output files will all start
 * with "client" or "server" followed by the process number of the bug process
 * and the process number of each seperate listening session.  That way, you
 * can follow the connection order.  Typically, the output files created by bug
 * may be read by using 'od -c'.
 *
 */

main(argc, argv)
int argc; char *argv[];
{
    struct sockaddr_in server;
    struct hostent *hp, *gethostbyname();
    char buf[1024];
    int rval;
    int outfile1, outfile2;
    int insock, outsock, msgsock;
    int insocknum, outsocknum;
    fd_set fdset;


    /* Check Arguments */

    if (argc < 6) {
	printf("usage: %s <in port> <out port> <machine> <client file> <server file>\n",argv[0]);
	exit(1);
	}

    insocknum = atoi(argv[1]);
    outsocknum = atoi(argv[2]);

    /* Create listening post on pseudo-port */

#ifdef DEBUG
     fprintf(stderr,"opening pseudo-port stream socket\n");
#endif
    insock = socket(AF_INET, SOCK_STREAM, 0);

    if (insock < 0) {
	perror("opening pseudo-port stream socket");
	exit(1);
	}


    /* Name socket using user supplied port number */

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(insocknum);

    if (bind(insock, &server, sizeof(server))) {
	perror("binding pseudo-port stream socket");
	exit(1);
	}


    /* Start accepting connections */

#ifdef DEBUG
     fprintf(stderr,"listening on port %d\n",insocknum);
#endif
    listen(insock, SOMAXCONN);


    /* Every connection spawns a child to handle the communication */

    do {
	msgsock = accept(insock, 0, 0);
	if (msgsock == -1)
		perror("accept");
	} while (fork() != 0);


    /* Close listening post - it's now called "msgsock" */

    close(insock);


    /* Create output socket to server */

    outsock = socket(AF_INET, SOCK_STREAM, 0);
    if (outsock < 0) {
	perror("opening server stream socket");
	exit(1);
	}


    /* Connect socket using machine & port specified on command line. */

    server.sin_family = AF_INET;
    hp = gethostbyname(argv[3]);
    if (hp == 0) {
	fprintf(stderr, "%s: unknown machine\n", argv[3]);
	exit(2);
	}

    bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
    server.sin_port = htons(outsocknum);

#ifdef DEBUG
     fprintf(stderr,"connecting to port %d on %s\n",outsocknum,argv[3]);
#endif
    if (connect(outsock, &server, sizeof(server)) < 0) {
	perror("connecting server stream socket");
	exit(1);
	}


    /* Open the output files */

    sprintf(buf,"%s.%d.%d",argv[4],getppid(),getpid());
    if ((outfile1 = open(buf, O_TRUNC|O_WRONLY|O_CREAT, 0644)) == -1) {
	fprintf(stderr,"cannot open file: %s\n",buf);
	exit(1);
	}

    sprintf(buf,"%s.%d.%d",argv[5],getppid(),getpid());
    if ((outfile2 = open(buf, O_TRUNC|O_WRONLY|O_CREAT, 0644)) == -1) {
	fprintf(stderr,"cannot open file: %s\n",buf);
	exit(1);
	}

    do {
	
	/* find out who's talking */

	FD_ZERO(&fdset);
	FD_SET(msgsock, &fdset);
	FD_SET(outsock, &fdset);
	if (select(getdtablesize(), &fdset, 0, 0, 0) == -1) {
	    perror("select");
	    exit(1);
	    }
	bzero(buf, 1024);

	if (FD_ISSET(msgsock, &fdset) && FD_ISSET(outsock, &fdset))
	    fprintf(stderr,"Two talkers - no listeners\n");


	/* Client is talking to server */

	if (FD_ISSET(msgsock, &fdset)) { 
	    if ((rval = read(msgsock, buf, 1024)) < 0)
		perror("reading stream message");
	    if (rval == 0)
		fprintf(stderr,"Ending client connection\n");
	    else {
		write(outfile1, buf, rval);
#ifndef NODAL
		write(outfile1, MARK, 6);
#endif
		if (write(outsock, buf, rval) < 0)
		    perror("writing on output stream socket");
		}
	    }

	/* Server is talking to client */

	else {
	    if (! FD_ISSET(outsock, &fdset)) {
		perror("weird behavior");
		exit(1);
		}
	    if ((rval = read(outsock, buf, 1024)) < 0)
		perror("reading stream message");
	    if (rval == 0)
		fprintf(stderr,"Ending server connection\n");
	    else {
		write(outfile2, buf, rval);
#ifndef NODAL
		write(outfile2, MARK, 6);
#endif
		if (write(msgsock, buf, rval) < 0)
		    perror("writing on output stream socket");
		}
	    }
	} while (rval != 0);


    /* Close up shop */

    fprintf(stderr, "Closing Connections\n");
    close(msgsock);
    close(outsock);
    close(outfile1);
    close(outfile2);
    }
