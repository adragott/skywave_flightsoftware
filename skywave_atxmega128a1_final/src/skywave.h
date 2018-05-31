/*
 * skywave.h
 *
 * Created: 5/6/2018 2:22:55 PM
 *  Author: Penguin
 */ 

#include "asf.h"

#ifndef _SKYWAVE_H_
#define _SKYWAVE_H_

//usart
#define USART_OPENLOGGER	(&USARTF0)
#define USART_XBEE			(&USARTC0)
#define USART_GPS			(&USARTC1)
#define SWDEBUG				(&USARTC0)

//twi/i2c
#define TWI_MASTER			(&TWIF)

//PORT D Pin Guide
//will change when the pcb gets here
#define PIN_DS3231_INT		0
#define PIN_MPU9250_INT		1
#define PIN_MPL_INT			2
#define PIN_MPU_ADDR		3
#define PIN_HW1				4 //Hardware interface 1
#define PIN_HW2				5 //Hardware interface 2


typedef enum
{
	SUCCESS =		0, 
	BAD_DATA =		1, 
	NO_RESPONSE =	2
}SW_INIT_STATUS;

typedef enum
{
	SW_STANDBY =		0,
	SW_ASCENT =			1,
	SW_FIRST_DESCENT =	2,
	SW_SEC_DESCENT =	3,
	SW_FINAL =			4
}SW_FLIGHT_STAGE;

typedef enum
{
	SWS_RTC =		0,
	SWS_GPS =		1,
	SWS_NTCLE100 =	2,
	SWS_MPU9250 =	3,
	SWS_MPL3 =		4,
	SWS_AK8963 =	5
}SW_SENSOR_STATUS;
uint8_t sensor_ready_status;
// 7 6 5 4 3 2 1 0
// 6-7 = RESERVED
// 5 = AK8963
// 4 = MPL3115A2
// 3 = MPU9250
// 2 = NTCLE100
// 1 = GPS
// 0 = RTC

void skywave_init(void);
void (*swprintf)(USART_t* usart_channel, const char* text, ...);
void change_flight_state(void);
void set_ntcle_off(float t_off);

void (*flight_handler)(void);
void sw_fs_standby_handler(void);
void sw_fs_ascent_handler(void);
void sw_fs_first_descent_handler(void);
void sw_fs_sec_descent_handler(void);
void sw_fs_final_handler(void);

void sw_fs_standby_init(void);
void sw_fs_ascent_init(void);
void sw_fs_first_descent_init(void);
void sw_fs_sec_descent_init(void);
void sw_fs_final_init(void);

void recovery_handler(void);

#endif