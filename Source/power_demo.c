
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
#include <string.h>
#include <stdlib.h>

#include "ql_api_osi.h"
#include "ql_log.h"
#include "ql_pin_cfg.h"
#include "power_demo.h"

#ifdef QL_APP_FEATURE_USB
#include "ql_usb.h"
#endif
#define QL_APP_FEATURE_GNSS

/*===========================================================================
 * Macro Definition
 ===========================================================================*/
#define QL_POWERDEMO_LOG_LEVEL             QL_LOG_LEVEL_INFO
#define QL_POWERDEMO_LOG(msg, ...)         QL_LOG(QL_POWERDEMO_LOG_LEVEL, "ql_POWER", msg, ##__VA_ARGS__)
#define QL_POWERDEMO_LOG_PUSH(msg, ...)    QL_LOG_PUSH("ql_POWER", msg, ##__VA_ARGS__)

#if !defined(require_action)
	#define require_action(x, action, str)																		\
			do                                                                                                  \
			{                                                                                                   \
				if(x != 0)                                                                        				\
				{                                                                                               \
					QL_POWERDEMO_LOG(str);																		\
					{action;}																					\
				}                                                                                               \
			} while( 1==0 )
#endif

/*===========================================================================
 * Variate
 ===========================================================================*/
ql_task_t power_task = NULL;
ql_timer_t power_timer = NULL;
int wake_lock_1, wake_lock_2;

ql_task_t pwrkey_task = NULL;

/*===========================================================================
 * Functions
 ===========================================================================*/
//Sleep callback function is executed before sleep, custom can close some pins to reduce leakage or saving some information in here
//Caution:callback functions cannot run too much code 
void ql_enter_sleep_cb(void* ctx)
{   
    //QL_POWERDEMO_LOG("enter sleep cb");

#ifdef QL_APP_FEATURE_GNSS
    ql_pin_set_func(QL_PIN_NUM_KEYOUT_5, QL_FUN_NUM_UART_2_CTS);  //keyout5 pin need be low level when enter sleep, adjust the function to uart2_rts can do it
    ql_gpio_set_level(GPIO_12, LVL_HIGH);                         //close mos linked to gnss, to avoid high current in sleep mode
    ql_gpio_set_level(GPIO_11, LVL_LOW);                          //gpio11 need be low level when enter sleep to reduce leakage current to gnss
#endif
}

//exit sleep callback function is executed after exiting sleep, custom can recover the information before sleep
//Caution:callback functions cannot run too much code 
void ql_exit_sleep_cb(void* ctx)
{   
    //QL_POWERDEMO_LOG("exit sleep cb");  
    
#ifdef QL_APP_FEATURE_GNSS
    ql_pin_set_func(QL_PIN_NUM_KEYOUT_5, QL_FUN_NUM_UART_3_TXD);  //keyout5 pin used as gnss uart3_txd function, after exit sleep, set it to uart3_txd
#endif    
}

#ifdef QL_APP_FEATURE_USB
int usb_hotplug_cb(QL_USB_HOTPLUG_E state, void *ctx)
{
	if(state == QL_USB_HOTPLUG_IN)
	{
		QL_POWERDEMO_LOG("USB inserted");
	}
	else
	{
		QL_POWERDEMO_LOG("USB plug out");
	}

	return 0;
}
#endif


static void ql_power_demo_thread(void *param)
{
    //QL_POWERDEMO_LOG("power demo thread enter, param 0x%x", param);

	ql_event_t event;
	int err;

    //register sleep callback function
    ql_sleep_register_cb(ql_enter_sleep_cb);
    
    //register wakeup callback function
    ql_wakeup_register_cb(ql_exit_sleep_cb);

#ifdef QL_APP_FEATURE_USB	
	//register usb hotplug callback function
	ql_usb_bind_hotplug_cb(usb_hotplug_cb);
#endif

	while(1)
	{
		if(ql_event_try_wait(&event) != 0)
		{
			continue;
		}	
		QL_POWERDEMO_LOG("receive event, id is %d", event.id);
		
		switch(event.id)
		{
			case QUEC_SLEEP_ENETR_AUTO_SLEPP:
				
				err = ql_autosleep_enable(QL_ALLOW_SLEEP);
				require_action(err, continue, "failed to set auto sleep");

				err = ql_lpm_wakelock_unlock(wake_lock_1);
				require_action(err, continue, "lock1 unlocked failed");

				err = ql_lpm_wakelock_unlock(wake_lock_2);
				require_action(err, continue, "lock2 unlocked failed");		
				
				QL_POWERDEMO_LOG("set auto sleep mode ok");
				
			break;

			case QUEC_SLEEP_EXIT_AUTO_SLEPP:
				err = ql_autosleep_enable(QL_NOT_ALLOW_SLEEP);
				require_action(err, continue, "failed to set auto sleep");
			break;

			case QUEC_SLEEP_QUICK_POWER_DOWM:
				ql_power_down(POWD_IMMDLY);
			break;

			case QUEC_SLEEP_NORMAL_POWER_DOWM:
				ql_power_down(POWD_NORMAL);
			break;

			case QUEC_SLEEP_QUICK_RESET:
				ql_power_reset(RESET_QUICK);
			break;

			case QUEC_SLEEP_NORMAL_RESET:
				ql_power_reset(RESET_NORMAL);
			break;

			default:
			break;
		}
	}

    ql_rtos_task_delete(NULL);
}

