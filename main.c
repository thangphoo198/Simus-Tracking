#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ql_adc.h"
#include "ql_api_osi.h"
#include "ql_api_dev.h"
#include "ql_gpio.h"
// #include "ql_pin_cfg.h"
#include "ql_log.h"
#include "DataDefine.h"
#include "cJSON.h"
#include "main.h"
#include "ql_api_rtc.h"
#include "ql_power.h"
#define OUT_LOG DebugPrint
#define INIT_CONFIG 101
#define MAIN_TICK_100MS 102
#define MAIN_TICK_3000MS 103
ql_task_t main_task = NULL;
ql_timer_t main_timer = NULL;
static uint32_t tickCount500MS = 0,tickCount5000MS=0;
static uint8_t Led=0,Led2=0;
ql_LvlMode in;
bool noti=false;
void timer_callback(void)
{
    ql_event_t event;

    if (++tickCount500MS > 0)
    {
        tickCount500MS = 0;
        SendEventToThread(main_task, MAIN_TICK_100MS);  

    }
    Led2 ^= 1;
    ql_gpio_set_level(LED_STT, Led2 == 0 ? LVL_LOW : LVL_HIGH);

    if (++tickCount5000MS > time_update_gps)
    {
        tickCount5000MS = 0;
        SendEventToThread(main_task, MAIN_TICK_3000MS);
    }

}

void SendEventToThread(ql_task_t thread, uint32_t sig)
{
    ql_event_t event;

    memset(&event, 0, sizeof(event));
    event.id = sig;

    ql_rtos_event_send(thread, &event);
}
uint8_t DebugInit(void)
{

    ql_uart_config_s uartConfig;
    uint8_t ret;

    uartConfig.baudrate = 115200;
    uartConfig.data_bit = 8;
    uartConfig.flow_ctrl = 0;
    uartConfig.parity_bit = 0;
    uartConfig.stop_bit = 1;

    ql_uart_set_dcbconfig(UART_DEBUG, &uartConfig);

    ret = ql_uart_open(UART_DEBUG);

    return ret;
}



void send_event()
{
    cJSON *pRoot = cJSON_CreateObject();
    cJSON_AddStringToObject(pRoot, "RES", "EVENT");
    cJSON *pValue = cJSON_CreateObject();
    cJSON_AddStringToObject(pValue, "type","OPEN");
    cJSON_AddItemToObject(pRoot, "DATA", pValue);
    char *event_info= cJSON_Print(pRoot);
    OUT_LOG(event_info);
    QlOSStatus ret;
    ret=pub_mqtt(topic_tb, event_info);
    if(ret!=QL_OSI_SUCCESS)
    {
        OUT_LOG("gui that bai\n");
        noti=false;
    }
    else
    {
        noti=true;
        OUT_LOG("gui thanh cong\n");
    }
    cJSON_free((void *)event_info);
}
static void _pwrkey_longpress_callback(void)
{
   
    if (strcmp(gps_ok, GPSOK) != 0)
    {
    OUT_LOG("\Khong co chuyen dong trong %dS - TAT NGUON sau 5s\n",time_sleep);
    ql_event_t event;
    event.id = QUEC_PWRKEY_LONGPRESS_IND;
    ql_rtos_event_send(main_task, &event);
    }
    OUT_LOG("\Khong co chuyen dong trong \n");

}
static void _pwrkey_release_callback(void)
{
    OUT_LOG("\nDA BAM NUT NGUON - chuyen dong!\n");
    // uint16_t dat;
    // mpu_read_reg(0x21, &dat); // read register to reset high-pass filter
    // mpu_read_reg(0x26, &dat); // read register to set reference acceleration
    // mpu_read_reg(0x31, &dat); // Read INT1_SRC to de-latch;
    // ql_gpio_set_level(SENSOR_IN, LVL_LOW);
    // mpu_read_reg8(0x21);
    // mpu_read_reg8(0x26);
    // mpu_read_reg(0x31);
    // OUT_LOG("\nRESET CAM BIEN LIS3DH XONG!\n");
    ql_pwrkey_longpress_cb_register(_pwrkey_longpress_callback,time_sleep*1000);    // k chuyen dong trong 3 phut
}
static void main_task_thread(void *param)
{
    ql_event_t event= {0};
    DebugInit();  
    //ql_pin_set_func(24, 0);
   // ql_gpio_init(GPIO_2, GPIO_OUTPUT, PULL_NONE, LVL_HIGH);
    ql_gpio_init(IO_LOCK, GPIO_OUTPUT, PULL_DOWN, LVL_LOW);
    ql_gpio_init(IO_SPEAKER, GPIO_OUTPUT, PULL_DOWN, LVL_LOW);
    ql_gpio_init(IO_LIGHT, GPIO_OUTPUT, PULL_DOWN, LVL_LOW);
    ql_gpio_init(SENSOR_IN, GPIO_INPUT, PULL_DOWN, LVL_LOW); // SDA
    // ql_gpio_deinit(SENSOR_IN);
    // ql_int_register(SENSOR_IN, EDGE_TRIGGER,DEBOUNCE_EN,EDGE_FALLING,PULL_UP,ngat,NULL);
    // ql_int_enable(SENSOR_IN);
    // PIN6: NET_STATUS - GPIO22 (FUNC:4)
   // ql_pin_set_func(6, 4);
    ql_gpio_init(LED_STT, GPIO_OUTPUT, PULL_NONE, LVL_HIGH);
    ql_gpio_init(LED_MODE, GPIO_OUTPUT, PULL_NONE, LVL_HIGH);
    doc_epprom();
    ql_pwrkey_release_cb_register(_pwrkey_release_callback);
    ql_pwrkey_shutdown_time_set(99999999); 
    SendEventToThread(main_task, INIT_CONFIG);
    while (1)
    {

        ql_event_try_wait(&event); 

        switch (event.id)
        {
        case INIT_CONFIG:
            OUT_LOG("khoi tao OK\n");
            ql_rtos_timer_start(main_timer, 1000, 1);
            break;
        case MAIN_TICK_100MS:
        get_time();
        ql_gpio_set_level(LED_MODE, Led == 0 ? LVL_LOW : LVL_HIGH);
        Led^=1;
            break;
        case MAIN_TICK_3000MS:
            send_gps();             
            break;
        case QUEC_PWRKEY_LONGPRESS_IND:
            ql_rtos_task_sleep_s(5);
            ql_power_down(POWD_NORMAL);
            break;
        default:

            break;
        }

    }
}

