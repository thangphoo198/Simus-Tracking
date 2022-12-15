#include "i2c.h"
#include "ql_api_osi.h"
#include "ql_api_dev.h"
#include "ql_gpio.h"
#define SCL  41
#define SDA  42
// ham khoi tao i2c, de khoi tao i2c, chan SDA phai dc keo xuong muc 0 trong khi chan SCL dang o muc 1
// sau 1 khoang thoi gian ngan (1ms) thi keo chan SCL xuong muc 0, tai thoi gian nay qua trinh khoi tao hoan tat!

void i2c_start()
{
//keo 2 chan len muc 1
    ql_gpio_set_level(SCL, LVL_HIGH); 
    ql_gpio_set_level(SDA, LVL_HIGH); 

	ql_gpio_set_level(SDA, LVL_LOW); 
	ql_rtos_task_sleep_ms(1);
    ql_gpio_set_level(SCL, LVL_LOW); 
    ql_gpio_set_level(SDA, LVL_HIGH); 
// qua trinh truyen du lieu bat dau

	
}
// ham stop i2c: de ket thuc 1 qua trinh truyen du lieu
// cach lam: dua chan sda len muc 1 trong khi chan scl dang o muc 1
void i2c_stop()
{
    ql_gpio_set_level(SDA, LVL_LOW); 
    ql_gpio_set_level(SCL, LVL_HIGH); 
    ql_gpio_set_level(SDA, LVL_HIGH); 
	//SDA=0; // keo chan sda xuong 0 de chuan bi dua len 1//
    //stop thanh cong
	
}
void i2c_write(unsigned char b)
{
	unsigned char i;
	char outbit;
	for(i=0;i<8;i++)
	{
		outbit= b&0x80; //lay bit cao truyen di truoc 
        if(outbit=0x01) ql_gpio_set_level(SCL, LVL_HIGH); 
        else ql_gpio_set_level(SCL, LVL_LOW); 
		// SDA=outbit;
		b<<=1;
	ql_gpio_set_level(SCL, LVL_HIGH); 
	ql_gpio_set_level(SCL, LVL_LOW); 
     // tao xung dich du lieu	
	}
	

//	SDA = 1;	
ql_gpio_set_level(SCL, LVL_HIGH); 
	//SCL = 1;	  // dua SDA va SCL len muc 1 chuan bi doc bit ack
//	outbit = SDA;	// lay bit ack tu chan SDA
	ql_gpio_set_level(SCL, LVL_LOW); 
  //  SCL = 0;		// cho SCL ve 0	 
//	return(outbit);	// lay gia tri ve	
}

unsigned char i2c_read()
{
	unsigned char i, dat;
	bool inbit;	
	dat = 0;
	for(i=1;i<=8;i++)	// moi lan doc 1 bit. 8 lan doc = 1 byte
	 {
        ql_gpio_set_level(SCL, LVL_HIGH); 	   // cho SCL len =1 , slaver se gui 1 bit vao chan SDA
		//inbit = SDA;	// lay gia tri cua chan SDA gán vao inbit
        ql_LvlMode BIT;
        ql_gpio_get_level(SDA, &BIT);
        if(BIT==LVL_LOW) inbit=0;
        else inbit =1;
		dat <<= 1;	 // dich byte data sang trái  1 lan
		dat |= inbit;	// muc dich la gán giá tri cua bit inbit vao byte dat
		ql_gpio_set_level(SCL, LVL_LOW); 
	// cho SLC xuong muc 0 san sang cho lan doc bit tiep thep
	 }
//	if (ack) SDA = 0;	 // cac cau lenh  nay dung de doc bit ack ( co the k can quan tam)
//	else SDA = 1;
	//SCL = 1;	
    ql_gpio_set_level(SCL, LVL_HIGH); 
    ql_gpio_set_level(SCL, LVL_LOW); 
//	SCL = 0;	
//	SDA = 1;							 
	return(dat);
}

void acc_write(unsigned char add, unsigned char dat) // chuyen dat sang BCD truoc khi gui
{
	i2c_start();
// bat dau ghi	
	i2c_write(0x32); //0xd0 la dia chi cua ds1307
	i2c_write(add);	// gia tri can ghi
    i2c_write(dat);
	//i2c_write(((dat/10)<<4)|(dat%10)); 	 // do du lieu trong ds la BCD nen ta can chuyen ca gia tri sang BCD ( day cau lenh chuyen du lieu sang BCD)
	i2c_stop();
}
unsigned char acc_read(unsigned char add)
{
unsigned char dat;
	
	i2c_start();   // bat dau doc
	
	i2c_write(0x32); 	// dau tien gui lenh ghi du lieu(ghi dia chi can lay du lieu trong DS1307)
	i2c_write(add); // Dia chi ma ta muon doc ( vi du, muon doc giay thi ta ghi dia chi 0x00)
	i2c_start(); 	 // bat dau doc du lieu
	i2c_write(0x33);  	// gui ma lenh doc du lieu tu dia chi(add)
	dat = i2c_read();	  // doc xong thi luu gia tri da doc dc vao dat
	i2c_stop();			// ket thuc qua trinh doc du lieu
	//dat = (dat & 0x0f) + (dat>>4)*10;	 // du lieu doc ra o dang BCD nen chuyen sang he 10					  
	return (dat);  // tra ve gia tri da doc duoc
}
void get_data(unsigned char *h,unsigned char *m,unsigned char *s)	// muc dich cua ham nay la lay gia tri thoi gian trong ds1307
{
   *h=ds_read(0x28);  
   *m=ds_read(0x29); 
   *s=ds_read(0x2A); 
}

char acc_check()
{
   char KQ = 0;
   // ql_I2cRead(i2c_1, SalveAddr_r_8bit, W_A_I, &KQ, 1);
    KQ= acc_read(0x0F);
    //	kq=IC_Read(W_A_I);
   // QL_I2C_LOG("WAIT :0x%x\n", KQ);
    if (KQ == 0x33)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
void acc_init()
{
    ql_rtos_task_sleep_ms(1);
    acc_write(0x20,0x57);
    //ql_I2cWrite(i2c_1, SalveAddr_w_8bit, REG1, 0x57, 1);
    // ql_I2cWrite(i2c_1, SalveAddr_w_8bit, REG2, 0x00, 1);
    // ql_I2cWrite(i2c_1, SalveAddr_w_8bit, REG3, 0x40, 1);
    // ql_I2cWrite(i2c_1, SalveAddr_w_8bit, REG4, 0x18, 1);
    acc_write(0x23,0x18);
}