#ifndef __MAIN_H__
#define __MAIN_H__

//system include
#include <stdio.h>
#include "Mini58Series.h"
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

//protocol include
#include "system_config.h"
#include "spi_driver.h"
#include "fm175xx.h"

//#include "cpu_card.h"
//#include "mifare_card.h"
#include "type_a.h"
//#include "type_b.h"
//#include "type_f.h"
#include "ultralight.h"
#include "nfc_app.h"
#include "i2c_driver.h"

//include des
#include "des.h"

//定义常量
#define OK (0)
#define ERROR (1)
#define TIMEOUT_Err	(0x20)
#define True	    (1)
#define False	    (0)


//天线选择开关
#define ANTENNA_SELECT_ANT1       {SN74_S0 = 0;SN74_S1 = 0;}
#define ANTENNA_SELECT_ANT2       {SN74_S0 = 1;SN74_S1 = 0;}
#define ANTENNA_SELECT_ANT3       {SN74_S0 = 0;SN74_S1 = 1;}
#define ANTENNA_SELECT_ANT4       {SN74_S0 = 1;SN74_S1 = 1;}

typedef union __TAG_POLL_RES {
	uint16_t result;
	struct {
		uint16_t tag1:4;
		uint16_t tag2:4;
		uint16_t tag3:4;
		uint16_t tag4:4;
	}RES_T;
}TAG_POLL_RES_T;

extern uint8_t WDT_ResetFlag;	//看门狗重启模块标志
extern uint16_t CurrentTestMode;	//进入电流测试模式，此模式下模块一直在寻卡
extern uint8_t I2C_HaveInterrupt;	//有I2C读取数据退出低功耗模式标志
extern uint32_t s_div_count;
extern uint32_t I2C_Count_Flag;	//I2C计数标志，有I2C读取数据，此标志清0，当半个小时还没有读到数据的话从启程序
extern uint8_t g_nfc_running;
extern uint8_t  g_FeedDogFlag;	//喂狗标志

#endif





