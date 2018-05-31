/*
 * skywave_util.c
 *
 * Created: 5/11/2018 7:34:33 PM
 *  Author: Penguin
 */ 
#include "drivers/skywave_util.h"
#include "devices/ntcle100.h"
#include "devices/mpl3115a2.h"
#include <inttypes.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

static struct pwm_config conf_32hz_timer;
static struct pwm_config conf_16hz_timer;
static _Bool timer_16hz_ready;
static _Bool timer_32hz_ready;
uint64_t milliseconds;

void cb_append_int(circ_buffer_int* buf, int16_t data)
{
	//making the active slot 0 first should be completely unnecessary, but I've experienced overwriting data and it not erasing it all
	buf->buffer[buf->active] = 0;
	buf->buffer[buf->active] = data;
	buf->active = (buf->active + 1) % buf->size_buffer;
	buf->size_active++;
}

void init_circ_buffer_int(circ_buffer_int* circ_buf, int16_t* databuffer, uint8_t size_buf)
{
	circ_buf->size_active = 0;
	circ_buf->size_buffer = size_buf;
	circ_buf->buffer = databuffer;
}

int16_t cb_getAvg_int(circ_buffer_int* buf)
{
	int64_t bufTotal = 0;
	uint8_t lowIndex = 0;
	uint8_t highIndex = 0;
	uint8_t temp_cnt = 0;
	if(buf->size_active != 0)
	{
		if(buf->size_active == buf->size_buffer)
		{
			temp_cnt = 2;
			for(uint8_t x = 0; x < buf->size_buffer; x++)
			{
				if(buf->buffer[x] > buf->buffer[highIndex])
				{
					highIndex = x;
				}
				else if(buf->buffer[x] < buf->buffer[lowIndex])
				{
					lowIndex = x;
				}
			}
			for(uint8_t x = 0; x < buf->size_buffer; x++)
			{
				if((x != lowIndex) && (x != highIndex))
				{
					bufTotal += (buf->buffer[x]);
				}
			}
		}
		else
		{
			temp_cnt = 0;
			for(uint8_t x = 0; x < buf->size_buffer; x++)
			{
				bufTotal += (buf->buffer[x]);
			}
		}
		return (int16_t)(bufTotal / (buf->size_active - temp_cnt));
	}
	return 0;
}


void cb_append_float(circ_buffer_float* buf, float data)
{
	buf->buffer[buf->active] = 0;
	buf->buffer[buf->active] = data;
	buf->active = (buf->active + 1) % buf->size_buffer;
	if(buf->size_active < buf->size_buffer)
	{
		buf->size_active++;
	}
}

float cb_getAvg_float(circ_buffer_float* buf)
{
	double bufTotal = 0;
	uint8_t lowIndex = 0;
	uint8_t highIndex = 0;
	uint8_t temp_cnt = 0;
	if(buf->size_active != 0)
	{
		if(buf->size_active == buf->size_buffer)
		{
			temp_cnt = 2;
			for(uint8_t x = 0; x < buf->size_buffer; x++)
			{
				if(buf->buffer[x] > buf->buffer[highIndex])
				{
					highIndex = x;
				}
				else if(buf->buffer[x] < buf->buffer[lowIndex])
				{
					lowIndex = x;
				}
			}
			for(uint8_t x = 0; x < buf->size_buffer; x++)
			{
				if((x != lowIndex) && (x != highIndex))
				{
					bufTotal += (double)(buf->buffer[x]);
				}
			}
		}
		else
		{
			temp_cnt = 0;
			for(uint8_t x = 0; x < buf->size_buffer; x++)
			{
				bufTotal += (double)(buf->buffer[x]);
			}
		}
		return (float)(bufTotal / (buf->size_active - temp_cnt));
	}
	return 0;
}

void init_circ_buffer_float(circ_buffer_float* circ_buf, float* databuffer, uint8_t size_buf)
{
	circ_buf->size_active = 0;
	circ_buf->size_buffer = size_buf;
	circ_buf->buffer = databuffer;
}

void cb_append_double(circ_buffer_double* buf, double data)
{
	buf->buffer[buf->active] = 0;
	buf->buffer[buf->active] = data;
	buf->active = (buf->active + 1) % buf->size_buffer;
	if(buf->size_active < buf->size_buffer)
	{
		buf->size_active++;
	}
}

