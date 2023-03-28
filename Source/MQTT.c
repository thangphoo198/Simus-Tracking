
#include "DataDefine.h"
#include <time.h>

#include "ql_api_osi.h"
#include "ql_api_nw.h"

#include "ql_log.h"
#include "ql_api_datacall.h"
#include "ql_lbs_client.h"

#include "MQTT.h"
#include "ql_power.h"
#include "cJSON.h"
#include "ql_api_nw.h"
#include "ql_fs.h"
#include "main.h"
#define MQTT_CLIENT_IDENTITY "VT_00001"
#define MQTT_CLIENT_USER "esp32-iot"
#define MQTT_CLIENT_PASS "thang123"

#define MQTT_CLIENT_ONENET_DEVICENAME "SmartDevice"
#define MQTT_CLIENT_ONENET_VERSION "2022-11-22"

#define MQTT_CLIENT_SRV_URL "mqtt://103.200.20.78:1883"        // onenet
#define MQTT_CLIENT_ONENET_SSL_URL "mqtt://103.200.20.78:1883" // onenet SSL

#define MQTT_TOPIC_SUB "MQTT_SUB"
#define MQTT_TOPIC_PUB "MQTT_PUB"

#define NSIM 0
// publist

#define QL_MQTT_LOG DebugPrint
extern char *buff;
ql_task_t mqtt_task = NULL;
static ql_sem_t mqtt_semp;
static int mqtt_connected = 0;
mqtt_client_t mqtt_cli;
ql_nw_cell_info_s cell_info;
ql_nw_seclection_info_s select_info;
// extern uint8_t NSIM = 0;
uint16_t sim_cid;
int profile_idx = 1;

#define QL_LBS_LOG DebugPrint
// #define QL_LBS_LOG_PUSH(msg, ...)	    DebugPrint

static ql_task_t lbs_task = NULL;

static lbs_client_hndl lbs_cli = 0;
static ql_sem_t lbs_semp;

static lbs_basic_info_t basic_info = {
    .type = 1,
    .encrypt = 1,
    .key_index = 1,
    .pos_format = 1,
    .loc_method = 4};

static lbs_auth_info_t auth_info = {
    .user_name = "quectel",
    .user_pwd = "123456",
    .token = "1111111122222222", //"A693EDC90C42E624",
    .imei = "861687000001091",
    .rand = 2346};

static lbs_cell_info_t lbs_cell_info[] = {
    {.radio = 3,
     .mcc = 460,
     .mnc = 0,
     .lac_id = 0x550B,
     .cell_id = 0xF2D4A48,
     .signal = 0,
     .tac = 3,
     .bcch = 0,
     .bsic = 0,
     .uarfcndl = 0,
     .psc = 0,
     .rsrq = 0,
     .pci = 0,
     .earfcn = 0}};

// static void lbs_result_cb(lbs_response_data_t *response_data)
// {
//     int i = 0;
//     if (NULL == response_data || lbs_cli != response_data->hndl)
//     {
//         return;
//     }

//     QL_LBS_LOG("lbs result: %08X", response_data->result);
//     if (response_data->result == QL_LBS_OK)
//     {
//         for (i = 0; i < response_data->pos_num; i++)
//         {
//             QL_LBS_LOG("Location[%d]: %f, %f, %d\n", i, response_data->pos_info[i].longitude,
//                        response_data->pos_info[i].latitude, response_data->pos_info[i].accuracy);
//         }
//     }
//     ql_rtos_semaphore_release(lbs_semp);
// }

