/*
 ============================================================================
 Name        : test_c_code.c
 Author      : Khomin
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#define NMEA_MESSAGE_MAX		4
#define NMEA_MESSAGE_MAX_LEN	150

typedef enum {
	_GGA,
	_GSA,
	_GSV,
	_VTG,
	_UNKNOWN=-1
}eGnssNmeaMsgType;

typedef struct {
	char header[15];
	eGnssNmeaMsgType type;
	char format[NMEA_MESSAGE_MAX_LEN];
}sNmeaParcer;

sNmeaParcer nmeaParcer[NMEA_MESSAGE_MAX] = {
		{"GGA", _GGA, "%2d%2d%2d.%3d,%f,%c,%f,%c,%d,%d,%f,%f,%c"},
		{"GSA", _GSA, "%c,%d, %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"}, // &mode_c, &mode_int, &pdop, &hdop, &vdop
		{"GSV", _GSV, "%i,%i,%i,%i,%f,%f,%i"}, // &message_counter, &message_current, &total_sv_view, &sv_prn_number, &elevation_degrees, &azimuth_degrees, &snr_db_value
		{"VTG", _VTG, "%f,%c,%...,%...,%i"} // &track_made_good, &fixet_text_T, &speed
};

bool gpsParceNmeaRaw(char *pData);

int main(void) {
	int i = 0;
	char nmea_buffer[NMEA_MESSAGE_MAX][NMEA_MESSAGE_MAX_LEN] = {
			{"$GNGGA,094650.000,5548.6477,N,03750.0997,E,1,11,1.00,274.4,M,14.3,M,,*76"},
			{"$GPGSA,A,3,03,17,19,12,14,01,32,11,22,,,,1.35,1.00,0.91*07"},
			{"$BDGSV,1,1,04,09,41,070,28,12,36,240,,11,31,308,31,17,10,072,*64"},
			{"$GNVTG,190.77,T,,M,0.00,N,0.00,K,A*2B"}
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
	//
	uint32_t timestamp = 0;
	char mode_c = 'A';	// automatic
	int mode_int = 1; // no fix
	float vdop, hdop, pdop;
	struct tm timeStruct;
	float lat, lon, lat_seconds, lon_seconds, HDOP, height;
	char lat_sign, lon_sign, height_units;
	int precision, satellites;
	int idle_int = 0;
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
	int speed = 0;
	char *pChar = 0;

	for(msgType = 0; msgType < NMEA_MESSAGE_MAX; msgType++) {
		if(strstr (pData, (char*)nmeaParcer[msgType].header) != NULL) {
			printf("Header found [%s]\r\n", nmeaParcer[msgType].header);

			pChar = strchr(pData, ',');
			if(pChar != NULL) {

				switch(nmeaParcer[msgType].type) {
				case _GGA: // {"$GNGGA,094650.000,5548.6477,N,03750.0997,E,1,11,1.00,274.4,M,14.3,M,,*76"}, //,%d,%d,%f,%f,%c
					sscanf(pChar+1, nmeaParcer[msgType].format, &timeStruct.tm_hour, &timeStruct.tm_min, &timeStruct.tm_sec, &idle_int,
							&lat, &lat_sign, &lon, &lon_sign, &precision, &satellites, &HDOP, &height, &height_units);
					printf("NMEA: parcing raw: %d,%d,%d,%c,%f,%c,%d,%d,%f,%f,%d\r\n", timeStruct.tm_hour, timeStruct.tm_min, timeStruct.tm_sec,
							lat_sign, lon, lon_sign, precision, satellites, HDOP, height, height_units);
					return true;
					break;
				case _GSA : // $GPGSA,A,3,03,17,19,12,14,01,32,11,22,,,,1.35,1.00,0.91*07
					sscanf(pChar+1, nmeaParcer[msgType].format, &mode_c, &mode_int,
							&pdop, &hdop, &vdop);
					printf("NMEA: parcing raw: %c,%i,%f,%f,%f\r\n", mode_c, mode_int, pdop, hdop, vdop);
					return true;
					break;
				case _GSV :
					sscanf(pChar+1, "%c,%i, %i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i, %i,%i,%i", &mode_c, &mode_int,
							&idle_int,&idle_int,&idle_int,&idle_int,&idle_int,&idle_int,&idle_int,&idle_int,&idle_int,&idle_int,&idle_int,&idle_int,
							&pdop, &hdop, &vdop);
					printf("NMEA: parcing raw: %c,%d,%d,%d,%d\r\n", mode_c, mode_int, pdop, hdop, vdop);
					return true;
					break;
				case _VTG :
					sscanf(pChar+1, "%c,%i, %i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i, %i,%i,%i", &mode_c, &mode_int,
							&idle_int,&idle_int,&idle_int,&idle_int,&idle_int,&idle_int,&idle_int,&idle_int,&idle_int,&idle_int,&idle_int,&idle_int,
							&pdop, &hdop, &vdop);
					printf("NMEA: parcing raw: %c,%d,%d,%d,%d\r\n", mode_c, mode_int, pdop, hdop, vdop);
					return true;
					break;
				case _UNKNOWN :
					sscanf(pChar+1, "%c,%i, %i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i, %i,%i,%i", &mode_c, &mode_int,
							&idle_int,&idle_int,&idle_int,&idle_int,&idle_int,&idle_int,&idle_int,&idle_int,&idle_int,&idle_int,&idle_int,&idle_int,
							&pdop, &hdop, &vdop);
					printf("NMEA: parcing raw: %c,%d,%d,%d,%d\r\n", mode_c, mode_int, pdop, hdop, vdop);
					return true;
					break;
				default : break;
				}
			}
		}
	}

	//	msgType = gnssGetNmeaMgsType(pData, size);
	//	if(_UNKNOWN != msgType) {
	//		gpsTakeNmeaValues(&gnssNmeaMask[msgType], &gnssDataStruct, pData, size);
	//		if (msgType == _GSA) {
	//			gnssDataStruct.gnssType = gnssGetGnssType(pData, size);
	//		}
	//		if(gnssDataStruct.sats >= 3) {
	//			 gnssLastFixData = gnssDataStruct;
	//#ifdef DEBUG_NMEA
	//			 Ol_sprintf(tGpsBuff, "GNSS: lat-%4.8f lon-%4.8f\0", gnssDataStruct.latitude, gnssDataStruct.longitude);
	//			 DBGLog(tGpsBuff);
	//#endif
	//		}
	//		DBGLog("NNEAL %s", pData);
	//	}
	return result;
}
