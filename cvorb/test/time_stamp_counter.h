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

#ifndef _TIME_STAMP_COUNTER_
#define _TIME_STAMP_COUNTER_

#include <unistd.h>
#ifdef __Lynx__
#include <time.h>
#else
#include <sys/time.h>
#endif

#define S_UNIT          1.0  //express delay in second
#define MS_UNIT         1.e3 //express delay in milli-second
#define YS_UNIT         1.e6 //express delay in micro-second
#define NS_UNIT         1.e9 //express delay in nano-second

double ts_getValue(double unit);
void ts_calibrateCountPeriod(unsigned int inDelay/*ms*/, unsigned int inTimes);

#endif
