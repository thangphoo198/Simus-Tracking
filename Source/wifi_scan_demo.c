
/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/

/*=================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

WHEN			  WHO		  WHAT, WHERE, WHY
------------	 -------	 -------------------------------------------------------------------------------

=================================================================*/

/*===========================================================================
 * include files
 ===========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ql_log.h"
#include "ql_api_osi.h"
#include "ql_wifi_scan.h"
#include "wifi_scan_demo.h"
#include "DataDefine.h"
#include "cJSON.h"
#include "main.h"

#define QL_WIFISACN_DEMO_LOG DebugPrint

#define QL_WIFISACN_TASK_STACK_SIZE 4096
#define QL_WIFISACN_TASK_PRIO APP_PRIORITY_NORMAL
#define QL_WIFISACN_TASK_EVENT_CNT 10
ql_task_t ql_wifiscan_app_task = NULL;
char *out;
extern pub_mqtt(char *topic, char *mess);
cJSON *root, *cars, *car;
void ql_wifiscan_ap_info_output(uint16_t ap_cnt, ql_wifi_ap_info_s *p_ap_infos)
{
    if (NULL == p_ap_infos)
    {
        return;
    }
    // char buff[100]={0};
    root = cJSON_CreateObject();
    cars = cJSON_CreateArray();
    /* add cars array to root */
    cJSON_AddStringToObject(root, "RES", "SCAN_WIFI");
    cJSON_AddItemToObject(root, "DATA", cars);

    for (uint16_t n = 0; n < ap_cnt; n++)
    {
        char macbuff[20];
        int8_t rssi = p_ap_infos->rssival;
        uint8_t channel = p_ap_infos->channel;
        sprintf(macbuff, "%x%lx", p_ap_infos->bssid_high, p_ap_infos->bssid_low);
        char mac_out[50] = {0};
        // convert to "60:38:E0:A1:A4:91"
        sprintf(mac_out, "%c%c:%c%c:%c%c:%c%c:%c%c:%c%c", macbuff[0], macbuff[1], macbuff[2], macbuff[3], macbuff[4],
                macbuff[5], macbuff[6], macbuff[7], macbuff[8], macbuff[9], macbuff[10], macbuff[11]);
        cJSON_AddItemToArray(cars, car = cJSON_CreateObject());
        cJSON_AddItemToObject(car, "macAddress", cJSON_CreateString(mac_out));
        cJSON_AddItemToObject(car, "signalStrength", cJSON_CreateNumber(rssi));
        cJSON_AddItemToObject(car, "channel", cJSON_CreateNumber(channel));
        p_ap_infos++;
    }

    out = cJSON_Print(root);
    QL_WIFISACN_DEMO_LOG("\n%s\n", out);
}

// all memory of msg_buf and it's sub item will be released by core after the call of callback.
void ql_wifiscan_app_callback(ql_wifiscan_ind_msg_s *msg_buf)
{
    if (NULL == msg_buf)
    {
        ql_wifiscan_close();
        return;
    }

    QL_WIFISACN_DEMO_LOG("\nid=0x%X, err=%d, buf=%p\n", msg_buf->msg_id, msg_buf->msg_err_code, msg_buf->msg_data);
    if (QUEC_WIFISCAN_EVENT_DO == msg_buf->msg_id)
    {
        if ((QL_WIFISCAN_SUCCESS == msg_buf->msg_err_code) && (NULL != msg_buf->msg_data))
        {
            ql_wifiscan_result_s *scan_result = msg_buf->msg_data;
            QL_WIFISACN_DEMO_LOG("ap_cnt=%d", scan_result->ap_cnt);
            ql_wifiscan_ap_info_output(scan_result->ap_cnt, scan_result->ap_infos);
        }
        ql_event_t scan_event = {0};
        scan_event.id = QUEC_WIFISCAN_EVENT_ASYNC_IND;
        ql_rtos_event_send(ql_wifiscan_app_task, &scan_event);
    }
}

