

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ql_api_osi.h"
#include "ql_log.h"
#include "ql_api_nw.h"
#include "DataDefine.h"
#include "cJSON.h"
#include "main.h"
#include "ql_adc.h"
#include "ql_fs.h"

#define OUT_LOG DebugPrint
ql_task_t setting_task = NULL;

extern pub_mqtt(char *topic, char *mess);
static void setting_thread(void *arg)
{

    setting(json_setting);
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
            OUT_LOG("time sleep:%d\n", pSleep->valueint);
        }
    }
    if (pGps)
    {
        if (cJSON_IsNumber(pGps)) /* code */
        {
            OUT_LOG("time gps:%d\n", pGps->valueint);
        }
    }
    if (pSos)
    {
        if (cJSON_IsString(pSos)) /* code */
        {
            OUT_LOG("phone SOS:%s\n", pSos->valuestring);
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
    // OUT_LOG(GPS_info);
    ghi_epprom(GPS_info);
    apply_setting(pSleep->valueint,pGps->valueint,pSos->valuestring);
    pub_mqtt(topic_gui, SETTING_OK);
    cJSON_free((void *)GPS_info);
    cJSON_Delete(pRoot);
}

void ghi_epprom(char *buff)
{
    if (ql_cust_nvm_fwrite(buff, strlen(buff), 1))
    {
        OUT_LOG("Ghi thanh cong:%d\n", strlen(buff));
    }
}

void doc_epprom()
{

    if (ql_cust_nvm_fread(json_setting, 128, 1) )
    {
        if(json_setting!=NULL&&strlen(json_setting)>30)
        {
        OUT_LOG("du lieu doc dc:%s", json_setting);
        apply_setting_epprom(json_setting);
        }
        else
        {
           OUT_LOG("\ndu lieu bi trong:%n"); 
           time_sleep=180;
        }
    }
    
}

apply_setting(uint32_t Time_sleep,uint32_t Time_gps, char *sdt_sos)
{
if (Time_sleep < 10 || Time_sleep > 4096)
{
   time_sleep=180; // mac dinh sleep trong 3 phut
}
else time_sleep=Time_sleep;
time_update_gps=Time_gps;
strcpy(phone_sos,sdt_sos);
OUT_LOG("phone:%s\n",phone_sos);
}
void apply_setting_epprom(char *json)
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

            OUT_LOG("time sleep:%d\n", pSleep->valueint);
        }
    }
    if (pGps)
    {
        if (cJSON_IsNumber(pGps)) /* code */
        {
            OUT_LOG("time gps:%d\n", pGps->valueint);
        }
    }
    if (pSos)
    {
        if (cJSON_IsString(pSos)) /* code */
        {
            OUT_LOG("phone SOS:%s\n", pSos->valuestring);
        }
    }
    apply_setting(pSleep->valueint,pGps->valueint,pSos->valuestring);
    cJSON_Delete(pJsonRoot);
}

void setting_init(void)
{
    QlOSStatus err = QL_OSI_SUCCESS;

    err = ql_rtos_task_create(&setting_task, 4*1024, APP_PRIORITY_LOW, "QNWDEMO", setting_thread, NULL, 5);
    if (err != QL_OSI_SUCCESS)
    {
        OUT_LOG("created task failed");
    }
}
