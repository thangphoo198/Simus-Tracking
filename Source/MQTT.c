
#include "DataDefine.h"
#include <time.h>

#include "ql_api_osi.h"
#include "ql_api_nw.h"

#include "ql_log.h"
#include "ql_api_datacall.h"
#include "ql_lbs_client.h"

#include "MQTT.h"
#include "cJSON.h"
#include "ql_api_nw.h"
#include "ql_fs.h"
#include "main.h"
#define MQTT_CLIENT_IDENTITY "VT_00001"
#define MQTT_CLIENT_USER "esp32-iot"
#define MQTT_CLIENT_PASS "thang123"

#define MQTT_CLIENT_SRV_URL "mqtt://103.200.20.78:1883"        // onenet
#define MQTT_CLIENT_ONENET_SSL_URL "mqtt://103.200.20.78:1883" // onenet SSL

#define NSIM 0
char *SIM_info;
// publist

#define QL_MQTT_LOG DebugPrint
extern char *buff;
ql_task_t mqtt_task = NULL;
static ql_sem_t mqtt_semp;
static int mqtt_connected = 0;
mqtt_client_t mqtt_cli;
// extern uint8_t NSIM = 0;
uint16_t sim_cid;
int profile_idx = 1;
unsigned char cell_index = 0, csq = 0;
ql_nw_mode_type_e nw_mode = 0;
ql_nw_reg_status_info_s *reg_info;
// ql_nw_signal_strength_info_s *signal_info = (ql_nw_signal_strength_info_s *)calloc(1, sizeof(ql_nw_signal_strength_info_s));
ql_nw_operator_info_s *oper_info;
ql_nw_seclection_info_s *select_info;
ql_nw_nitz_time_info_s *nitz_info;
ql_nw_cell_info_s *cell_info;

#define QL_LBS_LOG DebugPrint
// #define QL_LBS_LOG_PUSH(msg, ...)	    DebugPrint

 mqtt_connect_result_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_e status)
{
    QL_MQTT_LOG("\rstatus: %d", status);
    if (status == 0)
    {
        mqtt_connected = 1;
    }
    ql_rtos_semaphore_release(mqtt_semp);
}

static void mqtt_state_exception_cb(mqtt_client_t *client)
{
    QL_MQTT_LOG("\rmqtt session abnormal disconnect\n");
    mqtt_connected = 0;
}

extern gui_sms(char *sdt, char *noidung);
extern ql_fota_http_app_init();
extern print_GPS(char *dat);

static void mqtt_requst_result_cb(mqtt_client_t *client, void *arg, int err)
{
    QL_MQTT_LOG("\rerr1: %d", err);

    ql_rtos_semaphore_release(mqtt_semp);
}

