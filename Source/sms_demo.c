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
#include "ql_uart.h"
#include "DataDefine.h"

ql_task_t sms_task = NULL;
ql_sem_t sms_init_sem = NULL;
ql_sem_t sms_list_sem = NULL;

#define QL_SMS_LOG DebugPrint
uint8_t nSim = 0;
uint8_t INDEX_SMS = 0;

#define event_sms 999
void read_sms(uint8_t index)
{
    int nSim = 0;
    // Read one messages in SIM
    uint16_t msg_len = 512;
    char *msg = malloc(msg_len);
    if (msg == NULL)
    {
        QL_SMS_LOG("malloc ql_sms_msg_s fail");
        // goto exit;
    }
    memset(msg, 0, msg_len);
    // The first parameter specifies that SMS messages are read from SM
    ql_sms_set_storage(nSim, SM, SM, SM);

    ql_sms_mem_info_t sms_mem = {0};
    ql_sms_get_storage(nSim, &sms_mem);
    QL_SMS_LOG("mem1=%d, mem2=%d, mem3=%d", sms_mem.mem1, sms_mem.mem2, sms_mem.mem3);

    // Read SMS messages as text
    if (QL_SMS_SUCCESS == ql_sms_read_msg(nSim, index, msg, msg_len, TEXT))
    {
        QL_SMS_LOG("SMS=> %s\n", msg);
    }
    else
    {
        QL_SMS_LOG("read sms FAIL xoa SMS TASK\n");
        ql_rtos_task_delete(NULL);
    }
    // Read SMS messages as pdu
    //  memset(msg ,0 ,msg_len);
    //  if(QL_SMS_SUCCESS == ql_sms_read_msg(nSim,2, msg, msg_len, PDU)){
    //  	QL_SMS_LOG("read msg OK, msg=%s", msg);
    //  }else{
    //  	QL_SMS_LOG("read sms FAIL");
    //  }

    // uint8_t datalen = 0;
    // if(QL_SMS_SUCCESS == ql_sms_get_pdu_datalen((uint8_t *)msg, strlen(msg), 0, &datalen))
    // {
    // 	QL_SMS_LOG("pdu data len = %d",datalen);
    // }
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
        QL_SMS_LOG("New SMS: => ");
        ql_sms_new_s *msg = (ql_sms_new_s *)ctx;
        QL_SMS_LOG("sim=%d, index=%d, storage memory=%d\n", nSim, msg->index, msg->mem);
        INDEX_SMS = msg->index;
        SendEventToThread(sms_task, event_sms);
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
        ql_uart_write(QL_LOG_PORT_UART, "SMS OK", 5);
        QL_SMS_LOG("ql_sms_send_msg OK");
    }
    else
    {
        ql_uart_write(QL_LOG_PORT_UART, "SMS FAIL", 7);
        QL_SMS_LOG("ql_sms_send_msg FAIL");
    }
}
void delete_all_sms()
{
    // Delete message.
    if (QL_SMS_SUCCESS == ql_sms_delete_msg_ex(nSim, 0, QL_SMS_DEL_ALL))
    {
        QL_SMS_LOG("delete msg OK\n");
    }
    else
    {
        QL_SMS_LOG("delete sms FAIL\n");
    }
}

void sms_demo_task(void *param)
{
    ql_event_t event;
    char addr[20] = {0};
    QL_SMS_LOG("enter");
    ql_sms_callback_register(user_sms_event_callback);
    // wait sms ok
    if (ql_rtos_semaphore_wait(sms_init_sem, QL_WAIT_FOREVER))
    {
        QL_SMS_LOG("Waiting for SMS init timeout");
    }

    if (QL_SMS_SUCCESS == ql_sms_get_center_address(nSim, addr, sizeof(addr)))
    {
        QL_SMS_LOG("ql_sms_get_center_address OK, addr=%s\n", addr);
    }
    else
    {
        QL_SMS_LOG("ql_sms_get_center_address FAIL\n");
    }

    // Send English text message

    // Get how many SMS messages can be stored in the SIM card in total and how much storage is used
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
    // read_sms(3);
    delete_all_sms();
    ql_rtos_task_sleep_ms(100);
    uint8_t in_pre = 0;
    while (1)
    {

        // if(in_pre!=INDEX_SMS)
        // {

        //     QL_SMS_LOG("co SMS moi tai:%d\n",INDEX_SMS);
        //     read_sms(INDEX_SMS);
        //     ql_rtos_task_sleep_ms(100);
        //     in_pre=INDEX_SMS;
        //     //delete_all_sms();

        // }
        ql_event_try_wait(&event);
        switch (event.id)
        {

        case event_sms:
            QL_SMS_LOG("co SMS moi tai:%d\n", INDEX_SMS);
            read_sms(INDEX_SMS);
            break;
        default:
            break;

            ql_rtos_task_sleep_ms(100);
        }

    exit:

        ql_rtos_task_delete(NULL);
    }
}

QlOSStatus ql_sms_app_init(void)
{
    QlOSStatus err = QL_OSI_SUCCESS;

    err = ql_rtos_task_create(&sms_task, 4 * 4096, 24, "SMS_TASK", sms_demo_task, NULL, 2);
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

    return err;
}
