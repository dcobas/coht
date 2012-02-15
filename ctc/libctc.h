/*-
 * Copyright (c) 2012 Samuel I. Gonsalvez <siglesia@cern.ch>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of copyright holders nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*! \file */
/*!
 *  \mainpage libctc documentation main page
 *  \author Samuel Iglesias Gonsalvez BE/CO/HT
 *  \version 15 Feb 2012
 *
 * The CTC card is a VME module used in the OASIS project as multiple counters with
 * 40 external starts, six input clocks and eight outputs.
 * 
 * Each output pulse comes from the terminal count of a double counter of 16 bits called a channel.
 * 
 * So there are 16 counters in this module and eight channels corresponding to the eight
 * outputs. You can select a different external start by channel and a different clock by
 * counter. There is no interest to select the same clock in the two counters of one
 * channel apart to have a 17 bits counter in a channel.
 * 
 * A special up-down mode of the counters is also possible and will be described on the
 * documentation.
 *
 * The VME interface is 24 bits address and 32 bits data. There is no interrupt. The base
 * address must be set by corresponding 8 bits switches which determine a 16 bits
 * address space for a module.
 *
 * A specific address is reserved to program from the VME bus the on board FPGA.
 * Another corresponding eight switches might be also set (See SW1 on figure 1). This
 * feature is reserved for specialists only.
 *
 * Apart from these two groups of eight switches, the CTC card is a jumper free card.
 *
 * The four jumpers on the card are for specialists only. When removed they permit a
 * JTAG programming of the Prom of the onboard Xilinx FPGA.
 *
 * <p></p>
 * <p><h1>Explanation of some concepts</h1></p>
 * <p><h2>Delay on a counter</h2></p>
 * <p>Number of clock's ticks to count until the counter overflows. Example: if you want to count
 * only 5 ticks, you setup <i>delay = 5</i> on counter 1 and counter 2 with <i>delay = 0</i>.</p>
 * <p>Read the User Manual on sections "DELAY COUNTER 1 REGISTER" and "DELAY COUNTER 2 REGISTER" for
 * more information.</p>
 *
 * <p><h2>Output counter</h2></p>
 * <p>Number of pulses generated in the output of a channel since the switch on of the machine.</p>
 *
 * <p><h2>Disable of a channel</h2></p>
 * <p>Disable the output of a channel.</p>
 *
 * <p><h2>Mode</h2></p>
 * <p>The board has two modes: NORMAL_MODE and UPDOWN_MODE.</p>
 * 
 * <b>NORMAL_MODE</b>: use both counters per channel. When counter 1 finishes, the counter 2 
 * is starting counting, i.e., you have 17 bits counter in total.
 *
 * <b>UPDOWN_MODE</b>: it counts the difference between number of the falling edges of the clock connected to counter 1 and the falling
 * edge of the clock connected to counter 2. 
 * Each time the clock for counter 1 has a falling edge, it adds '1' to counter 1 register. Each time the clock for counter 2 
 * has a falling edge, it subtracts '1' to the counter 1 register.
 * 
 * Direction <b>FALLING_EDGE_COUNTER1</b>: it generates one pulse on the output when the actual value on counter 1 register 
 * reaches the predefined value on the Delay Counter 1 register due to the sums on the counter 1.
 *
 * Example: the actual value of the counter 1 is 0, it will generate and output when it reaches the value 3 (value previously written
 * on the Delay Counter 1 register). 
 * Imagine that the clock for counter 1 is working alone (the other clock is not plugged or something like that),
 * so the values will be 0, then 1, then 2 and then 3 and it generates a pulse in the output.
 *
 * Direction <b>FALLING_EDGE_COUNTER2</b>: it generates one pulse on the output when the actual value on counter 1 register 
 * reaches the predefined value on the Delay Counter 1 register due to the subtractions on the counter 1. 
 *
 * Example: the actual value of the counter 1 is 5, it will generate a pulse in the output when it reaches the value 3
 * (value previously written on the Delay Counter 1 register). 
 * Imagine that the clock for counter 2 is working alone (clock connected to counter 1 is turned off), 
 * so the values of the counter 1 will be 5, then 4, then 3 and it generates a pulse in the output.
 * 
 * Note: the value of the counter 2 register is not taken in account. The Delay Counter 2 register is ignored.
 * 
 * Note: UPDOWN_MODE is usually used to see if the threshold value (Delay Counter 1 register) is reached due
 * the reference clock setup for the counter 1 or due to the reference clock setup for the counter 2.
 *
 */

