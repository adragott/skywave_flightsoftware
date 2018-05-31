/*
 * gps.h
 *
 * Created: 5/28/2018 5:08:06 AM
 *  Author: Penguin
 */ 
#include "skywave.h"
#include "register_map.h"

//EXPAND FOR NMEA DOCUMENTATION
#ifndef GPS_DATA_INFO
#define GPS_DATA_INFO
//GPS Information Source: http://aprs.gids.nl/nmea/

//Global Positioning System Fix Data
//$GPGGA
//1. UTC of Position, aka Time
//2. Latitude
//3. North or South
//4. Longitude
//5. East or West
//6. GPS Quality Indicator: (0 = invalid, 1 = GPS Fix, 2 = Dif. GPS Fix)
//7. Number of GPS satellites in use
//8. Horizontal dilution of position (relative accuracy of horizontal position)
//9. Altitude above/below mean sea level
//10. Meters (This is simply the units of the measurement given in #9)
//11. Geoidal separation (Diff. between WGS-84 earth ellipsoid and mean sea level.  -=geoid is below WGS-84 ellipsoid) --- Thinking this just means difference in mean sea level and current altitude?
//12. Meters (Units for #11)
//13. Age in seconds since last update from diff. reference station
//14. Diff. Reference station ID#
//15. Checksum
//
//        <  Time,  Latitude, N/S,  Longitude, E/W, GPS Quality, Num. Satellites, Rel. Accuracy,  Alt., Units, Alt Diff., Units, DeltaT, Station ID, Checksum>
//Format: <HHMMSS, 3051.8007,   N, 10035.9989,   W,           1,               4,          2.18, 746.4,     M,     -22.2,     M,       ,           ,      *6B>


//Recommended minimum specific GPS/Transit data
//$GPRMC
//1. UTC of position fix, aka time
//2. Data Status (V = navigation receiver warning)
//3. Latitude of fix
//4. N or S
//5. Longitude of fix
//6. E or W
//7. Speed over grount in knots
//8. Track made good in degrees true ??
//9. UT date
//10. Magnetic variation degrees
//11. E or W
//12. Checksum

//Format: <194530.000,A,3051.8007,N,10035.9989,W,1.49,111.67,310714,,,A*74
#endif


#ifndef _GPS_H_
#define _GPS_H_

#define GPS_PKT_ID 0

void gps_init(void);

typedef enum  
{
	GPS_GPGGA = 0,
	GPS_GPRMC = 1,
	GPS_VACANT = 2
}GPS_PKT_TYPE;

typedef enum
{
	GPS_GPGGA_TIME =			1,
	GPS_GPGGA_LAT =				2,
	GPS_GPGGA_LAT_DIR =			3, //N/S
	GPS_GPGGA_LONG =			4,
	GPS_GPGGA_LONG_DIR =		5, //E/W
	GPS_GPGGA_QUALITY =			6,
	GPS_GPGGA_NUM_SATS =		7,
	GPS_GPGGA_REL_ACCURACY =	8,
	GPS_GPGGA_ALT =				9,
	GPS_GPGGA_ALT_UNITS =		10,
	GPS_GPGGA_GEO_SEP =			11,
	GPS_GPGGA_GEO_SEP_UNITS =	12,
	GPS_GPGGA_DELTA_TIME =		13,
	GPS_GPGGA_STATION_ID =		14,
	GPS_GPGGA_CHECKSUM =		15
}GPS_GPGGA_STRUCTURE;

typedef enum
{
	GPS_GPRMC_TIME =			1,
	GPS_GPRMC_DATA_STATUS =		2,
	GPS_GPRMC_LAT =				3,
	GPS_GPRMC_LAT_DIR =			4,
	GPS_GPRMC_LONG =			5,
	GPS_GPRMC_LONG_DIR =		6,
	GPS_GPRMC_SPEED =			7,
	GPS_GPRMC_TRACK_GOOD_DEG =	8, //I have no idea what this part of the NMEA sentence means
	GPS_GPRMC_DATE =			9,
	GPS_GPRMC_MAG_VARIATION =	10,
	GPS_GPRMC_MAG_VAR_DIR =		11,
	GPS_GPRMC_CHECKSUM =		12
}GPS_GPRMC_STRUCTURE;

typedef struct
{
	char* gps_str;
	
	_Bool b_parsed;
	GPS_PKT_TYPE pkt_type;
}gpsData;

typedef struct  
{
	gpsData* dataBuff;
	uint8_t current;
	uint8_t bufsize;
	uint8_t sizeActive;
}gpsCircBuff;

typedef struct  
{
	char* buff;
	uint8_t current;
	uint8_t bufsize;
	uint8_t sizeActive;
}charCircBuff;

void parseLine(const char* txt, char split);
void gps_init(void);
void ccb_init(charCircBuff* ccb, char* buff, uint16_t size);
void ccb_append(charCircBuff* ccb, char val);

void gcb_init(gpsCircBuff* gcb, gpsData* buff, uint8_t size);
void gcb_append(gpsCircBuff* gcb, gpsData* data);

void gps_data_init(gpsData* gps_data);
#endif