static void mqtt_inpub_data_cb(mqtt_client_t *client, void *arg, int pkt_id, const char *topic, const unsigned char *payload, unsigned short payload_len)
{
    QL_MQTT_LOG("topic:%s payload: %s\n", topic, payload);
    cJSON *pJsonRoot = cJSON_Parse(payload);
    cJSON *cmd = cJSON_GetObjectItem(pJsonRoot, "CMD");
    if (cmd)
    {
        if (cJSON_IsString(cmd))
        {
            char *val = cmd->valuestring;
            QL_MQTT_LOG("get CMD :%s", val);
            if (strcmp(val, "CMD_FOTA") == 0)
            {
                QL_MQTT_LOG("co lenh FOTA tu APP\n");

                ql_fota_http_app_init();
            }
            else if (strcmp(val, "GET_GPS") == 0)
            {
                float lat = g_gps_data.latitude;
                float lng = g_gps_data.longitude;
                uint8_t speed;
                cJSON *pRoot = cJSON_CreateObject();
                cJSON_AddStringToObject(pRoot, "RES", "GET_GPS");
                // cJSON_AddNumberToObject(pRoot, "lat", g_gps_data.latitude);
                speed = (int8_t)g_gps_data.gps_speed;
                int16_t signal = g_gps_data.avg_cnr;
                cJSON *pValue = cJSON_CreateObject();
                cJSON_AddNumberToObject(pValue, "lat", lat);
                cJSON_AddNumberToObject(pValue, "lng", lng);
                cJSON_AddNumberToObject(pValue, "speed", speed);
                cJSON_AddNumberToObject(pValue, "signal", signal);
                cJSON_AddItemToObject(pRoot, "GPS_INFO", pValue);
                char *GPS = cJSON_Print(pRoot);
                QL_MQTT_LOG("\n%s\n", GPS);
                if (lat != 0)
                {
                    pub_mqtt(topic_rec, GPS);
                }
                else
                {
                    pub_mqtt(topic_rec, "Khong co Du lieu GPS!");
                }
            }
            else if (strcmp(val, "GET_TEMP") == 0)
            {
                uint32_t temp = 0;
                ql_dev_get_temp_value(&temp);
                QL_MQTT_LOG("TEMP CHIP:  %d\n", temp);
            }
            else if (strcmp(val, "GET_VOL") == 0)
            {
                uint32_t vol = 0;
                ql_get_battery_vol(&vol);
                QL_MQTT_LOG("DIEN AP NGUON:  %d\n", vol);
            }
            else if (strcmp(val, "ENTER_SLEEP") == 0)
            {
                QL_MQTT_LOG("BAT CHE DO NGU SAU 5S:\n");
                ql_power_app_init();
                // ql_rtos_task_sleep_s(5);
                // ql_autosleep_enable(QL_ALLOW_SLEEP);
            }
            else if (strcmp(val, "SMS_PAIR") == 0)
            {
                cJSON *sdt = cJSON_GetObjectItem(pJsonRoot, "SDT");
                char *val1 = sdt->valuestring;
                cJSON *info = cJSON_GetObjectItem(pJsonRoot, "INFO");
                char *val2 = info->valuestring;
                gui_sms(val1, val2);
            }
            else if (strcmp(val, "GET_SIM") == 0)
            {

                // char buff[100];
                // sprintf(buff,"ret=0x%x, csq:%d name:%s %s", ret, csq,oper_info->long_oper_name,oper_info->short_oper_name);
                // QL_MQTT_LOG(buff);
                // read_sim_info();

                get_sim();
                // ql_rtos_task_sleep_ms(200);
                 pub_mqtt(topic_rec,SIM_info);
                 //ql_mqtt_connect(&mqtt_cli, MQTT_CLIENT_SRV_URL, mqtt_connect_result_cb, NULL, (const struct mqtt_connect_client_info_t *)&client_info, mqtt_state_exception_cb);
                // ret = ql_nw_get_nitz_time_info(nitz_info);
                // QL_MQTT_LOG("ret=0x%x, nitz_time:%s, abs_time:%ld", ret, nitz_info->nitz_time, nitz_info->abs_time);
            }
            else if (strcmp(val, "SMS_DELALL") == 0)
            {
                delete_all_sms();
            }
            else if (strcmp(val, "NWM_SAVE") == 0)
            {
                cJSON *info = cJSON_GetObjectItem(pJsonRoot, "INFO");
                char *val2 = info->valuestring;
                int err = ql_cust_nvm_fwrite(val2, strlen(val2), 1);
                if (err)
                {
                    QL_MQTT_LOG("Ghi thanh cong\n");
                }
            }
            else if (strcmp(val, "NWM_READ") == 0)
            {
                char buffer[100] = {0};
                int err = ql_cust_nvm_fread(buffer, 100, 1);
                if (err)
                {
                    QL_MQTT_LOG("du lieu doc dc:%s", buffer);
                }
            }
            else if (strcmp(val, "SCAN_WIFI") == 0)
            {
                ql_wifiscan_app_init();
            }
        }
    }
    cJSON_Delete(pJsonRoot);
}

