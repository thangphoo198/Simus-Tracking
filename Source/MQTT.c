
#include "DataDefine.h"


#include "MQTT.h"


#define MQTT_CLIENT_IDENTITY        "VT_00001"
#define MQTT_CLIENT_USER            "esp32-simus"
#define MQTT_CLIENT_PASS            "thang123"


#define MQTT_CLIENT_ONENET_DEVICENAME            "SmartDevice"
#define MQTT_CLIENT_ONENET_VERSION               "2022-11-22"



#define MQTT_CLIENT_SRV_URL                      "mqtt://broker.simus.vn:1883" //onenet
#define MQTT_CLIENT_ONENET_SSL_URL               "mqtt://broker.simus.vn:1883"//onenet SSL

#define MQTT_TOPIC_SUB "MQTT_SUB"
#define MQTT_TOPIC_PUB "MQTT_PUB"

// publist 


#define QL_MQTT_LOG DebugPrint
extern char* buff;
ql_task_t mqtt_task = NULL;
static ql_sem_t  mqtt_semp;
static int  mqtt_connected = 0;

static void mqtt_connect_result_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_e status)
{
	QL_MQTT_LOG("\rstatus: %d", status);
	if(status == 0){
		mqtt_connected = 1;
	}
	ql_rtos_semaphore_release(mqtt_semp);
}


static void mqtt_state_exception_cb(mqtt_client_t *client)
{
	QL_MQTT_LOG("\rmqtt session abnormal disconnect");
	mqtt_connected = 0;
}


static void mqtt_inpub_data_cb(mqtt_client_t *client, void *arg, int pkt_id, const char *topic, const unsigned char *payload, unsigned short payload_len)
{
	QL_MQTT_LOG("\rtopic: %s", topic);
	QL_MQTT_LOG("payload: %s", payload);
}

static void mqtt_requst_result_cb(mqtt_client_t *client, void *arg,int err)
{
	QL_MQTT_LOG("\rerr1: %d", err);
	
	ql_rtos_semaphore_release(mqtt_semp);
}

static void mqtt_disconnect_result_cb(mqtt_client_t *client, void *arg,int err){
	QL_MQTT_LOG("\rerr2: %d", err);
	
	ql_rtos_semaphore_release(mqtt_semp);
}


