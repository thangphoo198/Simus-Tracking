

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
ql_task_t stt_task = NULL;

extern pub_mqtt(char *topic, char *mess);
static void stt_app_thread(void *arg)
{

    get_stt();
    ql_rtos_task_delete(NULL);
}


void get_stt()
{
    ql_LvlMode stt, stt_sp, stt_light, stt_lock;
    uint8_t csq, temp = 0;
    uint32_t adc_value, adc_value1 = 0;
    float R1 = 10000.0; // resistance of R1 (10K)
    float R2 = 1000.0;  // resistance of R2 (1K)
    float vin = 0.0, vin1 = 0.0;
    ql_gpio_get_level(IO_LOCK, &stt);
    ql_gpio_get_level(IO_SPEAKER, &stt_sp);
    ql_gpio_get_level(IO_LIGHT, &stt_light);
    ql_gpio_get_level(ACC_IN, &stt_lock);
    cJSON *pRoot = cJSON_CreateObject();
    ql_adc_get_volt(QL_ADC0_CHANNEL, &adc_value);
    ql_adc_get_volt(QL_ADC2_CHANNEL, &adc_value1);
    vin = adc_value / (R2 / (R1 + R2));
    vin1 = adc_value1 / (R2 / (R1 + R2));
    ql_nw_get_csq(NSIM, &csq);
    cJSON_AddStringToObject(pRoot, "RES", "GET_STT");
    cJSON *pValue = cJSON_CreateObject();
    if (stt == LVL_LOW)
    {
        OUT_LOG("MAY DANG MO\n");
        cJSON_AddStringToObject(pValue, "power_stt", "ON");
    }
    else
    {
        OUT_LOG("MAY DANG TAT\n");
        cJSON_AddStringToObject(pValue, "power_stt", "OFF");
    }
    if (stt_sp == LVL_LOW)
    {
        OUT_LOG("LOA DANG TAT\n");
        cJSON_AddStringToObject(pValue, "speaker_stt", "OFF");
    }
    else
    {
        OUT_LOG("MAY DANG TAT\n");
        cJSON_AddStringToObject(pValue, "speaker_stt", "ON");
    }

    if (stt_light == LVL_LOW)
    {
        OUT_LOG("xi-nhan DANG TAT\n");
        cJSON_AddStringToObject(pValue, "light_stt", "OFF");
    }
    else
    {
        OUT_LOG("MAY DANG TAT\n");
        cJSON_AddStringToObject(pValue, "light_stt", "ON");
    }
    if (stt_lock == LVL_LOW)
    {
        OUT_LOG("MAY DANG TAT\n");
        cJSON_AddStringToObject(pValue, "acc_stt", "ON");
    }
    else
    {
        OUT_LOG("MAY DANG BAT\n");
        cJSON_AddStringToObject(pValue, "acc_stt", "OFF");
    }

    ql_dev_get_temp_value(&temp);
    cJSON_AddNumberToObject(pValue, "temp", temp);
    cJSON_AddNumberToObject(pValue, "power_volt", vin);
    cJSON_AddNumberToObject(pValue, "acc_volt", vin1);
    cJSON_AddNumberToObject(pValue, "csq", csq);
    cJSON_AddItemToObject(pRoot, "DATA", pValue);
    char *info = cJSON_Print(pRoot);

    OUT_LOG(info);
    pub_mqtt(topic_gui, info);

    cJSON_free((void *)info);
}

void ql_stt_app_init(void)
{
    QlOSStatus err = QL_OSI_SUCCESS;

    err = ql_rtos_task_create(&stt_task, 1024 * 4, APP_PRIORITY_LOW, "QNWDEMO", stt_app_thread, NULL, 5);
    if (err != QL_OSI_SUCCESS)
    {
        OUT_LOG("created task failed");
    }
}
