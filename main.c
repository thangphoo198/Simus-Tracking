#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ql_api_osi.h"
#include "ql_api_dev.h"
#include "ql_gpio.h"
// #include "ql_pin_cfg.h"
#include "ql_log.h"
#include "ql_adc.h"
#include "ql_uart.h"
#include "ql_power.h"

#include "DataDefine.h"
#include "gnss_demo.h"
#include "cJSON.h"
#include "ql_fs.h"
#include "main.h"

// #include "GNSS.h"
#define OUT_LOG DebugPrint

#define INIT_CONFIG 101
#define MAIN_TICK_100MS 102
#define MAIN_TICK_3000MS 103

ql_task_t main_task = NULL;
// ql_task_t gnss_task = NULL;
ql_timer_t main_timer = NULL;

static uint32_t tickCount100MS = 0;
static uint32_t tickCount3000MS = 0;

// int adc1_value, adc2_value, adc3_value;
static uint8_t Led = 0, Led2 = 0;

#define QL_FUN_NUM_UART_2_CTS 3
#define QL_FUN_NUM_UART_3_TXD 4

// for ledcfg demo
#define QL_PIN_NUM_KEYOUT_5 82

#define jsonRoot "{\r\n"                                                                             \
                 "\"imei\": \"8661111111111111\",\r\n"                                               \
                 "\"Num\": 142,\r\n"                                                                 \
                 "\"Value\": {\r\n"                                                                  \
                 "\"name\": \"cx\",\r\n"                                                             \
                 "\"age\": 18,\r\n"                                                                  \
                 "\"blog\": \"https://blog.csdn.net/weixin_44570083/article/details/104285283\"\r\n" \
                 "},\r\n"                                                                            \
                 "\"hexArry\": [31, 56, 36, 1365, 263]\r\n"                                          \
                 "}\r\n"

// JSON解析

// void cJSON_Parsing()
// {
//     OUT_LOG("[cJSON_Test] cJSON_Parsing Start");
//     cJSON *pJsonRoot = cJSON_Parse(jsonRoot);
//     if (pJsonRoot != NULL)
//     {
//         OUT_LOG("[cJSON_Test] cJSON TRUE");
//         OUT_LOG("[cJSON_Test] cJSON:%s", jsonRoot);
//     }
//     else
//     {
//         OUT_LOG("[cJSON_Test] cJSON ERROR");
//     }
//     cJSON *pimeiAdress = cJSON_GetObjectItem(pJsonRoot, "imei");
//     if (pimeiAdress)
//     {
//         if (cJSON_IsString(pimeiAdress))
//             OUT_LOG("[cJSON_Test] get imeiAdress:%s", pimeiAdress->valuestring);
//     }
//     else
//         OUT_LOG("[cJSON_Test] get imeiAdress failed");
// }

// void cJSON_Generate()
// {
//     // 取一下本地的station的mac地址，保存在全局变量tempMessage
//     OUT_LOG("[cJSON_Test] cJSON_Generate Start");
//     cJSON *pRoot = cJSON_CreateObject();

//     // 新增一个字段imei到根点，数值是tempMessage
//     char tempMessage[] = "8661111111111111";
//     cJSON_AddStringToObject(pRoot, "imei", tempMessage);

//     // 新增一个字段number到根点，数值是2
//     cJSON_AddNumberToObject(pRoot, "number", 2020);

//     cJSON *pValue = cJSON_CreateObject();
//     cJSON_AddStringToObject(pValue, "name", "cx");
//     cJSON_AddNumberToObject(pValue, "age", 17);
//     cJSON_AddItemToObject(pRoot, "value", pValue);

//     // 数组初始化
//     int hex[5] = {11, 12, 13, 14, 15};
//     cJSON *pHex = cJSON_CreateIntArray(hex, 5); // 创建一个长度为5的int型的数组json元素
//     cJSON_AddItemToObject(pRoot, "hex", pHex);  // 将数组元素添加进pRoot

//     char *s = cJSON_Print(pRoot);
//     OUT_LOG("\n[cJSON_Test] creatJson:%s\n", s);
//     // 释放内存
//     cJSON_free((void *)s);

//     // 释放内存
//     // cJSON_Delete(pHex);
//     // 释放内存
//     // cJSON_Delete(pValue);
//     // 释放内存
//     cJSON_Delete(pRoot);
//     OUT_LOG("[cJSON_Test] cJSON_Generate Stop");
// }

