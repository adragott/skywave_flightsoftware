/*
 * gps.c
 *
 * Created: 5/28/2018 5:08:24 AM
 *  Author: Penguin
 */ 
#include "drivers/usart_comms.h"
#include "devices/gps.h"
#include "drivers/skywave_util.h"

#define GPS_SIZE_CHAR_BUFF 1024
#define GPS_SIZE_GPS_BUFF 10

volatile char gps_response_temp;
char gps_buff[GPS_SIZE_CHAR_BUFF];
static gpsData gps_data_buff[GPS_SIZE_GPS_BUFF] = {};
gpsCircBuff gps_circ_buff;
charCircBuff char_circ_buff;
void gps_init(void)
{
	int16_t z = (int16_t)ascii_to_decimal_int((const char*)("-7826"));
	swprintf(SWDEBUG, "%d\n", z);
	for(uint8_t x = 0; x < 10; x++)
	{
		gps_data_init(&(gps_data_buff[x]));
	}
	gcb_init(&gps_circ_buff, gps_data_buff, GPS_SIZE_GPS_BUFF);
	ccb_init(&char_circ_buff, gps_buff, GPS_SIZE_CHAR_BUFF);
	
	gps_uart_conf.baudrate = 9600;
	gps_uart_conf.stopbits = true;
	gps_uart_conf.paritytype = USART_PMODE_DISABLED_gc;
	gps_uart_conf.charlength = USART_CHSIZE_8BIT_gc;
	usart_init_rs232(USART_GPS, &gps_uart_conf);
	//may seem redundant but, well yeah
	swprintf(USART_GPS, PMTK_SET_BAUD_9600);
	swprintf(USART_GPS, PGCMD_NOANTENNA);
	swprintf(USART_GPS, PMTK_SET_NMEA_OUTPUT_RMCONLY);
	swprintf(USART_GPS, PMTK_API_SET_FIX_CTL_10HZ);
	swprintf(USART_GPS, PMTK_SET_NMEA_UPDATE_2HZ);
	
	usart_set_rx_interrupt_level(USART_GPS, USART_INT_LVL_MED);
}

//lines must end in \r\n
void parseLineToChunkedPacket(const char* txt, char split, gpsData* data)
{
	
}

void parseLine(const char* txt, char split)
{
	
}

void ccb_init(charCircBuff* ccb, char* buff, uint16_t size)
{

}

void ccb_append(charCircBuff* ccb, char val)
{

}

void gcb_init(gpsCircBuff* gcb, gpsData* buff, uint8_t size)
{
	
}

void gcb_append(gpsCircBuff* gcb, gpsData* data)
{

}

void gps_data_init(gpsData* gps_data)
{
	gps_data->b_parsed = false;
	gps_data->gps_str[0] = '\0';
	gps_data->pkt_type = GPS_VACANT;
}

ISR(USARTC1_RXC_vect)
{
	gps_response_temp = usart_getchar(USART_GPS);
}