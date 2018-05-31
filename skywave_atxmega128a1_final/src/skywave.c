
#include "skywave.h"

#include "drivers/usart_comms.h"
#include "drivers/adc_util.h"

#include "devices/mpu9250.h"
#include "devices/ds3231.h"
#include "devices/mpl3115a2.h"
#include "devices/ntcle100.h"
#include "devices/gps.h"

#include "drivers/skywave_util.h"

#include <math.h>
typedef struct
{
	mpl3_Data mpl3_data;
	ntcle100_Data ntcle_data;
	vector3D_float accel_data;
	OrientationData rot_data;
	gpsData gps_data;
	sw_time mission_time;
}swDataBank;

static SW_FLIGHT_STAGE sw_state_manager;
static uint8_t redun_ctr = 0;
static swDataBank missionData;
static _Bool bSensorsReady(void);

int main(void)
{
	skywave_init();
	//I dedicate this infinite for loop to Chandler, who was traumatized by an infinite while loop at a young age
	for(;;)
	{
		wdt_reset();
		if(checK_32hz_ready())
		{
			set_32hz_read();
		}
		
		if(mpl3_get_data_status())
		{
			calc_mpl3_data();
		}
		if(mpu_data_ready())
		{
			mpu_read();
			set_mpu_data_status(false);
		}
		//1 second timer fires
		if(ds3231_is_ready())
		{
			swprintf(SWDEBUG, "\n");
			get_mpl3_data(&missionData.mpl3_data, true);
			get_ntcle_data(&missionData.ntcle_data, true);
			mpu_get_rot_data(&missionData.rot_data);
			mpu_get_accel_data(&missionData.accel_data);
			swprintf(SWDEBUG, "ACCEL X: %-2.2f\tACCEL Y: %-2.2f\tACCEL Z: %-2.2f\n", missionData.accel_data.x, 
			missionData.accel_data.y, missionData.accel_data.z);
			swprintf(SWDEBUG, "YAW: %-2.2f\tPitch: %-2.2f\tRoll: %-2.2f\n", (float)missionData.rot_data.yaw, 
			(float)missionData.rot_data.pitch, (float)missionData.rot_data.roll);
			swprintf(SWDEBUG, "Altitude: %-2.2f\nDigital Temp: %-2.2f\n", missionData.mpl3_data.pressure, missionData.mpl3_data.temp);
			ds3231_get_time(&missionData.mission_time);
			swprintf(SWDEBUG, "Time: HH:MM:SS PM/AM\n%-2.2d:%-2.2d:%-2.2d %s\n",
			missionData.mission_time.hour, missionData.mission_time.minute, missionData.mission_time.second,
			(missionData.mission_time.pm_or_am ? "PM" : "AM"));
			swprintf(SWDEBUG, "TEMP: %-2.2f\n", get_ntcle_temp_now());
			ds3231_clear_ready();
		}
	}
	
}

void recovery_handler(void)
{
	//get data from openlogger
	//parse
	//if cold start flag is marked then just start normally
	//else we need to check some things
	//if time is more than 5 minutes apart from current time or if flight state was standby and 
	////ground altitude readings are similar then we cold start because it doesnt really matter
	//
	//else warm start
		//use old ground altitude
		//use old packet count
		//find new flight state depending on what old one was
		//
}

void change_flight_state(void)
{
	switch(sw_state_manager)
	{
		case SW_STANDBY:
			sw_fs_standby_init();
			break;
		case SW_ASCENT: 
			sw_fs_ascent_init();
			break;
		case SW_FIRST_DESCENT:
			sw_fs_first_descent_init();
			break;
		case SW_SEC_DESCENT:
			sw_fs_sec_descent_init();
			break;
		case SW_FINAL:
			sw_fs_final_init();
			break;
		default:
			break;
	};
}
//
void set_ntcle_off(float t_off)
{
	temp_off = (get_ntcle_temp_now() - t_off);
}

void skywave_init(void)
{
	sw_state_manager = SW_STANDBY;
	sensor_ready_status = 0b11100000;
	//initialize all function pointers to avoid oopsies
	flight_handler = NULL;
	swprintf = NULL;
	//chip init
	board_init();
	sysclk_init();
	wdt_set_timeout_period(WDT_2S);
	
	//pin config
	//mpu addr pin

	//xbee
	PORTC.DIR |= 0b00001000;
	PORTC.OUT |= 0b00001000;
	//openlog
	PORTF.DIR |= 0b00001000;
	PORTF.OUT |= 0b00001000;

	sysclk_enable_peripheral_clock(TWI_MASTER);
	sysclk_enable_peripheral_clock(USART_XBEE);
	sysclk_enable_peripheral_clock(USART_GPS);
	sysclk_enable_peripheral_clock(USART_OPENLOGGER);
	sysclk_enable_peripheral_clock(&ADCA);
	//1 ms timer
	sysclk_enable_module(SYSCLK_PORT_C, SYSCLK_TC0);
	
	
	
	
	PORTF.PIN0CTRL = PORT_OPC_WIREDANDPULL_gc;
	PORTF.PIN1CTRL = PORT_OPC_WIREDANDPULL_gc;
	//initialize drivers
	usart_comms_init();
	twi_comms_init();
	adc_util_init();
	
	//enable interrupts at all levels
	irq_initialize_vectors();
	cpu_irq_enable();
	
	wdt_enable();
	//initialize device drivers
	ds3231_init();
	mpl3_init();
	mpu_wai();
	mpu_init();
	ntcle100_init();
	gps_init();
	sw_32hz_timer_init();
}

_Bool bSensorsReady()
{
	if(sensor_ready_status == 0xFF)
	{
		return true;
	}
	return false;
}


void sw_fs_standby_handler(void)
{
	//check if accel z axis is going crazy && altitude is rising (+3 redun)
}


void sw_fs_ascent_handler(void)
{
	
}
void sw_fs_first_descent_handler(void)
{
	
}
void sw_fs_sec_descent_handler(void)
{
	
}
void sw_fs_final_handler(void)
{
	
}

void sw_fs_standby_init(void)
{
	//check sd card for previous flight data
	//find out if we're recovering or not
	//enable mpu and mpl3
	//set redun counter to 0
	redun_ctr = 0;
	flight_handler = &sw_fs_standby_handler;
}
void sw_fs_ascent_init(void)
{
	//enable therm
	//start calculating pressure
	//enable gps
	redun_ctr = 0;
	flight_handler = &sw_fs_ascent_handler;
}
void sw_fs_first_descent_init(void)
{
	
	redun_ctr = 0;
	flight_handler = &sw_fs_first_descent_handler;
}

void sw_fs_sec_descent_init(void)
{
	//heat shield release
	//wait 2 seconds
	//deploy shoot
	redun_ctr = 0;
	flight_handler = &sw_fs_sec_descent_handler;
}

void sw_fs_final_init(void)
{
	//enable buzzer
	redun_ctr = 0;
	flight_handler = &sw_fs_final_handler;
}