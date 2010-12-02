/**
 * libipoctal.c
 *
 * library for the IPOCTAL boards
 * Copyright (c) 2011 Samuel Iglesias Gonsalvez <siglesia@cern.ch>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#include <stdio.h>
#define IPOCTAL_NAME "/dev/ipoctal.%d.%d"

int ipoctal_get_device_name(int lun, int mezz, int line, char *name)
{
	sprintf(name, IPOCTAL_NAME, lun, line);
	return 0;
}
