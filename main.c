// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
 *
 * Atmel Corporation
 *
 * - File              : main.c
 * - Compiler          : IAR EWAAVR 4.11a
 *
 * - Support mail      : avr@atmel.com
 *
 * - Supported devices : All devices with a UART/USART can be used.
 *                       The example is written for ATmega128
 *
 * - AppNote           : AVR323 - Interfacing GSM modems
 *
 * - Description       : Example of how to use AT-Commands to control a GSM modem
 *
 * $Revision: 1.1 $
 * $Date: Tuesday, November 08, 2005 12:26:20 UTC $
 *****************************************************************************/

/*! \mainpage
 * \section Intro Introduction
 * This documents data structures, functions, variables, defines, enums, and
 * typedefs in the software for application note AVR323.\n
 *
 * \section CI Compilation Info
 * This software was written for the IAR Embedded Workbench, 4.11A, but can
 * also be built using ###### (see note futher down).\n
 * To make project:\n
 * Add the .c files to project (main.c, AVRSMS_api.c, AVRSMS_com.c,\n
 * AVRSMS_header.h, AVRSMS_tools.c and AVRSMS_zip.c). Use device
 * --cpu=m128, enable bit definitions in I/O include files, optimization low
 * for debug target and high for release, output format: ubrof8 for Debug and
 * intel_extended for Release \n
 *
 * \section DI Device Info
 * Devices with a UART/USART can be used. The example is
 * written for ATmega128.\n
 *
 * \section TDL ToDo List
 * Add suport for GPRS.\n
 */

//Used for SPLINT
#ifdef S_SPLINT_S
#include"splint.h"
#endif

//Includes
#include <avr/io.h>
#include <avr/interrupt.h>

#include"lib/com/AVRSMS_com.h"
#include"lib/ATapi/AVRSMS_api.h"
#include"lib/ATapi/AVRSMS_tools.h"

//! Flag imported from AVRSMS_com.c
extern int rx_ack;

//! Storage for new message index, see main loop
volatile int read_index;

//! Function prototype
void MAIN_init(void);

/*! \brief Sytem initialization
 *
 *  This function will set:
 *  -Correct baud rate
 *  -Modem init
 *  -Wait message
 *  -Receiver on
 *
 *  \param void
 *  \retval void
 */
void MAIN_init(void)
{
	COM_init(12); //Find corret baud rate in data sheet, must run @ 9600bps
	sei();
	//__enable_interrupt( );           //Enable interrupt
	API_phoneinit(); //Initialize phone for textual messaging
	COM_rx_reset(); //Reset receive buffer after API_phoneinit()
	COM_setSearchString(CMTI_); //Wait for "+CMTI: ,Storage,index\r\n"
	COM_rx_on(); //Ready to receive
}

/*! \brief Main loop
 *
 *  This function is a simple main loop.
 *  Whenever a new message arrives, rx_ack  = 1, it is read and decoded.
 *  Toggle Breakpoint after API_readmsg, and view the result in msgbuff
 *  found in AVRSMS_api.c
 *
 *  \param void
 *  \retval void
 */
int main(void)
{

	MAIN_init(); //Setup USART and modem

	read_index = 0; //Init variable

	for (;;) //Forever
	{

		if (rx_ack == 1) //New Message has arrived
		{
			read_index = TOOLS_decodeCMTI(); //Find index
			API_readmsg(read_index); //Read new message, and decode

			COM_setSearchString(1); //PLACE BREAKPOINT AT THIS LINE. Set searchstring to "+CMTI: " again, wait for new message.
			COM_rx_reset(); //Reset buffer and interrupt routine
			COM_rx_on(); //Ready to receive
		}
	}
}
