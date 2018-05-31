/*
 * ds3231.c
 *
 * Created: 5/9/2018 6:39:55 AM
 *  Author: Penguin
 */ 
#include "devices/ds3231.h"
#include "drivers/twi_comms.h"

static uint8_t rtc_buffer[32];
static _Bool ds3231_status;
static sw_time timeBank;
void ds3231_init(void)
{	
	ds3231_status = false;
	timeBank.hour = 0;
	timeBank.minute = 0;
	timeBank.second = 0;
	timeBank.pm_or_am = 1;
	//reset and go
	twi_write(DS3231_ADDR, DS3231_CTRL, 0x0);
	twi_write(DS3231_ADDR, DS3231_CTRL_STATUS, 0x0);
	//configure and enable interrupt
	PORTD.PIN0CTRL = PORT_ISC_FALLING_gc;
	PORTD.INT1MASK = PIN0_bm;
	//or equals OR ELSE
	PORTD.INTCTRL |= PORT_INT1LVL_HI_gc;

	//swprintf(SWDEBUG, "%#x\n", rtc_buffer);
	ds3231_set_time(3, 58, 00);
}

void ds3231_set_ready(void)
{
	ds3231_status = true;
}

void ds3231_clear_ready(void)
{
	ds3231_status = false;
}

void ds3231_set_time(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
	//high is pm
	//low is am
	uint8_t pm_or_am = 0;
	if(hours >= 12)
	{
		if(hours == 24)
		{
			hours -= 12;
		}
		else if(hours == 12)
		{
			pm_or_am = 1;
		}
		else
		{
			hours -= 12;
			pm_or_am = 1;
		}
	}
	uint8_t formattedSeconds = ((seconds / 10) << 4) | (seconds % 10);
	uint8_t formattedMinutes = ((minutes / 10) << 4) | (minutes % 10);
	uint8_t formattedHours = (1 << 6) | (pm_or_am << 5) | ((hours / 10) << 4) | (hours % 10);
	twi_write(DS3231_ADDR, DS3231_HOURS, formattedHours);
	twi_write(DS3231_ADDR, DS3231_MINUTES, formattedMinutes);
	twi_write(DS3231_ADDR, DS3231_SECONDS, formattedSeconds);	
}

void ds3231_get_time(sw_time* time)
{
	twi_read(DS3231_ADDR, DS3231_SECONDS, 3, rtc_buffer);
	uint8_t hours = rtc_buffer[2];
	uint8_t minutes = rtc_buffer[1];
	uint8_t seconds = rtc_buffer[0];
	time->hour = (((hours & ~0b11101111) >> 4) * 10) + (hours & ~0b11110000);
	time->minute = (((minutes & ~0b00001111) >> 4) * 10) + (minutes & ~0b11110000);
	time->second = (((seconds & ~0b00001111) >> 4) * 10) + (seconds & ~0b11110000);
	//pm is high, am is low
	time->pm_or_am = (hours & (1 << 5)) ? 1 : 0;
}

_Bool ds3231_is_ready(void)
{
	return ds3231_status;
}

ISR(PORTD_INT1_vect)
{
	ds3231_set_ready();
}