void get_time()
{

    int ret = 0;
    int64_t time_sec=0,time_fix=0;
    ql_rtc_time_t tm={0};
    //int timezone=0;
    ret = ql_rtc_get_time(&tm);

    if(ret != QL_RTC_SUCCESS)
    {
        OUT_LOG("get time err");
        return false; 
    } 
    //QL_PSMDEMO_LOG("time zone:%d\n",timezone);
    uint16_t hour=tm.tm_hour+7;
    if(hour>23) {hour=0;}
    sprintf(buff_time,"%d:%d:%d %d/%d/%d", hour,tm.tm_min,tm.tm_sec,tm.tm_mday,tm.tm_mon,tm.tm_year);
    //OUT_LOG(buff_time);
}
void get_GPS()
{
    cJSON *pRoot = cJSON_CreateObject();
    cJSON_AddStringToObject(pRoot, "RES", "GET_GPS");
    cJSON *pValue = cJSON_CreateObject();
    cJSON_AddStringToObject(pValue, "time", buff_time);
    cJSON_AddStringToObject(pValue, "localtion", buff_local);
    cJSON_AddNumberToObject(pValue, "speed", speed);
    cJSON_AddStringToObject(pValue, "accuracy", buff_acc_dir);
    // cJSON_AddNumberToObject(pValue, "accuracy", acc);
    // cJSON_AddNumberToObject(pValue, "direction", dir);
    cJSON_AddItemToObject(pRoot, "DATA", pValue);
    char *GPS_info = cJSON_Print(pRoot);
    OUT_LOG(GPS_info);
    pub_mqtt(topic_gui, GPS_info);
    cJSON_free((void *)GPS_info);
    cJSON_Delete(pRoot);
}
void send_gps()
{

    if (strcmp(gps_ok, GPSOK) == 0)
    {
        if(speed>4)
        {
            get_GPS();
        }
        else
        {
            OUT_LOG("TOC DO BE HON 4KM\n");
        }

    }
    else
    {  
        OUT_LOG("\nkhong co GPS\n");
    }
}

extern pub_mqtt(char *topic, char *mess);


int appimg_enter(void *param)
{
    QlOSStatus err;
    ql_dev_cfg_wdt(1);
    ql_log_set_port(0);
    ql_quec_trace_enable(0);
    // ql_rtos_sw_dog_enable;
   // ql_rtos_sw_dog_disable;
    /*Create timer tick*/
    err = ql_rtos_timer_create(&main_timer, main_task, timer_callback, NULL);
    err= ql_autosleep_enable(QL_ALLOW_SLEEP);
    /* main task*/
    err = ql_rtos_task_create(&main_task, 4 * 1024, APP_PRIORITY_NORMAL, "Main_task", main_task_thread, NULL, 5);
    //  err = ql_rtos_semaphore_create(&sleep_sem, 0);
    // if (err != QL_OSI_SUCCESS)
    // {
    //     OUT_LOG("sem sleep created failed, ret = 0x%x\n", err);
    // }   
    // ql_rtos_semaphore_release(sleep_sem); 

   ql_sms_app_init();
   ql_mqtt_app_init();
    
#ifdef GNSS
    ql_gnss_app_init();     
#endif      
#ifdef SENSOR_LIS3DH
    ql_i2c_demo_init();
#endif

    return err;
}

void appimg_exit(void)
{
    
}