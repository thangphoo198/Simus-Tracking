#ifndef MAIN_H_
#define MAIN_H_
#include "ql_gnss.h"
char *GPS_info,*SIM_info;
static ql_gnss_data_t g_gps_data;
static uint8_t gps_status=0;
#define NSIM 0
#define MQTT_CLIENT_USER "esp32-iot"
#define MQTT_CLIENT_PASS "thang123"
#define MQTT_CLIENT_SRV_URL "mqtt://103.200.20.78:1883"        // onenet
#define MQTT_CLIENT_ONENET_SSL_URL "mqtt://103.200.20.78:1883" // onenet SSL
#define topic_remote "/EC200U_REMOTE"
#define topic_rec "/EC200U_REC"
#define UART_DEBUG QL_USB_PORT_MODEM
#define GPSOK "GPS_OK"
#define GPSFAIL "GPS_FAIL"

#define ACC_IN GPIO_25//WAKEUP
//#define ACC_IN GPIO_13//SLEEP INDEX
#define IO_LOCK GPIO_24
#define IO_SPEAKER GPIO_25
#define IO_LIGHT GPIO_26

char topic_gui[50],topic_nhan[50];
char imei[30];
uint16 speed;
static float lat,lng;
char buff_local[50];
char buff_time[50];
char gps_ok[10];
// APPLICATION
#define SENSOR_LIS3DH
#define SMS_APP
#define VOICE_APP

#endif /* JSON_H_ */