void get_sim()
{
    int ret;
    // ret = ql_nw_get_mode(NSIM, &nw_mode);
    // QL_MQTT_LOG("ret=0x%x, nw_mode:%d", ret, nw_mode);

    // ret = ql_nw_get_reg_status(NSIM, reg_info);
    // QL_MQTT_LOG("ret=0x%x", ret);
    // QL_MQTT_LOG("voice: state:%d, lac:0x%X, cid:0x%X, act:%d", reg_info->voice_reg.state, reg_info->voice_reg.lac, reg_info->voice_reg.cid, reg_info->voice_reg.act);
    // QL_MQTT_LOG("data:  state:%d, lac:0x%X, cid:0x%X, act:%d", reg_info->data_reg.state, reg_info->data_reg.lac, reg_info->data_reg.cid, reg_info->data_reg.act);

    ret = ql_nw_get_csq(NSIM, &csq);

    // ret = ql_nw_get_signal_strength(NSIM, signal_info);
    // QL_MQTT_LOG("ret=0x%x, rssi:%d, bitErrorRate:%d, rsrp:%d, rsrq:%d",
    //                ret, signal_info->rssi, signal_info->bitErrorRate,
    //                signal_info->rsrp, signal_info->rsrq);

    ret = ql_nw_get_operator_name(NSIM, oper_info);
    QL_MQTT_LOG("ret=0x%x, long_oper_name:%s, short_oper_name:%s, mcc:%s, mnc:%s",
                ret, oper_info->long_oper_name, oper_info->short_oper_name, oper_info->mcc, oper_info->mnc);

    //     ret = ql_nw_get_selection(NSIM, select_info);
    //    QL_MQTT_LOG("ret=0x%x, nw_selection_mode:%d, mcc:%s, mnc:%s, act:%d",
    //                    ret, select_info->nw_selection_mode, select_info->mcc, select_info->mnc, select_info->act);

    //     ret = ql_nw_get_nitz_time_info(nitz_info);
    //    QL_MQTT_LOG("ret=0x%x, nitz_time:%s, abs_time:%ld", ret, nitz_info->nitz_time, nitz_info->abs_time);

    ret = ql_nw_get_cell_info(NSIM, cell_info);
    QL_MQTT_LOG("ret=0x%x", ret);

    if (cell_info->gsm_info_valid)
    {
        for (cell_index = 0; cell_index < cell_info->gsm_info_num; cell_index++)
        {
            QL_MQTT_LOG("Cell_%d [GSM] flag:%d, cid:0x%X, mcc:%d, mnc:%02d, lac:0x%X, arfcn:%d, bsic:%d, rssi:%d",
                        cell_index,
                        cell_info->gsm_info[cell_index].flag,
                        cell_info->gsm_info[cell_index].cid,
                        cell_info->gsm_info[cell_index].mcc,
                        cell_info->gsm_info[cell_index].mnc,
                        cell_info->gsm_info[cell_index].lac,
                        cell_info->gsm_info[cell_index].arfcn,
                        cell_info->gsm_info[cell_index].bsic,
                        cell_info->gsm_info[cell_index].rssi);
        }

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
        QL_MQTT_LOG("\n%s\n", SIM_info);
        // cJSON_Delete(pRoot);
        // cJSON_Delete(pValue);
    }
    else if (cell_info->lte_info_valid)
    {
        for (cell_index = 0; cell_index < cell_info->lte_info_num; cell_index++)
        {
            QL_MQTT_LOG("Cell_%d [LTE] flag:%d, cid:0x%X, mcc:%d, mnc:%02d, tac:0x%X, pci:%d, earfcn:%d, rssi:%d",
                        cell_index,
                        cell_info->lte_info[cell_index].flag,
                        cell_info->lte_info[cell_index].cid,
                        cell_info->lte_info[cell_index].mcc,
                        cell_info->lte_info[cell_index].mnc,
                        cell_info->lte_info[cell_index].tac,
                        cell_info->lte_info[cell_index].pci,
                        cell_info->lte_info[cell_index].earfcn,
                        cell_info->lte_info[cell_index].rssi);
        }

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
        ;
        QL_MQTT_LOG("\n%s\n", SIM_info);
        // cJSON_Delete(pRoot);
        // cJSON_Delete(pValue);
    }
}

static void mqtt_disconnect_result_cb(mqtt_client_t *client, void *arg, int err)
{
    QL_MQTT_LOG("\rerr2: %d", err);

    ql_rtos_semaphore_release(mqtt_semp);
}

