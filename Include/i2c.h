#ifndef _I2C_H_
#define _I2C_H_
void i2c_start();
void i2c_stop();
void i2c_write(unsigned char b);
unsigned char i2c_read();
void acc_write(unsigned char add, unsigned char dat) ;
unsigned char acc_read(unsigned char add);
void get_data(unsigned char *h,unsigned char *m,unsigned char *s);
#endif