/*
 * mpu9250.c
 *
 * Created: 5/7/2018 4:35:56 AM
 *  Author: Penguin
 */ 

//LATEST
#include "devices/mpu9250.h"
#include "drivers/twi_comms.h"
#include "drivers/MahonyAHRS.h"
#include <math.h>

#define MPU_GYRO_SCALE			GYRO_2000DPS
#define MPU_ACCEL_SCALE			ACCEL_16G
#define MPU_MAG_SCALE			MAG_16BITS
#define MPU_ADDR				MPU_ADDR_HIGH
#define MAG_CALIBRATION_SAMPLES 1500
#define MPU_SAMPLE_RATE			100

static _Bool b_mpu_data_ready;
static _Bool b_mag_calibrate;

volatile float accel_resolution;
volatile float gyro_resolution;
volatile float mag_resolution;

static uint8_t mpu_buffer[64];

static vector3D_float mag_sens;
int16_t gyro_sens = 131;
int16_t accel_sens = 16384;

volatile mpuData_int mpu_raw_data;
volatile mpuData_float mpu_processed_data;
volatile OrientationData rot;

static circ_buffer_float cb_accel_x;
static float accel_buff_x[10];

static circ_buffer_float cb_accel_y;
static float accel_buff_y[10];

static circ_buffer_float cb_accel_z;
static float accel_buff_z[10];

static circ_buffer_double cb_yaw;
static double yaw_buff[10];

static circ_buffer_double cb_pitch;
static double pitch_buff[10];

static circ_buffer_double cb_roll;
static double roll_buff[10];



void mpu_init(void)
{
	b_mag_calibrate = false;
	init_circ_buffer_double(&cb_yaw, yaw_buff, 10);
	init_circ_buffer_double(&cb_pitch, pitch_buff, 10);
	init_circ_buffer_double(&cb_roll, roll_buff, 10);
	
	init_circ_buffer_float(&cb_accel_x, accel_buff_x, 10);
	init_circ_buffer_float(&cb_accel_y, accel_buff_y, 10);
	init_circ_buffer_float(&cb_accel_z, accel_buff_z, 10);
	
	//reset imu
	twi_write(MPU_ADDR, PWR_MGMT_1, 0x80);
	delay_ms(100);
	twi_write(MPU_ADDR, PWR_MGMT_1, 0x01);
	twi_write(MPU_ADDR, PWR_MGMT_2, 0x00);
	
	delay_ms(100);
	//clock source config
	//enable gyro + accel
	//gyro init
	twi_write(MPU_ADDR, MPU_CONFIG, 0x02);		//gyro low pass filter config
	set_gyro_resolution(GYRO_2000DPS);
	//accel config
	set_accel_resolution(ACCEL_16G);
	twi_write(MPU_ADDR, ACCEL_CONFIG2, 0x02);	//accel low pass filter config
	//sample rate config
	mpu_set_sample_rate(MPU_SAMPLE_RATE);
	//mpu int config
	twi_write(MPU_ADDR, INT_PIN_CFG, 0x22);
	twi_write(MPU_ADDR, INT_ENABLE, 0x01);
	//not using mag
	//mpu_mag_wai();
	//mag init
	//mag_init();
	//int pin config on mcu
	PORTA.PIN2CTRL = PORT_OPC_PULLDOWN_gc | PORT_ISC_RISING_gc;
	PORTA.INT0MASK = PIN2_bm;
	PORTA.INTCTRL = PORT_INT0LVL_HI_gc;
}

void mag_init(void)
{
	set_mag_resolution(MPU_MAG_SCALE);
	twi_write(AK8963_ADDRESS, AK8963_CNTL, 0x00);
	delay_ms(10);
	twi_write(AK8963_ADDRESS, AK8963_CNTL, 0x0F);
	delay_ms(10);
	twi_read(AK8963_ADDRESS, AK8963_ASAX, 3, mpu_buffer);
	mag_sens.x = (float)((float)mpu_buffer[0] - 128)/256.f + 1.f;
	mag_sens.y = (float)((float)mpu_buffer[1] - 128)/256.f + 1.f;
	mag_sens.z = (float)((float)mpu_buffer[2] - 128)/256.f + 1.f;
	twi_write(AK8963_ADDRESS, AK8963_CNTL, 0x00);
	delay_ms(10);
	twi_write(AK8963_ADDRESS, AK8963_CNTL, (MPU_MAG_SCALE << 4) | 0x06);
	delay_ms(10);
}

