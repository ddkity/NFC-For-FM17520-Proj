
#include "main.h"

/*****************************************************************************************/
/*���ƣ�ultra_read																		 */
/*���ܣ�ultralight��Ƭ����																 */
/*���룺	page�����ַ��0~15��														 */
/*�����	buff��������ָ�루���ݳ���Ϊ4�ֽڣ�											 */
/*       	OK: Ӧ����ȷ                                                              	 */
/*	 		ERROR: Ӧ�����																 */
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
/*���ƣ�ultra_write																		 */
/*���ܣ�ultralight��Ƭд��																 */
/*���룺	page��ҳ��ַ��0~15��														 */
/*�����	buff��д������ָ�루4�ֽڣ�													 */
/*       	OK: Ӧ����ȷ                                                              	 */
/*	 		ERROR: Ӧ�����																 */
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

    if ((ret != 0 ) | (rece_byte[0] != 0x0A))	//���δ���յ�0x0A����ʾ��ACK
        return ERROR;
    return OK;
}


/*****************************************************************************************/
/*���ƣ�ultra_read																		 */
/*���ܣ�ultralight��Ƭ��֤															 */
/* ��֤�ɹ�����0��ʧ�ܷ���-1 */

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


