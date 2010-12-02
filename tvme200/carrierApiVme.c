/**
 * @file carrierApiVme.c
 *
 * @brief Carrier supported API for VME boards
 *
 * @author Nicolas Serafini, EIC2 SA
 *
 * @date Created on 06/03/2009
 *
 * @version $Id: carrierApiVme.c,v 1.1 2009/12/01 14:13:03 lewis Exp dcobas $
 */
/*============================================================================*/
/* Include Section                                                            */
/*============================================================================*/
#include "carrierCommon.h"
#include "carrierTVME200.h"
#include "carrierVIPC61x.h"

/*============================================================================*/
/* Var section                                                                */
/*============================================================================*/
/** All boards known API */
carrierAPI_t *carriersAPI[] = { &carrierTVME200API,
                                &carrierVIPC610API,
                                &carrierVIPC616API,
                                &carrierVIPC626API,
                                NULL};
