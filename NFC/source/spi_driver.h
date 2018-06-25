#ifndef __SPI_DRIVER_H__
#define __SPI_DRIVER_H__

uint8_t SPI_Init(void);
uint8_t SPI_DeInit(void);
uint8_t SPIRead(uint8_t addr);				//SPI¶Áº¯Êý
void SPIWrite(uint8_t add,uint8_t wrdata);	//SPIÐ´º¯Êý
void SPIRead_Sequence(uint8_t sequence_length,uint8_t addr,uint8_t *reg_value);
void SPIWrite_Sequence(uint8_t sequence_length,uint8_t addr,uint8_t *reg_value);


#endif


