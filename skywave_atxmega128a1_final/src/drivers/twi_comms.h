/*
 * twi_comms.h
 *
 * Created: 5/6/2018 2:53:18 PM
 *  Author: Penguin
 */ 
#include "skywave.h"
#include "register_map.h"

#ifndef _TWI_COMMS_H_
#define _TWI_COMMS_H_

#define TWI_SPEED 400000


void twi_comms_init(void);
void twi_write(uint8_t slave_address, uint8_t regi, uint8_t data);
void twi_read(uint8_t slave_address, uint8_t regi, uint8_t data_length, uint8_t* dataBuffer);
uint8_t* get_i2c_buffer(void);

#endif