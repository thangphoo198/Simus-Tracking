
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
#include "ql_power.h"
#include "main.h"
#include "command.h"
#include "ql_adc.h"
#include "ql_gpio.h"
#include "ql_api_virt_at.h"
#include "ql_virt_at_demo.h"

#define NSIM 0
#define MQTT_CLIENT_IDENTITY "VT_00001"
char mqtt_client[50] = {0};
char *SIM_info;
// publist
u8_t ON_MQTT=1;
#define QL_MQTT_LOG DebugPrint
extern char *buff;
ql_task_t mqtt_task = NULL;
static ql_sem_t mqtt_semp;
static int mqtt_connected = 0;
mqtt_client_t mqtt_cli;
// extern uint8_t NSIM = 0;
uint16_t sim_cid;
int profile_idx = 1;
struct mqtt_connect_client_info_t client_info = {0};

#define QL_VIRT_AT_RX_BUFF_SIZE                1024

void put_kttk(char *buff)
{
        if(strlen(buff) > 50)
        {

            cJSON *pRoot = cJSON_CreateObject();
            cJSON_AddStringToObject(pRoot, "RES", "KTTK");
            xoamotkytu(buff,'\"');
            xoamotkytu(buff,'\r');
            xoamotkytu(buff,'\n');
            cJSON_AddStringToObject(pRoot, "DATA", buff);
            char *GPS_info = cJSON_Print(pRoot);
            QL_MQTT_LOG(GPS_info);        
            pub_mqtt(topic_gui, GPS_info);
        }
        else
        {
            QL_MQTT_LOG("size small!\n"); 
        }
}

void  xoamotkytu(char *a, char c)
{
    int n=strlen(a);
    for(int i=0;i<n;i++)
        if(a[i]==c)
        {
            for(int j=i--; j<n;j++)
                a[j]=a[j+1];
            n--;
        }
} 

