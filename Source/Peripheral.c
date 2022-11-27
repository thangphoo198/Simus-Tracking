

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ql_api_osi.h"
#include "ql_log.h"
#include "ql_api_camera.h"
#include "ql_i2c.h"
#include "DataDefine.h"


#define SalveAddr_w_8bit        (0x42 >> 1)
#define SalveAddr_r_8bit        (0x43 >> 1)

#define RegAddr_Read			0xf0
#define RegAddr_Write			0x55


uint8_t read_data = 0;
uint8_t data = 0xaa;



void Init_Peripheral(void){

	ql_I2cInit(i2c_1, STANDARD_MODE);

	ql_I2cRead (i2c_1, SalveAddr_r_8bit, RegAddr_Read,  &read_data, 1);
	ql_I2cWrite(i2c_1, SalveAddr_w_8bit, RegAddr_Write, &data, 1);

}


