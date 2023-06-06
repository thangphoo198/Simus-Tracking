

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
ql_task_t setting_task = NULL;

extern pub_mqtt(char *topic, char *mess);
static void setting_thread(void *arg)
{

    setting();
    ql_rtos_task_delete(NULL);
}

void setting(char *json)
{

    cJSON *pJsonRoot = cJSON_Parse(json);
    cJSON *info = cJSON_GetObjectItem(pJsonRoot, "DATA");
    cJSON *pSleep = cJSON_GetObjectItem(info, "time_sleep");
    cJSON *pGps = cJSON_GetObjectItem(info, "time_update_gps");
    cJSON *pSos = cJSON_GetObjectItem(info, "phone_sos");
    if (pSleep)
    {
        if (cJSON_IsNumber(pSleep))
        {
            OUT_LOG("time sleep:%d", pSleep->valueint);
        }
    }
    if (pGps)
    {
        if (cJSON_IsNumber(pGps)) /* code */
        {
            OUT_LOG("time gps:%d", pGps->valueint);
        }
    }
    if (pSos)
    {
        if (cJSON_IsNumber(pSos)) /* code */
        {
            OUT_LOG("time gps:%s", pSos->valuestring);
        }
    }
    cJSON *pRoot = cJSON_CreateObject();
    cJSON_AddStringToObject(pRoot, "RES", "GET_SETTING");
    cJSON *pValue = cJSON_CreateObject();
    cJSON_AddNumberToObject(pValue, "time_sleep", pSleep->valueint);
    cJSON_AddNumberToObject(pValue, "time_update_gps", pGps->valueint);
    cJSON_AddStringToObject(pValue, "phone_sos", pSos->valuestring);
    cJSON_AddItemToObject(pRoot, "DATA", pValue);

    char *GPS_info = cJSON_Print(pRoot);
    OUT_LOG(GPS_info);
    pub_mqtt(topic_gui, GPS_info);
    ghi_epprom(GPS_info);
    apply_setting(pSleep->valueint,pGps->valueint,pSos->valuestring);
    cJSON_free((void *)GPS_info);
    cJSON_Delete(pRoot);
    cJSON_Delete(pValue);
}
void ghi_epprom(char *buff)
{

    if (ql_cust_nvm_fwrite(buff, strlen(buff), 1))
    {
        OUT_LOG("Ghi thanh cong:%d\n", strlen(buff));
    }
}
void doc_epprom(char *out)
{

    if (ql_cust_nvm_fread(out, 128, 1))
    {
        OUT_LOG("du lieu doc dc:%s", out);
    }
}

apply_setting(uint32_t Time_sleep,uint32_t Time_gps, char *sdt_sos)
{

time_sleep=Time_sleep;
time_update_gps=Time_gps;
strcpy(phone_sos,sdt_sos);
}

void setting_init(void)
{
    QlOSStatus err = QL_OSI_SUCCESS;

    err = ql_rtos_task_create(&setting_task, 1024, APP_PRIORITY_LOW, "QNWDEMO", setting_thread, NULL, 5);
    if (err != QL_OSI_SUCCESS)
    {
        OUT_LOG("created task failed");
    }
}
