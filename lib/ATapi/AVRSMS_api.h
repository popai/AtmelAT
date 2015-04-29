// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
 *
 * Atmel Corporation
 *
 * - File              : AVR_api.h
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
#ifndef AVR_SMS_API_H_INCLUDED
#define AVR_SMS_API_H_INCLUDED

int API_phoneinit(void);
int API_deletemsg(int index);
int API_sendmsg(unsigned char *msg);
int API_readmsg(int ind);

#endif
