/*============================================================================
  Copyright (c) 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
 =============================================================================*/
/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.


WHEN        WHO            WHAT, WHERE, WHY
----------  ------------   ----------------------------------------------------

=============================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ql_api_osi.h"
#include "ql_log.h"
#include "ql_api_nw.h"
#include "DataDefine.h"
#include "cJSON.h"
#include "main.h"

#define QL_NW_DEMO_LOG DebugPrint
ql_task_t nw_task = NULL;
int ret = 0;
uint8_t sim_id = 0;
unsigned char cell_index = 0, csq = 0;

extern pub_mqtt(char *topic, char *mess);
static void nw_app_thread(void *arg)
{
    QlOSStatus err = 0;
    ql_nw_mode_type_e nw_mode = 0;
    ql_nw_operator_info_s *oper_info = (ql_nw_operator_info_s *)calloc(1, sizeof(ql_nw_operator_info_s));
    ql_nw_cell_info_s *cell_info = (ql_nw_cell_info_s *)calloc(1, sizeof(ql_nw_cell_info_s));

    if ((NULL == oper_info) || (NULL == cell_info))
    {
        QL_NW_DEMO_LOG("calloc fail");
        goto exit;
    }
    int ret;
    ret = ql_nw_get_csq(NSIM, &csq);
    ql_dev_get_imei(imei, 64, NSIM);
    ret = ql_nw_get_operator_name(NSIM, oper_info);
    ret = ql_nw_get_cell_info(NSIM, cell_info);
    if (cell_info->gsm_info_valid)
    {
        cJSON *pRoot = cJSON_CreateObject();
        cJSON_AddStringToObject(pRoot, "RES", "GET_SIM");
        cJSON *pValue = cJSON_CreateObject();
        cJSON_AddStringToObject(pValue, "imei", imei);
        cJSON_AddStringToObject(pValue, "sim_name", oper_info->long_oper_name);
        cJSON_AddStringToObject(pValue, "network", "GMS");
        cJSON_AddNumberToObject(pValue, "csq", csq);
        cJSON_AddNumberToObject(pValue, "mcc", cell_info->lte_info[0].mcc);
        cJSON_AddNumberToObject(pValue, "mnc", cell_info->lte_info[0].mnc);
        cJSON_AddNumberToObject(pValue, "cell_id", cell_info->lte_info[0].cid);
        cJSON_AddNumberToObject(pValue, "lac_id", cell_info->lte_info[0].tac);
        cJSON_AddItemToObject(pRoot, "SIM_INFO", pValue);
        SIM_info = cJSON_Print(pRoot);
        QL_NW_DEMO_LOG("\n%s\n", SIM_info);
        pub_mqtt(topic_rec,SIM_info);
        // cJSON_Delete(pRoot);
        // cJSON_Delete(pValue);
    }
    else if (cell_info->lte_info_valid)
    {
        cJSON *pRoot = cJSON_CreateObject();
        cJSON_AddStringToObject(pRoot, "RES", "GET_SIM");
        cJSON *pValue = cJSON_CreateObject();
        cJSON_AddStringToObject(pValue, "imei", imei);
        cJSON_AddStringToObject(pValue, "sim_name", oper_info->long_oper_name);
        cJSON_AddStringToObject(pValue, "network", "LTE");
        cJSON_AddNumberToObject(pValue, "csq", csq);
        cJSON_AddNumberToObject(pValue, "mcc", cell_info->lte_info[0].mcc);
        cJSON_AddNumberToObject(pValue, "mnc", cell_info->lte_info[0].mnc);
        cJSON_AddNumberToObject(pValue, "cell_id", cell_info->lte_info[0].cid);
        cJSON_AddNumberToObject(pValue, "lac_id", cell_info->lte_info[0].tac);
        cJSON_AddItemToObject(pRoot, "SIM_INFO", pValue);
        SIM_info = cJSON_Print(pRoot);
        QL_NW_DEMO_LOG("\n%s\n", SIM_info);
        pub_mqtt(topic_rec,SIM_info);
        // cJSON_Delete(pRoot);
        // cJSON_Delete(pValue);
    }
    // while(1)
    // {

    QL_NW_DEMO_LOG("====================nw demo end===================");
    //     }

exit:
    if (NULL != oper_info)
        free(oper_info);
    if (NULL != cell_info)
        free(cell_info);

    err = ql_rtos_task_delete(NULL);
    if (err != QL_OSI_SUCCESS)
    {
        QL_NW_DEMO_LOG("task deleted failed");
    }
}

void ql_nw_app_init(void)
{
    QlOSStatus err = QL_OSI_SUCCESS;

    err = ql_rtos_task_create(&nw_task, 1024 * 4, APP_PRIORITY_NORMAL, "QNWDEMO", nw_app_thread, NULL, 5);
    if (err != QL_OSI_SUCCESS)
    {
        QL_NW_DEMO_LOG("created task failed");
    }
}