void power_timer_callback(void *ctx)
{
	ql_event_t event = {0};

	event.id = QUEC_SLEEP_ENETR_AUTO_SLEPP;
	ql_rtos_event_send(power_task, &event);
}

void ql_power_app_init(void)
{
    QlOSStatus err = QL_OSI_SUCCESS;

    err = ql_rtos_task_create(&power_task, 1024, APP_PRIORITY_NORMAL, "ql_powerdemo", ql_power_demo_thread, NULL, 3);
	require_action(err, return, "power demo task created failed");

	err = ql_rtos_timer_create(&power_timer, power_task, power_timer_callback, NULL);
	require_action(err, return, "demo_timer created failed");

	err = ql_rtos_timer_start(power_timer, 10000, 0);   // 1秒后开启自动休眠
	require_action(err, return, "demo_timer start failed");

//	wake_lock_1 = ql_lpm_wakelock_create("my_lock_1", 10);
//	require_action((wake_lock_1 <= 0), return, "lock1 created failed");
	
//	wake_lock_2 = ql_lpm_wakelock_create("my_lock_2", 10);
//	require_action((wake_lock_2 <= 0), return, "lock2 created failed");

//	err = ql_lpm_wakelock_lock(wake_lock_1);
//	require_action(err, return, "lock1 locked failed");

//	err = ql_lpm_wakelock_lock(wake_lock_2);
//	require_action(err, return, "lock2 locked failed");	
}


/*****  pwrkey demo  *****/
static void _pwrkey_demo_callback(void)
{
    ql_event_t event;

    event.id = QUEC_PWRKEY_SHUTDOWN_START_IND;

    ql_rtos_event_send(pwrkey_task, &event);
}

static void _pwrkey_longpress_callback(void)
{
    ql_event_t event;

    event.id = QUEC_PWRKEY_LONGPRESS_IND;

    ql_rtos_event_send(pwrkey_task, &event);
}

static void _pwrkey_press_callback(void)
{
    ql_event_t event;

    event.id = QUEC_PWRKEY_PRESS_IND;

    ql_rtos_event_send(pwrkey_task, &event);
}

static void _pwrkey_release_callback(void)
{
    ql_event_t event;

    event.id = QUEC_PWRKEY_RELEASE_IND;

    ql_rtos_event_send(pwrkey_task, &event);
}

static void ql_pwrkey_demo_thread(void *param)
{
    //QL_POWERDEMO_LOG("pwrkey demo thread enter, param 0x%x", param);

    ql_event_t event;

    ql_pwrkey_shutdown_time_set(3000);                              // long pressed 3s shutdown
    ql_pwrkey_callback_register(_pwrkey_demo_callback);             // long press & release trigger
    ql_pwrkey_longpress_cb_register(_pwrkey_longpress_callback, 7000);    // long press & not release, long pressed 7s trigger
    ql_pwrkey_press_cb_register(_pwrkey_press_callback);
    ql_pwrkey_release_cb_register(_pwrkey_release_callback);

    while(1)
    {
        if(ql_event_try_wait(&event) != 0)
        {
            continue;
        }

        switch(event.id)
        {
            case QUEC_PWRKEY_SHUTDOWN_START_IND:
                QL_POWERDEMO_LOG("customer process");
                /* do something */
                ql_power_down(POWD_NORMAL);
                break;

            case QUEC_PWRKEY_LONGPRESS_IND:
                QL_POWERDEMO_LOG("pwrkey long press trigger");
                /* do something */
                ql_power_reset(RESET_NORMAL);
                break;

            case QUEC_PWRKEY_PRESS_IND:
                QL_POWERDEMO_LOG("pwrkey short press");
                /* do something */
                break;

            case QUEC_PWRKEY_RELEASE_IND:
                QL_POWERDEMO_LOG("pwrkey short release");
                /* do something */
                break;

            default:
                break;
        }
    }

    ql_rtos_task_delete(NULL);
}

void ql_pwrkey_app_init(void)
{
    QlOSStatus err = QL_OSI_SUCCESS;

    err = ql_rtos_task_create(&pwrkey_task, 1024, APP_PRIORITY_NORMAL, "ql_pwrkeydemo", ql_pwrkey_demo_thread, NULL, 3);
    require_action(err, return, "pwrkey demo task created failed");

}



