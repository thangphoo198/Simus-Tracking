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

#define QL_I2C_LOG DebugPrint

#define QL_I2C_TASK_STACK_SIZE 2048
#define QL_I2C_TASK_PRIO APP_PRIORITY_NORMAL
#define QL_I2C_TASK_EVENT_CNT 5

#define SalveAddr_w_8bit 24
#define SalveAddr_r_8bit 25

#define REG1 0x20 //=0101[100Hz]0[HR mode]111[ZYX enable]=01010111b=0x57;
#define REG2 0x21
#define REG3 0x22
#define REG4 0x23 //=00011000b [+-4g, High-resolution output mode]=0x18
#define REG5 0x24
#define W_A_I 0x0F // phai bang 0x33
#define OUT_XL 0x28
#define OUT_XH 0x29
#define OUT_YL 0x2A
#define OUT_YH 0x2B
#define OUT_ZL 0x2C
#define OUT_ZH 0x2D

char check()
{
    uint8_t KQ = 0;
    ql_I2cRead(i2c_1, SalveAddr_r_8bit, W_A_I, &KQ, 1);
    //	kq=IC_Read(W_A_I);
    QL_I2C_LOG("WAIT :0x%x\n", KQ);
    if (KQ == 0x33)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
void Acc_Init()
{
    ql_rtos_task_sleep_ms(1);
    ql_I2cWrite(i2c_1, SalveAddr_w_8bit, REG1, 0x57, 1);
    ql_I2cWrite(i2c_1, SalveAddr_w_8bit, REG2, 0x00, 1);
    ql_I2cWrite(i2c_1, SalveAddr_w_8bit, REG3, 0x40, 1);
    ql_I2cWrite(i2c_1, SalveAddr_w_8bit, REG4, 0x18, 1);
    ql_I2cWrite(i2c_1, SalveAddr_w_8bit, REG1, 0x00, 1);
}

int GetData(unsigned char Haddress, unsigned char Laddress)
{
    char H, L;
    ql_I2cRead(i2c_1, SalveAddr_w_8bit, Haddress, &H, 1);
    ql_I2cRead(i2c_1, SalveAddr_w_8bit, Laddress, &L, 1);
    QL_I2C_LOG("H:%x L: %x\n", H, L);
    return ((H << 8) + L);
}

void print_ACC()
{
    uint8_t x_l, x_h, y_l, y_h, z_l, z_h;
    uint16_t x_val, y_val, z_val;
    ql_I2cRead(i2c_1, SalveAddr_r_8bit, 0x08, &x_l, 1);
    ql_rtos_task_sleep_ms(1);
    ql_I2cRead(i2c_1, SalveAddr_r_8bit, 0x09, &x_h, 1);
    ql_rtos_task_sleep_ms(1);
    ql_I2cRead(i2c_1, SalveAddr_r_8bit, 0xA, &y_l, 1);
    ql_rtos_task_sleep_ms(1);
    ql_I2cRead(i2c_1, SalveAddr_r_8bit, 0xB, &y_h, 1);
    ql_rtos_task_sleep_ms(1);
    ql_I2cRead(i2c_1, SalveAddr_r_8bit, 0xC, &z_l, 1);
    ql_rtos_task_sleep_ms(1);
    ql_I2cRead(i2c_1, SalveAddr_r_8bit, 0xD, &z_h, 1);
    ql_rtos_task_sleep_ms(1);

    QL_I2C_LOG("XL:0x%x XH: 0x%x YL:0x%x YH: 0x%x ZL:0x%x ZH: 0x%x \n  ", x_l, x_h, y_l, y_h, z_l, z_h);

    x_val = (short int)(x_l + (x_h << 8));
    y_val = (short int)(y_l + (y_h << 8));
    z_val = (short int)(z_l + (z_h << 8));
    QL_I2C_LOG("\r\nx_val: %d", x_val);
    QL_I2C_LOG("\r\ny_val: %d", y_val);
    QL_I2C_LOG("\r\nz_val: %d", z_val);

    x_l = 0;
    x_h = 0;
    y_h = 0;
    y_l = 0;
    z_h = 0;
    z_l = 0;
}

void ql_i2c_demo_thread(void *param)
{
    /*operate the camera for the example*/
    // ql_CamInit(320, 240);
    // ql_CamPowerOn();
    ql_pin_set_func(41, 0);
    ql_pin_set_func(42, 0);

    // ql_I2cInit(i2c_1, STANDARD_MODE);
    // Acc_Init();
    while (1)
    {
        if (check())
        {
        uint8_t read_data = 0;
        ql_I2cRead(i2c_1, SalveAddr_r_8bit, OUT_XH, &read_data, 1);
        QL_I2C_LOG("I2C read_data = 0x%x", read_data);
        read_data = 0;
        ql_I2cRead(i2c_1, SalveAddr_r_8bit, OUT_XL, &read_data, 1);
        QL_I2C_LOG("I2C read_data 2 = 0x%x", read_data);
        read_data = 0;
            //print_ACC();
            // int x,y,z;
            // QL_I2C_LOG("I2CC OK");
            // //print_ACC();
            // x=GetData(0x29,0x28);
            // y=GetData(0x2B,0X2A);
            // z=GetData(0X2D,0X2C);
            // QL_I2C_LOG("x: %d y:%d z:%d\n",x,y,z);
        }
        else
        {
            QL_I2C_LOG("i2c failed\n");
        }
        // QL_APP_I2C_LOG("I2C read_data = 0x%x", read_data);
        // ql_I2cRead(i2c_1, SalveAddr_r_8bit, 0xf0, &read_data, 1);
        // QL_APP_I2C_LOG("I2C read_data = 0x%x", read_data);
        // ql_I2cWrite(i2c_1, SalveAddr_w_8bit, 0x55, &data, 1);
        // read_data = 0;
        ql_rtos_task_sleep_ms(1000);
    }
}

void ql_i2c_demo_init(void)
{
    QlI2CStatus err = QL_OSI_SUCCESS;
    ql_task_t i2c_task = NULL;

    err = ql_rtos_task_create(&i2c_task, QL_I2C_TASK_STACK_SIZE, 15, "I2C DEMO", ql_i2c_demo_thread, NULL, QL_I2C_TASK_EVENT_CNT);
    if (err != QL_OSI_SUCCESS)
    {
        QL_I2C_LOG("i2ctest1 demo task created failed\n");
    }
}
