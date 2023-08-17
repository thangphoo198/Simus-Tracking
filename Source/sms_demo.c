/*================================================================
  Copyright (c) 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
=================================================================*/
/*=================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

WHEN              WHO         WHAT, WHERE, WHY
------------     -------     -------------------------------------------------------------------------------
10/10/2020  marvin          create

=================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ql_api_common.h"
#include "ql_api_osi.h"
#include "ql_api_sms.h"
#include "ql_log.h"
#include "DataDefine.h"
#include "main.h"
#include "ql_power.h"

ql_task_t sms_task = NULL;
ql_sem_t sms_init_sem = NULL;
ql_sem_t sms_list_sem = NULL;
ql_queue_t new_sms = NULL;
#define QL_SMS_LOG DebugPrint
uint8_t nSim = 0;
int16_t INDEX_SMS = 0;
extern pub_mqtt(char *topic, char *mess);

void read_sms(uint8_t index)
{
    // Read one messages in SIM
    uint16_t msg_len = 512;
    char *msg = malloc(msg_len);
    if (msg == NULL)
    {
        QL_SMS_LOG("malloc ql_sms_msg_s fail");
        // goto exit;
    }
    memset(msg, 0, msg_len);
    if (QL_SMS_SUCCESS == ql_sms_read_msg(nSim, index, msg, msg_len, TEXT))
    {
        QL_SMS_LOG("SMS=> %s\n Length:%d", msg, strlen(msg));

        char *token = strtok(msg, ",");
        QL_SMS_LOG("\nSDT: %s\n", token);
        token = strtok(NULL, ",");
        token = strtok(NULL, ",");
        token = strtok(NULL, "V");
        QL_SMS_LOG("\nNOI DUNG: %s\n", token);
        if(strstr(token,"SETIP")!=NULL)
        {
             char *IP = strtok(token, ","); //IP
             IP= strtok(NULL, ","); //broker
             QL_SMS_LOG("\n broker: %s ",IP);
             char buff[30]={0};
             sprintf(buff,"mqtt://%s:",IP);
             IP = strtok(NULL, ","); //broker
             xoamotkytu(IP,'\"');
             char buff_sever[50]={0};
             strcpy(buff_sever, buff);
             strcat(buff_sever, IP);
             QL_SMS_LOG("MQTT: %s \n",buff_sever);
            //  sprintf(buff,"\n mqtt://%s:%s\n",IP); //mqtt://14.225.254.159:1883
        }
        // if(strcmp(token,"RESET")==0)
        // {
        //     ql_power_reset(RESET_NORMAL);
        // }
        // else if(strcmp(token,"FOTA")==0)
        // {
        //     ql_fota_http_app_init();
        // }
        
        // Lấy ra toàn bộ token
        // for (size_t i = 0; i < 3; i++)
        // {
        //     printf(" %s\n", token); // In mỗi token ra
        //     token = strtok(NULL, " ");
        //     /* code */
        // }
        pub_mqtt(topic_gui, token);
    }
    else
    {
        QL_SMS_LOG("read sms FAIL xoa SMS TASK\n");
        ql_rtos_task_delete(NULL);
    }
    ql_rtos_task_sleep_ms(100);

    if (msg)
        free(msg);
}
void read_all_sms()
{
    // Read all message in SIM
    ql_sms_set_storage(nSim, SM, SM, SM); // set sms storage as SIM.
    if (QL_SMS_SUCCESS == ql_sms_read_msg_list(nSim, TEXT))
    {
        if (ql_rtos_semaphore_wait(sms_list_sem, QL_WAIT_FOREVER))
        {
            QL_SMS_LOG("sms_list_sem time out");
        }
    }
    else
    {
        QL_SMS_LOG("get msg list FAIL");
    }
}