static void mqtt_connect_result_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_e status)
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
//extern pub_GPS();
// extern print_GPS(char *dat);

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
            else if (strcmp(val, "GET_VERSION") == 0)
            {
                char version_buf[128] = {0};
                ql_dev_get_firmware_version(version_buf, sizeof(version_buf));
                QL_MQTT_LOG("Phien phan mem hien tai:  %s\n", version_buf);
                if (mqtt_connected == 1)
                {
                    ql_mqtt_publish(&mqtt_cli, "EC200U_REC", version_buf, strlen(version_buf), 0, 0, mqtt_requst_result_cb, NULL == MQTTCLIENT_WOUNDBLOCK);
                }
            }
            else if (strcmp(val, "SMS_KTTK") == 0)
            {
                
                char version_buf[128] = {0};
                gui_sms("191", "KTTK");
                delete_all_sms();
            }
            else if (strcmp(val, "GET_GPS") == 0)
            {
                //char buf[256] = {0};
               // print_GPS(buf);
               // pub_GPS();
               char buf[30];
               sprintf(buf,"lat:%.6f",g_gps_data.latitude);
               QL_MQTT_LOG(buf);
            }

            else if (strcmp(val, "GET_MODEL") == 0)
            {
                char model_buf[128] = {0};
                ql_dev_get_model(model_buf, sizeof(model_buf));
                QL_MQTT_LOG("MODEL CHIP:  %s\n", model_buf);
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
                ql_rtos_task_sleep_s(5);
                ql_autosleep_enable(QL_ALLOW_SLEEP);
            }
            else if (strcmp(val, "SMS_PAIR") == 0)
            {
                cJSON *sdt = cJSON_GetObjectItem(pJsonRoot, "SDT");
                char *val1 = sdt->valuestring;

                cJSON *info = cJSON_GetObjectItem(pJsonRoot, "INFO");
                char *val2 = info->valuestring;

                gui_sms(val1, val2);
            }
            else if (strcmp(val, "GET_SN") == 0)
            {
                char serial_buf[128] = {0};
                ql_dev_get_sn(&serial_buf, sizeof(serial_buf), 0);
                ql_dev_get_model(serial_buf, sizeof(serial_buf));
                QL_MQTT_LOG("Serial Number CHIP:  %s\n", serial_buf);
                if (mqtt_connected == 1)
                {
                    ql_mqtt_publish(&mqtt_cli, "EC200U_REMOTE", serial_buf, strlen(serial_buf), 0, 0, mqtt_requst_result_cb, NULL == MQTTCLIENT_WOUNDBLOCK);
                }
            }
            else if (strcmp(val, "GET_SIM") == 0)
            {
                unsigned char csq = 0;
                int ret;
               // ql_nw_get_csq(NSIM, &csq);
               // QL_MQTT_LOG("ret=0x%x, csq:%d", ret, csq);
                read_sim_info();
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

            ql_rtos_task_sleep_ms(10000);
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

void read_sim_info()
{
    lbs_option_t user_option;
    if (ql_nw_get_cell_info(NSIM, &cell_info) != QL_NW_SUCCESS)
    {
        QL_LBS_LOG("===============lbs get cell info fail===============\n");
    }
    int ret = ql_nw_get_selection(NSIM, &select_info);
    if (ret != 0)
    {
        QL_LBS_LOG("ql_nw_get_selection ret: %d", ret);
    }
    QL_LBS_LOG("nw_act_type=%d", select_info.act);
    if (select_info.act == QL_NW_ACCESS_TECH_GSM)
    {
        QL_LBS_LOG("\n mang GMS\n");
        lbs_cell_info[0].radio = 1;
        lbs_cell_info[0].mcc = cell_info.gsm_info[0].mcc;
        lbs_cell_info[0].mnc = cell_info.gsm_info[0].mnc;
        lbs_cell_info[0].cell_id = cell_info.gsm_info[0].cid;
        lbs_cell_info[0].lac_id = cell_info.gsm_info[0].lac;
        lbs_cell_info[0].bsic = cell_info.gsm_info[0].bsic;
        lbs_cell_info[0].uarfcndl = cell_info.gsm_info[0].arfcn;
        lbs_cell_info[0].bcch = cell_info.gsm_info[0].arfcn;
        lbs_cell_info[0].signal = cell_info.gsm_info[0].rssi;
    }
    else if (select_info.act == QL_NW_ACCESS_TECH_E_UTRAN)
    {
        QL_LBS_LOG("\n mang LTE\n");
        lbs_cell_info[0].radio = 3;
        lbs_cell_info[0].mcc = cell_info.lte_info[0].mcc;
        lbs_cell_info[0].mnc = cell_info.lte_info[0].mnc;
        lbs_cell_info[0].cell_id = cell_info.lte_info[0].cid;
        lbs_cell_info[0].lac_id = cell_info.lte_info[0].tac;
        lbs_cell_info[0].tac = cell_info.lte_info[0].tac;
        lbs_cell_info[0].pci = cell_info.lte_info[0].pci;
        lbs_cell_info[0].earfcn = cell_info.lte_info[0].earfcn;
        lbs_cell_info[0].bcch = cell_info.lte_info[0].earfcn;
        lbs_cell_info[0].signal = cell_info.lte_info[0].rssi;
    }
    else
    {
        QL_LBS_LOG("\n Thoat \n");
    }
    char buff[100];
    sprintf(buff, "cell infoinfo: radio=%d,mcc=%d,mnc=%d,lac_id=%x,cell_id=%x,signal=%d",
            lbs_cell_info[0].radio, lbs_cell_info[0].mcc, lbs_cell_info[0].mnc, (int)lbs_cell_info[0].lac_id, (int)lbs_cell_info[0].cell_id, lbs_cell_info[0].signal);
    QL_LBS_LOG(buff);
    pub_mqtt("EC200U_REC", buff);
    // memset(&user_option, 0x00, sizeof(lbs_option_t));
    // user_option.pdp_cid = profile_idx;
    // user_option.sim_id = 0;
    // user_option.req_timeout = 60;
    // user_option.basic_info = &basic_info;
    // user_option.auth_info = &auth_info;
    // user_option.cell_num = 1;
    // user_option.cell_info = &lbs_cell_info[0];

    // if (QL_LBS_OK == ql_lbs_get_position(&lbs_cli, "wwww.opencellid.org", &user_option, lbs_result_cb, NULL))
    // {
    //     ql_rtos_semaphore_wait(lbs_semp, QL_WAIT_FOREVER);
    // }
    // else
    // {
    //     QL_LBS_LOG("lbs failed");
    // }
   // ql_rtos_semaphore_delete(lbs_semp);
    // ql_rtos_task_sleep_s(1);
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