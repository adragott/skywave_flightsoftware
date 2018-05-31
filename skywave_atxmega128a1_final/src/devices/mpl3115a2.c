/*
 * mpl3115a2.c
 *
 * Created: 5/10/2018 8:38:27 AM
 *  Author: Penguin
 */ 
#include "devices/mpl3115a2.h"
#include "drivers/skywave_util.h"
#include <math.h>

#define MPL3_GROUNDALT_SMPLCT 128
//i2c buffer
static uint8_t mpl3_buffer[128];

//data struct to hold ground altitude
static mpl3_Data groundAlt_Data;

//two circular buffers for moving averages
static circ_buffer_float mpl3_cb_temp;
static circ_buffer_float mpl3_cb_alt;

//buffers to put in the circular buffers
static float mpl3_cb_temp_buff[8];
static float mpl3_cb_alt_buff[8];

//Functions for processing data
static float get_altitude(uint8_t p_msb, uint8_t p_csb, uint8_t p_lsb);
static float get_pressure(uint8_t p_msb, uint8_t p_csb, uint8_t p_lsb);
static float get_temperature(uint8_t t_msb, uint8_t t_lsb);
//misc
static _Bool b_mpl3fired;
static _Bool bInitialAltitudeSet_MPL3;

void mpl3_init()
{
	init_circ_buffer_float(&mpl3_cb_alt, mpl3_cb_alt_buff, 8);
	init_circ_buffer_float(&mpl3_cb_temp, mpl3_cb_temp_buff, 8);
	b_mpl3fired = false;
	bInitialAltitudeSet_MPL3 = false;
	//reset sensor
	twi_write(MPL3_SLAVE_ADDR, MPL3_CTRL1, 0x04);

	uint8_t read_val = 1;
	//wait for sensor to boot back up
	while (read_val)
	{
		twi_read(MPL3_SLAVE_ADDR, MPL3_CTRL1, 1, mpl3_buffer);
		read_val = mpl3_buffer[0] & 0x04;
	}
	//config
	twi_write(MPL3_SLAVE_ADDR, MPL3_PT_DATA_CFG, 0x07);		//Enable Data Interrupt Flags for Temp and Pressure
	twi_write(MPL3_SLAVE_ADDR, MPL3_CTRL3, 0b00100000);		//active high push/pull int 1
	twi_write(MPL3_SLAVE_ADDR, MPL3_CTRL4, 0x80);			//enable data ready interrupt
	twi_write(MPL3_SLAVE_ADDR, MPL3_CTRL5, 0x80);			//move to int 1
	twi_write(MPL3_SLAVE_ADDR, MPL3_CTRL1, 0b10011000);		//set to standby mode
	delay_ms(100);
	//enable interrupts for normal operation
	PORTD.PIN2CTRL = PORT_OPC_WIREDORPULL_gc | PORT_ISC_RISING_gc;
	PORTD.INT0MASK = (1<<2);
	PORTD.INTCTRL |= PORT_INT0LVL_HI_gc;
}

float get_temperature(uint8_t t_msb, uint8_t t_lsb)
{
	float temp_lsb = (float)(t_lsb>>4)/16.0;
	return ((float)t_msb)+temp_lsb;
}