/* lib_ctc API */
#ifndef _LIBCTC_H
#define _LIBCTC_H

#ifdef __cplusplus
extern "C" {
#endif

/*!  
 * This enum is used in ctc_set_mode to select the mode
 */
enum ctc_mode {
	NORMAL_MODE = 0,
	UPDOWN_MODE
};

/*!  
 * This enum is used in ctc_set_mode to adjust the direction when UPDOWN_MODE is selected
 */
enum ctc_dir {
	FALLING_EDGE_COUNTER2 = 0, 	/*!< In UPDOWN_MODE use the falling edge of counter 2.  Explained in \ref main  */ 
	FALLING_EDGE_COUNTER1		/*!< In UPDOWN_MODE use the falling edge of counter 1. Explained in \ref main */
};


/* ctc lib declarations */

//! Open the device.
/*!
  \param lun an integer argument to select the device.
  \return returns file descriptor on success. On error, -1 is returned, and errno is set appropriately.
*/
int ctc_open(int lun);

//! Close the device.
/*!
  \param fd file descriptor
  \return returns 0 on success. On error, -1 is returned, and errno is set appropriately.
*/
int ctc_close(int fd);

//! Reset the device to a known state 
/*!
  \brief <b>NOTICE:</b> the board will have all the channels disabled, no delays, no external starts setup, no clocks selected, NORMAL mode, FALLING_EDGE_COUNTER2 direction.
  \param fd file descriptor
*/
void ctc_reset(int fd);

//! Get firmware version of the HW
/*!
  \brief Only the 8 least significant bits are used: if Version number is 2.5, you will read 0x00000025 (hexa) in this register.
  \param fd 		file descriptor
  \param hw_version 	pointer to int to receive the value
  \return returns 0 on success. On error, negative value is returned, and errno is set appropriately.
*/
int ctc_get_hw_version(int fd, int *hw_version);

//! Get Status (enable/disable register)
/*!
  \brief It gives back the value of the register "Counter Enable" that shows the state (enable/disable) for all channels.
  \param fd 		file descriptor
  \param chan 		channel number [1-8]
  \param status 	pointer to int to receive the value. 1 is enabled, 0 is disabled.
  \return returns 0 on success. On error, negative value is returned, and errno is set appropriately.
*/
int ctc_chan_get_status(int fd, int chan, int *status);

//! Enable a channel
/*!
  \param fd 	file descriptor
  \param chan	channel number [1-8]
  \return returns 0 on success. On error, negative value is returned, and errno is set appropriately.
*/
int ctc_chan_enable(int fd, int chan);

//! Disable a channel
/*!
  \param fd 	file descriptor
  \param chan	channel number [1-8]
  \return returns 0 on success. On error, negative value is returned, and errno is set appropriately.
*/
int ctc_chan_disable(int fd, int chan);

//! Set external start of a specific channel
/*!
  \param fd 		file descriptor
  \param chan		channel number [1-8]
  \param ext_start	external start input [1-40]
  \return returns 0 on success. On error, negative value is returned, and errno is set appropriately.
*/
int ctc_chan_set_ext_start(int fd, int chan, int ext_start);

//! Get external start of a specific channel
/*!
  \param fd 		file descriptor
  \param chan		channel number [1-8]
  \param ext_start	pointer to receive external start input [1-40]
  \return returns 0 on success. On error, negative value is returned, and errno is set appropriately.
*/
int ctc_chan_get_ext_start(int fd, int chan, int *ext_start);

//! Set external clock source of a counter 1 of a specific channel
/*!
  \param fd 		file descriptor
  \param chan		channel number [1-8]
  \param clk		clock source number [1-6]
  \return returns 0 on success. On error, negative value is returned, and errno is set appropriately.
*/
int ctc_chan_set_clk_counter1(int fd, int chan, int clk);

//! Get external clock source of a counter 1 of a specific channel
/*!
  \param fd 		file descriptor
  \param chan		channel number [1-8]
  \param clk		pointer to a variable to receive clock source number [1-6]
  \return returns 0 on success. On error, negative value is returned, and errno is set appropriately.
*/
int ctc_chan_get_clk_counter1(int fd, int chan, int *clk);

//! Set external clock source of a counter 2 of a specific channel
/*!
  \param fd 		file descriptor
  \param chan		channel number [1-8]
  \param clk		clock source number [1-6]
  \return returns 0 on success. On error, negative value is returned, and errno is set appropriately.
*/
int ctc_chan_set_clk_counter2(int fd, int chan, int clk);

//! Get external clock source of a counter 1 of a specific channel
/*!
  \param fd 		file descriptor
  \param chan		channel number [1-8]
  \param clk		pointer to a variable to receive clock source number [1-6]
  \return returns 0 on success. On error, negative value is returned, and errno is set appropriately.
*/
int ctc_chan_get_clk_counter2(int fd, int chan, int *clk);

//! Set mode to a specific channel
/*!
  \param fd 		file descriptor
  \param chan		channel number [1-8]
  \param mode		variable to indicate the mode used. Please use enum ctc_mode values.
  \param direction	variable to indicate the direction used. Please use enum ctc_mode values.
  \return returns 0 on success. On error, negative value is returned, and errno is set appropriately.
*/
int ctc_chan_set_mode(int fd, int chan, int mode, int direction);

//! Get mode to a specific channel
/*!
  \param fd 		file descriptor
  \param chan		channel number [1-8]
  \param mode		pointer to int to receive the mode used.
  \param direction	pointer to int to receive the direction used. 
  \return returns 0 on success. On error, negative value is returned, and errno is set appropriately.
*/
int ctc_chan_get_mode(int fd, int chan, int *mode, int *direction);

//! Set delay for the counter 1 of a specific channel
/*!
  \param fd 		file descriptor
  \param chan		channel number [1-8]
  \param delay		32 bits delay value
  \return returns 0 on success. On error, negative value is returned, and errno is set appropriately.
*/
int ctc_chan_set_delay_counter1(int fd, int chan, int delay);

//! Get delay for the counter 1 of a specific channel
/*!
  \param fd 		file descriptor
  \param chan		channel number [1-8]
  \param delay		pointer to receive the 32 bits delay value
  \return returns 0 on success. On error, negative value is returned, and errno is set appropriately.
*/
int ctc_chan_get_delay_counter1(int fd, int chan, int *delay);

//! Set delay for the counter 2 of a specific channel
/*!
  \param fd 		file descriptor
  \param chan		channel number [1-8]
  \param delay		32 bits delay value
  \return returns 0 on success. On error, negative value is returned, and errno is set appropriately.
*/
int ctc_chan_set_delay_counter2(int fd, int chan, int delay);

//! Get delay for the counter 2 of a specific channel
/*!
  \param fd 		file descriptor
  \param chan		channel number [1-8]
  \param delay		pointer to receive the 32 bits delay value
  \return returns 0 on success. On error, negative value is returned, and errno is set appropriately.
*/
int ctc_chan_get_delay_counter2(int fd, int chan, int *delay);

//! Get current value of the output counter
/*!
  \param fd 		file descriptor
  \param chan		channel number [1-8]
  \param value		pointer to receive the 32 bits output value. Only 8 least significant bits are useful.
  \return returns 0 on success. On error, negative value is returned, and errno is set appropriately.
*/
int ctc_chan_get_output_counter(int fd, int chan, int *value);

//! Get current value of the counter 1 of a specific channel
/*!
  \param fd 		file descriptor
  \param chan		channel number [1-8]
  \param value		pointer to receive the 32 bits output value.
  \return returns 0 on success. On error, negative value is returned, and errno is set appropriately.
*/
int ctc_chan_get_cur_val_counter1(int fd, int chan, int *value);

//! Get current value of the counter 2 of a specific channel
/*!
  \param fd 		file descriptor
  \param chan		channel number [1-8]
  \param value		pointer to receive the 32 bits output value.
  \return returns 0 on success. On error, negative value is returned, and errno is set appropriately.
*/
int ctc_chan_get_cur_val_counter2(int fd, int chan, int *value);


#ifdef __cplusplus
}
#endif
#endif /* _LIBCTC_H */
