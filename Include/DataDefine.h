#ifndef __DATADEFINE_H__
#define __DATADEFINE_H__


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "ql_api_osi.h"
#include "ql_api_common.h"
#include "ql_uart.h"

#define DEBUG_LEVEL			1

extern void UART_Printf(uint8_t UARTx, char* str, ...);
#if DEBUG_LEVEL
	#define DebugPrint(String...)		 UART_Printf(QL_UART_PORT_1, String)
#else
	#define DebugPrint(String...)
#endif

// #define NWY_EXT_SIO_RX_MAX          (2*1024)

// #define NWY_EXT_INPUT_RECV_MSG       (NWY_APP_EVENT_ID_BASE + 1)
#define INIT_CONFIG				      (QL_EVENT_APP_START + 1)
#define MAIN_TICK_10MS              QL_EVENT_APP_START + 2
#define MAIN_TICK_100MS              QL_EVENT_APP_START + 3
#define MAIN_TICK_1000MS              QL_EVENT_APP_START + 4
#define MAIN_TICK_5000MS              QL_EVENT_APP_START + 5


#define TINY_BUFFER_SIZE			50
#define SMALL_BUFFER_SIZE			200
#define MEDIUM_BUFFER_SIZE			600
#define LARGE_BUFFER_SIZE			1500

#define NUMBER_OF_GSM_ERR		10

typedef enum
{
	SIM_INIT,
	SIM_OK,
	SIM_ERROR,
	SIM_NOT_INSERT,
	SIM_SLOT_ERROR,
	SIM_END
} SimState_e;



#define MAX(x,y) 				((x > y)? x : y)
#define MIN(x,y) 				((x < y)? x : y)
#define ABS(x) 					((x)>0 ? (x) : -(x))
#define CHECKVAL(val, min,max) 	((val < min || val > max) ? FALSE : TRUE)


typedef enum {
	EVENT_OK = 0,
	EVENT_TIMEOUT,
	EVENT_DONE,
	EVENT_ERROR
} ResponseEvent_t;

typedef struct
{
	uint8_t Buffer[TINY_BUFFER_SIZE];
	uint16_t Index;
	uint8_t State;
} TinyBuffer_t;

typedef struct
{
	uint8_t Buffer[SMALL_BUFFER_SIZE];
	uint16_t Index;
	uint8_t State;
} SmallBuffer_t;

typedef struct
{
	uint8_t Buffer[MEDIUM_BUFFER_SIZE];
	uint16_t Index;
	uint8_t State;
} MediumBuffer_t;

typedef struct
{
	uint8_t Buffer[LARGE_BUFFER_SIZE];
	uint16_t Index;
	uint8_t State;
} LargeBuffer_t;

typedef union {
    float value;
    uint8_t bytes[4];
} Float_t;

typedef union {
    uint32_t value;
    uint8_t bytes[4];
} Long_t;

typedef union{
	unsigned long long value;
	uint8_t bytes[8];
}LongLong_t;

typedef union {
    uint16_t value;
    uint8_t bytes[2];
} Int_t;

typedef struct
{
	uint8_t SubA;
	uint8_t SubB;
	uint8_t SubC;
	uint8_t SubD;
	uint16_t Port;
} IPStruct;

typedef struct {
    Float_t KinhDo;
    Float_t ViDo;
} Point_t;

typedef struct{
	char		ThreadName[20];
	ql_task_t 	Thread;
	void (*func) (void*);
}Thread_t;


typedef struct{
	uint8_t (*Init) (uint8_t UARTx, uint32_t baudrate);
	void (*Send) (uint8_t UARTx, uint8_t* Buffer, uint16_t len);
	void 				(*Putc)		(uint8_t UARTx, uint8_t c);
	void 				(*Puts)		(uint8_t UARTx, char* s);
	void 				(*Printf)	(uint8_t UARTx, char* str, ...);
}const DriverUART_t;


typedef struct{
	void (*Init) (uint8_t pin_map, uint8_t pin_func, uint8_t gpio_num, uint8_t gpio_dir, uint8_t gpio_pull, uint8_t gpio_lvl);
	void (*PinControl) (uint8_t gpio_num, uint8_t gpio_lvl);
	uint8_t (*ReadPin) (uint8_t gpio_num);
}const DriverGPIO_t;

typedef struct
{
	uint8_t InitSystemDone;
	uint8_t SysInitState;
	uint8_t InitConfig;
	uint32_t TickCount;
	uint8_t SIMState[10];
} Status_t;


typedef struct{
	uint8_t CSQ;
	// nwy_nw_regs_info_type_t RegInfo;
} NetworkInfo_t;


typedef struct
{
	Status_t Status;

	Thread_t 		GSM_Thread;
	Thread_t 		GPS_Thread;
	Thread_t 		MQTT_Thread;

	NetworkInfo_t 	NetworkInfo;
	DriverGPIO_t* 	GPIO;
} System_t;

void SendEventToThread(ql_task_t thread, uint32_t sig);
void SystemReset(uint8_t Reason);

#endif