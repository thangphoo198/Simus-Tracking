#include "ql_gnss.h"
#include "GNSS.h"


void gps_timer_cb(void);
uint8_t GPS_Init(void);
void GPS_Tick(void);
uint8_t GPS_GetState(void);
uint8_t DecodeRMC(char *Buffer, GPSInfo_t *GPSInfo);
uint8_t DecodeGGA(char *Buffer, GPSInfo_t *GPSInfo);
uint8_t GPS_GetTotalData(GPSInfo_t *Info, uint8_t* NMEA_sentence);
void ResetGPSInfo(GPSInfo_t *tmpGPSInfo);
void UpdateTimeFromGPS (void);

extern ql_task_t gnss_task;

GPSInfo_t GPSInfo;
GPS_Manager_t GPS_Manager;

LargeBuffer_t GPS_RecData;

uint8_t GPSDataLen = 0;
uint8_t InitGSPOK = 0xFF;


void GPS_task_thread (void *param){
	ql_event_t event;
    ql_timer_t gps_timer = NULL;

    ql_rtos_timer_create(&gps_timer ,gnss_task,(void*) gps_timer_cb, NULL);
    ql_rtos_timer_start(gps_timer, 200, 1);

    while(1){
        ql_event_try_wait(&event);

         switch (event.id)
        {
            case GPS_INIT_EVENT:
                GPS_Manager.State = GPS_INIT_EVENT;
                GPS_Init();
            break;

            case GPS_TICK: 
                GPS_Manager.State = GPS_TICK;
                GPS_Tick();
                break;
		
        }
    }
}

void Gnss_Data_Callback(uint32 ind_type, ql_uart_port_number_e port, uint32 size){
    if( ind_type == QUEC_UART_RX_RECV_DATA_IND )    {
        GPSDataLen = size;
        SendEventToThread(gnss_task, GPS_TICK);
       
    }
    else    {
    }
}


/* GNSS_CLOSE,
   GNSS_FIRMWARE_UPDATE,
   GNSS_POSITIONING,
   GNSS_FIX*/
uint8_t GPS_GetState(void){
    return ql_gnss_state_info_get();
}

uint8_t GPS_Init(void){
    int ret;
    
    
	ret = ql_gnss_switch(GNSS_ENABLE);

    /*    0  GPS only锛坰upport�??    
          1  GPS+GLONASS+BeiDou+Galileo
          2  GPS+GLONASS+BeiDou
          3  GPS+GLONASS+Galileo
          4  GPS+GLONASS
          5  GPS+BeiDou锛坰upport�??
          6  GPS+Galileo
          7  BeiDou only锛坰upport�??*/

    ql_gnss_sys_type_cfg(1);

    if(GPS_GetState() > 1)
    {
        ql_gnss_callback_register(Gnss_Data_Callback);

        GPS_Manager.State = GPS_TICK;

        InitGSPOK = 1;

        SendEventToThread(gnss_task, GPS_TICK);
    }
    else{
       InitGSPOK = 0;
    }

 
    return ret;
}

static uint8_t  Led=0;

extern pub_mqtt(char* topic,char* mess);

void GPS_Tick(void){

    if(GPSDataLen){
        if(GPS_RecData.Index + GPSDataLen > LARGE_BUFFER_SIZE){
            GPS_RecData.Index = 0;
            memset(GPS_RecData.Buffer, 0x00, LARGE_BUFFER_SIZE);
        }
        ql_gnss_nmea_get(2 , &GPS_RecData.Buffer[GPS_RecData.Index], GPSDataLen);

        GPS_RecData.Index += GPSDataLen;  
		Led^=1;
		ql_gpio_set_level(GPIO_2, Led==0?LVL_LOW:LVL_HIGH);
    }

	if(GPS_GetState() < 2){
		GPS_RecData.Index = 0;
        memset(GPS_RecData.Buffer, 0x00, LARGE_BUFFER_SIZE);
		return;
	}

    if(strstr((char*)GPS_RecData.Buffer,(char*)"RMC") && GPS_RecData.Buffer[GPS_RecData.Index - 2] == '\r'
        && GPS_RecData.Buffer[GPS_RecData.Index - 1] == '\n'){

        pub_mqtt("EC200U_REC",(char*)GPS_RecData.Buffer);

       //	ql_uart_write(QL_UART_PORT_1,(uint8_t*)"\rNMEA full: ",12);
       	ql_uart_write(QL_UART_PORT_1,GPS_RecData.Buffer,GPS_RecData.Index);
        GPS_RecData.Index = 0;
        memset(GPS_RecData.Buffer, 0x00, LARGE_BUFFER_SIZE);
    }

}



void gps_timer_cb(void){

    if(InitGSPOK == 0) SendEventToThread(gnss_task, GPS_INIT_EVENT);
   
   if(InitGSPOK == 1){
       if(GPS_GetState() < 2) SendEventToThread(gnss_task, GPS_INIT_EVENT);
   }

    if(GPSInfo.ErrorCnt > 200){
        ql_gnss_switch(GNSS_RESET);
        GPSInfo.ErrorCnt = 0;
        SendEventToThread(gnss_task, GPS_INIT_EVENT);
    }

    if(GPSInfo.Valid){
		GPSInfo.Valid--;
	}

}