void set_mpu_data_status(_Bool status)
{
	b_mpu_data_ready = status;
}

_Bool mpu_data_ready(void)
{
	return b_mpu_data_ready;
}

void mpu_set_sample_rate(uint8_t smplrt)
{
	uint8_t temp = ((uint8_t)((1000/((uint16_t)smplrt)) - 1));
	twi_write(MPU_ADDR, SMPLRT_DIV, temp);
}

void mpu_read(void)
{
	twi_read(MPU_ADDR, INT_STATUS, 15, mpu_buffer);
	if(mpu_buffer[0] & 0x01)
	{
		mpu_raw_data.accel.x = (((int16_t)mpu_buffer[1] << 8) | mpu_buffer[2]);
		mpu_raw_data.accel.y = (((int16_t)mpu_buffer[3] << 8) | mpu_buffer[4]);
		mpu_raw_data.accel.z = (((int16_t)mpu_buffer[5] << 8) | mpu_buffer[6]);
		
		mpu_raw_data.gyro.x = (((int16_t)mpu_buffer[9] << 8) | mpu_buffer[10]);
		mpu_raw_data.gyro.y = (((int16_t)mpu_buffer[11] << 8) | mpu_buffer[12]);
		mpu_raw_data.gyro.z = (((int16_t)mpu_buffer[13] << 8) | mpu_buffer[14]);
		
		//not using mag because it only provides one value then stops updating
		//twi_read(AK8963_ADDRESS, AK8963_XOUT_L, 6, mpu_buffer);
		//mpu_raw_data.mag.x = ((int16_t)mpu_buffer[1] << 8 | mpu_buffer[0]);
		//mpu_raw_data.mag.y = ((int16_t)mpu_buffer[3] << 8 | mpu_buffer[2]);
		//mpu_raw_data.mag.z = ((int16_t)mpu_buffer[5] << 8 | mpu_buffer[4]);

		mpu_processed_data.accel.x = (float)(((float)mpu_raw_data.accel.x)*accel_resolution);
		mpu_processed_data.accel.y = (float)(((float)mpu_raw_data.accel.y)*accel_resolution);
		mpu_processed_data.accel.z = (float)(((float)mpu_raw_data.accel.z)*accel_resolution);;
			
		mpu_processed_data.gyro.x = (float)(((float)mpu_raw_data.gyro.x)*gyro_resolution);
		mpu_processed_data.gyro.y = (float)(((float)mpu_raw_data.gyro.y)*gyro_resolution);
		mpu_processed_data.gyro.z = (float)(((float)mpu_raw_data.gyro.z)*gyro_resolution);
		
		//not using mag because it only provides one value then stops updating
		mpu_processed_data.mag.x = 0.0; //(float)(((float)mpu_raw_data.mag.x)*mag_sens.x);
		mpu_processed_data.mag.y = 0.0; //(float)(((float)mpu_raw_data.mag.y)*mag_sens.y);
		mpu_processed_data.mag.z = 0.0; //(float)(((float)mpu_raw_data.mag.z)*mag_sens.z);
		swprintf(SWDEBUG, "%-2.2f\t%-2.2f\t%-2.2f\n", mpu_processed_data.accel.x, mpu_processed_data.accel.y, mpu_processed_data.accel.z);
		cb_append_float(&cb_accel_x, mpu_processed_data.accel.x);
		cb_append_float(&cb_accel_y, mpu_processed_data.accel.y);
		cb_append_float(&cb_accel_z, mpu_processed_data.accel.z);
		
		mpu_filter_data((const mpuData_float*)(&mpu_processed_data));
		twi_read(MPU_ADDR, INT_STATUS, 1, mpu_buffer);
	}
}

