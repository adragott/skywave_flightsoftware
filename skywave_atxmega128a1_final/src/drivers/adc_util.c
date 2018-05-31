/*
 * adc_util.c
 *
 * Created: 5/10/2018 9:27:49 AM
 *  Author: Penguin
 */ 

#include "drivers/adc_util.h"

void adc_util_init(void)
{
	//enable adca
	ADCA.CTRLA = 0b00000001;
	//enable unsigned 12 bit res
	ADCA.CTRLB = 0b00000000;
	//[5:4] = 01 for Vcc/1.6 as ref voltage
	ADCA.REFCTRL = 0b00010000;
		
	ADCA.PRESCALER = 0b00000101;
	ADCA.CAL = adc_get_calibration_data(ADC_CAL_ADCA);

	////enable adc 1
	//ADCA.CH1.CTRL = 0b00000001;
//
	////pin 1 for ch1
	//ADCA.CH1.MUXCTRL = 0b00001000;
}

