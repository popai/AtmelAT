// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
 *
 * Atmel Corporation
 *
 * - File              : AVR_api.c
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

//Used for SPLINT
#ifdef S_SPLINT_S
#include"splint.h"
#endif

//Includes
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "../com/AVRSMS_com.h"
#include "AVRSMS_zip.h"
#include "../../AVRSMS_header.h"
#include "AVRSMS_tools.h"
#include "AVRSMS_api.h"

//! Extern flag from AVRSMS_com.c
extern int rx_ack;

//! Message buffer for coded and decoded messages
unsigned char msgbuff[161];

//! AT-Command set used

const unsigned char ATE0[] PROGMEM = "ATE0\r\n"; //!< Echo off
const unsigned char AT_CNMI[] PROGMEM = "AT+CNMI=1,1,,,1\r\n"; //!< Identification of new sms
const unsigned char AT_CPMS[] PROGMEM = "AT+CPMS=\"ME\",\"ME\",\"ME\"\r\n"; //!< Preferred storage
const unsigned char AT_CMGD[] PROGMEM = "AT+CMGD="; //!< Delete message at index
const unsigned char AT_CMGR[] PROGMEM = "AT+CMGR="; //!< Read from index
const unsigned char AT_CMGS[] PROGMEM = "AT+CMGS="; //!< Send mmessage
const unsigned char CRLF[] PROGMEM = "\r\n"; //!< Carrige return Line feed

//! Function prototype
int API_check_acknowledge(void); //Checks acknowledge from phone

/*! \brief Used to setup the connected GSM modem
 *
 *  This function will send AT-Commands to the phone. These commands
 *  will setup the phone to:
 *  -Use correct storage, AT+CPMS
 *  -Indicate new message, AT+CNMI
 *  -Turn echo off, ATE0
 *
 *  \param    void
 *
 *  \retval   1 Success
 *  \retval   0 Error with echo off
 *  \retval   -1 Error with preferred storage
 *  \retval   -2 Error with indication
 */
int API_phoneinit(void)
{

	COM_rx_reset(); //Reset system
	COM_setSearchString(OK_); //Set OK to be search string
	COM_putsf(ATE0); //Send turn echo off
	COM_rx_on(); //Receiver on

	if (API_check_acknowledge() > 0) //Echo off = OK
	{
		COM_putsf(AT_CPMS); //Send preferred storage
		COM_rx_on(); //Receiver on

		if (API_check_acknowledge() > 0) //Preferred storage = OK
		{
			COM_putsf(AT_CNMI); //Send preferred indication of new messages
			COM_rx_on(); //Receiver on

			if (API_check_acknowledge() > 0) //Preferred indication = OK
			{
				return 1;
			}

			else //Preferred indication != OK
			{
				return -2;
			}
		}

		else //Preferred storage != OK
		{
			return -1;
		}
	}

	else //Echo off != OK
	{
		return 0;
	}
}

/*! \brief Delete a message from a given index
 *
 *  This function will use the "AT+CMGD" command to delete
 *  the message @ index
 *
 *  \param    index index to delete from
 *
 *  \retval   1 Success
 *  \retval   0 Error
 */
int API_deletemsg(int index)
{

	COM_rx_reset(); //Reset system
	COM_setSearchString(OK_); //Set OK to be search string
	COM_putsf(AT_CMGD); //Delete message
	COM_put_integer(index); //@index
	COM_putsf(CRLF); //CR+LF
	COM_rx_on(); //Receiver on

	if (API_check_acknowledge() > 0) //Delete = OK
	{
		return 1;
	}

	else //Delete != OK
	{
		return 0;
	}
}

/*! \brief Send message
 *
 *  This function will take your user defined message, encode this
 *  text, add the header information found in AVRSMS_header.h.
 *  If successful, the message will be forwarded to the connected GSM modem
 *
 *  \param    *msg unsigned char pointer user defined message
 *
 *  \retval   1 Success, message sent
 *  \retval   0 Error doing compression
 *  \retval  -1 No "> " from phone
 *  \retval  -2 No message sent acknowledge
 */
