/*
 *  Piece of code adapted for driver benchmark.
 *  Original version references:
 *
 *  Portable Agile C++ Classes (PACC)
 *  Copyright (C) 2004 by Marc Parizeau
 *  http://manitou.gel.ulaval.ca/~parizeau/PACC
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Contact:
 *  Laboratoire de Vision et Systemes Numeriques
 *  Departement de genie electrique et de genie informatique
 *  Universite Laval, Quebec, Canada, G1K 7P4
 *  http://vision.gel.ulaval.ca
 *
 */

#include <stdio.h>
#include <time.h>

#include "time_stamp_counter.h"

static int mHardware = 1;
static double mPeriod = 0;
static unsigned long long mValueCount = 0;	//duration from start

static unsigned long long getCount(void)
{
	unsigned long long lCount = 0;
	struct timeval lCurrent;

	if (mHardware) {
#if defined (__GNUC__) && ( defined (__i386__) || defined(__x86_64__) )
		__asm__ volatile ("rdtsc":"=A" (lCount));
#else
#if defined (__GNUC__) && defined (__PPC__)
		register unsigned int lLow;
		register unsigned int lHigh1;
		register unsigned int lHigh2;
		unsigned int *lPtr;
		do {
			// make sure that high bits have not changed
			__asm__ volatile ("mftbu %0":"=r" (lHigh1));
			__asm__ volatile ("mftb  %0":"=r" (lLow));
			__asm__ volatile ("mftbu %0":"=r" (lHigh2));
		} while (lHigh1 != lHigh2);
		// transfer to lCount
		lPtr = (unsigned int *) &lCount;
		*lPtr++ = lHigh1;
		*lPtr = lLow;
#else
		gettimeofday(&lCurrent, 0);
		lCount =
		    (unsigned long long) lCurrent.tv_sec * 1000000 +
		    lCurrent.tv_usec;
#endif
#endif
	} else {
		gettimeofday(&lCurrent, 0);
		lCount =
		    (unsigned long long) lCurrent.tv_sec * 1000000 +
		    lCurrent.tv_usec;
	}

	return lCount;
}

double ts_getValue(double unit)
{
	return (double) (getCount() - mValueCount) * mPeriod * unit;
}

void ts_calibrateCountPeriod(unsigned int inDelay /*ms */ ,
			     unsigned int inTimes)
{
	double lPeriod = 0;
	unsigned int i;
	struct timeval lStartTime, lTime;
	unsigned long long lStartCount, lCount;
	struct timespec theDelay, time_left_before_wakeup;
	int sec;
	long nsec;

	if (mHardware) {
#if defined (__GNUC__) && ( defined (__i386__) || defined(__x86_64__) || defined (__PPC__) )
		// calibrate by matching the time-stamps with the micro-seconds of gettimeofday
		for (i = 0; i < inTimes; ++i) {
			gettimeofday(&lStartTime, 0);
			lStartCount = getCount();

			sec = inDelay / 1000;	// number of seconds since delay is expressed in ms
			nsec = 0;
			if ((nsec = inDelay % 1000))	// check remainder
				nsec *= (long) 1E6;	// convert remainder into ns
			theDelay.tv_sec = sec;
			theDelay.tv_nsec = nsec;
#ifdef __Lynx__
			//UNIX variants often use a kernel timer resolution (HZ value)
			//of about 10ms. So it' not possible to manage delays less than 10ms
			//to wait 10ms you need specify a delay 0 othewhise your delay will b 20ms
			if ((inDelay % 10) < 5) {
				theDelay.tv_nsec -= 20000000;
			} else {
				theDelay.tv_nsec -= 10000000;
			}
			if (theDelay.tv_nsec < 0)
				theDelay.tv_nsec = 0;
#endif
			nanosleep(&theDelay, &time_left_before_wakeup);

			gettimeofday(&lTime, 0);
			lCount = getCount() - lStartCount;
			lTime.tv_sec -= lStartTime.tv_sec;
			lTime.tv_usec -= lStartTime.tv_usec;
			// dismiss the first run of the loop
			if (i != 0)
				lPeriod +=
				    (lTime.tv_sec +
				     lTime.tv_usec * 0.000001) / lCount;
		}
		mPeriod = lPeriod / (inTimes - 1);
		fprintf(stderr,
			"***********************************************************************************\n");
		fprintf(stderr,
			"TimeStampCounter period: %g.(Computed CPU frequency: %g GHz)\n",
			mPeriod, (1 / mPeriod) * 1E-9);
		fprintf(stderr,
			"***********************************************************************************\n");
#else
		// use the microseconds of gettimeofday
		mPeriod = 0.000001;
#endif
	} else {
		// use the microseconds of gettimeofday
		mPeriod = 0.000001;
	}
}
