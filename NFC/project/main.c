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

uint8_t WDT_ResetFlag = 0;	//¿´ÃÅ¹·ÖØÆôÄ£¿é±êÖ¾
uint16_t CurrentTestMode = 0x00;	//½øÈëµçÁ÷²âÊÔÄ£Ê½£¬´ËÄ£Ê½ÏÂÄ£¿éÒ»Ö±ÔÚÑ°¿¨
uint8_t I2C_HaveInterrupt = 0;	//ÓĞI2C¶ÁÈ¡Êı¾İÍË³öµÍ¹¦ºÄÄ£Ê½±êÖ¾
uint32_t s_div_count = 0;
uint32_t I2C_Count_Flag = 0;	//I2C¼ÆÊı±êÖ¾£¬ÓĞI2C¶ÁÈ¡Êı¾İ£¬´Ë±êÖ¾Çå0£¬µ±°ë¸öĞ¡Ê±»¹Ã»ÓĞ¶Áµ½Êı¾İµÄ»°´ÓÆô³ÌĞò
uint8_t  g_FeedDogFlag = 1;

int main(void)
{
	uint8_t ret;
	//³õÊ¼»¯Ë³Ğò²»Òª±ä
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
		g_FeedDogFlag = 1;	//Î¹¹·
		
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

		//Ñ°¿¨²Ù×÷ºÍ¿¨Æ¬´¦Àí
		ret = NFC_findTag();
		
		NFC_LED = 0;
		//±íÊ¾ÓĞÑ°µ½¿¨£¬×öÒ»¸ö±êÖ¾£¬·½±ãÏÂÃæÇå³ıÃ»ÓĞÑ°µ½¿¨µÄÈ«¾Ö¿¨ĞÅÏ¢
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
					clear_tag_info(0);	//±êÖ¾Çå³ıº¯Êı
				}
				if (g_poll_result.RES_T.tag2 == 0) {
					clear_tag_info(1);	//±êÖ¾Çå³ıº¯Êı
				}
				if (g_poll_result.RES_T.tag3 == 0) {
					clear_tag_info(2);	//±êÖ¾Çå³ıº¯Êı
				}
				if (g_poll_result.RES_T.tag4 == 0) {
					clear_tag_info(3);	//±êÖ¾Çå³ıº¯Êı
				}
				g_poll_result.result = 0;
				g_nfc_running = 0;
				TIMER_Start(TIMER0);
				s_div_count = 0;
				FM175XX_DeepPowerdown(1);	//FM17520½øÈëµÍ¹¦ºÄÄ£Ê½		
				if(CurrentTestMode == 0){	//·ÇµçÁ÷²âÊÔÄ£Ê½ÏÂ²Å»á½øÈëµÍ¹¦ºÄÄ£Ê½
					while(s_div_count < 1)
					{
						if(I2C_HaveInterrupt == 0){
							//SYS_UnlockReg();	//¼ÓÉÏµÍ¹¦ºÄµÄ¹¦ÄÜ»áµ¼ÖÂI2CµÚÒ»´Î¶ÁÊı¾İ²»ÕıÈ·
							//CLK_PowerDown();
						}
						if(CurrentTestMode != 0){	//µçÁ÷²âÊÔÄ£Ê½
							break;
						}
					}
					if(CurrentTestMode == 0){
						g_nfc_running = 1;
					}
				}
				else{
					CurrentTestMode++;
					if(CurrentTestMode >= 3)	//´ó¸Åxsµ½xÃëÖ®¼ä£¬ÎŞ¿¨¾ÍÊÇxÃë£¬4ÕÅ¿¨¾ÍÊÇxs
					{
						CurrentTestMode = 0;	//ÍË³öµçÁ÷²âÊÔÄ£Ê½
					}
				}
				FM175XX_DeepPowerdown(0);	//FM17520ÍË³öµÍ¹¦ºÄÄ£Ê½
				TIMER_Stop(TIMER0);
				I2C_HaveInterrupt = 0;
			}
		}

#if 0
		if(I2C_Count_Flag > 546)	//Ã¿¸ô3sÔö¼Ó1£¼ÓÉÏ1ÃëÑ°¿¨Ê±¼ä£¬450´ó¸Å¾ÍÊÇ30·ÖÖÓ£¬ÖĞ¼äÓĞÑ°¿¨Ê±¼ä£¬Ó¦¸ÃÃ¿´ÎÔÙ¼õ1Ãë£¬×Ü¹²Ïàµ±ÓÚ4Ãë
		{
			SYS_ResetChip();
		}
#endif
	}
}


