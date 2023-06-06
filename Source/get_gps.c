

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ql_api_osi.h"
#include "ql_log.h"
#include "ql_api_nw.h"
#include "DataDefine.h"
#include "cJSON.h"
#include "main.h"
#include "ql_gpio.h"
#include "ql_adc.h"

#define OUT_LOG DebugPrint
ql_task_t stt_gps_task = NULL;

extern pub_mqtt(char *topic, char *mess);
static void stt_gps_thread(void *arg)
{

    get_GNSS();
    ql_rtos_task_delete(NULL);
}


void get_GNSS()
{

    if (strcmp(gps_ok, GPSOK) == 0)
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
    }
    else
    {

    pub_mqtt(topic_gui, GPS_NULL);
    }

}

void ql_stt_gps_init(void)
{
    QlOSStatus err = QL_OSI_SUCCESS;

    err = ql_rtos_task_create(&stt_gps_task, 1024, APP_PRIORITY_LOW, "QNWDEMO", stt_gps_thread, NULL, 5);
    if (err != QL_OSI_SUCCESS)
    {
        OUT_LOG("created task failed");
    }
}