// to start a async scan
void ql_wifiscan_async_start(void)
{
    if (QL_WIFISCAN_SUCCESS != ql_wifiscan_option_set(QL_WIFI_SCAN_DEFAULT_AP_CNT,
                                                      QL_WIFI_SCAN_DEFAULT_ROUND,
                                                      QL_WIFISCAN_CHANNEL_ALL,
                                                      QL_WIFI_SCAN_DEFAULT_TIME))
    {
        QL_WIFISACN_DEMO_LOG("option set err");
        return;
    }
    if (QL_WIFISCAN_SUCCESS != ql_wifiscan_register_cb(ql_wifiscan_app_callback))
    {
        QL_WIFISACN_DEMO_LOG("register cb err");
        return;
    }
    if (QL_WIFISCAN_SUCCESS != ql_wifiscan_open())
    {
        QL_WIFISACN_DEMO_LOG("device open err");
        return;
    }
    if (QL_WIFISCAN_SUCCESS != ql_wifiscan_async())
    {
        QL_WIFISACN_DEMO_LOG("to do a async scan err");
        return;
    }
}

// complete flow to do one time scan: option->open->scan->close->output
void ql_wifiscan_synchro_complete_flow(void)
{
    if (QL_WIFISCAN_SUCCESS != ql_wifiscan_option_set(QL_WIFI_SCAN_DEFAULT_AP_CNT,
                                                      QL_WIFI_SCAN_DEFAULT_ROUND,
                                                      QL_WIFISCAN_CHANNEL_ALL,
                                                      QL_WIFI_SCAN_DEFAULT_TIME))
    {
        QL_WIFISACN_DEMO_LOG("option set err");
        return;
    }
    if (QL_WIFISCAN_SUCCESS != ql_wifiscan_register_cb(ql_wifiscan_app_callback))
    {
        QL_WIFISACN_DEMO_LOG("register cb err");
        return;
    }
    if (QL_WIFISCAN_SUCCESS != ql_wifiscan_open())
    {
        QL_WIFISACN_DEMO_LOG("device open err");
        return;
    }

    uint16_t ap_cnt = 0;
    ql_wifi_ap_info_s *ap_infos = (ql_wifi_ap_info_s *)calloc(1, QL_WIFI_SCAN_DEFAULT_AP_CNT * sizeof(ql_wifi_ap_info_s));
    if (NULL == ap_infos)
    {
        QL_WIFISACN_DEMO_LOG("memory calloc err");
    }

    if (QL_WIFISCAN_SUCCESS != ql_wifiscan_do(&ap_cnt, ap_infos))
    {
        free(ap_infos);
        QL_WIFISACN_DEMO_LOG("to do a scan err");
        return;
    }

    if (QL_WIFISCAN_SUCCESS != ql_wifiscan_close())
    {
        free(ap_infos);
        QL_WIFISACN_DEMO_LOG("close device err");
        return;
    }

    QL_WIFISACN_DEMO_LOG("ap_cnt=%d", ap_cnt);
    ql_wifiscan_ap_info_output(ap_cnt, ap_infos);

    free(ap_infos);
}

static void ql_wifiscan_app_thread(void *param)
{
    // ql_rtos_task_sleep_ms(15000);// sleep some time, then to do demo
    ql_wifiscan_synchro_complete_flow();
    ql_rtos_task_sleep_ms(100);
    ql_wifiscan_async_start(); // start a async scan, and it should trigger one event sent at callback

    while (1)
    {
        ql_event_t event = {0};
        if (ql_event_try_wait(&event) != 0)
        {
            continue;
        }

        if (event.id == QUEC_WIFISCAN_EVENT_ASYNC_IND)
        {
            ql_wifiscan_close(); // close the wifiscan device fot the async scan
            pub_mqtt(topic_gui, out);
            QL_WIFISACN_DEMO_LOG("\n thoat scan wifi\n");
            // ql_rtos_task_sleep_ms(15000);

            // next cycle demo
            // ql_wifiscan_synchro_complete_flow();
            // ql_rtos_task_sleep_ms(5000);
            // ql_rtos_semaphore_delete(mqtt_semp);
            cJSON_free((void *)out);
            cJSON_Delete(root);
            ql_rtos_task_delete(ql_wifiscan_app_task);
            // ql_wifiscan_async_start();
        }
    }
}

void ql_wifiscan_app_init(void)
{
    QlOSStatus err = QL_OSI_SUCCESS;

    err = ql_rtos_task_create(&ql_wifiscan_app_task, QL_WIFISACN_TASK_STACK_SIZE, APP_PRIORITY_LOW, "wifiscan DEMO", ql_wifiscan_app_thread, NULL, QL_WIFISACN_TASK_EVENT_CNT);
    if (err != QL_OSI_SUCCESS)
    {
        QL_WIFISACN_DEMO_LOG("demo task created failed");
    }
}
