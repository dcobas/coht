/**
 * ipoctal_drvr.h
 *
 * driver for the IPOCTAL boards
 * Copyright (c) 2009 Nicolas Serafini, EIC2 SA
 * Copyright (c) 2010,2011 Samuel Iglesias Gonsalvez <siglesia@cern.ch>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#ifndef _IPOCTALDRVR_H_
#define _IPOCTALDRVR_H_

#define NR_CHANNELS 		8
#define IPOCTAL_MAX_BOARDS	16
#define MAX_DEVICES		NR_CHANNELS * IPOCTAL_MAX_BOARDS
#define RELEVANT_IFLAG(iflag) ((iflag) & (IGNBRK|BRKINT|IGNPAR|PARMRK|INPCK))

/**
 * enum uart_parity_e - UART supported parity.
 * 
 */
typedef enum {
    UART_NONE  = 0,
    UART_ODD   = 1,
    UART_EVEN  = 2,
} uart_parity_e;

/**
 * enum uart_error - UART error type
 *
 */
typedef enum {
    UART_NOERROR = 0,      /* No error during transmission */
    UART_TIMEOUT = 1 << 0, /* Timeout error */
    UART_OVERRUN = 1 << 1, /* Overrun error */
    UART_PARITY  = 1 << 2, /* Parity error */
    UART_FRAMING = 1 << 3, /* Framing error */
    UART_BREAK   = 1 << 4, /* Received break */
} uart_error;

/**
 * struct ipoctal_readwrite - Structure used for read and write on serial line
 * 
 */
struct ipoctal_readwrite {
    char buffer;          /* Where the data are stored (read write)
                               Warning : the buffer size must be equal to the nbBytes value */
    unsigned int nb_bytes;  /* Number of bytes that would be read/write */
    unsigned long timeout; /* Timeout management (in millisecond) */
    uart_error error_flag; /* Flag for error */
} ;

/**
 * struct ipoctal_config - Serial configuration
 *
 */
struct ipoctal_config{
    unsigned int baud;        /* Baud rate\n
                                   Valid value : 75, 110, 150, 300, 600, 1200, 1800,
                                   2000, 2400, 4800, 9600, 19200 and 38400 */
    unsigned int stop_bits;    /* Stop bits (1 or 2) */
    unsigned int bits_per_char; /* Number of bits (6, 7 or 8) */
    uart_parity_e parity;      /* Parity */
    unsigned int flow_control; /* Flow control management (Rts/Cts) (0 no, 1 yes) */
};

/**
 * \struct ipoctal_stats
 * \brief Ip Octal channel stats structure
 */
struct ipoctal_stats{
    unsigned long tx;         /* Number of bytes transmitted since last reset */
    unsigned long rx;         /* Number of bytes received since last reset */
    unsigned long overrun_err; /* Number of overrun error since last reset */
    unsigned long parity_err;  /* Number of parity error since last reset */
    unsigned long framing_err; /* Number of framing error since last reset */
    unsigned long rcv_break;   /* Number of break received since last reset */
};

/**
 * \struct ipoctal_channel
 * \brief Ip Octal channel identification
 */
struct ipoctal_channel{
    unsigned int carrier_number;/* Logical carrier board number */
    unsigned int slot_position; /* Slot position on the carrier */
    unsigned int channel;      /* Channel on the ipoctal */
};

#endif /* _IPOCTALDRVR_H_ */
