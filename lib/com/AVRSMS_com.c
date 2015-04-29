// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
 *
 * Atmel Corporation
 *
 * - File              : AVR_com.c
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
#include <avr/io.h>
#include <avr/interrupt.h>

#include"AVRSMS_com.h"

//! Overflow and acknowledge flag
int rx_overflow, rx_ack;

//! private buffer
static unsigned char rx_buffer[RX_BUFFER_SIZE];

//! Private pointer
static unsigned char searchStr;

//! Buffer counter
static int rx_i;

//! Buffer write index
static int rx_wr_i;

volatile unsigned char *searchFor; //!< Flash pointer
//! Possible strings pointed to by searchStr
const unsigned char OK[] PROGMEM = "OK\r\n"; //!< "OK"
const unsigned char CMTI[] PROGMEM = "+CMTI: "; //!< New Message arrived
const unsigned char READY[] PROGMEM = "> "; //!< Phone ready to receive text message
const unsigned char CR_LF[] PROGMEM = "\r\n"; //!< Carrige Return Line Feed
const unsigned char *searchStrings[4] PROGMEM =
{ OK, CMTI, READY, CR_LF }; //!< Initialize pointer

/*! \brief USART0 initialization
 *
 *  This function set correct baurate and functionality of
 *  the USART0. See data sheet for more details.
 *
 *  \param    input     Desired baudrate...see datasheet
 *
 *  \retval             void
 */
void COM_init(unsigned int baudrate)
{

	UBRR0H = (unsigned char) (baudrate >> 8); //Setting baudrate
	UBRR0L = (unsigned char) baudrate; //Setting baudrate

	UCSR0B = (1 << RXEN0) | (1 << TXEN0); //Enable receiver and transmitter
	UCSR0C = (1 << USBS0) | (1 << UCSZ01) | (1 << UCSZ00); //8N1...see Datasheet for more information

	COM_rx_reset(); //Reset buffers etc.
}

/*! \brief Reset receive interrupt
 *
 *  \param    void
 *
 *  \retval   void
 */
void COM_rx_reset(void)
{

	UCSR0B &= ~(1 << RXCIE0); // Disable RX interrupt
	rx_i = rx_wr_i = 0; //Init variables
	rx_overflow = rx_ack = 0; //Zero overflow flag
	rx_buffer[rx_wr_i] = '\0'; //Buffer init.
}

/*! \brief RX interrupt enable
 *
 *  \param    void
 *
 *  \retval   void
 */
void COM_rx_on(void)
{

	UCSR0B |= (1 << RXCIE0); // Enable RX interrupt
}

/*! \brief RX interrupt disable
 *
 *  \param    void
 *
 *  \retval   void
 */
void COM_rx_off(void)
{

	UCSR0B &= ~(1 << RXCIE0); // Disable RX interrupt
}

/*! \brief Set desired search string
 *
 *  \param    void
 *
 *  \retval   void
 */
void COM_setSearchString(unsigned char Response)
{

	UCSR0B &= ~(1 << RXCIE0); // Disable RX interrupt
	searchFor = (unsigned char *)pgm_read_word(&(searchStrings[Response])); // Necessary casts and dereferencing, just copy.
	//searchFor	= (unsigned char *)pgm_read_word(&(searchStrings[Response]));
	//searchFor = searchStrings[Response]; //Set desired search dtring
	searchStr = Response; //Used in rx_isr
	rx_i = 0;
}

/*! \brief Receive interrupt routine
 *
 *  This receive routine buffer incomming messages from the connected GSM modem
 *  But also check if the received string was a acknowledge
 */

