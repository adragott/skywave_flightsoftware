/*
 * ntcle100.c
 *
 * Created: 5/10/2018 9:32:55 AM
 *  Author: Penguin
 */ 

#include "devices/ntcle100.h"
#include "drivers/adc_util.h"
#include <math.h>

#define RESISTOR_ONE 13000
#define NTCLE100_OVERSAMPLE_AMOUNT 4

static ntcle100_Data ntcle100_data;
//should be measured in every change in testing environment
static const float v_in = 3.3;

void start_ntcle_read(void)
{
	ADCA.CH0.CTRL |= 0b10000000;
}

void ntcle100_init(void)
{
	temp_off = 0;
	//enable adc channel 0
	ADCA.CH0.CTRL = 0b00000001;
	//pin 0 for ch0
	ADCA.CH0.MUXCTRL = 0b00000000;
	swprintf(SWDEBUG, "Test temp: %-2.2f\n", get_ntcle_temp_now());
}

uint16_t get_adc_reading(void)
{
	start_ntcle_read();
	while(ADCA.CH0.INTFLAGS == 0);
	uint16_t reading = ADCA.CH0.RES;
	ADCA.INTFLAGS = 0;
	return reading;
}

float get_voltage(uint16_t adc_reading)
{
	float voltage = (adc_reading * (v_in / 1.6)) / 4096.0;
	return voltage;
}

uint32_t get_resistance(float voltage)
{
	float voltage_drop = v_in - voltage;
	uint32_t resistance = (voltage * RESISTOR_ONE) / voltage_drop;
	return resistance;
}

float calc_temp(uint32_t resistance, _Bool bShouldReturnInCelsius)
{
	double var_a = 0.003354016;
	double var_b =  0.0002569850 * log(resistance/10000.0);
	double var_c =  0.000002620131 * (2 * (log(resistance/10000.0)));
	double var_d =  0.00000006383091 * (3 * (log(resistance/10000.0)));
	float temp_in_kelvin = (float)(1/(var_a + var_b + var_c + var_d));
	return bShouldReturnInCelsius ? (temp_in_kelvin - 273.15) : temp_in_kelvin;
}

void get_ntcle_temp(void)
{
	ntcle100_data.temp += (get_oversampled_ntcle_temp(NTCLE100_OVERSAMPLE_AMOUNT) - temp_off);
	ntcle100_data.samplect++;
}

void get_ntcle_data(ntcle100_Data* ntcle100Data, _Bool bShouldResetData)
{
	ntcle100Data->temp = ntcle100_data.temp / ntcle100_data.samplect;
	if(bShouldResetData)
	{
		reset_ntcle_data();
	}
}

void reset_ntcle_data(void)
{
	ntcle100_data.temp = 0.0;
	ntcle100_data.samplect = 0;
}

float get_ntcle_temp_now(void)
{
	return (get_oversampled_ntcle_temp(NTCLE100_OVERSAMPLE_AMOUNT) - temp_off);
}

float get_oversampled_ntcle_temp(int sample_amt)
{
	float sum = 0;
	for(int x = 0; x < sample_amt; x++)
	{
		float temp = calc_temp(get_resistance(get_voltage(get_adc_reading())), true);
		sum += temp;
	}
	return sum/(float)sample_amt;
}