static void mqtt_app_thread(void * arg)
{
	int ret = 0;
	int i = 0, run_num = 1;
	int profile_idx = 1;
    ql_data_call_info_s info;
	char ip4_addr_str[16] = {0};
	mqtt_client_t  mqtt_cli;
	uint8_t nSim = 0;
	uint16_t sim_cid;
    struct mqtt_connect_client_info_t  client_info = {0};
    char *token = NULL;
    int is_user_onenet = 0;
	ql_rtos_semaphore_create(&mqtt_semp, 0);
	ql_rtos_task_sleep_s(10);

    char* client_id = (char*)malloc(256);
    char* client_user = (char*)malloc(256);
    char* client_pass = (char*)malloc(256);
    
	QL_MQTT_LOG("\r========== mqtt demo start ==========");
	QL_MQTT_LOG("\rwait for network register done");
		
	while((ret = ql_network_register_wait(nSim, 120)) != 0 && i < 10){
    	i++;
		ql_rtos_task_sleep_s(1);
	}
	if(ret == 0){
		i = 0;
		QL_MQTT_LOG("\r====network registered!!!!====");
	}else{
		QL_MQTT_LOG("\r====network register failure!!!!!====");
		//goto exit;
	}

	ql_set_data_call_asyn_mode(nSim, profile_idx, 0);

	QL_MQTT_LOG("\r===start data call====");
	ret=ql_start_data_call(nSim, profile_idx, QL_PDP_TYPE_IP, "v-internet", NULL, NULL, 0); 
	QL_MQTT_LOG("\r===data call result:%d", ret);
	if(ret != 0){
		QL_MQTT_LOG("\r====data call failure!!!!=====");
	}
	memset(&info, 0x00, sizeof(ql_data_call_info_s));
	
	ret = ql_get_data_call_info(nSim, profile_idx, &info);
	if(ret != 0){
		QL_MQTT_LOG("\rql_get_data_call_info ret: %d", ret);
		ql_stop_data_call(nSim, profile_idx);
		//goto exit;
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

	
	while(1)
	{	
        int test_num = 0;
        int case_id = 0;
        if(is_user_onenet != 1)
        {   
            case_id = run_num%2;
        }

		int ret = MQTTCLIENT_SUCCESS;
		

		if(QL_DATACALL_SUCCESS != ql_bind_sim_and_profile(nSim, profile_idx, &sim_cid))
		{
			QL_MQTT_LOG("\rnSim or profile_idx is invalid!!!!\n");
			break;
		}
		
		if(ql_mqtt_client_init(&mqtt_cli, sim_cid) != MQTTCLIENT_SUCCESS){
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
        QL_MQTT_LOG("\rconnect ssl %d onenet mode %d",case_id,is_user_onenet);
		if(case_id == 0){
			client_info.ssl_cfg = NULL;
            ret = ql_mqtt_connect(&mqtt_cli, MQTT_CLIENT_SRV_URL, mqtt_connect_result_cb, NULL, (const struct mqtt_connect_client_info_t *)&client_info, mqtt_state_exception_cb);
			
		}else{
			struct mqtt_ssl_config_t quectel_ssl_cfg = {
				.ssl_ctx_id = 1,
				.verify_level = MQTT_SSL_VERIFY_NONE,
				.cacert_path = NULL,
				.client_cert_path = NULL,
				.client_key_path = NULL,
				.client_key_pwd = NULL
			};
            struct mqtt_ssl_config_t ontnet_ssl_cfg = {
                .ssl_ctx_id = 1,
                .verify_level = MQTT_SSL_VERIFY_NONE,
                .cacert_path = "UFS:MQTTS-certificate.pem",
                .client_cert_path = NULL,
                .client_key_path = NULL,
                .client_key_pwd = NULL
            };
            client_info.ssl_cfg = &quectel_ssl_cfg;
            ret = ql_mqtt_connect(&mqtt_cli, MQTT_CLIENT_ONENET_SSL_URL, mqtt_connect_result_cb, NULL, (const struct mqtt_connect_client_info_t *)&client_info, mqtt_state_exception_cb);
            
		}
		if(ret  == MQTTCLIENT_WOUNDBLOCK){
			QL_MQTT_LOG("\r====wait connect result:\n");
			ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
			if(mqtt_connected == 0){
				ql_mqtt_client_deinit(&mqtt_cli);
				break;
			}
		}else{
			QL_MQTT_LOG("\r===mqtt connect failed ,ret = %d\n",ret);
			break;
		}

		ql_mqtt_set_inpub_callback(&mqtt_cli, mqtt_inpub_data_cb, NULL);
    		while(mqtt_connected == 1){
    			if(ql_mqtt_sub_unsub(&mqtt_cli, "EC200U_REC", 1, mqtt_requst_result_cb,NULL, 1) == MQTTCLIENT_WOUNDBLOCK){
    				QL_MQTT_LOG("\r======wait subscrible result");
    				ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
    			}
				if(ql_mqtt_publish(&mqtt_cli, "EC200U_REMOTE", "hello", strlen("hello"), 0, 0, mqtt_requst_result_cb,NULL) == MQTTCLIENT_WOUNDBLOCK){
    				QL_MQTT_LOG("\r======wait publish result\n");
    				ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
    			}


    			ql_rtos_task_sleep_ms(10000);
    		}
        }
        if(mqtt_connected == 1 && ql_mqtt_disconnect(&mqtt_cli, mqtt_disconnect_result_cb, NULL) == MQTTCLIENT_WOUNDBLOCK){
            QL_MQTT_LOG("\r=====wait disconnect result");
            ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
			ret = ql_mqtt_connect(&mqtt_cli, MQTT_CLIENT_SRV_URL, mqtt_connect_result_cb, NULL, (const struct mqtt_connect_client_info_t *)&client_info, mqtt_state_exception_cb);


        }
		QL_MQTT_LOG("\r==============mqtt_client_test[%d] end=======%x=========\n",run_num,&mqtt_cli);
		ql_mqtt_client_deinit(&mqtt_cli);
		mqtt_connected = 0;
		ql_rtos_task_sleep_s(1);
	exit:
    if(is_user_onenet == 1)
    {
        free((void*)client_id);
        free((void*)client_user);
        free((void*)client_pass);
        client_id = NULL;
        client_user = NULL;
        client_pass = NULL;
    }

    ql_rtos_semaphore_delete(mqtt_semp);
    ql_rtos_task_delete(mqtt_task);	

   return;	


	}

int ql_mqtt_app_init(void)
{
	QlOSStatus err = QL_OSI_SUCCESS;
	
    err = ql_rtos_task_create(&mqtt_task, 16*1024, 23, "mqtt_app", mqtt_app_thread, NULL, 5);
	if(err != QL_OSI_SUCCESS)
    {
		QL_MQTT_LOG("\rmqtt_app init failed");
	}

	return err;
}
