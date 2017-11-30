/*
 ============================================================================
 Name        : test_c_code.c
 Author      : Khomin
 Version     :
 Copyright   : Your copyright notice
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#define NMEA_MESSAGE_MAX		5
#define NMEA_MESSAGE_MAX_LEN	150

typedef enum {
	GGA__,
	GSA__,
	GSV__,
	VTG__,
	RMC__,
	UNKNOWN__=-1
}eGnssNmeaMsgType;

typedef struct {
	char header[15];
	eGnssNmeaMsgType type;
	char format[NMEA_MESSAGE_MAX_LEN];
}sNmeaParcer;

sNmeaParcer nmeaParcer[NMEA_MESSAGE_MAX] = {
		{"GGA", GGA__, "%2d%2d%2d.%*3d,%f,%c,%f,%c,%d,%d,%f,%f,%c"},
		{"GSA", GSA__, "%c,%i,%*i,%*i,%*i,%*i,%*i,%*i,%*i,%*i,%*i,%*i,%*i,%*i,%f,%f,%f"},
		{"GSV", GSV__, "%3d,%3d,%3d,%3d,%f,%f,%i"},
		{"VTG", VTG__, "%f,%c,%*d,%*c,%f"},
		{"RMC", RMC__, "%2d%2d%2d.%*3d,%*c,%f,%c,%f,%c,%f,%f,%2d%2d%2d"}
};

bool gpsParceNmeaRaw(char *pData);

int main(void) {
	int i = 0;
	char nmea_buffer[NMEA_MESSAGE_MAX][NMEA_MESSAGE_MAX_LEN] = {
			{"$GNGGA,094650.000,5548.6477,N,03750.0997,E,1,11,1.00,274.4,M,14.3,M,,*76"},
			{"$GPGSA,A,3,03,17,19,12,14,01,32,11,22,,,,1.35,1.00,0.91*07"},
			{"$BDGSV,1,1,04,09,41,070,28,12,36,240,,11,31,308,31,17,10,072,*64"},
			{"$GNVTG,190.77,T,,M,15.8,N,0.00,K,A*2B"},
			{"$GNRMC,093005.000,A,5548.6223,N,03750.1092,E,0.4,0.6,291117,0,0,A*7C"}
	};

	printf("NMEA: startParcer\r\n");

	while(i<NMEA_MESSAGE_MAX) {
		printf("NMEA: message -%s\r\n", nmea_buffer[i]);
		gpsParceNmeaRaw((char*)nmea_buffer[i]);
		i++;
	}

	printf("NMEA: END\r\n");

	return EXIT_SUCCESS;
}

bool gpsParceNmeaRaw(char *pData) {
	bool result = false;
	uint8_t msgType = 0;
	char data[NMEA_MESSAGE_MAX_LEN] = {0};
	uint16_t len = 0;
	//
	int dd = 0;
	float ss = 0;
	char mode_c = 'A';	// automatic
	int mode_int = 1; // no fix
	float vdop, hdop, pdop;
	struct tm timeStruct;
	float lat, lon, height;
	char lat_sign, lon_sign, height_units;
	int precision, satellites;
	int message_counter = 0;
	int message_current = 0;
	int total_sv_view = 0;
	int sv_prn_number = 0;
	float elevation_degrees = 0;
	float azimuth_degrees = 0;
	int snr_db_value = 0;
	//
	float track_made_good = 0;
	char fixet_text_T = 0;
	//
	float speed = 0;
	float course_over_ground = 0;
	char *pChar = 0;
	len = strlen(pData);
	if(len == 0) {
		return false;
	}

	uint16_t index = 0;
	uint16_t repIndex = 0;
	while(index < len) {
		if(repIndex > 0) {
			if(pData[index] == ',') {
				if(data[repIndex-1] == ',') {
					data[repIndex++] = '0';
				}
			}
		}
		data[repIndex++] = pData[index];
		index++;
	}
	pData = data;

	for(msgType = 0; msgType < NMEA_MESSAGE_MAX; msgType++) {
		if(strstr (pData, (char*)nmeaParcer[msgType].header) != NULL) {
			printf("Header found [%s]\r\n", nmeaParcer[msgType].header);

			pChar = strchr(pData, ',');
			if(pChar != NULL) {

				switch(nmeaParcer[msgType].type) {
				case GGA__: //	$GNGGA,094650.000,5548.6477,N,03750.0997,E,1,11,1.00,274.4,M,14.3,M,,*76
					sscanf(pChar+1, nmeaParcer[msgType].format,
							&timeStruct.tm_hour, &timeStruct.tm_min, &timeStruct.tm_sec,
							&lat, &lat_sign, &lon, &lon_sign, &precision, &satellites, &hdop, &height, &height_units);
					dd = (int)((float)(lat)/100); //37
					ss = (float)(lat) - (dd * 100); // 50,1122
					lat = dd + (ss/60); //31.6227773333333333
					if(lat != 0) {
						if (lat_sign == 'S') {
							lat = (0-lat);
						}
					}
					dd = (int)((float)(lon)/100); //37
					ss = (float)(lon) - (dd * 100); // 50,1122
					lon = dd + (ss/60); //31.6227773333333333
					if(lon != 0) {
						if (lon_sign=='S') {
							lon = (0-lon);
						}
					}
					printf("NMEA: parcing raw: %d,%d,%d,%f,%c,%f,%c,%d,%d,%f,%f,%d\r\n",
							timeStruct.tm_hour, timeStruct.tm_min, timeStruct.tm_sec,
							lat, lat_sign, lon, lon_sign, precision, satellites, hdop, height, height_units);
					return true;
					break;
				case GSA__ : //	$GPGSA,A,3,03,17,19,12,14,01,32,11,22,,,,1.35,1.00,0.91*07
					sscanf(pChar+1, nmeaParcer[msgType].format, &mode_c, &mode_int,
							&pdop, &hdop, &vdop);
					printf("NMEA: parcing raw: %c,%i,%f,%f,%f\r\n", mode_c, mode_int, pdop, hdop, vdop);
					return true;
					break;
				case GSV__ : //	$BDGSV,1,1,04,09,41,070,28,12,36,240,,11,31,308,31,17,10,072,*64
					sscanf(pChar+1, nmeaParcer[msgType].format,
							&message_counter, &message_current, &total_sv_view, &sv_prn_number,
							&elevation_degrees, &azimuth_degrees, &snr_db_value);
					printf("NMEA: parcing raw: %i,%i,%i,%i,%f,%f,%i\r\n", message_counter, message_current, total_sv_view, sv_prn_number, elevation_degrees, azimuth_degrees, snr_db_value);
					return true;
					break;
				case VTG__ : //	$GNVTG,190.77,T,,M,15.12,N,0.00,K,A*2B
					sscanf(pChar+1, nmeaParcer[msgType].format, &track_made_good, &fixet_text_T, &speed); //!!!
					printf("NMEA: parcing raw: %f,%c,%f\r\n", track_made_good, fixet_text_T, speed);
					return true;
					break;
				case RMC__: //	$GNRMC,093005.000,A,5548.6223,N,03750.1092,E,0.4,0.46,291117,0,0,A*7C
					sscanf(pChar+1, nmeaParcer[msgType].format,
							&timeStruct.tm_hour, &timeStruct.tm_min, &timeStruct.tm_sec,
							&lat, &lat_sign, &lon, &lon_sign, &speed, &course_over_ground,
							&timeStruct.tm_mday, &timeStruct.tm_mon, &timeStruct.tm_year);
					printf("NMEA: parcing raw: %d.%d.%d %f %f\r\n",
							timeStruct.tm_mday, timeStruct.tm_mon, timeStruct.tm_year,
							speed, course_over_ground);
					return true;
					break;
				case UNKNOWN__ :
					break;
				default : break;
				}
			}
		}
	}
	return result;
}
