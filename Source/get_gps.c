

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

static void stt_gps_thread(void *arg)
{

    if (strcmp(gps_ok, GPSOK) == 0)
    {
        get_GPS();
    }
    else
    {
        OUT_LOG("\nkhong co GPS\n");
        pub_mqtt(topic_gui, GPS_NULL);
    }
    ql_rtos_task_delete(NULL);
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
