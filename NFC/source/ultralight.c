
#include "main.h"

/*****************************************************************************************/
/*名称：ultra_read																		 */
/*功能：ultralight卡片读块																 */
/*输入：	page：块地址（0~15）														 */
/*输出：	buff：块数据指针（数据长度为4字节）											 */
/*       	OK: 应答正确                                                              	 */
/*	 		ERROR: 应答错误																 */
/*****************************************************************************************/
uint8_t Ultra_Read(uint8_t page, uint8_t *buff)
{
    unsigned char ret;
    unsigned char send_byte[2];
    unsigned int rece_bitlen;
    Write_Reg(TxModeReg, 0x80);
    Write_Reg(RxModeReg, 0x80);
    Write_Reg(BitFramingReg, 0x00);
    send_byte[0] = 0x30;
    send_byte[1] = page;
    Pcd_SetTimer(1);
    ret = Pcd_Comm(Transceive, send_byte, 2, buff, &rece_bitlen);
    return ret;
}

/*****************************************************************************************/
/*名称：ultra_write																		 */
/*功能：ultralight卡片写块																 */
/*输入：	page：页地址（0~15）														 */
/*输出：	buff：写块数据指针（4字节）													 */
/*       	OK: 应答正确                                                              	 */
/*	 		ERROR: 应答错误																 */
/*****************************************************************************************/
uint8_t Ultra_Write(uint8_t page, uint8_t *buff)
{
    unsigned char ret;
    unsigned char send_byte[6], rece_byte[1];
    unsigned int rece_bitlen;
    send_byte[0] = 0xA2;
    send_byte[1] = page;
    send_byte[2] = buff[0];
    send_byte[3] = buff[1];
    send_byte[4] = buff[2];
    send_byte[5] = buff[3];
    Pcd_SetTimer(10);

    ret = Pcd_Comm(Transceive, send_byte, 6, rece_byte, &rece_bitlen);

    if ((ret != 0 ) | (rece_byte[0] != 0x0A))	//如果未接收到0x0A，表示无ACK
        return ERROR;
    return OK;
}


/*****************************************************************************************/
/*名称：ultra_read																		 */
/*功能：ultralight卡片认证															 */
/* 认证成功返回0，失败返回-1 */

/*****************************************************************************************/
uint8_t Ultra_PwdAuth(uint8_t pwd[4], uint8_t pack[2])
{
	unsigned char ui8Status = 1;
    unsigned char send_byte[5], rece_byte[2];
    unsigned int rece_bitlen;

	ui8Status = OK;
    send_byte[0] = 0x1B;
    send_byte[1] = pwd[0];
    send_byte[2] = pwd[1];
    send_byte[3] = pwd[2];
    send_byte[4] = pwd[3];
    Pcd_SetTimer(10);

  	Pcd_Comm(Transceive, send_byte, 5, rece_byte, &rece_bitlen);
	if ((rece_byte[0] != pack[0]) || (rece_byte[1] != pack[1]))
	{
		ui8Status = ERROR;		// Mark tag has been successfully read
	}
	
    return ui8Status;
}


