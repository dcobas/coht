#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <stdlib.h>
#include "CvorbDrvr.h"

#define _DRVR_NM_ "CVORB"

#define matchEndian(bytes, size, depth) { }

/**
 * @brief Open device driver node
 *
 * @param lun   -- Logical Unit Number assigned to the module. Negative in case
 *                 of driver simulator
 * @param chanN -- Minor Device Number. There can be several entry points for
 *                 current Logical Unit Number (ChannelNumber).
 *
 * @return Open file decriptor (normally >= 3) - if success
 * @return -1                                  - if fails
 *                                               Error message is printing out.
 */
int CvorbEnableAccess(int lun, int chanN)
{
	int  fd;			/* open file descriptor */
	char fileName[0x100];		/* device file name */
	char *tmp;

	if (!MODULE_NAME_OK(_DRVR_NM_)) {
		fprintf(stderr, "Spurious Module Name '%s'.\n"
			"Normally _should not_ contain any lowercase"
			" letters!\n",  _DRVR_NM_);
		return -1;
	}

	tmp = _ncf(_DRVR_NM_);
	sprintf(fileName, "/dev/" NODE_NAME_FMT,
		tmp, (lun < 0)?_SIML_:_DRVR_, abs(lun), chanN);
	free(tmp);
	if ((fd = open(fileName, O_RDWR)) < 0) { /* error */
		perror(NULL);
		fprintf(stderr, "Error [%s] in CvorbEnableAccess()"
			" while opening '%s' file.\nCheck if '%s' module is"
			" installed.\n", strerror(errno), fileName, _DRVR_NM_);
		return -1;
	}

	return fd;
}

/**
 * @brief  Close driver file descriptor.
 *
 * @param fd -- open file descriptor, retuned by CvorbEnableAccess call
 *
 * @return void
 */
