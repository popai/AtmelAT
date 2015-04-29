// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
 *
 * Atmel Corporation
 *
 * - File              : AVRSMS_tools.c
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

//Include
#include"../com/AVRSMS_com.h"
#include"AVRSMS_zip.h"
#include"AVRSMS_tools.h"

/*! \brief Used to get index of newly received message
 *
 *  Will search through the receive buffer found in AVRSMS_com.c, until a \n
 *  ',' is found. If this charcter is found in the buffer, the next parameter will \n
 *  be the new message index.
 *
 *  \param    void
 *
 *  \retval   i index of new message
 *  \retval   0 Error with echo off
 */
int TOOLS_decodeCMTI(void)
{

	//! Local variables
	unsigned char *temp;
	int length, i;

	//init
	length = 0;

	length = COM_trim(); //Trim off trailing OK\r\n
	temp = COM_gets(); //Get rx_buff

	for (i = 0; (i < length) && (*temp++ != ','); i++) //Loop until we get a ','
	{
		;
	}

	//Error
	if (i >= length)
	{
		return 0;
	}

	else
	{
		return ZIP_atoi(temp); //Convert ascii int to integer
	}
}

/*! \brief Get start of compressed string
 *
 *  When a new message has been read from a given index, thsi function\n
 *  will run through it, and find start of the encoded user text.
 *
 *  \param    void
 *
 *  \retval   in_handle if error in_handle points to '\0', else it will point to the string ready for decompression
 *
 */
unsigned char* TOOLS__decodeCMGR(int index)
{

	//! Local variables
	unsigned char *in_handle;
	int i, len, field_length;

	len = COM_trim(); //Trim off OK\r\n
	in_handle = COM_gets(); //Get rx_buff

	in_handle += 2; //Skip first \r\n

	for (i = 0; (i < len) && (*in_handle++ != '\n'); i++) //Loop until we get '\n'
	{
		;
	}

	//Error
	if (i >= len)
	{
		in_handle = '\0';
		return in_handle;
	}

	//Everything OK
	else
	{

		field_length = 16 * ZIP_htoi(*in_handle++);
		field_length = ZIP_htoi(*in_handle++);

		in_handle += 2 * field_length + 2; //Skip Service center nmbr

		//Find length of sender address
		field_length = 16 * ZIP_htoi(*in_handle++);
		field_length = ZIP_htoi(*in_handle++);

		if (field_length % 2)
		{
			field_length++; //Check for trailing F
		}

		in_handle += 2 + field_length; //Skip fields in header

		in_handle += 14 + 4; //Skip more fields...TP-PID, TP-DCS and TP-SCTS

		return (in_handle + 2); //Return correct pointer
	}
}
