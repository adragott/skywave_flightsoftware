/*
 * mpu9250.h
 *
 * Created: 5/7/2018 4:35:48 AM
 *  Author: Penguin
 */ 
#include "skywave.h"
#include "register_map.h"
#include "drivers/skywave_util.h"
#ifndef _MPU9250_H_
#define _MPU9250_H_


typedef enum
{
	GYRO_250DPS = 0,
	GYRO_500DPS = 1,
	GYRO_1000DPS = 2,
	GYRO_2000DPS = 3
}gyroScale;

typedef enum
{
	ACCEL_2G = 0,
	ACCEL_4G = 1,
	ACCEL_8G = 2,
	ACCEL_16G = 3
}accelScale;

typedef enum
{
	MAG_14BITS = 0,
	MAG_16BITS = 1
}magScale;

typedef struct  
{
	vector3D_int accel;
	vector3D_int gyro;
	vector3D_int mag;
}mpuData_int;

typedef struct 
{
	vector3D_float accel;
	vector3D_float gyro;
	vector3D_float mag;
}mpuData_float;

typedef mpuData_float imuData;

typedef struct  
{
	double yaw;
	double pitch;
	double roll;
}OrientationData;

_Bool mpu_data_ready(void);
void set_mpu_data_status(_Bool status);

void mpu_set_sample_rate(uint8_t smplrt);
void mpu_read(void);
void mpu_get_accel_data(vector3D_float* data);
void mpu_get_rot_data(OrientationData* data);
void mpu_filter_data(const mpuData_float* data);

void mpu_wai(void);
void mpu_init(void);

void mag_init(void);
void mpu_mag_wai(void);
void mpu_mag_hard_calibrate(const vector3D_int* magData);
void mpu_mag_cal(void);

void set_gyro_resolution(gyroScale scale);
void set_accel_resolution(accelScale scale);
void set_mag_resolution(magScale scale);

#endif