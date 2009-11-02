/**
 * @file utilsDrvr.h
 *
 * @brief Extra driver functions are declared here
 *
 * @author Copyright (C) 2009 CERN. Yury GEORGIEVSKIY <ygeorgie@cern.ch>
 *
 * @date Created on 31/10/2009
 *
 * @section license_sec License
 *          Released under the GPL
 */
#ifndef _UTILS_DRIVER_H_INCLUDE_
#define _UTILS_DRIVER_H_INCLUDE_

void print_load_sram_ioctl_args(ushort *);
int  load_sram(CVORBUserStatics_t *, char *);
int  read_sram(CVORBUserStatics_t *, char *);
int  read_vhdl(CVORBUserStatics_t *, char *);
int  read_pcb(CVORBUserStatics_t *, char *);
int  read_temp(CVORBUserStatics_t *, char *);
int  read_mod_config_reg(CVORBUserStatics_t *, char *);
int  write_mod_config_reg(CVORBUserStatics_t *, char *);
int  read_ch_config_reg(CVORBUserStatics_t *, char *);
int  write_ch_config_reg(CVORBUserStatics_t *, char *);
int  read_mod_stat(CVORBUserStatics_t *, char *);
int  read_ch_stat(CVORBUserStatics_t *, char *);
int  enable_function(CVORBUserStatics_t *, char *);
int  disable_function(CVORBUserStatics_t *, char *);
int  read_recurrent_cycles_reg(CVORBUserStatics_t *, char *);
int  write_recurrent_cycles_reg(CVORBUserStatics_t *, char *);

#endif	/* _UTILS_DRIVER_H_INCLUDE_ */
