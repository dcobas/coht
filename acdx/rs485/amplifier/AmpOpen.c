// =====================================================================
// Do once only initial stuff
// =====================================================================

#include <sys/termios.h>

char *GetRouteName(char *name);
char *GetFile(char *name);

static int fnamp = 0;

// =====================================================================
// Set options
// Ripped off from cutecom
// =====================================================================

int SetOptions(int baudrate,
	       int databits,
	       char *parity,
	       char *stop,
	       int softwareHandshake,
	       int hardwareHandshake) {

struct termios newtio;
int mcs=0;
speed_t _baud=0;

   bzero((void *) &newtio, sizeof(struct termios));

   if (tcgetattr(fnamp, &newtio) != 0) {
      fprintf(stderr,"SetOptions:Error\n");
      perror("tcgetattr");
      return 0;
   }

   switch (baudrate) {
      case 600:
	 _baud=B600;
	 break;
      case 1200:
	 _baud=B1200;
	 break;
      case 2400:
	 _baud=B2400;
	 break;
      case 4800:
	 _baud=B4800;
	 break;
      case 9600:
	 _baud=B9600;
	 break;
      case 19200:
	 _baud=B19200;
	 break;
      case 38400:
	 _baud=B38400;
	 break;
      case 57600:
	 _baud=B57600;
	 break;
      case 115200:
	 _baud=B115200;
	 break;
      case 230400:
	 _baud=B230400;
	 break;
      case 460800:
	 _baud=B460800;
	 break;
      case 576000:
	 _baud=B576000;
	 break;
      case 921600:
	 _baud=B921600;
	 break;
      default: break;
   }
   cfsetospeed(&newtio, (speed_t)_baud);
   cfsetispeed(&newtio, (speed_t)_baud);

   if ( (databits == 7)
   &&   ((strcmp(parity,"Mark" ) == 0)
   ||    (strcmp(parity,"Space") == 0)) ) databits = 8;

   switch (databits) {
      case 5:
	 newtio.c_cflag = (newtio.c_cflag & ~CSIZE) | CS5;
	 break;
      case 6:
	 newtio.c_cflag = (newtio.c_cflag & ~CSIZE) | CS6;
	 break;
      case 7:
	 newtio.c_cflag = (newtio.c_cflag & ~CSIZE) | CS7;
	 break;
      case 8:
      default:
	 newtio.c_cflag = (newtio.c_cflag & ~CSIZE) | CS8;
	 break;
   }
   newtio.c_cflag |= CLOCAL | CREAD;

   newtio.c_cflag &= ~(PARENB | PARODD);
   if (strcmp(parity,"Even") == 0)
      newtio.c_cflag |= PARENB;
   else if (strcmp(parity,"Odd") == 0)
      newtio.c_cflag |= (PARENB | PARODD);

   newtio.c_cflag &= ~CRTSCTS;

   if (strcmp(stop,"2") == 0)
      newtio.c_cflag |= CSTOPB;
   else
      newtio.c_cflag &= ~CSTOPB;

    newtio.c_iflag=IGNBRK;

   if (softwareHandshake)
      newtio.c_iflag |= IXON | IXOFF;
   else
      newtio.c_iflag &= ~(IXON|IXOFF|IXANY);

   newtio.c_lflag=0;
   newtio.c_oflag=0;
   newtio.c_cc[VTIME]=1;
   newtio.c_cc[VMIN]=60;

   if (tcsetattr(fnamp, TCSANOW, &newtio) != 0) {
      fprintf(stderr,"SetOptions:Error\n");
      perror("tcsetattr");
      return 0;
   }

   ioctl(fnamp, TIOCMGET, &mcs);
   mcs |= TIOCM_RTS;
   ioctl(fnamp, TIOCMSET, &mcs);

   if (tcgetattr(fnamp, &newtio) != 0) {
      fprintf(stderr,"SetOptions:Error\n");
      perror("tcgetattr");
      return 0;
   }

   if (hardwareHandshake)
      newtio.c_cflag |= CRTSCTS;
   else
      newtio.c_cflag &= ~CRTSCTS;

   if (tcsetattr(fnamp, TCSANOW, &newtio) != 0) {
      fprintf(stderr,"SetOptions:Error\n");
      perror("tcgetattr");
      return 0;
   }
   return 1;
}

// =====================================================================
// Open the ttyMI0 RS485 driver for the powersoft amplifier control
// =====================================================================

// Use the default device if not defined in configuration

#define DEFAULT_AMP_DEVICE "/dev/ttyMI0"

int AmpOpen() {

char *cp;

   cp = GetRouteName(GetFile("AmpDevice"));
   if ((cp == NULL) || (*cp == '.')) cp = DEFAULT_AMP_DEVICE;

   fprintf(stderr,"Opening:AmpDevice:%s",cp);

   if ((fnamp = open(cp,O_RDWR|O_SYNC|O_NDELAY,0)) > 0) {
      if (SetOptions(19200,8,"None","1",0,0) > 0) return fnamp;
      if (close(fnamp) != 0) perror("close");
   }
   fnamp = 0;

   return fnamp;
}

// =====================================================================
// Try to recover from an error
// =====================================================================

int AmpRecover() {

   tcflush(fnamp, TCIOFLUSH);
   if (close(fnamp) != 0) perror("close");
   AmpOpen();
   return fnamp;
}
