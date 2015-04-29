// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
 *
 * Atmel Corporation
 *
 * - File              : AVR_header.h
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

#ifndef AVRSMS_HEADER_H_INCLUDED
#define AVRSMS_HEADER_H_INCLUDED

/*! \brief This flash string holds the message header
 *
 *  This string should be modified with the desired\n
 *  receiver number etc. Please see the GSM modem data sheet.
 */

const unsigned char  PDU_HEADER[] =
{
	'0' , '0',
	'0' , '1', //<! SUBMIT SMS
	'0' , '0',
	'0' , 'A',//<! Phone nmbr length: 10
	'8' , '1',//<! Adr type
	'7' , '4', '3', '9', '0', '2', '1', '8', '4', '4',//<! Receivers phone nmbr
	'0' , '0',//<! TP_PID
	'0' , '0',//<! TP_DCS
	'\0'
};

#define HEADER_LEN 12                                       //<! Number of octets in header
#endif
