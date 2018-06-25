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

uint8_t WDT_ResetFlag = 0;	//���Ź�����ģ���־
uint16_t CurrentTestMode = 0x00;	//�����������ģʽ����ģʽ��ģ��һֱ��Ѱ��
uint8_t I2C_HaveInterrupt = 0;	//��I2C��ȡ�����˳��͹���ģʽ��־
uint32_t s_div_count = 0;
uint32_t I2C_Count_Flag = 0;	//I2C������־����I2C��ȡ���ݣ��˱�־��0�������Сʱ��û�ж������ݵĻ���������
uint8_t  g_FeedDogFlag = 1;

int main(void)
{
	uint8_t ret;
	//��ʼ��˳��Ҫ��
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
		g_FeedDogFlag = 1;	//ι��
		
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

		//Ѱ�������Ϳ�Ƭ����
		ret = NFC_findTag();
		
		NFC_LED = 0;
		//��ʾ��Ѱ��������һ����־�������������û��Ѱ������ȫ�ֿ���Ϣ
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
					clear_tag_info(0);	//��־�������
				}
				if (g_poll_result.RES_T.tag2 == 0) {
					clear_tag_info(1);	//��־�������
				}
				if (g_poll_result.RES_T.tag3 == 0) {
					clear_tag_info(2);	//��־�������
				}
				if (g_poll_result.RES_T.tag4 == 0) {
					clear_tag_info(3);	//��־�������
				}
				g_poll_result.result = 0;
				g_nfc_running = 0;
				TIMER_Start(TIMER0);
				s_div_count = 0;
				FM175XX_DeepPowerdown(1);	//FM17520����͹���ģʽ		
				if(CurrentTestMode == 0){	//�ǵ�������ģʽ�²Ż����͹���ģʽ
					while(s_div_count < 1)
					{
						if(I2C_HaveInterrupt == 0){
							//SYS_UnlockReg();	//���ϵ͹��ĵĹ��ܻᵼ��I2C��һ�ζ����ݲ���ȷ
							//CLK_PowerDown();
						}
						if(CurrentTestMode != 0){	//��������ģʽ
							break;
						}
					}
					if(CurrentTestMode == 0){
						g_nfc_running = 1;
					}
				}
				else{
					CurrentTestMode++;
					if(CurrentTestMode >= 3)	//���xs��x��֮�䣬�޿�����x�룬4�ſ�����xs
					{
						CurrentTestMode = 0;	//�˳���������ģʽ
					}
				}
				FM175XX_DeepPowerdown(0);	//FM17520�˳��͹���ģʽ
				TIMER_Stop(TIMER0);
				I2C_HaveInterrupt = 0;
			}
		}

#if 0
		if(I2C_Count_Flag > 546)	//ÿ��3s����1�����1��Ѱ��ʱ�䣬450��ž���30���ӣ��м���Ѱ��ʱ�䣬Ӧ��ÿ���ټ�1�룬�ܹ��൱��4��
		{
			SYS_ResetChip();
		}
#endif
	}
}


