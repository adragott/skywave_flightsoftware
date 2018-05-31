/*
 * usart_comms.c
 *
 * Created: 5/6/2018 3:54:52 PM
 *  Author: Penguin
 */ 
#include "usart_comms.h"

volatile char xbee_response_temp;


static char gps_buff[256];
//making a printf function for use with rs232 usart, must be attached to swprintf to be used globally
static void sw_rs232_printf(USART_t* usart_channel, const char* text, ...);
static uint16_t zee = 0;
void usart_comms_init()
{	
	//set our global swprintf function to use our local and private sw_rs232_printf function
	swprintf = &sw_rs232_printf;
	static usart_rs232_options_t uconf =
	{
		.baudrate = 115200,
		.stopbits = true,
		.paritytype = USART_PMODE_DISABLED_gc,
		.charlength = USART_CHSIZE_8BIT_gc
	};

	static usart_rs232_options_t openlogger_uart_conf =
	{
		.baudrate = 115200,
		.stopbits = true,
		.paritytype = USART_PMODE_DISABLED_gc,
		.charlength = USART_CHSIZE_8BIT_gc,
	};
	usart_init_rs232(USART_XBEE, &uconf);
	//usart_init_rs232(USART_OPENLOGGER, &openlogger_uart_conf);

	//enable interrupts for incoming messages through usart
	usart_set_rx_interrupt_level(USART_XBEE, USART_INT_LVL_HI);
	//uncomment if file system is implemented with open logger
	//usart_set_rx_interrupt_level(USART_OPENLOGGER, USART_INT_LVL_HI);
	
	//swprintf(SWDEBUG, "USART ONLINE\n");
	
}

void sw_rs232_printf(USART_t* usart_channel, const char* text, ...)
{
	char write_buffer[256];
	va_list args;
	va_start(args, text);
	vsprintf(write_buffer, text, args);
	va_end(args);
	//must make const string because hacky syntax below only works with const string
	const char* p_wb = write_buffer;
	//this syntax is hacky and I don't like it, BUT it cuts down on code and saves me time. I'll clean it up if I have time
	while(*p_wb)
	{
		usart_putchar(usart_channel, *p_wb++);
	}
}

ISR(USARTC0_RXC_vect)
{
	xbee_response_temp = usart_getchar(USART_XBEE);
	//usart_putchar(USART_XBEE, '!');
	if(xbee_response_temp == '`')
	{
		wdt_reset_mcu();
	}
	
}

////uncomment if file system is implemented for open logger
//ISR(USARTF0_RXC_vect)
//{
	//gps_response_temp = usart_getchar(USART_OPENLOGGER);
	//usart_putchar(SWDEBUG, gps_response_temp);
//}
