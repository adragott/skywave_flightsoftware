/*
 * twi_comms.c
 *
 * Created: 5/6/2018 2:53:26 PM
 *  Author: Penguin
 */ 
#include "twi_comms.h"

#define TWI_MAX_TIMEOUT 10000
//universal packet we will be using for all twi/i2c interactions
static twi_package_t univ_pkt;
static uint8_t buffer[64];





void twi_comms_init(void)
{
	static twi_options_t twi_conf =
	{
		.speed = TWI_SPEED,
		.speed_reg = TWI_BAUD(32000000, TWI_SPEED)
	};
	twi_master_init(TWI_MASTER, &twi_conf);
	twi_master_enable(TWI_MASTER);
	univ_pkt.no_wait = true;
	univ_pkt.addr_length = 1;
	univ_pkt.length = 1;
	univ_pkt.buffer = buffer;
	
	
	
	//swprintf(SWDEBUG, "TWI INIT SUCCESS\n");
}

void twi_write(uint8_t slave_address, uint8_t regi, uint8_t data)
{
	univ_pkt.chip = slave_address;
	univ_pkt.addr[0] = regi;
	buffer[0] = data;
	univ_pkt.length = 1;
	twi_master_write(TWI_MASTER, &univ_pkt);
}

void twi_read(uint8_t slave_address, uint8_t regi, uint8_t data_length, uint8_t* dataBuffer)
{
	univ_pkt.chip = slave_address;
	univ_pkt.addr[0] = regi;
	univ_pkt.length = data_length;
	twi_master_read(TWI_MASTER, &univ_pkt);
	for(uint8_t x = 0; x < data_length; x++)
	{
		dataBuffer[x] = 0;
		dataBuffer[x] = buffer[x];
	}
}

uint8_t* get_i2c_buffer(void)
{
	return buffer;
}
