/**
 * @file carrierTVME200.h
 *
 * @brief TVME200 Carrier interface header file
 *
 * @author Nicolas Serafini, EIC2 SA
 *
 * @date Created on 21/01/2009
 *
 * Specific carrier interface for the TEWS TVME200 and SBS VIPC626.
 *
 * @version $Id: carrierTVME200.h,v 1.1 2009/12/01 14:13:03 lewis Exp dcobas $
 */
#ifndef _CARRIERTVME200_H_
#define _CARRIERTVME200_H_

/*============================================================================*/
/* Include Section                                                            */
/*============================================================================*/
#include "carrierCommon.h"

/*============================================================================*/
/* External Section                                                           */
/*============================================================================*/
/** Specific interface of the TVME200 carrier */
extern carrierAPI_t carrierTVME200API;
/** Specific interface of the VIPC626 carrier */
extern carrierAPI_t carrierVIPC626API;

#endif /* _CARRIERTVME200_H_ */