ISR(USART_RX_vect)
{

	unsigned char data; //Local variable

	data = UDR0; //Always read something

	rx_buffer[rx_wr_i++] = data; //Store new data

	if (rx_wr_i > RX_BUFFER_MASK) //Check for overflow
	{
		rx_wr_i = 0; //Reset write index
		rx_overflow = 1; //Set flag high
		UCSR0B &= ~(1 << RXCIE0); // Disable RX interrupt
	}

	if (pgm_read_byte(&(searchFor[rx_i])) == data) //Test response match
	{
		rx_i++;

		if( !pgm_read_byte(&(searchFor[rx_i]))) //End of new_message string...received new message!
		{
			rx_i = 0;

			if (searchStr == CMTI_) //+CMTI:
			{
				searchFor = (unsigned char*) pgm_read_word(&searchStrings[CRLF_]); //Wait for
				searchStr = CRLF_;
			}

			else //Normal acknowledgement
			{
				rx_ack = 1; //Set new message flag
				UCSR0B &= ~(1 << RXCIE0); // Disable RX interrupt
			}
		}
	}

	else
	{
		rx_i = 0; //Not valid search pattern...start again.
	}
}

/*! \brief Adapted putchar method...no need for interrup driven tx-isr.
 *
 *  Adpation of ansi c putchar() method
 *
 *  \param    data  Character to send
 *
 *  \retval   data  if successful
 *  \retval   -1    if timeout
 */
int COM_putchar(unsigned char data)
{

	//Local variables
	unsigned int i;

	for (i = 0; !(UCSR0A & (1 << UDRE0)); i++) // Wait for empty transmit buffer
	{
		if (i > RX_WAIT) //How long one should wait
		{
			return -1; //Give feedback to function caller
		}
	}

	UDR0 = data; // Start transmittion 	

	return (int) data; //Cast and return int value
}

/*! \brief This method will print an integer in the range +-(10^5 - 1 )
 *
 *  \param    i  integer to be printed
 *
 *  \retval   void
 */
void COM_put_integer(int i)
{

	//! Local variables
	int ii;
	unsigned char int_buf[5];

	if (i < 0) //Integer is negative
	{
		i = -i; //Convert to positive Integer
		COM_putchar('-'); //Print - sign
	}

	for (ii = 0; ii < 5;) //Convert Integer to char array
	{
		int_buf[ii++] = '0' + i % 10; //Find carry using modulo operation
		i = i / 10; //Move towards MSB
	}
	do
	{
		ii--;
	} while ((int_buf[ii] == '0') && (ii != 0)); //Remove leading 0's
	do
	{
		COM_putchar(int_buf[ii--]);
	} while (ii >= 0); //Print int->char array convertion

}

/*! \brief Print unsigned char string
 *
 *  \param    str    Pointer to the string
 *
 *  \retval   void
 */
void COM_puts(unsigned char *str)
{

	for (; *str != '\0';)
	{
		COM_putchar(*str++);
	}
}

/*! \brief Print const unsigned char __flash string
 *
 *  \param    fstr  Pointer to the string
 *
 *  \retval   void
 */
void COM_putsf(PGM_P fstr)
{
	uint16_t i = 0;
	size_t stringlength;

	stringlength = strlen_P(fstr);

	while (i < stringlength)
		COM_putchar(pgm_read_byte(&fstr[i++]));

}

/*! \brief Remove trailing O, K, \r and \n
 *
 *  If the receive buffer have trailing "OK\r\n"
 *  These characters will be deleted.
 *
 *  \param  void
 *
 *  \retval i   Length of trimmed buffer
 */
int COM_trim(void)
{

	//! Local variables
	int i;
	unsigned char temp;

	for (i = rx_wr_i - 1; i >= 0; i--) //Run through COM_in[] backwards
	{
		temp = rx_buffer[i]; //rx_buff[i] into temp
		if ((temp != '\r') && (temp != '\n') && (temp != 'O') && (temp != 'K')) //If not equal to 'O', 'K', '\r' or '\n', break
		{
			break; //Do break
		}
	}

	rx_buffer[i + 1] = '\0'; //Terminate trimmed string

	return i; //Return new length
}

/*! \brief Return pointer to receive buffer
 *
 *  \param  void
 *
 *  \retval rx_buffer   Pointer to receive buffer
 */
unsigned char* COM_gets(void)
{
	return rx_buffer;
}