void CvorbDisableAccess(int fd)
{
	close(fd);
}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbGetINT_SRC(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CVORB_GET_INT_SRC, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbGetINT_EN(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CVORB_GET_INT_EN, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbSetINT_EN(
			     int fd,
			     unsigned long arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) &arg; /* where to take data from */
	arguments[1] = 1;		     /* number of elements write */
	arguments[2] = 0;		     /* element index */

	/* handle endianity */
	matchEndian((char*)&arg, sizeof(unsigned long), 0);

	return ioctl(fd, CVORB_SET_INT_EN, (char *)arguments);

}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbGetINT_L(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CVORB_GET_INT_L, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbSetINT_L(
			     int fd,
			     unsigned long arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) &arg; /* where to take data from */
	arguments[1] = 1;		     /* number of elements write */
	arguments[2] = 0;		     /* element index */

	/* handle endianity */
	matchEndian((char*)&arg, sizeof(unsigned long), 0);

	return ioctl(fd, CVORB_SET_INT_L, (char *)arguments);

}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbGetINT_V(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CVORB_GET_INT_V, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbSetINT_V(
			     int fd,
			     unsigned long arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) &arg; /* where to take data from */
	arguments[1] = 1;		     /* number of elements write */
	arguments[2] = 0;		     /* element index */

	/* handle endianity */
	matchEndian((char*)&arg, sizeof(unsigned long), 0);

	return ioctl(fd, CVORB_SET_INT_V, (char *)arguments);

}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbGetVHDL_V(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CVORB_GET_VHDL_V, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbGetPCB_SN_H(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CVORB_GET_PCB_SN_H, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbGetPCB_SN_L(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CVORB_GET_PCB_SN_L, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbGetTEMP(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CVORB_GET_TEMP, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbGetADC(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CVORB_GET_ADC, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbGetLastHisSOFT_PULSE(
				    int fd,
				    unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* umber of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CVORB_GET_HISTORY_SOFT_PULSE, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbSetSOFT_PULSE(
			     int fd,
			     unsigned long arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) &arg; /* where to take data from */
	arguments[1] = 1;		     /* number of elements write */
	arguments[2] = 0;		     /* element index */

	/* handle endianity */
	matchEndian((char*)&arg, sizeof(unsigned long), 0);

	return ioctl(fd, CVORB_SET_SOFT_PULSE, (char *)arguments);

}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbGetEXT_CLK_FREQ(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CVORB_GET_EXT_CLK_FREQ, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbGetCLK_GEN_CNTL(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CVORB_GET_CLK_GEN_CNTL, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbSetCLK_GEN_CNTL(
			     int fd,
			     unsigned long arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) &arg; /* where to take data from */
	arguments[1] = 1;		     /* number of elements write */
	arguments[2] = 0;		     /* element index */

	/* handle endianity */
	matchEndian((char*)&arg, sizeof(unsigned long), 0);

	return ioctl(fd, CVORB_SET_CLK_GEN_CNTL, (char *)arguments);

}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbGetMOD_STAT(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CVORB_GET_MOD_STAT, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbGetMOD_CFG(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CVORB_GET_MOD_CFG, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbSetMOD_CFG(
			     int fd,
			     unsigned long arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) &arg; /* where to take data from */
	arguments[1] = 1;		     /* number of elements write */
	arguments[2] = 0;		     /* element index */

	/* handle endianity */
	matchEndian((char*)&arg, sizeof(unsigned long), 0);

	return ioctl(fd, CVORB_SET_MOD_CFG, (char *)arguments);

}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbGetDAC_VAL(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CVORB_GET_DAC_VAL, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbSetDAC_VAL(
			     int fd,
			     unsigned long arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) &arg; /* where to take data from */
	arguments[1] = 1;		     /* number of elements write */
	arguments[2] = 0;		     /* element index */

	/* handle endianity */
	matchEndian((char*)&arg, sizeof(unsigned long), 0);

	return ioctl(fd, CVORB_SET_DAC_VAL, (char *)arguments);

}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbGetSRAM_SA(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CVORB_GET_SRAM_SA, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbSetSRAM_SA(
			     int fd,
			     unsigned long arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) &arg; /* where to take data from */
	arguments[1] = 1;		     /* number of elements write */
	arguments[2] = 0;		     /* element index */

	/* handle endianity */
	matchEndian((char*)&arg, sizeof(unsigned long), 0);

	return ioctl(fd, CVORB_SET_SRAM_SA, (char *)arguments);

}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbGetSRAM_DATA(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CVORB_GET_SRAM_DATA, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbSetSRAM_DATA(
			     int fd,
			     unsigned long arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) &arg; /* where to take data from */
	arguments[1] = 1;		     /* number of elements write */
	arguments[2] = 0;		     /* element index */

	/* handle endianity */
	matchEndian((char*)&arg, sizeof(unsigned long), 0);

	return ioctl(fd, CVORB_SET_SRAM_DATA, (char *)arguments);

}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbGetDAC_CNTL(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CVORB_GET_DAC_CNTL, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbSetDAC_CNTL(
			     int fd,
			     unsigned long arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) &arg; /* where to take data from */
	arguments[1] = 1;		     /* number of elements write */
	arguments[2] = 0;		     /* element index */

	/* handle endianity */
	matchEndian((char*)&arg, sizeof(unsigned long), 0);

	return ioctl(fd, CVORB_SET_DAC_CNTL, (char *)arguments);

}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbGetCH_STAT(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CVORB_GET_CH_STAT, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbGetCH_CFG(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CVORB_GET_CH_CFG, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbSetCH_CFG(
			     int fd,
			     unsigned long arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) &arg; /* where to take data from */
	arguments[1] = 1;		     /* number of elements write */
	arguments[2] = 0;		     /* element index */

	/* handle endianity */
	matchEndian((char*)&arg, sizeof(unsigned long), 0);

	return ioctl(fd, CVORB_SET_CH_CFG, (char *)arguments);

}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbGetFUNC_SEL(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CVORB_GET_FUNC_SEL, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbSetFUNC_SEL(
			     int fd,
			     unsigned long arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) &arg; /* where to take data from */
	arguments[1] = 1;		     /* number of elements write */
	arguments[2] = 0;		     /* element index */

	/* handle endianity */
	matchEndian((char*)&arg, sizeof(unsigned long), 0);

	return ioctl(fd, CVORB_SET_FUNC_SEL, (char *)arguments);

}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbGetFCT_EM_H(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CVORB_GET_FCT_EM_H, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbSetFCT_EM_H(
			     int fd,
			     unsigned long arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) &arg; /* where to take data from */
	arguments[1] = 1;		     /* number of elements write */
	arguments[2] = 0;		     /* element index */

	/* handle endianity */
	matchEndian((char*)&arg, sizeof(unsigned long), 0);

	return ioctl(fd, CVORB_SET_FCT_EM_H, (char *)arguments);

}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbGetFCT_EM_L(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CVORB_GET_FCT_EM_L, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbSetFCT_EM_L(
			     int fd,
			     unsigned long arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) &arg; /* where to take data from */
	arguments[1] = 1;		     /* number of elements write */
	arguments[2] = 0;		     /* element index */

	/* handle endianity */
	matchEndian((char*)&arg, sizeof(unsigned long), 0);

	return ioctl(fd, CVORB_SET_FCT_EM_L, (char *)arguments);

}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbGetSLOPE(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CVORB_GET_SLOPE, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbSetSLOPE(
			     int fd,
			     unsigned long arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) &arg; /* where to take data from */
	arguments[1] = 1;		     /* number of elements write */
	arguments[2] = 0;		     /* element index */

	/* handle endianity */
	matchEndian((char*)&arg, sizeof(unsigned long), 0);

	return ioctl(fd, CVORB_SET_SLOPE, (char *)arguments);

}

/**
 * @brief
 *
 * @param fd     -- driver node descriptor
 * @param result -- buffer to put results
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbGetCH_REC_CYC(
			     int fd,
			     unsigned long *result)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) result; /* where to put results */
	arguments[1] = 1;		       /* number of elements to read */
	arguments[2] = 0;		       /* element index */

	/* driver call */
	if (ioctl(fd, CVORB_GET_CH_REC_CYC, (char*)arguments))
		return -1;

	/* handle endianity */
	matchEndian((char*)result, sizeof(unsigned long), 0);

	return 0;
}

/**
 * @brief
 *
 * @param fd  -- driver node descriptor
 * @param arg -- new values
 *
 * @return  0 - on success.
 * @return -1 - if error occurs. errno is set appropriately.
 */
int CvorbSetCH_REC_CYC(
			     int fd,
			     unsigned long arg)
{
	unsigned long arguments[3];

	/* pack ioctl args */
	arguments[0] = (unsigned long) &arg; /* where to take data from */
	arguments[1] = 1;		     /* number of elements write */
	arguments[2] = 0;		     /* element index */

	/* handle endianity */
	matchEndian((char*)&arg, sizeof(unsigned long), 0);

	return ioctl(fd, CVORB_SET_CH_REC_CYC, (char *)arguments);

}

