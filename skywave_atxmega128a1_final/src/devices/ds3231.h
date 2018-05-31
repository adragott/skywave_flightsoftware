/*
 * ds3231.h
 *
 * Created: 5/9/2018 6:40:17 AM
 *  Author: Penguin
 */ 
#include "skywave.h"
#ifndef _DS3231_H_
#define _DS3231_H_

typedef struct
{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t pm_or_am; //high is pm, low is am
}sw_time;
	

void ds3231_init(void);

void ds3231_set_ready(void);

_Bool ds3231_is_ready(void);

void ds3231_clear_ready(void);

void ds3231_set_time(uint8_t hours, uint8_t minutes, uint8_t seconds);

void ds3231_get_time(sw_time* time);
#endif