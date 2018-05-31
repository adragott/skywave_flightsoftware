/*
 * mpl3115a2.h
 *
 * Created: 5/10/2018 8:37:55 AM
 *  Author: Penguin
 */ 
#include "skywave.h"
#include "drivers/twi_comms.h"
#ifndef _MPL3115A2_H_
#define _MPL3115A2_H_

typedef enum
{
	MPL3_MODE_ALT,
	MPL3_MODE_PRES
}mpl3_read_mode;


typedef struct
{
	uint8_t status;
	float pressure;
	float temp;
	int samplect;
}mpl3_Data;

void mpl3_init(void);

//main data return function
_Bool calc_mpl3_data(void);

void get_mpl3_data(mpl3_Data* mpl3Data, _Bool bShouldResetData);

//interrupt handling functions
void mpl3_clear_data_flag(void);
void mpl3_set_data_flag(void);
_Bool mpl3_get_data_status(void);

//read data from sensor once
void mpl3_single_read(mpl3_read_mode read_mode);

//Urgent Read for misc purposes
float get_altitude_now(void);
void mpl3_setInitialAltitude(mpl3_Data* groundAlt);

_Bool mpl3_checkbInitialAltitudeSet(void);
#endif