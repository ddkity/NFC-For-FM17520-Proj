#include "main.h"


void SYS_Init(void)
{
#if 0	//调试OK版本，带晶振
	/* Unlock protected registers */
    SYS_UnlockReg();

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set P5 multi-function pins for XTAL1 and XTAL2 */
    SYS->P5_MFP &= ~(SYS_MFP_P50_Msk | SYS_MFP_P51_Msk);
    SYS->P5_MFP |= (SYS_MFP_P50_XT1_IN | SYS_MFP_P51_XT1_OUT);

    /* Enable external 12MHz XTAL, HIRC, LIRC (for timer)*/
    CLK->PWRCTL |= CLK_PWRCTL_XTL12M | CLK_PWRCTL_HIRCEN_Msk | CLK_PWRCTL_LIRCEN_Msk;

    /* Waiting for clock ready */																		/* 看门狗需要 */
    CLK_WaitClockReady(CLK_STATUS_XTLSTB_Msk | CLK_STATUS_HIRCSTB_Msk | CLK_PWRCTL_LIRCEN_Msk | CLK_STATUS_LIRCSTB_Msk);

	/* Enable Timer 0 clock */
    CLK->APBCLK = CLK_APBCLK_TMR0CKEN_Msk;

	/* Switch HCLK clock source to XTL */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_XTAL,CLK_CLKDIV_HCLK(1));

    /* STCLK to XTL STCLK to XTL */
    CLK_SetSysTickClockSrc(CLK_CLKSEL0_STCLKSEL_XTAL);

	/* Select Timer clock source from LIRC */
    CLK->CLKSEL1 = (CLK->CLKSEL1 & ~CLK_CLKSEL1_TMR0SEL_Msk) | CLK_CLKSEL1_TMR0SEL_LIRC;

    /* Enable IP clock */
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_EnableModuleClock(I2C0_MODULE);
	CLK_EnableModuleClock(WDT_MODULE);

    /* Select IP clock source */
    CLK_SetModuleClock(UART0_MODULE,CLK_CLKSEL1_UARTSEL_XTAL,CLK_CLKDIV_UART(1));
	CLK_SetModuleClock(WDT_MODULE, CLK_CLKSEL1_WDTSEL_LIRC, 0);

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set P3.4 and P3.5 for I2C SDA and SCL */
	SYS->P3_MFP &= ~(SYS_MFP_P34_Msk | SYS_MFP_P34_Msk);
    SYS->P3_MFP = SYS_MFP_P34_I2C0_SDA | SYS_MFP_P35_I2C0_SCL;

    /* Set P1 multi-function pins for UART RXD and TXD */
    SYS->P1_MFP &= ~(SYS_MFP_P12_Msk | SYS_MFP_P13_Msk);
    SYS->P1_MFP |= (SYS_MFP_P12_UART0_RXD | SYS_MFP_P13_UART0_TXD);

    /* Lock protected registers */
    SYS_LockReg();

    /* Update System Core Clock */
    SystemCoreClockUpdate();
#endif