void user_sms_event_callback(uint8_t nSim, int event_id, void *ctx)
{
    switch (event_id)
    {
    case QL_SMS_INIT_OK_IND: {
        QL_SMS_LOG("QL_SMS_INIT_OK_IND");
        ql_rtos_semaphore_release(sms_init_sem);
        break;
    }
    case QL_SMS_NEW_MSG_IND: {
        QL_SMS_LOG("\nNew SMS: => ");
        ql_sms_new_s *msg = (ql_sms_new_s *)ctx;
        QL_SMS_LOG("sim=%d, index=%d, storage memory=%d\n", nSim, msg->index, msg->mem);
        INDEX_SMS = msg->index;
        if (ql_rtos_queue_release(new_sms, 10, &INDEX_SMS, QL_WAIT_FOREVER) == QL_OSI_SUCCESS)
        {
            QL_SMS_LOG("GUI HANG DOI OK => \n");
        }
        else
        {
            QL_SMS_LOG("GUI HANG DOI FAILED => \n");
        }
        break;
    }
    case QL_SMS_LIST_IND: {
        ql_sms_msg_s *msg = (ql_sms_msg_s *)ctx;
        QL_SMS_LOG("sim=%d,index=%d, msg = %s", nSim, msg->index, msg->buf);
        break;
    }
    case QL_SMS_LIST_END_IND: {
        QL_SMS_LOG("QL_SMS_LIST_END_IND");
        ql_rtos_semaphore_release(sms_list_sem);

        break;
    }
    case QL_SMS_MEM_FULL_IND: {
        ql_sms_new_s *msg = (ql_sms_new_s *)ctx;
        QL_SMS_LOG("QL_SMS_MEM_FULL_IND sim=%d, memory=%d", nSim, msg->mem);
        break;
    }
    default:
        break;
    }
}

void gui_sms(char *sdt, char *noidung)
{
    if (QL_SMS_SUCCESS == ql_sms_send_msg(0, sdt, noidung, GSM))
    {
        QL_SMS_LOG("ql_sms_send_msg OK");
    }
    else
    {
        QL_SMS_LOG("send SMS FAIL\n");
        QL_SMS_LOG("ql_sms_send_msg FAIL");
    }
}
void delete_all_sms()
{
    // Delete message.
    if (QL_SMS_SUCCESS == ql_sms_delete_msg_ex(nSim, 0, QL_SMS_DEL_ALL))
    {
        QL_SMS_LOG("\ndelete msg OK\n");
    }
    else
    {
        QL_SMS_LOG("\ndelete sms FAIL\n");
    }
}

void sms_demo_task(void *param)
{
   // char addr[20] = {0};
    ql_sms_callback_register(user_sms_event_callback);
    // wait sms ok
    if (ql_rtos_semaphore_wait(sms_init_sem, QL_WAIT_FOREVER))
    {
        QL_SMS_LOG("Waiting for SMS init timeout");
    }

    ql_sms_stor_info_s stor_info;
    if (QL_SMS_SUCCESS == ql_sms_get_storage_info(nSim, &stor_info))
    {
        QL_SMS_LOG("ql_sms_get_storage_info OK");
        QL_SMS_LOG("SM used=%u,SM total=%u,SM unread=%u,ME used=%u,ME total=%u,ME unread=%u, newSmsStorId=%u",
                   stor_info.usedSlotSM, stor_info.totalSlotSM, stor_info.unReadRecordsSM,
                   stor_info.usedSlotME, stor_info.totalSlotME, stor_info.unReadRecordsME,
                   stor_info.newSmsStorId);
    }
    else
    {
        QL_SMS_LOG("ql_sms_get_storage_info FAIL");
    }
    delete_all_sms();
    ql_rtos_task_sleep_s(1);
    while (1)
    {
        uint16_t rec = 0;
        if (ql_rtos_queue_wait(new_sms, &rec, sizeof(uint32_t), 0) == QL_OSI_SUCCESS)
        {

            QL_SMS_LOG("co SMS moi tai:%d\n", rec);
            read_sms(rec);
            ql_rtos_task_sleep_ms(100);
        }
        else
        {
            // QL_SMS_LOG("Dang doi SMS\n");
            ql_rtos_task_sleep_ms(500);
        }

    }

exit:

    ql_rtos_task_delete(NULL);
}

QlOSStatus ql_sms_app_init(void)
{
    QlOSStatus err = QL_OSI_SUCCESS;

    err = ql_rtos_task_create(&sms_task, 4 * 4096, APP_PRIORITY_NORMAL, "SMS_TASK", sms_demo_task, NULL, 2);
    if (err != QL_OSI_SUCCESS)
    {
        QL_SMS_LOG("sms_task created failed, ret = 0x%x", err);
    }

    err = ql_rtos_semaphore_create(&sms_init_sem, 0);
    if (err != QL_OSI_SUCCESS)
    {
        QL_SMS_LOG("sms_init_sem created failed, ret = 0x%x", err);
    }

    err = ql_rtos_semaphore_create(&sms_list_sem, 0);
    if (err != QL_OSI_SUCCESS)
    {
        QL_SMS_LOG("sms_init_sem created failed, ret = 0x%x", err);
    }

    err = ql_rtos_queue_create(&new_sms, 10, 20);
    if (err != QL_OSI_SUCCESS)
    {
        QL_SMS_LOG("queue created failed, ret = 0x%x", err);
    }

    return err;
}
