
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "ql_gpio.h"
#include "ql_uart.h"

#include "DataDefine.h"

extern void DebugUart_rev_callback(uint32 ind_type, ql_uart_port_number_e port, uint32 size);

uint8_t UART_Init(uint8_t UARTx, uint32_t baudrate){

    ql_uart_config_s uartConfig;
    uint8_t ret;

    uartConfig.baudrate = baudrate;
    uartConfig.data_bit = 8;
    uartConfig.flow_ctrl = 0;
    uartConfig.parity_bit = 0;
    uartConfig.stop_bit = 1;

    ql_uart_set_dcbconfig(UARTx, &uartConfig);

    ret = ql_uart_open(UARTx);
    if(ret == 0){
        if(UARTx == QL_UART_PORT_1)  ret = ql_uart_register_cb(UARTx, DebugUart_rev_callback);
    }

    return ret;
}

void UART_Send(uint8_t UARTx, uint8_t* Str, uint16_t len){

    ql_uart_write(UARTx, Str, len);
}

void UART_Putc(uint8_t UARTx, uint8_t c){
    ql_uart_write(UARTx, &c, 1);
}

void UART_Puts(uint8_t UARTx, char* s)
{
	 while(*s != '\0')
    {		
		UART_Putc(UARTx, (uint16_t) (*s));
		s++;
    }	
}



void USARTItoa(uint8_t UARTx,long val, int radix, int len)
{
    uint8_t c, r, sgn = 0, pad = ' ';    
    uint8_t s[20], i = 0;    
    uint32_t v;

    if (radix < 0)
    {    
        radix = -radix;    
        if (val < 0) 
        {
    
            val = -val;    
            sgn = '-';    
        }    
    }    
    v = val;    
    r = radix;    
    if (len < 0) 
    {    
        len = -len;    
        pad = '0';    
    }    
    if (len > 20) return;    
    do 
    {    
        c = (uint8_t)(v % r);    
        if (c >= 10) c += 7;    
        c += '0';    
        s[i++] = c;    
        v /= r;    
    } while (v);    
    if (sgn) s[i++] = sgn;    
    while (i < len)    
        s[i++] = pad;    
    do    
        UART_Putc(UARTx,s[--i]);    
    while (i);

}


void UART_Printf(uint8_t UARTx, char* str, ...)
{

  	va_list arp;
    int d, r, w, s, l;
    va_start(arp, str);

	while ((d = *str++) != 0) 
        {
            if (d != '%')
            {    
                UART_Putc(UARTx, d); continue;    
            }
            d = *str++; w = r = s = l = 0;    
            if (d == '0') 
            {    
                d = *str++; s = 1;    
            }    
            while ((d >= '0')&&(d <= '9')) 
            {    
                w += w * 10 + (d - '0');    
                d = *str++;    
            }    
            if (s) w = -w;    
            if (d == 'l') 
            {    
                l = 1;    
                d = *str++;    
            }    
            if (!d) break;    
            if (d == 's') 
            {    
                UART_Puts(UARTx,va_arg(arp, char*));    
                continue;    
            }    
            if (d == 'c') 
            {    
                UART_Putc(UARTx,(char)va_arg(arp, int));    
                continue;    
            }    
            if (d == 'u') r = 10;    
            if (d == 'd') r = -10;    
            if (d == 'X' || d == 'x') r = 16; // 'x' added by mthomas in increase compatibility    
            if (d == 'b') r = 2;    
            if (!r) break;    
            if (l) 
            {    
                USARTItoa(UARTx,(long)va_arg(arp, long), r, w);    
            } 
            else 
            {    
                if (r > 0)        
                    USARTItoa(UARTx,(unsigned long)va_arg(arp, int), r, w);        
                else        
                    USARTItoa(UARTx,(long)va_arg(arp, int), r, w);
            }    
        }  
        va_end(arp);
	
}

DriverUART_t DriverUART = {
    UART_Init,
	UART_Send,
	UART_Putc,
	UART_Puts,
	UART_Printf
};
