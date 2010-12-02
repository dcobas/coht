/**
 * @file carrierVIPC61x.h
 *
 * @brief VIPC61x Carrier interface header file
 *
 * @author Nicolas Serafini, EIC2 SA
 *
 * @date Created on 21/01/2009
 *
 * Specific carrier interface for the SBS VIPC610 and SBS VIPC616 boards.
 *
 * @version $Id: carrierVIPC61x.h,v 1.1 2009/12/01 14:13:03 lewis Exp dcobas $
 */
#ifndef _CARRIERVIPC61X_H_
#define _CARRIERVIPC61X_H_

/*============================================================================*/
/* Include Section                                                            */
/*============================================================================*/
#include "carrierCommon.h"

/*============================================================================*/
/* External Section                                                           */
/*============================================================================*/
/** Specific interface of the VIPC610 carrier */
extern carrierAPI_t carrierVIPC610API;
/** Specific interface of the VIPC616 carrier */
extern carrierAPI_t carrierVIPC616API;

#endif /* _CARRIERVIPC61X_H_ */