void pub_mqtt(char *topic, char *mess)
{
    if (mqtt_connected == 1)
    {

        if (ql_mqtt_publish(&mqtt_cli, topic, mess, strlen(mess), 0, 0, mqtt_requst_result_cb, NULL) == MQTTCLIENT_WOUNDBLOCK)
        {
            QL_MQTT_LOG("\rTHANHCONG CHO KET QUA!\n");
            ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
        }
        else
        {
            QL_MQTT_LOG("that bai\n");
        }
    }
    else
    {
        QL_MQTT_LOG("FAIL: MQTT DISCONNECTED!");
    }
}
static void mqtt_app_thread(void *arg)
{
    int ret = 0;
    int i = 0, run_num = 1;
    ql_data_call_info_s info;
    char ip4_addr_str[16] = {0};
    struct mqtt_connect_client_info_t client_info = {0};
    int is_user_onenet = 0;
    oper_info = (ql_nw_operator_info_s *)calloc(1, sizeof(ql_nw_operator_info_s));
    // ql_nw_signal_strength_info_s *signal_info = (ql_nw_signal_strength_info_s *)calloc(1, sizeof(ql_nw_signal_strength_info_s));
    select_info = (ql_nw_seclection_info_s *)calloc(1, sizeof(ql_nw_seclection_info_s));
    nitz_info = (ql_nw_nitz_time_info_s *)calloc(1, sizeof(ql_nw_nitz_time_info_s));
    cell_info = (ql_nw_cell_info_s *)calloc(1, sizeof(ql_nw_cell_info_s));
    ql_rtos_task_sleep_s(10);
    ql_rtos_semaphore_create(&mqtt_semp, 0);

    char *client_id = (char *)malloc(256);
    char *client_user = (char *)malloc(256);
    char *client_pass = (char *)malloc(256);

    while ((ret = ql_network_register_wait(NSIM, 120)) != 0 && i < 10)
    {
        i++;
        ql_rtos_task_sleep_s(1);
    }
    if (ret == 0)
    {
        i = 0;
        QL_MQTT_LOG("\r====network registered!!!!====\n");
    }
    else
    {
        QL_MQTT_LOG("\r====network register failure!!!!!====\n");
        // goto exit;
    }

    ql_set_data_call_asyn_mode(NSIM, profile_idx, 0);

    QL_MQTT_LOG("\r===start data call====");
    ret = ql_start_data_call(NSIM, profile_idx, QL_PDP_TYPE_IP, "v-internet", NULL, NULL, 0);
    QL_MQTT_LOG("\r===data call result:%d", ret);
    if (ret != 0)
    {
        QL_MQTT_LOG("\r====data call failure!!!!=====");
    }
    memset(&info, 0x00, sizeof(ql_data_call_info_s));

    ret = ql_get_data_call_info(NSIM, profile_idx, &info);
    if (ret != 0)
    {
        QL_MQTT_LOG("\rql_get_data_call_info ret: %d", ret);
        ql_stop_data_call(NSIM, profile_idx);
        // goto exit;
    }
    QL_MQTT_LOG("\rinfo->profile_idx: %d", info.profile_idx);
    QL_MQTT_LOG("\rinfo->ip_version: %d", info.ip_version);

    QL_MQTT_LOG("\rinfo->v4.state: %d", info.v4.state);
    inet_ntop(AF_INET, &info.v4.addr.ip, ip4_addr_str, sizeof(ip4_addr_str));
    QL_MQTT_LOG("\rinfo.v4.addr.ip: %s\r\n", ip4_addr_str);

    inet_ntop(AF_INET, &info.v4.addr.pri_dns, ip4_addr_str, sizeof(ip4_addr_str));
    QL_MQTT_LOG("\rinfo.v4.addr.pri_dns: %s\r\n", ip4_addr_str);

    inet_ntop(AF_INET, &info.v4.addr.sec_dns, ip4_addr_str, sizeof(ip4_addr_str));
    QL_MQTT_LOG("\rinfo.v4.addr.sec_dns: %s\r\n", ip4_addr_str);

    while (1)
    {
        int case_id = 0;
        if (is_user_onenet != 1)
        {
            case_id = run_num % 2;
        }

        int ret = MQTTCLIENT_SUCCESS;

        if (QL_DATACALL_SUCCESS != ql_bind_sim_and_profile(NSIM, profile_idx, &sim_cid))
        {
            QL_MQTT_LOG("\rnSim or profile_idx is invalid!!!!\n");
            break;
        }

        if (ql_mqtt_client_init(&mqtt_cli, sim_cid) != MQTTCLIENT_SUCCESS)
        {
            QL_MQTT_LOG("\rmqtt client init failed!!!!\n");
            break;
        }

        QL_MQTT_LOG("\rmqtt_cli:%d", mqtt_cli);

        client_info.keep_alive = 60;
        client_info.clean_session = 1;
        client_info.will_qos = 0;
        client_info.will_retain = 0;
        client_info.will_topic = NULL;
        client_info.will_msg = NULL;
        client_info.client_id = MQTT_CLIENT_IDENTITY;
        client_info.client_user = MQTT_CLIENT_USER;
        client_info.client_pass = MQTT_CLIENT_PASS;
        QL_MQTT_LOG("\rconnect ssl %d onenet mode %d\n", case_id, is_user_onenet);
        if (case_id == 0)
        {
            client_info.ssl_cfg = NULL;
            ret = ql_mqtt_connect(&mqtt_cli, MQTT_CLIENT_SRV_URL, mqtt_connect_result_cb, NULL, (const struct mqtt_connect_client_info_t *)&client_info, mqtt_state_exception_cb);
        }
        else
        {
            struct mqtt_ssl_config_t quectel_ssl_cfg = {
                .ssl_ctx_id = 1,
                .verify_level = MQTT_SSL_VERIFY_NONE,
                .cacert_path = NULL,
                .client_cert_path = NULL,
                .client_key_path = NULL,
                .client_key_pwd = NULL};
            client_info.ssl_cfg = &quectel_ssl_cfg;
            ret = ql_mqtt_connect(&mqtt_cli, MQTT_CLIENT_ONENET_SSL_URL, mqtt_connect_result_cb, NULL, (const struct mqtt_connect_client_info_t *)&client_info, mqtt_state_exception_cb);
        }
        if (ret == MQTTCLIENT_WOUNDBLOCK)
        {
            QL_MQTT_LOG("\r====wait connect result:\n");
            ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
            if (mqtt_connected == 0)
            {
                ql_mqtt_client_deinit(&mqtt_cli);
                break;
            }
        }
        else
        {
            QL_MQTT_LOG("\r===mqtt connect failed ,ret = %d\n", ret);
            break;
        }

        ql_mqtt_set_inpub_callback(&mqtt_cli, mqtt_inpub_data_cb, NULL);
        while (mqtt_connected == 1)
        {
            if (ql_mqtt_sub_unsub(&mqtt_cli, "EC200U_REMOTE", 1, mqtt_requst_result_cb, NULL, 1) == MQTTCLIENT_WOUNDBLOCK)
            {
                QL_MQTT_LOG("dang sub topic:EC200U_REMOTE");
                ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
            }

            ql_rtos_task_sleep_ms(5000);
        }
    }
    if (mqtt_connected == 1 && ql_mqtt_disconnect(&mqtt_cli, mqtt_disconnect_result_cb, NULL) == MQTTCLIENT_WOUNDBLOCK)
    {
        QL_MQTT_LOG("\r=====wait disconnect result");
        ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
        ret = ql_mqtt_connect(&mqtt_cli, MQTT_CLIENT_SRV_URL, mqtt_connect_result_cb, NULL, (const struct mqtt_connect_client_info_t *)&client_info, mqtt_state_exception_cb);
    }
    //  QL_MQTT_LOG("\r==============mqtt_client_test[%d] end=======%x=========\n", run_num, &mqtt_cli);
    ql_mqtt_client_deinit(&mqtt_cli);
    mqtt_connected = 0;
    ql_rtos_task_sleep_s(1);
exit:
    if (is_user_onenet == 1)
    {
        free((void *)client_id);
        free((void *)client_user);
        free((void *)client_pass);
        client_id = NULL;
        client_user = NULL;
        client_pass = NULL;
    }

    ql_rtos_semaphore_delete(mqtt_semp);
    ql_rtos_task_delete(mqtt_task);

    return;
}

extern delete_all_sms();
int ql_mqtt_app_init(void)
{
    QlOSStatus err = QL_OSI_SUCCESS;

    err = ql_rtos_task_create(&mqtt_task, 16 * 1024, 24, "mqtt_app", mqtt_app_thread, NULL, 5);
    if (err != QL_OSI_SUCCESS)
    {
        QL_MQTT_LOG("\rmqtt_app init failed");
    }

    return err;
}