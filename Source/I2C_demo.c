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
#include "main.h"
#include "ql_gpio.h"

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
#define PI (3.14592)
// typedef enum
// {
//     DET_STOP,
//     DET_MOVE,
//     //...
// } event_t;
// typedef struct
// {
//     float ax, ay, az;
// } lis_angel;

char check()
{
    uint8_t KQ = 0;
    ql_I2cRead(i2c_1, SalveAddr_r_8bit, W_A_I, &KQ, 1);
    //	kq=IC_Read(W_A_I);
    // QL_I2C_LOG("\nWAIT :0x%x\n", KQ);
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
// void ngat2(void *ctx)
// {
//     // mpu_read_reg(0x21); //read register to reset high-pass filter
//     // mpu_read_reg(0x26); //read register to set reference acceleration
//     // mpu_read_reg(0x31); //Read INT1_SRC to de-latch;
//     QL_I2C_LOG("\nDA NGAT 22\n");
//     ql_gpio_set_level(SENSOR_IN, LVL_HIGH);
//     // if(noti==false)
//     // {
//     //     OUT_LOG("da gui thong bao\n");
//     //     send_event();

//     // }
// }
void keu()
{
    ql_gpio_set_level(IO_SPEAKER, LVL_HIGH);
    ql_rtos_task_sleep_ms(500);
    ql_gpio_set_level(IO_SPEAKER, LVL_LOW);
    ql_rtos_task_sleep_ms(500);
    ql_gpio_set_level(IO_SPEAKER, LVL_HIGH);
    ql_rtos_task_sleep_ms(500);
    ql_gpio_set_level(IO_SPEAKER, LVL_LOW);
    ql_rtos_task_sleep_ms(500);
    ql_gpio_set_level(IO_SPEAKER, LVL_HIGH);
    ql_rtos_task_sleep_ms(500);
    ql_gpio_set_level(IO_SPEAKER, LVL_LOW);
}
void Acc_Init()
{
    // ql_int_register(SENSOR_IN, EDGE_TRIGGER,DEBOUNCE_EN,EDGE_RISING,PULL_DOWN,ngat2,NULL);
    // ql_int_enable(SENSOR_IN);
    if (check())
    {
        ql_rtos_task_sleep_ms(2000);
        init_sen();
        // QL_I2C_LOG("\n tim thay CAM BIEN\n");
        // mpu_write_reg(REG1, 0x2F); // Turn on the sensor, enable X, Y, and Z ODR = 100 Hz
        // mpu_write_reg(REG2, 0x09); // High-pass filter enabled on interrupt activity 1
        // mpu_write_reg(REG3, 0x40); // Interrupt activity 1 driven to INT1 pad
        // mpu_write_reg(REG4, 0x00); //+-2g
        // //  mpu_write_reg(REG6, 0x20); // High-pass filter enabled on interrupt activity 2
        // mpu_write_reg(REG5, 0x00); //08    // Default value is 00 for no latching. Interrupt signals on INT1 pin is not latched.
        // mpu_write_reg(INT1_THS, 0x20); // Threshold = 250 mg

        // mpu_write_reg(INT1_DURATION, 0x00); // Duration = 0
           mpu_read_reg8(LIS3DH_REG_REFERENCE);
        // mpu_write_reg(INT1_CFG, 0x2A); // Configure desired wake-up event
             uint8_t x = mpu_read_reg8(INT1_CFG);
             QL_I2C_LOG("doc cau hinh:%u\n", x);

    }
}
void init_sen()
{

 applySettings(50,6); // 10Hz, 2range (4,6,8,16g)
  //Detection threshold can be from 1 to 127 and depends on the Range
  //chosen above, change it and test accordingly to your application
  //Duration = timeDur x Seconds / sampleRate
 config_int(8,1); //thresold, time_du

}

void applySettings(accelSampleRate,accelRange)
{
	uint8_t dataToWrite = 0;  //Temporary variable
	
	//Build CTRL_REG1

	// page 16 set CTRL_REG1[3](LPen bit)
	dataToWrite |= 0x08;

	//  Convert ODR
	switch(accelSampleRate)
	{
		case 1:
		dataToWrite |= (0x01 << 4);
		break;
		case 10:
		dataToWrite |= (0x02 << 4);
		break;
		case 25:
		dataToWrite |= (0x03 << 4);
		break;
		case 50:
		dataToWrite |= (0x04 << 4);
		break;
		case 100:
		dataToWrite |= (0x05 << 4);
		break;
		case 200:
		dataToWrite |= (0x06 << 4);
		break;
		default:
		case 400:
		dataToWrite |= (0x07 << 4);
		break;
		case 1600:
		dataToWrite |= (0x08 << 4);
		break;
		case 5000:
		dataToWrite |= (0x09 << 4);
		break;
	}

	dataToWrite |= (1 & 0x01) << 2;
	dataToWrite |= (1 & 0x01) << 1;
	dataToWrite |= (1 & 0x01);
	//Now, write the patched together data

	mpu_write_reg(REG1, dataToWrite);

	//Build CTRL_REG4
	dataToWrite = 0; //Start Fresh!

	//  Convert scaling
	switch(accelRange)
	{	
		default:
		case 2:
		dataToWrite |= (0x00 << 4);
		break;
		case 4:
		dataToWrite |= (0x01 << 4);
		break;
		case 8:
		dataToWrite |= (0x02 << 4);
		break;
		case 16:
		dataToWrite |= (0x03 << 4);
		break;
	}

	//Now, write the patched together data
	mpu_write_reg(REG4, dataToWrite);
}
void config_int( uint8_t threshold,uint8_t timeDur,bool	polarity )
{
	uint8_t dataToWrite = 0;  //Temporary variable
	uint8_t regToWrite = INT1_CFG;

	//Build INT_CFG 0x30 or 0x34
	//Detect movement or stop
     dataToWrite |= 0x0A;
	// else 							dataToWrite |= 0x05;

	mpu_write_reg(regToWrite, dataToWrite);
	//Build INT_THS 0x32 or 0x36
	regToWrite += 2;
	mpu_write_reg(regToWrite, threshold);

	//Build INT_DURATION 0x33 or 0x37
	regToWrite++;

	mpu_write_reg(regToWrite, timeDur);

	dataToWrite = 0 | (polarity << 1);

	//Attach configuration to Interrupt X
    mpu_write_reg(REG3, 0x40);
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
    uint8 retry_count = 5;

    param_data[0] = (uint8)((RegData >> 8) & 0xFF);
    param_data[1] = (uint8)(RegData & 0xff);
    // RegAddr = (RegAddr << 1) & 0xFE;
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
    } while (--retry_count);
}