void ql_virt_at0_notify_cb(unsigned int ind_type, unsigned int size)
{
    unsigned char *recv_buff = calloc(1, QL_VIRT_AT_RX_BUFF_SIZE+1);
    unsigned int real_size = 0;
    
    if(QUEC_VIRT_AT_RX_RECV_DATA_IND == ind_type)
    {
        memset(recv_buff, 0, QL_VIRT_AT_RX_BUFF_SIZE+1);
        real_size= MIN(size, QL_VIRT_AT_RX_BUFF_SIZE);
        ql_virt_at_read(QL_VIRT_AT_PORT_0, recv_buff, real_size);
        QL_MQTT_LOG("VAT0 => %s \n", recv_buff);
        put_kttk(recv_buff);
        //put_kttk(buff_kttk);

    }
    free(recv_buff);
    recv_buff = NULL;

}

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
    mqtt_connected = 0;
    QL_MQTT_LOG("\rmqtt session abnormal disconnect\n");
    QL_MQTT_LOG("\rthu ket noi lai\n");
    reconnect();
    
}

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
            // for(uint8_t i=0;i<strlen(val);i++)
            // {
            //     val1[i]=val[i];
            // }
            if (strcmp(val, "CMD_FOTA") == 0)
            {
                QL_MQTT_LOG("co lenh FOTA tu APP\n");

                ql_fota_http_app_init();
            }
             else if (strcmp(val, "SEND_AT") == 0)
            {
                cJSON *sdt = cJSON_GetObjectItem(pJsonRoot, "INFO");
                char *val1 = sdt->valuestring;               
                //char *cmd2 ="AT+QSCLK=1\r\n";
                ql_virt_at_write(QL_VIRT_AT_PORT_0, (unsigned char*)val1, strlen((char *)val1));   
                 QL_MQTT_LOG("\n da gui:%s\n",val1);               
                //l_power_app_init();

            }   

              else if (strcmp(val, "KTTK") == 0)
            {
                ql_virt_at_write(QL_VIRT_AT_PORT_0, (unsigned char*)CMD_KTTK, strlen((char *)CMD_KTTK));   
                QL_MQTT_LOG("\n da gui:%s\n",CMD_KTTK);  
                //ON_MQTT=0;
            }
               else if (strcmp(val, "SAVE_CONFIG") == 0)
            {
                //ql_mxml_app_init();  
            }                             

            else if (strcmp(val, "GET_STT") == 0)
            {
               ql_stt_app_init();
            //ql_power_down(POWD_NORMAL);
            }

             else if (strcmp(val, "GET_GPS") == 0)
            {
                ql_stt_gps_init();
            }
         
            // else if (strcmp(val, "SMS_PAIR") == 0)
            // {
            //     cJSON *sdt = cJSON_GetObjectItem(pJsonRoot, "SDT");
            //     char *val1 = sdt->valuestring;
            //     cJSON *info = cJSON_GetObjectItem(pJsonRoot, "INFO");
            //     char *val2 = info->valuestring;
            //     gui_sms(val1, val2);
                
            // }
            else if (strcmp(val, "GET_SIM") == 0)
            {
                ql_nw_app_init();
            }
            else if (strcmp(val, "SETTING") == 0)
            {
                 //QL_MQTT_LOG("\n%s\n",payload);
                strcpy(json_setting,payload);
                QL_MQTT_LOG("\nget SETTINGSS :%s\n", json_setting);
                setting_init();
            }
            else if (strcmp(val, "SCAN_WIFI") == 0)
            {
                ql_wifiscan_app_init();
            }
            else
            {
                // ql_wifiscan_app_init();
                if (strlen(val) < 20)
                {
                    strcpy(val1, val);
                    QL_MQTT_LOG("\nget REMOTE :%s\n", val1);
                    ql_remote_app_init();
                }
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

QlOSStatus pub_mqtt(char *topic, char *mess)
{
    if (mqtt_connected == 1)
    {

        if (ql_mqtt_publish(&mqtt_cli, topic, mess, strlen(mess), 0, 0, mqtt_requst_result_cb, NULL) == MQTTCLIENT_WOUNDBLOCK)
        {
            QL_MQTT_LOG("\rTHANHCONG CHO KET QUA!\n");
            ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
            return QL_OSI_SUCCESS;
        }
        else
        {
            QL_MQTT_LOG("that bai\n");
        }
    }
    else
    {
        QL_MQTT_LOG("FAIL: MQTT DISCONNECTED!\n");
        return 1;
    }
}
static void mqtt_app_thread(void *arg)
{
    int ret = 0;
    int i = 0, run_num = 1;
    ql_data_call_info_s info;
    char ip4_addr_str[16] = {0};
    int is_user_onenet = 0;
    ql_rtos_task_sleep_s(7);
    ql_rtos_semaphore_create(&mqtt_semp, 0);

    char *client_id = (char *)malloc(256);
    char *client_user = (char *)malloc(256);
    char *client_pass = (char *)malloc(256);
    ql_dev_get_imei(imei, 30, NSIM);
    strcpy(topic_gui, imei);
    strcat(topic_gui, topic_rec);
    strcpy(topic_nhan, imei);
    strcat(topic_nhan, topic_remote);
    strcpy(topic_tb,imei);
    strcat(topic_tb,topic_event);
    strcpy(mqtt_client, imei);
    strcat(mqtt_client, MQTT_CLIENT_IDENTITY);
    // strcpy(topic_rec,imei);
    // strcpy(topic_gui, imei);
    QL_MQTT_LOG("\ntopic gui:%s", topic_gui);
    QL_MQTT_LOG("\ntopic nhan:%s", topic_nhan);
    QL_MQTT_LOG("\ntopic thong bao:%s", topic_tb);
    ret = ql_virt_at_open(QL_VIRT_AT_PORT_0, ql_virt_at0_notify_cb);
    if (QL_VIRT_AT_SUCCESS != ret)
    {
        QL_MQTT_LOG("\nvirt at0 open error,ret: 0x%x\n", ret);
        goto exit;
    }

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

        QL_MQTT_LOG("\rmqtt_cli:%d\n", mqtt_cli);

        client_info.keep_alive = 60;
        client_info.clean_session = 1;
        client_info.will_qos = 0;
        client_info.will_retain = 0;
        client_info.will_topic = NULL;
        client_info.will_msg = NULL;
        client_info.client_id = mqtt_client;
        client_info.client_user = MQTT_CLIENT_USER;
        client_info.client_pass = MQTT_CLIENT_PASS;
        QL_MQTT_LOG("\rconnect ssl %d onenet mode %d\n", case_id, is_user_onenet);
        // if (case_id == 0)
        // {
            QL_MQTT_LOG("\nchon id-0\n");
            client_info.ssl_cfg = NULL;
            ret = ql_mqtt_connect(&mqtt_cli, MQTT_CLIENT_SRV_URL, mqtt_connect_result_cb, NULL, (const struct mqtt_connect_client_info_t *)&client_info, mqtt_state_exception_cb);
            QL_MQTT_LOG("\rmqtt_cli:%d\n", mqtt_cli);
     //   }
        // else
        // {
        //     struct mqtt_ssl_config_t quectel_ssl_cfg = {
        //         .ssl_ctx_id = 1,
        //         .verify_level = MQTT_SSL_VERIFY_NONE,
        //         .cacert_path = NULL,
        //         .client_cert_path = NULL,
        //         .client_key_path = NULL,
        //         .client_key_pwd = NULL};
        //     client_info.ssl_cfg = &quectel_ssl_cfg;
        //     ret = ql_mqtt_connect(&mqtt_cli, MQTT_CLIENT_ONENET_SSL_URL, mqtt_connect_result_cb, NULL, (const struct mqtt_connect_client_info_t *)&client_info, mqtt_state_exception_cb);
        //      QL_MQTT_LOG("\nchon id-11\n");
        // }
        if (ret == MQTTCLIENT_WOUNDBLOCK)
        {
            QL_MQTT_LOG("\r====wait connect result:\n");
            ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
            if (mqtt_connected == 0)
            {
                ql_mqtt_client_deinit(&mqtt_cli);
                break;
            }
             QL_MQTT_LOG("\nDA KET NOI\n");
        }
        else
        {
            QL_MQTT_LOG("\r===mqtt connect failed ,ret = %d\n", ret);
            break;
        }

        ql_mqtt_set_inpub_callback(&mqtt_cli, mqtt_inpub_data_cb, NULL);
        if (sub_mqtt(topic_nhan) == QL_OSI_SUCCESS)
        {
            QL_MQTT_LOG("SUB TOPIC:%s OK", topic_nhan);
        }
        while (mqtt_connected == 1)
        {


            ql_event_t test_event = {0};
            if (ql_event_try_wait(&test_event) != 0)
            {
                continue;
            }

            if (test_event.id == QUEC_KERNEL_FEED_DOG)
            {
                QL_MQTT_LOG("\ndemo task receive feed dog event\n");

            }
             if(sub_mqtt(topic_nhan)==QL_OSI_SUCCESS)
            {
                QL_MQTT_LOG("\nSUB TOPIC:%s OK\n",topic_nhan);
                if (ql_rtos_feed_dog() != QL_OSI_SUCCESS)
                {
                    QL_MQTT_LOG("feed dog failed\n");
                }
                else
                {
                     QL_MQTT_LOG("FEEED DOG OK\n");
                }
            }           
          ql_rtos_task_sleep_s(10);
        }
    }
    if (mqtt_connected == 1 && ql_mqtt_disconnect(&mqtt_cli, mqtt_disconnect_result_cb, NULL) == MQTTCLIENT_WOUNDBLOCK)
    {
        QL_MQTT_LOG("\r=====wait disconnect result");
        ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
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

QlOSStatus sub_mqtt(char *topic)
{
    if (ql_mqtt_sub_unsub(&mqtt_cli, topic, 1, mqtt_requst_result_cb, NULL, 1) == MQTTCLIENT_WOUNDBLOCK)
    {
        QL_MQTT_LOG("\ndang sub topic:%s\n", topic_nhan);
        ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
        return QL_OSI_SUCCESS;
    }
    else
    {
        return 1;
    }
}

void feed_dog_callback1(uint32 id_type, void *ctx)
{	
	ql_event_t event;

	if(id_type == QUEC_KERNEL_FEED_DOG)
	{
		QL_MQTT_LOG("feed dog callback run\n");
		event.id = QUEC_KERNEL_FEED_DOG;
		if(ql_rtos_event_send(mqtt_task, &event) != QL_OSI_SUCCESS)
		{
			QL_MQTT_LOG("send feed_dog event to demo task failed\n");
		}	
		else
		{
			QL_MQTT_LOG("send feed dog event to mqtttask ok\n");
            if(sub_mqtt(topic_nhan)==QL_OSI_SUCCESS)
            {
                QL_MQTT_LOG("\n===>> SUB TOPIC:%s OK\n",topic_nhan);
            }

		}
	}
}




void reconnect()
{
    int adc_value = 0;
    ql_adc_get_volt(QL_ADC0_CHANNEL, &adc_value);
    char MQTT_CLI[50] = {0};
    sprintf(MQTT_CLI, "%s0x%d", client_info.client_id, adc_value);
    client_info.client_id = MQTT_CLI;
    QL_MQTT_LOG("\rKET NOI LAI MQTT clien:%s\n", client_info.client_id);
    ql_mqtt_connect(&mqtt_cli, MQTT_CLIENT_SRV_URL, mqtt_connect_result_cb, NULL, (const struct mqtt_connect_client_info_t *)&client_info, mqtt_state_exception_cb);
    if (sub_mqtt(topic_nhan) == QL_OSI_SUCCESS)
    {
        QL_MQTT_LOG("SUB TOPIC:%s OK", topic_nhan);
    }
    
    else
    {
        QL_MQTT_LOG("\n ket noi lai failed\n");
        ql_rtos_task_sleep_ms(5000);
    }
}

extern delete_all_sms();
int ql_mqtt_app_init(void)
{
    QlOSStatus err = QL_OSI_SUCCESS;

    err = ql_rtos_task_create(&mqtt_task, 16 * 1024,APP_PRIORITY_HIGH, "mqtt_app", mqtt_app_thread, NULL, 5);
    //ql_rtos_swdog_register((ql_swdog_callback)feed_dog_callback1, mqtt_task);  
   // ql_rtos_sw_dog_enable(10000, 3);
    if (err != QL_OSI_SUCCESS)
    {
        QL_MQTT_LOG("\rmqtt_app init failed");
    }

    return err;
}