void mpu_get_accel_data(vector3D_float* data)
{
	data->x = cb_getAvg_float(&cb_accel_x);
	data->y = cb_getAvg_float(&cb_accel_y);
	data->z = cb_getAvg_float(&cb_accel_z);
}

void mpu_get_rot_data(OrientationData* data)
{
	data->yaw = cb_getAvg_double(&cb_yaw);
	data->pitch = cb_getAvg_double(&cb_pitch);
	data->roll = cb_getAvg_double(&cb_roll);
}

void mpu_filter_data(const mpuData_float* data)
{
	//God bless Thomas AND God bless the United States of America
	MahonyAHRSupdate((double)data->gyro.x,
					(double)data->gyro.y,
					(double)data->gyro.z,
					(double)data->accel.x,
					(double)data->accel.y,
					(double)data->accel.z,
					(double)data->mag.x,
					(double)data->mag.y,
					(double)-data->mag.z);
	rot.yaw   = atan2(2.0 * (q1 * q2 + q0 * q3), q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3);
	rot.pitch = -asin(2.0 * (q1 * q3 - q0 * q2));
	rot.roll  = atan2(2.0 * (q0 * q1 + q2 * q3), q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3);
	rot.pitch *= 180.0 / PI;
	rot.yaw   *= 180.0 / PI;
	rot.roll  *= 180.0 / PI;
	cb_append_double(&cb_yaw, rot.yaw);
	cb_append_double(&cb_pitch, rot.pitch);
	cb_append_double(&cb_roll, rot.roll);
	
}

void mpu_wai(void)
{
	twi_read(MPU_ADDR, WHO_AM_I_MPU9250, 1, mpu_buffer);
	if(mpu_buffer[0] == 0x71)
	{
		swprintf(SWDEBUG, "MPU INITIALIZED\n");
	}
}

void mpu_mag_wai(void)
{
	twi_read(AK8963_ADDRESS, AK8963_WHO_AM_I, 1, mpu_buffer);
	if(mpu_buffer[0] == 0x48)
	{
		swprintf(SWDEBUG, "MAG FOUND\n");
	}
}

void mpu_mag_hard_calibrate(const vector3D_int* magData)
{
	//
}

void set_accel_resolution(accelScale scale)
{
	switch(scale)
	{
		case ACCEL_2G:
			accel_resolution = (float)(2.0/32768.0);
			break;
		case ACCEL_4G:
			accel_resolution = (float)(4.0/32768.0);
			break;
		case ACCEL_8G:
			accel_resolution = (float)(8.0/32768.0);
			break;
		case ACCEL_16G:
			accel_resolution = (float)(16.0/32768.0);
			break;
		default:
			break;
	};
	twi_write(MPU_ADDR, ACCEL_CONFIG, (MPU_ACCEL_SCALE << 3));
}

void set_mag_resolution(magScale scale)
{
	switch(scale)
	{
		case MAG_14BITS:
			mag_resolution = (float)(10.f * 4912.f / 8190.f);
			break;
		case MAG_16BITS:
			mag_resolution = (float)(10.f * 4912.f / 32760.f);
			break;
	}
}

void set_gyro_resolution(gyroScale scale)
{
	switch(scale)
	{
		case GYRO_250DPS:
			gyro_resolution = (float)(250.0/32760.0);
			break;
		case GYRO_500DPS:
			gyro_resolution = (float)(500.0/32760.0);
			break;
		case GYRO_1000DPS:
			gyro_resolution = (float)(1000.0/32760.0);
			break;
		case GYRO_2000DPS:
			gyro_resolution = (float)(2000.0/32768.0);
			break;
		default:
			break;
	}
	twi_write(MPU_ADDR, GYRO_CONFIG, (MPU_GYRO_SCALE << 3));
}

ISR(PORTA_INT0_vect)
{
	set_mpu_data_status(true);
}