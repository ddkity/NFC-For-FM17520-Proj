/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 3 $
 * $Date: 15/02/13 2:19p $
 * @brief    A project template for Mini58 MCU.
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include "main.h"

static uint8_t s_ant_select = 0;
static uint8_t s_poll_count = 0;
uint8_t g_nfc_running = 1;
TAG_POLL_RES_T g_poll_result = {0};

uint8_t WDT_ResetFlag = 0;	//看门狗重启模块标志
uint16_t CurrentTestMode = 0x00;	//进入电流测试模式，此模式下模块一直在寻卡
uint8_t I2C_HaveInterrupt = 0;	//有I2C读取数据退出低功耗模式标志
uint32_t s_div_count = 0;
uint32_t I2C_Count_Flag = 0;	//I2C计数标志，有I2C读取数据，此标志清0，当半个小时还没有读到数据的话从启程序
uint8_t  g_FeedDogFlag = 1;

int main(void)
{
	uint8_t ret;
	//初始化顺序不要变
	SYS_Init();
	GPIO_Init();
	SPI_Init();
	I2C_Init();	
	Timer_Init();
    SYS_UnlockReg();
	WDT_Init();
	g_FeedDogFlag = 1;
    WDT_ResetFlag = 0x00;
	while(1)
	{
		g_FeedDogFlag = 1;	//喂狗
		
		if((g_poll_result.result == 0) && (CurrentTestMode == 0))
	    {
	      clear_tag_info(0);
	      clear_tag_info(1);
	      clear_tag_info(2);
	      clear_tag_info(3);    
	    }
		
		set_cur_tag_id(s_ant_select);
		switch(s_ant_select)
		{
			case 0:
				ANTENNA_SELECT_ANT1;
				break;
			case 1:
				ANTENNA_SELECT_ANT2;   
				break;
			case 2:
				ANTENNA_SELECT_ANT3;
				break;
			case 3:
				ANTENNA_SELECT_ANT4;
				break;	
		}

		//寻卡操作和卡片处理
		ret = NFC_findTag();
		
		NFC_LED = 0;
		//表示有寻到卡，做一个标志，方便下面清除没有寻到卡的全局卡信息
		if (ret != 0) {
			g_poll_result.result |= ((1 << s_ant_select*4) << s_poll_count);
		}

		s_ant_select++;
		if(s_ant_select >= 4)
		{
			s_ant_select = 0;
			s_poll_count++;

			if (s_poll_count >= 4) 
			{
				s_poll_count = 0;
				//NFC_test();
				//NFC_Adjustment();
				//NFC_test();
				if (g_poll_result.RES_T.tag1 == 0) {
					clear_tag_info(0);	//标志清除函数
				}
				if (g_poll_result.RES_T.tag2 == 0) {
					clear_tag_info(1);	//标志清除函数
				}
				if (g_poll_result.RES_T.tag3 == 0) {
					clear_tag_info(2);	//标志清除函数
				}
				if (g_poll_result.RES_T.tag4 == 0) {
					clear_tag_info(3);	//标志清除函数
				}
				g_poll_result.result = 0;
				g_nfc_running = 0;
				TIMER_Start(TIMER0);
				s_div_count = 0;
				FM175XX_DeepPowerdown(1);	//FM17520进入低功耗模式		
				if(CurrentTestMode == 0){	//非电流测试模式下才会进入低功耗模式
					while(s_div_count < 1)
					{
						if(I2C_HaveInterrupt == 0){
							//SYS_UnlockReg();	//加上低功耗的功能会导致I2C第一次读数据不正确
							//CLK_PowerDown();
						}
						if(CurrentTestMode != 0){	//电流测试模式
							break;
						}
					}
					if(CurrentTestMode == 0){
						g_nfc_running = 1;
					}
				}
				else{
					CurrentTestMode++;
					if(CurrentTestMode >= 3)	//大概xs到x秒之间，无卡就是x秒，4张卡就是xs
					{
						CurrentTestMode = 0;	//退出电流测试模式
					}
				}
				FM175XX_DeepPowerdown(0);	//FM17520退出低功耗模式
				TIMER_Stop(TIMER0);
				I2C_HaveInterrupt = 0;
			}
		}

#if 0
		if(I2C_Count_Flag > 546)	//每隔3s增加1＜由�1秒寻卡时间，450大概就是30分钟，中间有寻卡时间，应该每次再减1秒，总共相当于4秒
		{
			SYS_ResetChip();
		}
#endif
	}
}


