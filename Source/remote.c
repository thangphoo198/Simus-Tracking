

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
#include "command.h"
#include "ql_power.h"

#define OUT_LOG DebugPrint
ql_task_t remote_task = NULL;

extern pub_mqtt(char *topic, char *mess);
static void stt_remote_thread(void *arg)
{

    OUT_LOG("\n TASK REMOTE %s\n",val1);
    remote(val1);

    ql_rtos_task_delete(NULL);
}
void remote()
{
   if (strcmp(val1, "POWER_ON") == 0)
    {
         OUT_LOG("BAT NGUON\n");
        ql_gpio_set_level(IO_LOCK, LVL_LOW);
        pub_mqtt(topic_gui, RSP_POWER_ON_OK);
   //     ql_rtos_task_sleep_ms(100);
    }
    else if (strcmp(val1, "POWER_OFF") == 0)
    {
        ql_gpio_set_level(IO_LOCK, LVL_HIGH);
        pub_mqtt(topic_gui, RSP_POWER_OFF_OK);
        // ql_rtos_task_sleep_ms(100);
    }
    else if (strcmp(val1, "SPEAKER_ON") == 0)
    {
        OUT_LOG("BAT LOA\n");
        ql_gpio_set_level(IO_SPEAKER, LVL_HIGH);
        pub_mqtt(topic_gui, RSP_SPEAKER_ON_OK);
      //  ql_rtos_task_sleep_ms(100);
        // ql_power_down(POWD_NORMAL);
    }
    else if (strcmp(val1, "SPEAKER_OFF") == 0)
    {
        OUT_LOG("TAT MAY:\n");

        ql_gpio_set_level(IO_SPEAKER, LVL_LOW);
        pub_mqtt(topic_gui, RSP_SPEAKER_OFF_OK);
      //  ql_rtos_task_sleep_ms(100);

        // ql_power_down(POWD_NORMAL);
    }
    else if (strcmp(val1, "LIGHT_ON") == 0)
    {
        OUT_LOG("BAT XI NHANH\n");
        ql_gpio_set_level(IO_LIGHT, LVL_HIGH);
        pub_mqtt(topic_gui, RSP_LIGHT_ON_OK);
      //  ql_rtos_task_sleep_ms(100);

        // ql_power_down(POWD_NORMAL);
    }
    else if (strcmp(val1, "LIGHT_OFF") == 0)
    {
        OUT_LOG("TAT MAY:\n");

        ql_gpio_set_level(IO_LIGHT, LVL_LOW);
        pub_mqtt(topic_gui, RSP_LIGHT_OFF_OK);

        // ql_power_down(POWD_NORMAL);
    }
    else if (strcmp(val1, "GET_SETTING") == 0)
    {
        doc_epprom();
        pub_mqtt(topic_gui, json_setting);
    }

     else if (strcmp(val1, "SHUTDOWN") == 0)
    {
        OUT_LOG("TAT MAY:\n");

        //ql_gpio_set_level(IO_LIGHT, LVL_LOW);
        pub_mqtt(topic_gui, "shutdownw OK");

        ql_power_down(POWD_NORMAL);
    } 
            for(uint8_t i=0;i<strlen(val1);i++)
            {
                val1[i]=0;
            }              

}
void ql_remote_app_init(void)
{
    QlOSStatus err = QL_OSI_SUCCESS;

    err = ql_rtos_task_create(&remote_task, 1024, APP_PRIORITY_LOW, "xxx", stt_remote_thread, NULL, 5);
    if (err != QL_OSI_SUCCESS)
    {
        OUT_LOG("created task failed");
    }
}