int16_t GetData(unsigned char Haddress, unsigned char Laddress)
{
    uint8_t H, L;
    ql_I2cRead(i2c_1, SalveAddr_r_8bit, Haddress, &H, 1);
    ql_I2cRead(i2c_1, SalveAddr_r_8bit, Laddress, &L, 1);
    // QL_I2C_LOG("H:%d L: %d\n", H, L);
    return (int16_t)((((uint16)H) << 8) | L);
}
// float rad2deg(float rad)
// {
//     return rad * 180 / PI;
// }

void print_ACC()
{
    x = GetData(OUT_XH, OUT_XL);
    y = GetData(OUT_YH, OUT_YL);
    z = GetData(OUT_ZH, OUT_ZL);
    float x_g, y_g, z_g;
    uint8_t lsb_value = 4; // +-4g
    x_g = lsb_value * ((float)x / LIS3DH_LSB16_TO_KILO_LSB10);
    y_g = lsb_value * ((float)y / LIS3DH_LSB16_TO_KILO_LSB10);
    z_g = lsb_value * ((float)z / LIS3DH_LSB16_TO_KILO_LSB10);
    x_g *= SENSORS_GRAVITY_EARTH;
    y_g *= SENSORS_GRAVITY_EARTH;
    z_g *= SENSORS_GRAVITY_EARTH;
   // char buff[50] = {0};
    // float ay = atan2(z_g, x_g);
    // float angle_degrees = rad2deg(ay);

    //sprintf(buff, "\nGx: %.2f  Gy:%.2f  Gz:%.2f  X:%d Y:%d Z:%d   \n", x_g, y_g, z_g, x, y, z);
    //QL_I2C_LOG(buff);
   // ql_LvlMode stt_sen;
    // ql_gpio_get_level(SENSOR_IN, &stt_sen);
    // if (stt_sen == LVL_HIGH)
    // {
    //     QL_I2C_LOG("\nNGAT int1: %d\n", stt_sen);
    //     keu();
    //     uint16_t dat;
    //     mpu_read_reg(0x21, &dat); // read register to reset high-pass filter
    //     mpu_read_reg(0x26, &dat); // read register to set reference acceleration
    //     mpu_read_reg(0x31, &dat); // Read INT1_SRC to de-latch;
    //     ql_gpio_set_level(SENSOR_IN, LVL_LOW);
    // }
}
void ql_i2c_demo_thread(void *param)
{
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
            QL_I2C_LOG("\ni2c failed\n");
        }
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