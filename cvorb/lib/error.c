/*
 * error.c
 *
 * Error and log-level handling
 * This code has been adapted from comedilib:
 *
 *    COMEDILIB - Linux Control and Measurement Device Interface Library
 *    Copyright (C) 1997-2001 David A. Schleef <ds@schleef.org>
 *    License: GPL v2.1.
 *
 * Copyright (c) 2010 Emilio G. Cota <cota@braap.org>
 */

#include <stdio.h>
#include <string.h>

#include "cvorb_errno.h"
#include "libinternal.h"

char *__cvorb_error_strings[] = {
	"No error",
	"Invalid vector table: time between two consecutive vector is negative",
        "Invalid vector table: vector duration exceed HW limits(should be split)",
	"No such device",
	"Feature not supported"
};

#define ARRAY_SIZE(x)	(sizeof(x)/sizeof((x)[0]))
#define LIBCVORB_NR_ERRCODES ARRAY_SIZE(__cvorb_error_strings)

int __cvorb_loglevel = 1;
int __cvorb_errno;

/**
 * @brief Change libcvorb logging properties
 * @param loglevel - loglevel to be set
 *
 * @return Previous loglevel
 *
 * This function controls the debugging output from libcvorb. By increasing
 * the loglevel, additional debugging information will be printed.
 * Error and debugging messages are printed to the stream stderr.
 *
 * The default loglevel can be set by using the environment variable
 * LIBCVORB_LOGLEVEL. The default loglevel is 1.
 *
 * The meaning of the loglevels is as follows:
 * loglevel = 0 libcvorb prints nothing
 *
 * loglevel = 1 (default) libcvorb prints error messages
 * when there is a self-consistency error (i.e., an internal bug.)
 *
 * loglevel = 2 libcvorb prints an error message when an invalid
 * parameter is passed.
 *
 * loglevel = 3 libcvorb prints an error message whenever an error is
 * generated in the libcvorb library or in the C library, when called by
 * libcvorb.
 *
 * loglevel = 4 libcvorb prints as much debugging info as it can
 */
int cvorb_loglevel(int loglevel)
{
	int old_loglevel = __cvorb_loglevel;

	if (old_loglevel >= 4 || loglevel >= 4) {
		fprintf(stderr, "%s: old_loglevel %d new %d\n",
			__func__, old_loglevel, loglevel);
	}
	__cvorb_loglevel = loglevel;

	return old_loglevel;
}

/**
 * @brief Retrieve the number of the last libcvorb error
 *
 * @return Most recent libcvorb error
 *
 * When a libcvorb function fails, an internal library variable stores the
 * error number, which can be retrieved with this function. See
 * cvorb_perror() and cvorb_strerror() to convert this number into
 * a text string.
 */
int cvorb_errno(void)
{
	return __cvorb_errno;
}

/**
 * @brief Convert a libcvorb error number into human-readable form
 * @param errnum - Error number to convert
 *
 * @return String describing the given error number
 *
 * See cvorb_errno() for further info.
 */
char *cvorb_strerror(int errnum)
{
	if (errnum < LIBCVORB_ERROR_BASE ||
	    errnum >= LIBCVORB_ERROR_BASE + LIBCVORB_NR_ERRCODES) {
		return strerror(errnum);
	}
	return __cvorb_error_strings[errnum - LIBCVORB_ERROR_BASE];
}

/**
 * @brief Print a libcvorb error message
 * @param string - Prepend this string to the error message
 *
 * This function prints an error message obtained from the current value
 * of CVORB's error number.  See cvorb_errno() for further info.
 */
void cvorb_perror(const char *string)
{
	if (!string)
		string = "libcvorb";
	fprintf(stderr, "%s: %s\n", string, cvorb_strerror(__cvorb_errno));
}

void __cvorb_internal_error(int err)
{
	__cvorb_errno = err;
	if (__cvorb_loglevel >= 1)
		cvorb_perror("internal error");
}

void __cvorb_param_error(int err)
{
	__cvorb_errno = err;
	if (__cvorb_loglevel >= 2)
		cvorb_perror("parameter error");
}

void __cvorb_libc_error(const char *string)
{
	__cvorb_errno = errno;
	if (__cvorb_loglevel >= 3) {
		char pre[64];

		snprintf(pre, sizeof(pre) - 1, "%s: libc error", string);
		pre[sizeof(pre) - 1] = '\0';
		cvorb_perror(pre);
	}
}

void __cvorb_lib_error(const char *string, int err)
{
	__cvorb_errno = err;
	if (__cvorb_loglevel >= 3)
	{
                char pre[64];

                snprintf(pre, sizeof(pre) - 1, "%s: libcvorb error", string);
                pre[sizeof(pre) - 1] = '\0';
                cvorb_perror(pre);
	}
}
