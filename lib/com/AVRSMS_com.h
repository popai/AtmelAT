// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
 *
 * Atmel Corporation
 *
 * - File              : AVR_com.h
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

#ifndef AVR_SMS_COM_H_INCLUDED
#define AVR_SMS_COM_H_INCLUDED

#include <avr/pgmspace.h>

#define RX_BUFFER_SIZE 256                  //!< rx_buffer size
#define RX_BUFFER_MASK  RX_BUFFER_SIZE - 2  //!< Used to set overflow flag
#define RX_WAIT        65000                //!< Timeout value
#define OK_     0                           //!< Used to look up in COM_setSearchString( unsigned char Response )
#define CMTI_   1                           //!< Used to look up in COM_setSearchString( unsigned char Response )
#define READY_  2                           //!< Used to look up in COM_setSearchString( unsigned char Response )
#define CRLF_   3                           //!< Used to look up in COM_setSearchString( unsigned char Response )

// Prototypes
void COM_init(unsigned int baudrate);
void COM_rx_reset(void);
void COM_rx_on(void);
void COM_rx_off(void);
void COM_setSearchString(unsigned char Response);
int COM_putchar(unsigned char data);
void COM_put_integer(int i);
void COM_puts(unsigned char *str);
void COM_putsf(PGM_P fstr);
int COM_trim(void);
unsigned char* COM_gets(void);
#endif
