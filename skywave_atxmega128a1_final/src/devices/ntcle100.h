/*
 * ntcle100.h
 *
 * Created: 5/10/2018 9:32:40 AM
 *  Author: Penguin
 */ 

#include "skywave.h"

#ifndef _NTCLE100_H_
#define _NTCLE100_H_

typedef struct
{
	float temp;
	int samplect;
}ntcle100_Data;

void ntcle100_init(void);

uint16_t get_adc_reading(void);

float get_voltage(uint16_t adc_reading);

uint32_t get_resistance(float voltage);

float calc_temp(uint32_t resistance, _Bool bShouldReturnInCelsius);
float temp_off;
void get_ntcle_temp(void);

void get_ntcle_data(ntcle100_Data* ntcle100Data, _Bool bShouldResetData);

void reset_ntcle_data(void);

float get_oversampled_ntcle_temp(int sample_amt);

float get_ntcle_temp_now(void);

void start_ntcle_read(void);

#endif