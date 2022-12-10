#include "string.h"
#include "stdio.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "ql_api_osi.h"
#include "ql_api_dev.h"
#include "ql_gpio.h"
//#include "ql_pin_cfg.h"
#include "ql_log.h"
#include "ql_adc.h"
#include "ql_uart.h"
#include "ql_power.h"

#include "DataDefine.h"

#include "json.h"
#include "gnss_demo.h" 


//#include "GNSS.h"
#define OUT_LOG DebugPrint


#define INIT_CONFIG         101
#define MAIN_TICK_100MS     102
#define MAIN_TICK_3000MS    103



ql_task_t main_task = NULL;
//ql_task_t gnss_task = NULL;
ql_timer_t main_timer = NULL;



static uint32_t tickCount100MS = 0;
static uint32_t tickCount3000MS = 0;

//int adc1_value, adc2_value, adc3_value;
static uint8_t Led=0, Led2=0;

#define QL_FUN_NUM_UART_2_CTS      3
#define QL_FUN_NUM_UART_3_TXD      4

//for ledcfg demo
#define QL_PIN_NUM_KEYOUT_5        82
char *topic_rec="EC200U_REC";
char *topic_remote="EC200U_REMOTE";

void timer_callback(void){
    ql_event_t event;

    if(++tickCount100MS>0){
        tickCount100MS=0;
        SendEventToThread(main_task, MAIN_TICK_100MS);    
    }

    if(++tickCount3000MS>500){
        tickCount3000MS=0;
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
void Uart1_rev_callback(uint32 ind_type, ql_uart_port_number_e port, uint32 size){

    uint8_t Buffer[256], Index;


   switch(ind_type)
    {
        case QUEC_UART_RX_OVERFLOW_IND:  //rx buffer overflow
        case QUEC_UART_RX_RECV_DATA_IND:
        while (size) {
            memset(Buffer, 0x00, 255);
            Index = 0;

            Index = size > 255? 255: size;

            uint8_t ret = ql_uart_read(port, Buffer, Index);
       
            Buffer[Index] = 0;

            size -= Index;

        }
    }
   
}
uint8_t DebugInit(void){

    ql_uart_config_s uartConfig;
    uint8_t ret;

    uartConfig.baudrate = 115200;
    uartConfig.data_bit = 8;
    uartConfig.flow_ctrl = 0;
    uartConfig.parity_bit = 0;
    uartConfig.stop_bit = 1;

    ql_uart_set_dcbconfig(QL_UART_PORT_1, &uartConfig);

    ret = ql_uart_open(QL_UART_PORT_1);

    if(ret == 0){
        ret = ql_uart_register_cb(QL_UART_PORT_1, Uart1_rev_callback);
    }

    return ret;
}


extern print_GPS(char *dat);
extern pub_mqtt(char *topic,char*mess);

static void main_task_thread(void *param)
{

    ql_event_t event;

    DebugInit();
// string x="\r du lieu GNSS =>>> \n";
// ql_uart_write(QL_UART_PORT_1,x,x.length());

    OUT_LOG("DANG khoi tao he thong... \n");


    // PIN24 GPIO2 (FUNC0)
    ql_pin_set_func(24, 0);
    ql_gpio_init(GPIO_2,GPIO_OUTPUT,PULL_NONE,LVL_HIGH);

    // PIN6: NET_STATUS - GPIO22 (FUNC:4)
    ql_pin_set_func(6, 4);
    ql_gpio_init(GPIO_22,GPIO_OUTPUT,PULL_NONE,LVL_HIGH);

    ql_uart_write(QL_UART_PORT_1,"\r",21);
    
    // Init
    SendEventToThread(main_task, INIT_CONFIG);

    //SendEventToThread(gnss_task, QL_EVENT_APP_START + 21);
    char   version_buf[128] = {0};
	ql_dev_get_firmware_version(version_buf, sizeof(version_buf));
	OUT_LOG("Phien phan mem hien tai:  %s\n", version_buf);



    while(1){
        ql_event_try_wait(&event);

        switch (event.id)
        {
          
            case INIT_CONFIG:
                ql_uart_write(QL_UART_PORT_1,"\r==>Init Configs",16);
                ql_rtos_timer_start(main_timer, 2, 1);

            break;
            case MAIN_TICK_100MS:
                Led^=1;
            break;
            
            case MAIN_TICK_3000MS:
                Led2^=1;
                OUT_LOG("TIMER CT CHINH:\n");
                char buff[256]={0};
                print_GPS(&buff);
                pub_mqtt(topic_rec,buff);
                //ql_gpio_set_level(GPIO_2, Led==0?LVL_LOW:LVL_HIGH);
                OUT_LOG("DU LIEU LAY DC:%s\n",(char*)buff);          
                ql_gpio_set_level(GPIO_22, Led2==0?LVL_LOW:LVL_HIGH);
//ql_rtos_task_get_userdata(gnss_task,&buff);
//                 QlOSStatus ql_rtos_task_get_userdata
// (
// 	ql_task_t taskRef, /* OS task reference	*/
// 	void **userData     /* The user data of pointer type */
// );


               // ql_uart_write(QL_UART_PORT_1,"\r\r========== DU LIEU GPS : ==========\r",39);
                break;

            default:

            break;
        }
    }
}



extern void GPS_task_thread (void *param);
extern void mqtt_app_thread(void * arg);
extern void sms_demo_task(void * param);
extern void ql_i2c_demo_thread(void *param);
extern void ql_gnss_demo_thread(void *param);
//extern ql_gnss_app_init(void);


void ql_enter_sleep_cb(void* ctx)
{   
    OUT_LOG("enter sleep cb\n");
    ql_pin_set_func(QL_PIN_NUM_KEYOUT_5, QL_FUN_NUM_UART_2_CTS);  //keyout5 pin need be low level when enter sleep, adjust the function to uart2_rts can do it
    ql_gpio_set_level(GPIO_12, LVL_HIGH);                         //close mos linked to gnss, to avoid high current in sleep mode
    ql_gpio_set_level(GPIO_11, LVL_LOW);                          //gpio11 need be low level when enter sleep to reduce leakage current to gnss
}

//exit sleep callback function is executed after exiting sleep, custom can recover the information before sleep
//Caution:callback functions cannot run too much code 
void ql_exit_sleep_cb(void* ctx)
{   
    OUT_LOG("exit sleep cb\n");  
    
    ql_pin_set_func(QL_PIN_NUM_KEYOUT_5, QL_FUN_NUM_UART_3_TXD);  //keyout5 pin used as gnss uart3_txd function, after exit sleep, set it to uart3_txd
}

int appimg_enter(void *param)
{
    QlOSStatus err;
    ql_dev_cfg_wdt(0);
    ql_log_set_port(2);
    ql_quec_trace_enable(1);

    ql_rtos_sw_dog_disable();
    //register sleep callback function
    ql_sleep_register_cb(ql_enter_sleep_cb);
    //register wakeup callback function
    ql_wakeup_register_cb(ql_exit_sleep_cb);


 
    /*Create timer tick*/
    err = ql_rtos_timer_create(&main_timer, main_task, timer_callback, NULL);

    /* main task*/
    err = ql_rtos_task_create(&main_task, 5*1024, APP_PRIORITY_NORMAL, "Main_task", main_task_thread, NULL, 5);


    /*GNSS task*/
    //err = ql_rtos_task_create(&gnss_task, 5 * 1024, 25, "GNSS_task",  GPS_task_thread, NULL,3);
    ql_sms_app_init();
   // ql_i2c_demo_init();
    ql_mqtt_app_init();
    ql_gnss_app_init();
    //ql_fota_http_app_init();


    return err;
// char *x="name queue";
// ql_rtos_queue_create (x,1024,10);
// (
// 	ql_queue_t   	*msgQRef,       	/* OS message queue reference              */
// 	uint32         	maxSize,        	/* max message size the queue supports     */
// 	uint32         	maxNumber	      	/* max # of messages in the queue          */
// );

}

void appimg_exit(void)
{
   
}