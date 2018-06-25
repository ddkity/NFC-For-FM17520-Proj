#ifndef __SYSTEM_CONFIG_H__
#define __SYSTEM_CONFIG_H__

#define SN74_S0 P15		//天线开关选择
#define SN74_S1 P14		//天线开关选择
#define NFC_LED P32		//LED指示灯
#define	NPD		P26 /* RST pin, FM17520复位引脚 */
#define	SPI_CS	P04		//SPI_CS


#define Slave_Add 0x7E	//I2C slave address

void SYS_Init(void);
void GPIO_Init(void);
void Delay_100us(unsigned int delay_time);
void Timer_Init(void);
void WDT_Init(void);

#endif