_Bool calc_mpl3_data(void)
{
	mpl3_clear_data_flag();
	twi_read(MPL3_SLAVE_ADDR, MPL3_STATUS, 6, mpl3_buffer);
	uint8_t status = mpl3_buffer[0];
	if(status & 0x08)
	{
		uint8_t p_msb = mpl3_buffer[1];
		uint8_t p_csb = mpl3_buffer[2];
		uint8_t p_lsb = mpl3_buffer[3];
		uint8_t t_msb = mpl3_buffer[4];
		uint8_t t_lsb = mpl3_buffer[5];
		twi_read(MPL3_SLAVE_ADDR, MPL3_CTRL1, 1, mpl3_buffer);
		uint8_t singleRead = mpl3_buffer[0];
		float tempValue = 0.0;
		if(singleRead & 0x80)
		{
			tempValue = get_altitude(p_msb, p_csb, p_lsb);
		}
		else
		{
			tempValue = get_pressure(p_msb, p_csb, p_lsb);
		}
		
		float newTemp = get_temperature(t_msb, t_lsb);
		//make sure we didn't mess up somewhere
		if((isnan(tempValue) || tempValue == 0) || (isnan(newTemp) || newTemp == 0) )
		{
			return false;
		}
		
		if(mpl3_checkbInitialAltitudeSet())
		{
			cb_append_float(&mpl3_cb_temp, newTemp);
			cb_append_float(&mpl3_cb_alt, tempValue);
		}
		else
		{
			groundAlt_Data.pressure += tempValue;
			groundAlt_Data.temp += newTemp;
			groundAlt_Data.samplect++;
			if(groundAlt_Data.samplect >= MPL3_GROUNDALT_SMPLCT)
			{
				mpl3_setInitialAltitude(&groundAlt_Data);
			}
		}
		
		return true;
	
	}
	return false;
}

void get_mpl3_data(mpl3_Data* mpl3Data, _Bool bShouldResetData)
{
	if(mpl3_checkbInitialAltitudeSet())
	{
		mpl3Data->temp = cb_getAvg_float(&mpl3_cb_temp);
		mpl3Data->pressure = (cb_getAvg_float(&mpl3_cb_alt) - groundAlt_Data.pressure);
	}
	else
	{
		mpl3Data->pressure = 0.0f;
		mpl3Data->temp = 0.0f;
	}
}

void mpl3_clear_data_flag(void)
{
	b_mpl3fired = false;
}

void mpl3_set_data_flag(void)
{
	b_mpl3fired = true;
}

_Bool mpl3_get_data_status(void)
{
	return b_mpl3fired;
}

void mpl3_single_read(mpl3_read_mode read_mode)
{
	if(read_mode == MPL3_MODE_ALT)
	{
		twi_write(MPL3_SLAVE_ADDR, MPL3_CTRL1, MPL3_SINGLE_READ_ALTITUDE);
	}
	else
	{
		twi_write(MPL3_SLAVE_ADDR, MPL3_CTRL1, MPL3_SINGLE_READ_PRESSURE);
	}
}

float get_altitude(uint8_t p_msb, uint8_t p_csb, uint8_t p_lsb)
{
	float tempRead = ((float)((short)(p_msb << 8) | p_csb) + (float)(p_lsb >> 4) * 0.0625);
	return tempRead;
}

float get_pressure(uint8_t p_msb, uint8_t p_csb, uint8_t p_lsb)
{
	//temp here means temporary--not temperature
	long tempPressure = (long)p_msb << 16 | (long)p_csb << 8 | (long)p_lsb;
	tempPressure >>= 6;
	uint8_t tempPresDecimal = p_lsb & 0b00110000;
	tempPresDecimal >>= 4;
	float pressureDecimal = (float)tempPresDecimal/4.0;
	return (float)tempPressure + pressureDecimal;
}

float get_altitude_now(void)
{
	return 0.0f;
}

void mpl3_setInitialAltitude(mpl3_Data* groundAlt)
{
	groundAlt_Data.pressure /= groundAlt_Data.samplect;
	groundAlt_Data.temp /= groundAlt_Data.samplect;
	groundAlt_Data.samplect = 1;
	bInitialAltitudeSet_MPL3 = true;
	set_ntcle_off(groundAlt_Data.temp);
	swprintf(SWDEBUG, "GROUND ALT: %-2.2f\n", groundAlt_Data.pressure);
}

_Bool mpl3_checkbInitialAltitudeSet(void)
{
	return bInitialAltitudeSet_MPL3;
}

ISR(PORTD_INT0_vect)
{
	mpl3_set_data_flag();
}