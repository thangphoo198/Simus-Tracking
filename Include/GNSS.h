#ifndef __GPS_H__
#define __GPS_H__

#include <stdio.h>
#include <string.h>


#include "ql_api_osi.h"
#include "ql_api_dev.h"
#include "ql_gpio.h"



#define GPS_INIT_EVENT				QL_EVENT_APP_START + 21
#define GPS_TICK					QL_EVENT_APP_START + 22


#if (DEBUG_GPS == 1)
#define GNSS_RECV_BUFFERSIZE		850
#else
#define GNSS_RECV_BUFFERSIZE		1024
#endif

#define GPS_ERROR					255

#define LARGE_BUFFER_SIZE			1500
typedef struct
{
	uint8_t Buffer[LARGE_BUFFER_SIZE];
	uint16_t Index;
	uint8_t State;
} LargeBuffer_t;


typedef struct{
    uint32_t State;
    uint8_t NeedReset;
}GPS_Manager_t;

typedef struct {
	float Longitude;	// Kinh do
	float Latitude;	    // Vi do
    uint8_t Speed;		// Van toc GPS
    float Course;		// Huong di chuyen
    uint8_t SVCount;	// So ve tinh quan sat duoc
    uint8_t Error;
    uint8_t ErrorCnt;
    uint8_t Valid;		// Co valid hay khong
 
} GPSInfo_t;


uint8_t DecodeRMC(char *Buffer, GPSInfo_t *GPSInfo);
uint8_t DecodeGGA(char *Buffer, GPSInfo_t *GPSInfo);

extern void SendEventToThread(ql_task_t thread, uint32_t sig);


#endif // __GPS_H__