int API_sendmsg(unsigned char *msg)
{

	//! Local variables
	int payload_len, len; //Total length of message, and length of user text
	unsigned char payload_len_c[3], jump;

	//Init
	payload_len = len = 0;
	payload_len_c[0] = jump = '\0';

	//If necessary turn interrupts off
	if ((payload_len = ZIP_compress(msg, &jump, msgbuff)) == 0) //Convert user text to pdu format
	{
		return 0; //Something wrong happend during compression
	}

	//Compression ok
	else
	{
		ZIP_itoh(payload_len, &payload_len_c[0]); //Convert Integer payload to hex in string format
		len = HEADER_LEN + payload_len - jump; //Calculate overall length

		COM_rx_reset(); //Clear rx_buff
		COM_setSearchString(READY_); //Set "> " to be search string
		COM_putsf(AT_CMGS); //Send message
		COM_put_integer(len); //append length
		COM_putsf(CRLF); //CR+LF
		COM_rx_on(); //Receiver on

		//Append payload
		if (API_check_acknowledge() > 0) //Wait for acknowledge = "> "
		{
			COM_rx_reset(); //Clear rx_buff
			COM_setSearchString(OK_); //Set "OK" to be search string
			COM_putsf(PDU_HEADER);
			COM_puts(payload_len_c);
			COM_puts(msgbuff);
			COM_putchar(26);
			COM_rx_on(); //Receiver on

			if (API_check_acknowledge() > 0) //Acknowledge = OK
			{
				return 1;
			}

			else //Acknowledge != OK
			{
				return -2;
			}

		}

		else //Acknowledge != "> "
		{
			return -1;
		}
	}
}

/*! \brief Read message from a given index
 *
 *  This function is used to read a newly arrived message
 *  from a given index. The message is decoded, and stored
 *  in the msgbuff.
 *
 *  \param    ind index to read message from
 *
 *  \retval   i Length of new message
 *  \retval   0 Error, No acknowledge from phone
 */
int API_readmsg(int ind)
{

	//! Local variables
	unsigned char *encodedmsg; //Pointer to encoded message
	int i;

	//Init
	encodedmsg = '\0';
	i = 0;

	COM_rx_reset(); //Reset system
	COM_setSearchString(OK_); //Set OK to be search string
	COM_putsf(AT_CMGR); //Read message
	COM_put_integer(ind); //@index
	COM_putsf(CRLF); //CR+LF
	COM_rx_on(); //Receiver on, wait for acknowledge

	if (API_check_acknowledge() > 0) //Read = OK
	{
		encodedmsg = TOOLS__decodeCMGR(ind); //Get encoded message from the data returned from the phone
		i = ZIP_decompress(encodedmsg, msgbuff); //Decompress this message

		//COM_puts( msgbuff );                      //Could be used during test, where the GSM modem is switched with a terminal application

		return i;
	}

	else //Read != OK
	{
		return 0;
	}
}

/*! \brief Check acknowledge returned from phone
 *
 *  This function is very important. It checks if an acknowledge
 *  has been received from the phone. A counting loop is also
 *  included to avoid waiting for a acknowledge that never arrives.
 *
 *  \param    void
 *
 *  \retval   1 Success, correct acknowledge
 *  \retval   0 Error, returned "ERROR" or timed out
 */
int API_check_acknowledge(void)
{

	//! Local variables
	static unsigned int i, ii;

	for (i = 0; (rx_ack == 0) && (i < 65535); i++) //Wait loop
	{
		for (ii = 0; ii < 10; ii++)
		{
			;
		}
	}

	if (rx_ack > 0) //Everything worked out fine...rx turned off
	{
		rx_ack = 0; //Reset flag
		return 1;
	}

	else //A timeout could result from no acknowledge, wrong acknowledge or buffer overrun
	{
		COM_rx_off(); //Receiver off
		COM_rx_reset(); //Reset buffer and interrupt routine
		return 0; //Timed out, or wrong acknowledge from phone
	}
}
