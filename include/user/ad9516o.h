/**
 * @file ad9516o.h
 *
 * @brief ad9516 chip library
 *
 * @author Copyright (C) 2009 CERN. Yury GEORGIEVSKIY <ygeorgie@cern.ch>
 *
 * @date Created on 16/11/2009
 *
 * @section license_sec License
 *          Released under the GPL
 */
#ifndef _AD_9516_H_INCLUDE_
#define _AD_9516_H_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif
/** @defgroup ad9516lib ad9516 chip Library API functions
 *@{
 */
	int    ad9516o_on(int, int, int);
	int    ad9516o_off(int, int);
	double ad9516o_get_freq(int, char **);
/*@} end of group*/
#ifdef __cplusplus
}
#endif


#endif	/* _AD_9516_H_INCLUDE_ */