float cb_getAvg_double(circ_buffer_double* buf)
{
	double bufTotal = 0;
	uint8_t lowIndex = 0;
	uint8_t highIndex = 0;
	uint8_t temp_cnt = 0;
	if(buf->size_active != 0)
	{
		if(buf->size_active == buf->size_buffer)
		{
			temp_cnt = 2;
			for(uint8_t x = 0; x < buf->size_buffer; x++)
			{
				if(buf->buffer[x] > buf->buffer[highIndex])
				{
					highIndex = x;
				}
				else if(buf->buffer[x] < buf->buffer[lowIndex])
				{
					lowIndex = x;
				}
			}
			for(uint8_t x = 0; x < buf->size_buffer; x++)
			{
				if((x != lowIndex) && (x != highIndex))
				{
					bufTotal += (buf->buffer[x]);
				}
			}
		}
		else
		{
			temp_cnt = 0;
			for(uint8_t x = 0; x < buf->size_buffer; x++)
			{
				bufTotal += (double)(buf->buffer[x]);
			}
		}
		return (bufTotal / (buf->size_active - temp_cnt));
	}
	return 0;
}

void init_circ_buffer_double(circ_buffer_double* circ_buf, double* databuffer, uint8_t size_buf)
{
	circ_buf->size_active = 0;
	circ_buf->size_buffer = size_buf;
	circ_buf->buffer = databuffer;
}

void sw_32hz_timer_init()
{
	timer_32hz_ready = false;
	pwm_init(&conf_32hz_timer, PWM_TCE0, PWM_CH_B, 32);
	pwm_start(&conf_32hz_timer, 100);
	
	pwm_overflow_int_callback(&conf_32hz_timer, set_32hz_ready);
}


void set_32hz_read(void)
{
	mpl3_single_read(MPL3_MODE_ALT);
	//get_ntcle_temp();
}

void set_32hz_ready(void)
{
	timer_32hz_ready = true;
}

void clear_32hz_ready(void)
{
	timer_32hz_ready = false;
}

_Bool checK_32hz_ready(void)
{
	return timer_32hz_ready;
}

void sw_16hz_timer_init(void)
{
	timer_16hz_ready = false;
	pwm_init(&conf_16hz_timer, PWM_TCE0, PWM_CH_C, 16);
	pwm_start(&conf_16hz_timer, 100);
	pwm_overflow_int_callback(&conf_16hz_timer, set_16hz_ready);
}

void set_16hz_read(void)
{
	//do stuff here
}

void set_16hz_ready(void)
{
	timer_16hz_ready = true;
}

void clear_16hz_ready(void)
{
	timer_16hz_ready = false;
}

_Bool checK_16hz_ready(void)
{
	return timer_16hz_ready;
}

void vector3D_int_init(vector3D_int* data, int16_t value)
{
	data->x = value;
	data->y = value;
	data->z = value;
}

void vector3D_float_init(vector3D_float* data, float value)
{
	data->x = value;
	data->y = value;
	data->z = value;
}

void skywave_util_init(void)
{
	//TCC0.CTRLA = 0b00000001;
	//TCC0.CTRLB = 0b00000000;
	//TCC0.PER = 31999; //every 1 ms
	//TCC0.CCA = 31999;
	//milliseconds = 0;	
}

ISR(TCC0_CCA_vect)
{
	milliseconds += 1;
}

uint64_t millis(void)
{
	return milliseconds;
}

int ascii_to_decimal_int(const char* str)
{
	int numlen = strlen(str);
	_Bool bNeg = false;
	int y = numlen;
	int retVal = 0;
	for(uint8_t x = 0; x < numlen; x++)
	{
		if(!isdigit(str[x]))
		{
			if(str[x] == '-')
			{
				bNeg = true;
				--y;
			}
		}
		else
		{
			int temp = (int)str[x] - (int)'0';
			retVal += (temp * (int)(round(pow(10.0, --y))));
		}
		
	}
	if(bNeg)
	{
		return retVal * -1;
	}
	return retVal;
	
}

float ascii_to_decimal_float(const char* str)
{
	int numlen = strlen(str);
	int y = numlen;
	for(int x = 0; x < numlen; x++)
	{
		
	}
	return 0.f;
}

