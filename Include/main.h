/*
 * json.h
 *
 * Created: 8/28/2022 12:50:40 AM
 *  Author: hoang
 */ 


#ifndef MAIN_H_
#define MAIN_H_
#include "ql_gnss.h"
static char *GPS_info;
static ql_gnss_data_t g_gps_data;
static char imei[64];
#define NSIM 0
#define MQTT_CLIENT_USER "esp32-iot"
#define MQTT_CLIENT_PASS "thang123"
#define MQTT_CLIENT_SRV_URL "mqtt://103.200.20.78:1883"        // onenet
#define MQTT_CLIENT_ONENET_SSL_URL "mqtt://103.200.20.78:1883" // onenet SSL
#define topic_remote "EC200U_REMOTE"
#define topic_rec "EC200U_REC"

// APPLICATION
//#define SENSOR_LIS3DH
#define SMS_APP
#define VOICE_APP

#endif /* JSON_H_ */