/**  @file
  I2C_demo.c

  @brief

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

#define SalveAddr_w_8bit 0x19 //0x19
#define SalveAddr_r_8bit 0x19 //0x19

#define REG1 0x20 //=0101[100Hz]0[HR mode]111[ZYX enable]=01010111b=0x57;
#define REG2 0x21
#define REG3 0x22
#define REG4 0x23 //=00011000b [+-4g, High-resolution output mode]=0x18
#define REG5 0x24
#define REG6 0x25
#define LIS3DH_REG_REFERENCE 0x26

#define INT1_CFG 0x30
#define INT1_SRC 0x31
#define INT1_THS 0x32
#define INT1_DURATION 0x33

#define INT2_CFG 0x34
#define INT2_SRC 0x35
#define INT2_THS 0x36
#define INT2_DURATION 0x37

#define LIS3DH_REG_TEMPCFG 0x1F

// #define W_A_I 0x75 // phai bang 0x34
#define W_A_I 0x0F // phai bang 0x34
#define OUT_XL 0x28
#define OUT_XH 0x29
#define OUT_YL 0x2A
#define OUT_YH 0x2B
#define OUT_ZL 0x2C
#define OUT_ZH 0x2D



#define LIS3DSH_OUT_TEMP 0x0C

#define LIS3DH_LSB16_TO_KILO_LSB10 \
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
    QL_I2C_LOG("\nWAIT :0x%x\n", KQ);
    if (KQ == 0x33) // 0x68
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t mpu_read_reg8(uint8_t RegAddr)
{
    uint8_t x;
    ql_I2cRead(i2c_1, SalveAddr_r_8bit, RegAddr, &x, 1);
    return x;
}

void Acc_Init()
{
    if (check())
    {
        ql_rtos_task_sleep_ms(50);
        QL_I2C_LOG("\n tim thay CAM BIEN\n");
        mpu_write_reg(REG1, 0x57); //Turn on the sensor, enable X, Y, and Z ODR = 100 Hz
        mpu_write_reg(REG4, 0x18); //+4g
       // mpu_write_reg(REG2, 0x09); // High-pass filter enabled on interrupt activity 1
      //  mpu_write_reg(REG3, 0x40); // Interrupt activity 1 driven to INT1 pad

      //  mpu_write_reg(REG6, 0x20); // High-pass filter enabled on interrupt activity 2
        // mpu_write_reg(REG5, 0x08); // move
        // mpu_write_reg(INT2_THS, 0); // Threshold = 250 mg
        // mpu_write_reg(INT2_CFG,0x0A);
        // mpu_write_reg(INT2_DURATION, 0x01); // Duration = 0
        // uint16_t dataToWrite = 0 | (4 << 1);
        // dataToWrite|=0x20;
        // mpu_write_reg(REG6, dataToWrite); // High-pass filter enabled on interrupt activity 2
        uint8_t x = mpu_read_reg8(INT2_CFG);
        QL_I2C_LOG("doc cau hinh:%u\n",x);

        // mpu_write_reg(INT1_DURATION, 0x00); // Duration = 0

        //  ql_I2cWrite(i2c_1, SalveAddr_w_8bit, LIS3DH_REG_TEMPCFG, 0x80, 1);
        // ql_I2cWrite(i2c_1,SalveAddr_w_8bit,0x6c,tmp,0x01);
        // mpu_write_reg(LIS3DH_REG_TEMPCFG,0x80);
    }
}

// Write 57h into CTRL_REG1 // Turn on the sensor, enable X, Y, and Z
// // ODR = 100 Hz
// 2. Write 09h into CTRL_REG2 // High-pass filter enabled on interrupt activity 1
// 3. Write 40h into CTRL_REG3 // Interrupt activity 1 driven to INT1 pad
// 4. Write 00h into CTRL_REG4 // FS = ±2 g
// 5. Write 08h into CTRL_REG5 // Interrupt 1 pin latched
// 6. Write10h into INT1_THS // Threshold = 250 mg
// 7. Write 00h into INT1_DURATION // Duration = 0
// 8. Read REFERENCE
// // Dummy read to force the HP filter to
// // current acceleration value
// // (i.e. set reference acceleration/tilt value)
// 9. Write 2Ah into INT1_CFG // Configure desired wake-up event
// 10. Poll INT1 pad; if INT1 = 0 then go to 9 // Poll INT1 pin waiting for the
// // wake-up event
// 11. (Wake-up event has occurred; insert your
// code here) // Event handling
// 12. Read INT1_SRC // Return the event that has triggered the
// // interrupt and clear interrupt
// 13. (Insert your code here) // Event handling
// 14. Go to 9


void mpu_write_reg(uint8 RegAddr, uint16 RegData)
{
    uint8 param_data[3] = {0x00};
    // uint8 retry_count = 5;

    param_data[0] = (uint8)((RegData >> 8) & 0xFF);
    param_data[1] = (uint8)(RegData & 0xff);
    // RegAddr = (RegAddr << 1) & 0xFE;
    // RegAddr = RegAddr|((RegData>>8)&0x01);
    param_data[0] = (uint8)(RegData & 0xFF);

    // do
    // {
    ql_I2cWrite(i2c_1, SalveAddr_w_8bit, RegAddr, param_data, 1);
    // } while (--retry_count);
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


int16_t GetData(unsigned char Haddress, unsigned char Laddress)
{
    uint8_t H, L;
    ql_I2cRead(i2c_1, SalveAddr_r_8bit, Haddress, &H, 1);
    ql_I2cRead(i2c_1, SalveAddr_r_8bit, Laddress, &L, 1);
    // QL_I2C_LOG("H:%d L: %d\n", H, L);
    return (int16_t)((((uint16)H) << 8) | L);
}

void print_ACC()
{
    int16_t x = GetData(OUT_XH, OUT_XL);
    int16_t y = GetData(OUT_YH, OUT_YL);
    int16_t z = GetData(OUT_ZH, OUT_ZL);
    float x_g, y_g, z_g;
    uint8_t lsb_value = 8; // +-4g
    x_g = lsb_value * ((float)x / LIS3DH_LSB16_TO_KILO_LSB10);
    y_g = lsb_value * ((float)y / LIS3DH_LSB16_TO_KILO_LSB10);
    z_g = lsb_value * ((float)z / LIS3DH_LSB16_TO_KILO_LSB10);
    x_g *= SENSORS_GRAVITY_EARTH;
    y_g *= SENSORS_GRAVITY_EARTH;
    z_g *= SENSORS_GRAVITY_EARTH;
    char buff[50] = {0};
    float alpha = 0.8;                           // Hệ số bộ lọc trung bình trượt
    float x2 = x * x * alpha + x2 * (1 - alpha); // Bộ lọc trung bình trượt trên trục X
    float y2 = y * y * alpha + y2 * (1 - alpha); // Bộ lọc trung bình trượt trên trục Y
    float z2 = z * z * alpha + z2 * (1 - alpha); // Bộ lọc trung bình trượt trên trục Z
    float rms = sqrt(x2 + y2 + z2) / 16384.0;    // Độ rung lắc, chuyển đổi từ đơn v
    sprintf(buff, "\nGx: %.2f  Gy:%.2f  Gz:%.2f rung:%.2f X:%d Y:%d Z:%d  \n", x_g, y_g, z_g,rms,x,y,z);
    QL_I2C_LOG(buff);

}

void ql_i2c_demo_thread(void *param)
{
    /*operate the camera for the example*/
    ql_pin_set_func(41, 0);
    ql_pin_set_func(42, 0);
    ql_I2cInit(i2c_1, STANDARD_MODE);
     //ql_rtos_task_sleep_ms(5000);
    Acc_Init();
    while (1)
    {
        if (check())
        {
            print_ACC();
        }
        else
        {
            QL_I2C_LOG("\ni2c failed\n");
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

    err = ql_rtos_task_create(&i2c_task, QL_I2C_TASK_STACK_SIZE, APP_PRIORITY_NORMAL, "I2C DEMO", ql_i2c_demo_thread, NULL, QL_I2C_TASK_EVENT_CNT);
    if (err != QL_OSI_SUCCESS)
    {
        QL_I2C_LOG("i2ctest1 demo task created failed\n");
    }
}
