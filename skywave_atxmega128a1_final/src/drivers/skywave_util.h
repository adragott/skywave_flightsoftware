/*
 * skywave_util.h
 *
 * Created: 5/11/2018 7:34:25 PM
 *  Author: Penguin
 */ 
#include "skywave.h"
#include "register_map.h"
#ifndef _SKYWAVE_UTIL_H_
#define _SKYWAVE_UTIL_H_

#define WDT_1S WDT_TIMEOUT_PERIOD_1KCLK
#define WDT_2S WDT_TIMEOUT_PERIOD_2KCLK
#define WDT_4S WDT_TIMEOUT_PERIOD_4KCLK

typedef struct
{
	uint8_t size_buffer;
	int16_t* buffer;
	uint8_t active;
	uint8_t size_active;
}circ_buffer_int;

typedef struct
{
	uint8_t size_buffer;
	float* buffer;
	uint8_t active;
	uint8_t size_active;
}circ_buffer_float;

typedef struct  
{
	uint8_t size_buffer;
	double* buffer;
	uint8_t active;
	uint8_t size_active;
}circ_buffer_double;

typedef struct
{
	int16_t x;
	int16_t y;
	int16_t z;
}vector3D_int;

typedef struct
{
	float x;
	float y;
	float z;	
}vector3D_float;

void cb_append_int(circ_buffer_int* buf, int16_t data);
int16_t cb_getAvg_int(circ_buffer_int* buf);
void init_circ_buffer_int(circ_buffer_int* circ_buf, int16_t* databuffer, uint8_t size_buf);

void cb_append_float(circ_buffer_float* buf, float data);
float cb_getAvg_float(circ_buffer_float* buf);
void init_circ_buffer_float(circ_buffer_float* circ_buf, float* databuffer, uint8_t size_buf);

void cb_append_double(circ_buffer_double* buf, double data);
float cb_getAvg_double(circ_buffer_double* buf);
void init_circ_buffer_double(circ_buffer_double* circ_buf, double* databuffer, uint8_t size_buf);

void sw_32hz_timer_init(void);
void set_32hz_read(void);
void set_32hz_ready(void);
void clear_32hz_ready(void);
_Bool checK_32hz_ready(void);

void sw_16hz_timer_init(void);
void set_16hz_read(void);
void set_16hz_ready(void);
void clear_16hz_ready(void);
_Bool checK_16hz_ready(void);

void vector3D_int_init(vector3D_int* data, int16_t value);
void vector3D_float_init(vector3D_float* data, float value);
void skywave_util_init(void);
uint64_t millis(void);

int ascii_to_decimal_int(const char* str);
float ascii_to_decimal_float(const char* str);

#endif