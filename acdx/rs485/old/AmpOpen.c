/**************************************************************************/
/* Open the ttyUSB0 USB RS485 driver for the powersoft amplifier control  */
/**************************************************************************/

#include <sys/termios.h>

int AmpOpen() {

char fnm[32];
int  fn, cc, mcs;
struct termios tios;

   bzero((void *) &tios, sizeof(struct termios));

   sprintf(fnm,"/dev/ttyUSB0");
   if ((fn = open(fnm,O_RDWR | O_NONBLOCK, 0)) > 0) {

      tcflush(fn, TCIOFLUSH);

      cc = fcntl(fn, F_GETFL, 0);
      fcntl(fn, F_SETFL, cc & ~O_NDELAY);
      if (tcgetattr(fn, &tios)==0) {

	 ioctl(fn, TIOCMGET, &mcs);
	 mcs |= TIOCM_RTS;
	 ioctl(fn, TIOCMSET, &mcs);

	 cfsetospeed(&tios, (speed_t) B19200);
	 cfsetispeed(&tios, (speed_t) B19200);

	 tios.c_cflag &= ~CSIZE;     /* Clear data bits */
	 tios.c_cflag |= CS8;        /* Set eight data bits */
	 tios.c_cflag &= ~PARENB;    /* No parity */
	 tios.c_cflag &= ~CSTOPB;    /* Stop bits */
	 tios.c_cflag |= CLOCAL;
	 tios.c_cflag |= CREAD;
	 tios.c_cflag &= ~CRTSCTS;
	 tios.c_iflag  = IGNBRK;

	 tios.c_cc[VTIME]=1;
	 tios.c_cc[VMIN]=60;

	 if (tcsetattr(fn, TCSANOW, &tios) ==0) return fn;
      }
      return fn;
   }
   return 0;
}