#if 1	//无晶振版本调试
	/* Unlock protected registers */
    SYS_UnlockReg();

	/* Enable HIRC clock (Internal RC 22.1184MHz) */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);

	/* Wait for HIRC clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

	/* Enable LIRC clock (Internal RC 10k) */
    CLK_EnableXtalRC(CLK_PWRCTL_LIRCEN_Msk);

	/* Wait for LIRC clock ready */
    CLK_WaitClockReady(CLK_STATUS_LIRCSTB_Msk);

	/* Select HCLK clock source as HIRC and and HCLK source divider as 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV_HCLK(1));

	/* STCLK to HIRC */
    CLK_SetSysTickClockSrc(CLK_CLKSEL0_HCLKSEL_HIRC);

	CLK_SetCoreClock(FREQ_25MHZ);

	/* Enable IP clock */
    CLK_EnableModuleClock(I2C0_MODULE);
	CLK_EnableModuleClock(WDT_MODULE);
	CLK_EnableModuleClock(TMR0_MODULE);
	CLK_EnableModuleClock(SPI0_MODULE);

	/* Select IP clock source */
	CLK_SetModuleClock(WDT_MODULE, CLK_CLKSEL1_WDTSEL_LIRC, 0);
	CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_LIRC, 0);
	CLK_SetModuleClock(SPI0_MODULE, CLK_CLKSEL1_SPISEL_HCLK, 0);
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set P3.4 and P3.5 for I2C SDA and SCL */
	SYS->P3_MFP &= ~(SYS_MFP_P34_Msk | SYS_MFP_P34_Msk);
    SYS->P3_MFP = (SYS_MFP_P34_I2C0_SDA | SYS_MFP_P35_I2C0_SCL);

	/*	SPI pin config */
	SYS->P0_MFP &= ~(SYS_MFP_P04_Msk | SYS_MFP_P05_Msk | SYS_MFP_P06_Msk | SYS_MFP_P07_Msk);
	SYS->P0_MFP |= (SYS_MFP_P04_GPIO | SYS_MFP_P05_SPI0_MOSI | SYS_MFP_P06_SPI0_MISO | SYS_MFP_P07_SPI0_CLK);
	SYS->P2_MFP &= ~(SYS_MFP_P26_Msk);
	SYS->P2_MFP |= (SYS_MFP_P26_GPIO);

	/* other pin config */
	SYS->P1_MFP &= ~(SYS_MFP_P14_Msk | SYS_MFP_P15_Msk);	//天线s1和天线s2
	SYS->P1_MFP |= (SYS_MFP_P14_GPIO | SYS_MFP_P15_GPIO);
	SYS->P3_MFP &= ~(SYS_MFP_P32_Msk);	//LED管脚
	SYS->P3_MFP |= (SYS_MFP_P32_GPIO);
	
    /* Lock protected registers */
    SYS_LockReg();

    /* Update System Core Clock */
    SystemCoreClockUpdate();

	GPIO_SetMode(P2, BIT6, GPIO_MODE_OUTPUT);	//FM17520复位引脚
    GPIO_SetMode(P0, BIT4, GPIO_MODE_OUTPUT);	//SPI_CS管脚
		
#endif
}

void GPIO_Init(void)
{
	GPIO_SetMode(P1, BIT5 | BIT4, GPIO_MODE_OUTPUT);	//天线开关选择引脚
	GPIO_SetMode(P3, BIT2, GPIO_MODE_OUTPUT);	//LED管脚
	
	SN74_S0 = 0;
	SN74_S1 = 0;
	NFC_LED = 0;
}

void Delay_100us(unsigned int delay_time)		//0.1ms*delay_time
{
    unsigned int i;
    for(i = 0; i < delay_time; i++)
    {
        CLK_SysTickDelay(100);
    }
}


void Timer_Init(void)
{
	//TIMER0 debug
	/*Initial Timer0 to periodic mode with 1Hz */
    TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 1);		//每秒中断一次
    TIMER_SET_CMP_VALUE(TIMER0, 30000);		//重新设置timer->CMP的值，时钟为10000Hz，所以设置为10000的时候就是1秒中断，30000就是4秒中断
	 /* Enable timer wake up system */
    TIMER_EnableWakeup(TIMER0);
    /* Enable Timer0 interrupt */
    TIMER_EnableInt(TIMER0);
    NVIC_EnableIRQ(TMR0_IRQn);
    /* Start Timer0 counting */
    TIMER_Start(TIMER0);
}


void TMR0_IRQHandler(void)
{
	//需要同时清除两个标志位才能再次进入低功耗模式
	// Clear interrupt flag
    TIMER_ClearIntFlag(TIMER0);
	// Clear wake up flag
	TIMER_ClearWakeupFlag(TIMER0);
	s_div_count++;
	I2C_Count_Flag++;	//3s钟加1
}

void WDT_Init(void)
{
	SYS_UnlockReg();
	WWDT_Open(WWDT_PRESCALER_1536, 0x03, TRUE);	//打开看门狗，让程序重启
	NVIC_EnableIRQ(WDT_IRQn);
}


void WDT_IRQHandler(void)
{
	if(g_FeedDogFlag == 1)
	{
    	WWDT_RELOAD_COUNTER();
    	WWDT_CLEAR_INT_FLAG();
	}

	WWDT_CLEAR_INT_FLAG();
	g_FeedDogFlag = 0;
}

