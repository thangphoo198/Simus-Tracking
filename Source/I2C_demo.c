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
#include "ql_audio.h"

/*===========================================================================
 * Macro Definition
 ===========================================================================*/

#define QL_I2C_LOG DebugPrint

#define QL_I2C_TASK_STACK_SIZE 2048
#define QL_I2C_TASK_PRIO APP_PRIORITY_NORMAL
#define QL_I2C_TASK_EVENT_CNT 5

#define SalveAddr_w_8bit 0x19
#define SalveAddr_r_8bit 0x19

#define REG1 0x20 //=0101[100Hz]0[HR mode]111[ZYX enable]=01010111b=0x57;
#define REG2 0x21
#define REG3 0x22
#define REG4 0x23 //=00011000b [+-4g, High-resolution output mode]=0x18
#define REG5 0x24

#define LIS3DH_REG_TEMPCFG 0x1F

//#define W_A_I 0x75 // phai bang 0x34
#define W_A_I 0x0F // phai bang 0x34
#define OUT_XL 0x28
#define OUT_XH 0x29
#define OUT_YL 0x2A
#define OUT_YH 0x2B
#define OUT_ZL 0x2C
#define OUT_ZH 0x2D
#define LIS3DSH_OUT_TEMP     	0x0C

#define LIS3DH_LSB16_TO_KILO_LSB10                                             \
  64000 
#define SENSORS_GRAVITY_EARTH (9.80665F)

typedef enum
{
	DET_STOP,
	DET_MOVE,
	//...
} event_t;

char check()
{
    uint8_t KQ = 0;
    ql_I2cRead(i2c_1, SalveAddr_r_8bit, W_A_I, &KQ, 1);
    //	kq=IC_Read(W_A_I);
    //QL_I2C_LOG("\nWAIT :0x%x\n", KQ);
    if (KQ == 0x33) //0x68
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
    if(check())
    {
    ql_rtos_task_sleep_ms(100);
    QL_I2C_LOG("\n tim thay CAM BIEN\n");
    mpu_write_reg(REG1, 0x57);
  //  ql_I2cWrite(i2c_1, SalveAddr_w_8bit, REG4, 0x88, 1);
   // mpu_write_reg(REG4, 0x5f);
    mpu_write_reg(REG4, 0x18);
    // mpu_write_reg(REG5, 0x80);
    // mpu_write_reg(0x2E, 0);
    // mpu_write_reg(0x25, 0x10);
  //  ql_I2cWrite(i2c_1, SalveAddr_w_8bit, LIS3DH_REG_TEMPCFG, 0x80, 1);
   // ql_I2cWrite(i2c_1,SalveAddr_w_8bit,0x6c,tmp,0x01);
  // mpu_write_reg(LIS3DH_REG_TEMPCFG,0x80);
    }
    
}
void mpu_write_reg(uint8 RegAddr, uint16 RegData)
{
    uint8 param_data[3] = {0x00};
    uint8 retry_count = 5;
 
    param_data[0] = (uint8)((RegData >> 8) & 0xFF);
    param_data[1] = (uint8)(RegData & 0xff);
	//RegAddr = (RegAddr << 1) & 0xFE;
   // RegAddr = RegAddr|((RegData>>8)&0x01);
    param_data[0] = (uint8)(RegData & 0xFF);

    do 
    {
        ql_I2cWrite(i2c_1, SalveAddr_w_8bit, RegAddr, param_data, 1);      
    } while (--retry_count);

}

void mpu_read_reg(uint8 RegAddr, uint16 *p_value)
{
	ql_audio_errcode_e status = QL_AUDIO_SUCCESS;
	uint8 temp_buf[2];
	uint8 retry_count = 5;
  //  RegAddr = (RegAddr << 1) & 0xFE;

	do 
    {
        status = (ql_audio_errcode_e)ql_I2cRead(i2c_1, SalveAddr_r_8bit, RegAddr, temp_buf, 2);
        		if (status != QL_AUDIO_SUCCESS)
		{
            QL_I2C_LOG("\nError:[%dth] device[0x%x] addr[0x%x] failed\n", retry_count, SalveAddr_r_8bit, RegAddr);
        }
        else
        {
		*p_value = (((uint16)temp_buf[0]) << 8) | temp_buf[1];
		break;
        }
	}
     while (--retry_count);

}


int32_t GetData(unsigned char Haddress, unsigned char Laddress)
{
    uint8_t H, L;
    ql_I2cRead(i2c_1, SalveAddr_r_8bit, Haddress, &H, 1);
    ql_I2cRead(i2c_1, SalveAddr_r_8bit, Laddress, &L, 1);
   // QL_I2C_LOG("H:%d L: %d\n", H, L);
    return ((H << 8) + L);
}

void print_ACC()
{
  int x = GetData(OUT_XH,OUT_XL);
  int y = GetData(OUT_YH,OUT_YL);
  int z = GetData(OUT_ZH,OUT_ZL);
  float x_g,y_g,z_g;
  int lsb_value = 8; // +-4g
  x_g = lsb_value * ((float)x / LIS3DH_LSB16_TO_KILO_LSB10);
  y_g = lsb_value * ((float)y / LIS3DH_LSB16_TO_KILO_LSB10);
  z_g = lsb_value * ((float)z / LIS3DH_LSB16_TO_KILO_LSB10);
  float gx,gy,gz;
  gx=x_g*SENSORS_GRAVITY_EARTH;
  gy=y_g*SENSORS_GRAVITY_EARTH;
  gz=z_g*SENSORS_GRAVITY_EARTH;
  float out=(float)x / 7840;
   // uint16_t x,y,z;
   // mpu_read_reg(OUT_XH,&x);
   // mpu_read_reg(OUT_YH,&y);
   // mpu_read_reg(OUT_ZH,&z);
   char buff[100]={0};
    sprintf(buff,"\nX: %d GX:%.4f Y: %d GY:%.4f  Z:%d GZ:%.4f XX %.4f \n",x,gx,y,gy,z,gz,out);
    QL_I2C_LOG(buff);
}

void ql_i2c_demo_thread(void *param)
{
    /*operate the camera for the example*/
    ql_pin_set_func(41, 0);
    ql_pin_set_func(42, 0);
    ql_I2cInit(i2c_1, STANDARD_MODE);
    Acc_Init();
    while (1)
    {
        if (check())
        {
        print_ACC();
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
        ql_rtos_task_sleep_ms(500);
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
