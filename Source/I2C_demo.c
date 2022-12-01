/**  @file
  I2C_demo.c

  @brief
  This file is demo of I2C.

*/

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

=================================================================*/

/*===========================================================================
 * include files
 ===========================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "DataDefine.h"
#include "ql_api_osi.h"
#include "ql_log.h"
#include "ql_api_camera.h"
#include "ql_i2c.h"
#include "I2C_demo.h"

/*===========================================================================
 * Macro Definition
 ===========================================================================*/

#define QL_APP_I2C_LOG_LEVEL             QL_LOG_LEVEL_INFO
#define QL_APP_I2C_LOG(msg, ...)         QL_LOG(QL_APP_I2C_LOG_LEVEL, "QL_APP_I2C", msg, ##__VA_ARGS__)
#define QL_APP_I2C_LOG_PUSH(msg, ...)    QL_LOG_PUSH("QL_APP_I2C", msg, ##__VA_ARGS__)
    
#define QL_I2C_TASK_STACK_SIZE     		1024
#define QL_I2C_TASK_PRIO          	 	APP_PRIORITY_NORMAL
#define QL_I2C_TASK_EVENT_CNT      		5

#define SalveAddr_w_8bit        (0x32 >> 1)
#define SalveAddr_r_8bit        (0x33 >> 1)


#define icwriteadd 0x32
#define icreadadd 0x33
#define REG1 0x20//=0101[100Hz]0[HR mode]111[ZYX enable]=01010111b=0x57;
#define REG4 0x23//=00011000b [+-4g, High-resolution output mode]=0x18
#define W_A_I 0x0F//phai bang 0x33
#define OUT_XL 0x28
#define OUT_XH 0x29
#define OUT_YL 0x2A
#define OUT_YH 0x2B
#define OUT_ZL 0x2C
#define OUT_ZH 0x2D

#define demo_for_8bit_or_16bit  (1)       //1:test 8bit register address     0:test 16bit register address

static char check()
{
    uint8_t KQ;
    int ret = ql_I2cRead(i2c_2, SalveAddr_r_8bit, 0x0F, &KQ, 1);
    //	kq=IC_Read(W_A_I);
    if (KQ == 0x33)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
static void Acc_Init()
{
   // ql_rtos_task_sleep_ms(1);
    uint8_t data = 0x57;
    ql_I2cWrite(i2c_2, SalveAddr_w_8bit, REG1, &data, 1);
    data = 0x18;
    ql_I2cWrite(i2c_2, SalveAddr_w_8bit, REG4, &data, 1);
    // IC_Write(REG1,0x57);
    // IC_Write(REG4,0x18);
}

void ql_i2c_demo_thread(void *param)
{
    /*operate the camera for the example*/
    ql_CamInit(320, 240);
    ql_CamPowerOn();
    
    ql_I2cInit(i2c_2, STANDARD_MODE);
    Acc_Init();
    if(check())
    {
        ql_uart_write(QL_LOG_PORT_UART,"I2CC OK",5);
         QL_APP_I2C_LOG("THANH CONG\n");

    }
    else
    {
        QL_APP_I2C_LOG("i2c failed\n");
       ql_uart_write(QL_LOG_PORT_UART,"I2CC FAIL",10);
    }

    while(1)
    {
        // QL_APP_I2C_LOG("I2C read_data = 0x%x", read_data);
        // ql_I2cRead(i2c_2, SalveAddr_r_8bit, 0xf0, &read_data, 1);
        // QL_APP_I2C_LOG("I2C read_data = 0x%x", read_data);
        // ql_I2cWrite(i2c_2, SalveAddr_w_8bit, 0x55, &data, 1);
        // read_data = 0;
        ql_rtos_task_sleep_ms(200); 
    }
 

}

void ql_i2c_demo_init(void)
{
    QlI2CStatus err = QL_OSI_SUCCESS;
    ql_task_t i2c_task = NULL;
        
    err = ql_rtos_task_create(&i2c_task, QL_I2C_TASK_STACK_SIZE, QL_I2C_TASK_PRIO, "I2C DEMO", ql_i2c_demo_thread, NULL, QL_I2C_TASK_EVENT_CNT);
    if (err != QL_OSI_SUCCESS)
    {
        QL_APP_I2C_LOG("i2ctest1 demo task created failed\n");
    }
}