void timer_callback(void)
{
    ql_event_t event;

    if (++tickCount100MS > 10)
    {
        tickCount100MS = 0;
        SendEventToThread(main_task, MAIN_TICK_100MS);
    }

    if (++tickCount3000MS > 2000)
    {
        tickCount3000MS = 0;
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
void Uart1_rev_callback(uint32 ind_type, ql_uart_port_number_e port, uint32 size)
{

    uint8_t Buffer[256], Index;

    switch (ind_type)
    {
    case QUEC_UART_RX_OVERFLOW_IND: // rx buffer overflow
    case QUEC_UART_RX_RECV_DATA_IND:
        while (size)
        {
            memset(Buffer, 0x00, 255);
            Index = 0;

            Index = size > 255 ? 255 : size;

            uint8_t ret = ql_uart_read(port, Buffer, Index);

            Buffer[Index] = 0;

            size -= Index;
        }
    }
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

    ql_uart_set_dcbconfig(QL_USB_PORT_MODEM, &uartConfig);

    ret = ql_uart_open(QL_USB_PORT_MODEM);

    if (ret == 0)
    {
        ret = ql_uart_register_cb(QL_USB_PORT_MODEM, Uart1_rev_callback);
    }

    return ret; 
}

static void main_task_thread(void *param)
{
    ql_event_t event;
    DebugInit();
    ql_pin_set_func(24, 0);
    ql_gpio_init(GPIO_2, GPIO_OUTPUT, PULL_NONE, LVL_HIGH);

    // PIN6: NET_STATUS - GPIO22 (FUNC:4)
    ql_pin_set_func(6, 4);
    ql_gpio_init(GPIO_22, GPIO_OUTPUT, PULL_NONE, LVL_HIGH);

    SendEventToThread(main_task, INIT_CONFIG);

    while (1)
    {
        ql_event_try_wait(&event);

        switch (event.id)
        {

        case INIT_CONFIG:
            ql_uart_write(QL_USB_PORT_MODEM, "\r==>Init Configs\n", 16);
            ql_rtos_timer_start(main_timer, 2, 1);

            break;
        case MAIN_TICK_100MS:
            Led ^= 1;
            break;

        case MAIN_TICK_3000MS:
            Led2 ^= 1;
         //   send_gps();
         if (strlen(GPS_info)>120)
         {
            OUT_LOG(GPS_info);
            pub_mqtt(topic_rec,GPS_info);
         }
         // long s = ql_fs_free_size("UFS");
            OUT_LOG("\ntime chinh\n");
            ql_gpio_set_level(GPIO_22, Led2 == 0 ? LVL_LOW : LVL_HIGH);
            break;

        default:

            break;
        }
    }
}


// void send_gps()
// {
//     float lat = g_gps_data.latitude;
//     float lng = g_gps_data.longitude;
//     uint8_t speed;
//     cJSON *pRoot = cJSON_CreateObject();
//     cJSON_AddStringToObject(pRoot, "RES", "GET_GPS");
//     // cJSON_AddNumberToObject(pRoot, "lat", g_gps_data.latitude);
//     speed = (int8_t)g_gps_data.gps_speed;
//     int16_t signal = g_gps_data.avg_cnr;
//     cJSON *pValue = cJSON_CreateObject();
//     cJSON_AddNumberToObject(pValue, "lat", lat);
//     cJSON_AddNumberToObject(pValue, "lng", lng);
//     cJSON_AddNumberToObject(pValue, "speed", speed);
//     cJSON_AddNumberToObject(pValue, "signal", signal);
//     cJSON_AddItemToObject(pRoot, "GPS_INFO", pValue);
//     char *GPS = cJSON_Print(pRoot);
//     pub_mqtt(topic_rec,GPS);
// }
extern pub_mqtt(char *topic, char *mess);

int appimg_enter(void *param)
{
    QlOSStatus err;
    ql_dev_cfg_wdt(0);
    ql_log_set_port(0);
    ql_quec_trace_enable(1);
    ql_rtos_sw_dog_disable();

    /*Create timer tick*/
    err = ql_rtos_timer_create(&main_timer, main_task, timer_callback, NULL);

    /* main task*/
    err = ql_rtos_task_create(&main_task, 10 * 1024, APP_PRIORITY_NORMAL, "Main_task", main_task_thread, NULL, 5);

    ql_sms_app_init();
    ql_mqtt_app_init();
    ql_gnss_app_init();
#ifdef SENSOR_LIS3DH
    ql_i2c_demo_init();
#endif

    return err;
}

void appimg_exit(void)
{
}