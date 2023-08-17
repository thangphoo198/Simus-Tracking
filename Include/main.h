#ifndef MAIN_H_
#define MAIN_H_
#include "ql_gnss.h"
#include "ql_api_osi.h"
static ql_gnss_data_t g_gps_data;
static uint8_t gps_status=0;
#define NSIM 0
// #define MQTT_CLIENT_SRV_URL "mqtt://14.225.254.159:1883"        // onenet
// #define MQTT_CLIENT_ONENET_SSL_URL "mqtt://14.225.254.159:1883" // onenet SSL
#define MQTT_CLIENT_SRV_URL "mqtt://14.225.204.254:1883"        // onenet
#define MQTT_CLIENT_USER "vstracking-pro"
#define MQTT_CLIENT_PASS "thang123"
//#define MQTT_CLIENT_ONENET_SSL_URL "mqtt://broker.hivemq.com:1883" // onenet SSL
#define topic_remote "/EC200U_REMOTE"
#define topic_rec "/EC200U_REC"
#define topic_event "/EC200U_EVENT"

#define GPS_NULL "{\"RES\":\"GET_GPS\",\"DATA\":\"null\"}"
#define SETTING_OK "{\"RES\":\"SETTING_OK\"}"
#define SETTING_FAIL "{\"RES\":\"SETTING_FAIL\"}"
#define GET_SETTING_FAIL "{\"RES\":\"GET_SETTING_FAIL\"}"
//#define UART_DEBUG QL_USB_PORT_MODEM
#define UART_DEBUG QL_USB_PORT_MODEM
#define GPSOK "GPS_OK"
#define GPSFAIL "GPS_FAIL"

#define LED_STT GPIO_22//LED STATUS // GPIO22
#define LED_MODE GPIO_13//LED MODE

//#define ACC_IN GPIO_30//AP-READY
#define SENSOR_IN GPIO_29// WAKEUP  chuan
#define ACC_IN GPIO_30// SDIO1-DATA0
//#define ACC_IN GPIO_13//SLEEP INDEX
#define IO_LOCK GPIO_25
//#define IO_LOCK GPIO_24
#define IO_SPEAKER GPIO_26
#define IO_LIGHT GPIO_13
char topic_gui[50],topic_nhan[50],topic_tb[50],val1[50];
char imei[30];
uint16 speed;
int16 x,y,z;
static float lat,lng;
double acc,dir;
char buff_local[50];
char buff_time[50];
char buff_kttk[100];
char buff_acc_dir[50];
char gps_ok[10];
static uint32_t run_time = 0,run_on;
uint8_t INDEX_SLEEP;
ql_queue_t sleep_index;
ql_sem_t sleep_sem;

char json_setting[128],phone_sos[20];

uint32_t time_sleep,time_update_gps;

// APPLICATION
#define SENSOR_LIS3DH
#define GNSS
#define SMS_APP
//#define VOICE_APP

#endif /* MAIN_H_ */