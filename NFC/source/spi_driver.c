#include "main.h"
uint8_t SPI_Init(void)
{
	
	/*	SPI pin config */
	SYS->P0_MFP &= ~(SYS_MFP_P04_Msk | SYS_MFP_P05_Msk | SYS_MFP_P06_Msk | SYS_MFP_P07_Msk);
	SYS->P0_MFP |= (SYS_MFP_P04_GPIO | SYS_MFP_P05_SPI0_MOSI | SYS_MFP_P06_SPI0_MISO | SYS_MFP_P07_SPI0_CLK);
	SYS->P2_MFP &= ~(SYS_MFP_P26_Msk);
	SYS->P2_MFP |= (SYS_MFP_P26_GPIO);

	GPIO_SetMode(P2, BIT6, GPIO_MODE_OUTPUT);	//FM17520复位引脚
    GPIO_SetMode(P0, BIT4, GPIO_MODE_OUTPUT);	//SPI_CS管脚
    //片选芯片然后复位
    SPI_CS = 1;
    NPD = 1; // 3-5ms

	GPIO_SetMode(P1, BIT5 | BIT4, GPIO_MODE_OUTPUT);	//天线开关选择引脚
	GPIO_SetMode(P3, BIT2, GPIO_MODE_OUTPUT);	//LED管脚
	SN74_S0 = 0;
	SN74_S1 = 0;
	NFC_LED = 0;

    Delay_100us(30);

    /* Configure as a master, clock idle low, falling clock edge Tx, rising edge Rx and 8-bit transaction */
    /* Set IP clock divider. SPI clock rate = 5MHz */
    SPI_Open(SPI, SPI_MASTER, SPI_MODE_0, 8, 5000000);	//5MHz，FM17520的SPI最高支持10MHz

    SPI_ENABLE_3WIRE_MODE(SPI);
    SPI_DisableAutoSS(SPI);
    SPI_ClearTxFIFO(SPI);
    SPI_ClearRxFIFO(SPI);

    return OK;
}

uint8_t SPI_DeInit(void)
{
	/*	SPI pin config */
	SYS->P0_MFP &= ~(SYS_MFP_P04_Msk | SYS_MFP_P05_Msk | SYS_MFP_P06_Msk | SYS_MFP_P07_Msk);
	SYS->P0_MFP |= (SYS_MFP_P04_GPIO | SYS_MFP_P05_GPIO | SYS_MFP_P06_GPIO | SYS_MFP_P07_GPIO);
	SYS->P2_MFP &= ~(SYS_MFP_P26_Msk);
	SYS->P2_MFP |= (SYS_MFP_P26_GPIO);

	GPIO_SetMode(P2, BIT6, GPIO_MODE_INPUT);	//FM17520复位引脚
    GPIO_SetMode(P0, BIT4 | BIT5 | BIT6 | BIT7, GPIO_MODE_INPUT);	//SPI_CS管脚

	GPIO_SetMode(P1, BIT5 | BIT4, GPIO_MODE_INPUT);	//天线开关选择引脚
	GPIO_SetMode(P3, BIT2, GPIO_MODE_INPUT);	//LED管脚
	return OK;
}



/*******************************************************************************************************/
/*		addr:	读取FM175XX内的寄存器地址[0x00~0x3f]	reg_address						   */
/*		rddata:	读取的数据							reg_data						   */
/*******************************************************************************************************/
uint8_t SPIRead(uint8_t addr)				//SPI读函数
{
    uint8_t reg_value, send_data;

    SPI_CS = 0;

    send_data = addr << 1;
    send_data |= 0x80;

    SPI_WRITE_TX(SPI, send_data);
    SPI_TRIGGER(SPI);
    while(SPI_IS_BUSY(SPI));

    SPI_WRITE_TX(SPI, 0x00);
    SPI_TRIGGER(SPI);
    while(SPI_IS_BUSY(SPI));

    reg_value = SPI_READ_RX(SPI);

    SPI_CS = 1;

    return(reg_value);
}

//SPI连续读函数,用于READ FIFO函数
void SPIRead_Sequence(uint8_t sequence_length, uint8_t addr, uint8_t *reg_value)
{
    uint8_t i, send_data;

    if (sequence_length == 0)
        return;

    SPI_CS = 0;

    send_data = addr << 1;
    send_data |= 0x80;
    SPI_WRITE_TX(SPI, send_data);
    SPI_TRIGGER(SPI);
    while(SPI_IS_BUSY(SPI));

    for (i = 0; i < sequence_length; i++)
    {
        if (i == sequence_length - 1)
        {
            SPI_WRITE_TX(SPI, 0x00);
        }
        else
        {
            SPI_WRITE_TX(SPI, send_data);
        }
        
        SPI_TRIGGER(SPI);
        while(SPI_IS_BUSY(SPI));
        *(reg_value + i) = SPI_READ_RX(SPI);
    }

    SPI_CS = 1;
    return;
}

/*******************************************************************************************************/
/*		add:	写入FM17XX内的寄存器地址[0x00~0x3f]	  reg_address							   */
/*		wrdata:   要写入的数据						  reg_data						   */
/*******************************************************************************************************/
void SPIWrite(uint8_t addr, uint8_t wrdata)	//SPI写函数
{
    uint8_t send_data;

    SPI_CS = 0;
    send_data = addr << 1;
    send_data = send_data & 0x7e;

    SPI_WRITE_TX(SPI, send_data);
    SPI_TRIGGER(SPI);
    while(SPI_IS_BUSY(SPI));

    SPI_WRITE_TX(SPI, wrdata);
    SPI_TRIGGER(SPI);
    while(SPI_IS_BUSY(SPI));

    SPI_CS = 1;
    return ;
}

//SPI连续写函数,用于WRITE FIFO函数
void SPIWrite_Sequence(uint8_t sequence_length, uint8_t addr, uint8_t *reg_value)
{
    uint8_t send_data, i;

    if(sequence_length == 0)
        return;

    SPI_CS = 0;

    send_data = addr << 1;
    send_data = send_data & 0x7e;

    SPI_WRITE_TX(SPI, send_data);
    SPI_TRIGGER(SPI);
    while(SPI_IS_BUSY(SPI));

    for (i = 0; i < sequence_length; i++)
    {
        SPI_WRITE_TX(SPI, *(reg_value + i));
        SPI_TRIGGER(SPI);
        while(SPI_IS_BUSY(SPI));
    }

    SPI_CS = 1;
    return ;
}

