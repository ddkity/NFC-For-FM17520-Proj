/*************************************************************/
//2015.04.13�޸İ�
/*************************************************************/
#include "main.h"

struct PICC_A_STR PICC_A;

#define Anticoll_Debug 0
#define Request_Debug 0

/****************************************************************************************/
/*���ƣ�TypeA_Request																	*/
/*���ܣ�TypeA_Request��ƬѰ��															*/
/*���룺																				*/
/*       			    			     												*/
/*	       								 												*/
/*�����																			 	*/
/*	       	pTagType[0],pTagType[1] =ATQA                                         		*/
/*       	OK: Ӧ����ȷ                                                              	*/
/*	 		ERROR: Ӧ�����																*/
/****************************************************************************************/
unsigned char TypeA_Request(unsigned char *pTagType)
{
    unsigned char result, send_buff[1], rece_buff[2];
    unsigned int rece_bitlen = 0;
    Clear_BitMask(TxModeReg, 0x80); //�ر�TX CRC
    Clear_BitMask(RxModeReg, 0x80); //�ر�RX CRC
    Set_BitMask(RxModeReg, 0x08);//�ر�λ����
    Clear_BitMask(Status2Reg, 0x08);
    Write_Reg(BitFramingReg, 0x07);
    send_buff[0] = 0x26;
    Pcd_SetTimer(1);
#if Request_Debug
    Uart_Send_Msg("Request start\r\n");
#endif
    result = Pcd_Comm(Transceive, send_buff, 1, rece_buff, &rece_bitlen);  //����REQA������ѯ
    if ((result == OK) && (rece_bitlen == 2 * 8))
    {
        *pTagType = rece_buff[0];
        *(pTagType + 1) = rece_buff[1];
#if Request_Debug
        Uart_Send_Msg("Request ok\r\n");
#endif
        return OK;
    }
#if Request_Debug
    Uart_Send_Msg("Request error\r\n");
#endif
    return ERROR;
}

/****************************************************************************************/
/*���ƣ�TypeA_WakeUp																	*/
/*���ܣ�TypeA_WakeUp��ƬѰ��															*/
/*���룺																				*/
/*       			    			     												*/
/*	       								 												*/
/*�����																			 	*/
/*	       	pTagType[0],pTagType[1] =ATQA                                         		*/
/*       	OK: Ӧ����ȷ                                                              	*/
/*	 		ERROR: Ӧ�����																*/
/****************************************************************************************/
unsigned char TypeA_WakeUp(unsigned char *pTagType)
{
    unsigned char result, send_buff[1], rece_buff[2];
    unsigned int rece_bitlen;
    Clear_BitMask(TxModeReg, 0x80); //�ر�TX CRC
    Clear_BitMask(RxModeReg, 0x80); //�ر�RX CRC
    Set_BitMask(RxModeReg, 0x08);//�ر�λ����
    Clear_BitMask(Status2Reg, 0x08);
    Write_Reg(BitFramingReg, 0x07);
    send_buff[0] = 0x52;
    Pcd_SetTimer(2);
    result = Pcd_Comm(Transceive, send_buff, 1, rece_buff, &rece_bitlen);

    if ((result == OK) && (rece_bitlen == 2 * 8))
    {
        *pTagType     = rece_buff[0];
        *(pTagType + 1) = rece_buff[1];
    }
    return result;
}

/****************************************************************************************/
/*���ƣ�TypeA_Anticollision																*/
/*���ܣ�TypeA_Anticollision��Ƭ����ͻ													*/
/*���룺selcode =0x93��0x95��0x97														*/
/*       			    			     												*/
/*	       								 												*/
/*�����																			 	*/
/*	       	pSnr[0],pSnr[1],pSnr[2],pSnr[3]pSnr[4] =UID                            		*/
/*       	OK: Ӧ����ȷ                                                              	*/
/*	 		ERROR: Ӧ�����																*/
/****************************************************************************************/
unsigned char TypeA_Anticollision(unsigned char selcode, unsigned char *pSnr)
{
    unsigned char result, send_buff[7], rece_buff[5], CollPos, NVB, Row, Col, Bit_Framing, Send_Len, Snr[5], i, Rece_Len;

    unsigned int rece_bitlen = 0;
    memset(rece_buff, 0x00, 5);
    memset(Snr, 0x00, 5); //��ʱUID
    memset(pSnr, 0x00, 5);
    NVB = 0x20; //NVB��ʼֵ
    Bit_Framing = 0x00; //Bit_Framing��ʼֵ
    Row = 0; //�У�0Ϊ�޳�ͻ��bit1~bit8
    Col = 0; //�У�0Ϊ�޳�ͻ��1~4
    CollPos = 0;
    //4��8�У���32��λ�ã���ӦCollPos��ֵ 0~31
    Clear_BitMask(TxModeReg, 0x80);
    Clear_BitMask(RxModeReg, 0x80);
    Write_Reg(CollReg, 0x80); //��ͻλ��Ľ�������Ϊ0
    //Write_Reg(RxThresholdReg,0x44);//0x18�Ĵ���

    Write_Reg(MfRxReg, 0x00); //ʹ����żУ��
    //Clear_BitMask(CollReg, 0x80);

    while(1)
    {
        send_buff[0] = selcode;

        send_buff[1] = NVB;


        Write_Reg(BitFramingReg, Bit_Framing); //���÷��͡����յ�bit����
        memcpy(send_buff + 2, pSnr, Col);
        Send_Len = Col + 2; //���÷����ֽ���

        Pcd_SetTimer(1);

        result = Pcd_Comm(Transceive, send_buff, Send_Len, rece_buff, &rece_bitlen);

        if (result == TIMEOUT_Err)
        {
            return ERROR;
        }


        if(result & 0x08) //CollErr
        {
            //���ֳ�ͻ����
            CollPos = Read_Reg(CollReg) & 0x1F; //��ȡ��ͻλ��

            TypeA_Set_NVB(CollPos, &NVB, &Row, &Col); //�����´ν�����Ҫ��NVB�����汾���յ���bit����������


            TypeA_Set_Bit_Framing(CollPos, &Bit_Framing); //�����´ν�����Ҫ���͡����յ�bit����
            memcpy(Snr, rece_buff, Col); //������յ��Ĳ���UID��Snr
            Snr[Col - 1] = Snr[Col - 1] & Row; //������յ�����ȷbit

            for (i = 0; i < Col; i++)
            {
                pSnr[i] = pSnr[i] | Snr[i]; //���Snr��pSnr�����浽pSnr
            }
        }
        else
        {
            //û�з��ֳ�ͻ
            if(rece_bitlen > 8)
                Rece_Len = rece_bitlen / 8;
            else
                Rece_Len = 1;
            if(Col == 0)
            {
                //�ϴν����޳�ͻ
                if ((Rece_Len + Col) != 5)
                {
                    return ERROR;
                }
                memcpy(pSnr, rece_buff, Rece_Len); //����rece buff�� pSnr
            }
            else
            {
                //�ϴν����г�ͻ
                if ((Rece_Len + Col) != 6)
                {
                    return ERROR;
                }

                memcpy(Snr + Col - 1, rece_buff, Rece_Len); //����rece buff�� Snr
                for (i = 0; i < Rece_Len; i++)
                {
                    pSnr[i + Col - 1] = pSnr[i + Col - 1] | Snr[i + Col - 1]; //���Snr��pSnr�����浽pSnr
                }
            }

            if (pSnr[4] == (pSnr[0]^pSnr[1]^pSnr[2]^pSnr[3]))
            {
                return OK;
            }
            else
            {
                return ERROR;
            }
        }
    }
	
    //return result;
}


void TypeA_Set_NVB(unsigned char collpos, unsigned char *nvb, unsigned char *row, unsigned char *col)
{
    if (collpos == 0)
    {
        *nvb = 0x58;
        *col = 4;
    }
    if ((collpos > 0) && (collpos < 9)) //1~8
    {
        *nvb = 0x20 + (collpos); //21~28
        *col = 1;
    }
    if ((collpos > 8) && (collpos < 17)) //9~16
    {
        *nvb = 0x30 + (collpos - 8); //31~38
        *col = 2;
    }
    if ((collpos > 16) && (collpos < 25)) //17~24
    {
        *nvb = 0x40 + (collpos - 16); //41~48
        *col = 3;
    }
    if ((collpos > 24) && (collpos < 32)) //25~31
    {
        *nvb = 0x50 + (collpos - 24); //51~57
        *col = 4;
    }

    switch ((*nvb) & 0x07)
    {
    case 1:
        *row = 0x01;
        break;
    case 2:
        *row = 0x03;
        break;
    case 3:
        *row = 0x07;
        break;
    case 4:
        *row = 0x0F;
        break;
    case 5:
        *row = 0x1F;
        break;
    case 6:
        *row = 0x3F;
        break;
    case 7:
        *row = 0x7F;
        break;
    case 8:
        *row = 0xFF;
        break;
    }
    return;
}
void TypeA_Set_Bit_Framing(unsigned char collpos, unsigned char *bit_framing)
{
    if (collpos == 0)
    {
        *bit_framing = 0x00;
        return;
    }
    if ((collpos > 0) && (collpos < 9)) //1~8
    {
        *bit_framing = ((collpos) << 4) + (collpos);
        return;
    }
    if ((collpos > 8) && (collpos < 17)) //9~16
    {
        *bit_framing = (((collpos - 8)) << 4) + (collpos - 8);
        return;
    }
    if ((collpos > 16) && (collpos < 25)) //17~24
    {
        *bit_framing = (((collpos - 16)) << 4) + (collpos - 16);

        return;
    }
    if ((collpos > 24) && (collpos < 32)) //25~31
    {
        *bit_framing = (((collpos - 24)) << 4) + (collpos - 24);
        return;
    }
    return;
}

/****************************************************************************************/
/*���ƣ�TypeA_Select																	*/
/*���ܣ�TypeA_Select��Ƭѡ��															*/
/*���룺selcode =0x93��0x95��0x97														*/
/*      pSnr[0],pSnr[1],pSnr[2],pSnr[3]pSnr[4] =UID 			    			     	*/
/*	       								 												*/
/*�����																			 	*/
/*	       	pSak[0],pSak[1],pSak[2] =SAK			                            		*/
/*       	OK: Ӧ����ȷ                                                              	*/
/*	 		ERROR: Ӧ�����																*/
/****************************************************************************************/
unsigned char TypeA_Select(unsigned char selcode, unsigned char *pSnr, unsigned char *pSak)
{
    unsigned char result, i, send_buff[7], rece_buff[5];
    unsigned int rece_bitlen;
    Write_Reg(BitFramingReg, 0x00);
    Set_BitMask(TxModeReg, 0x80);
    Set_BitMask(RxModeReg, 0x80);
    //Clear_BitMask(Status2Reg, 0x08);

    send_buff[0] = selcode;
    send_buff[1] = 0x70;

    for (i = 0; i < 5; i++)
    {
        send_buff[i + 2] = *(pSnr + i);
    }

    Pcd_SetTimer(1);
    result = Pcd_Comm(Transceive, send_buff, 7, rece_buff, &rece_bitlen);
    if (result == OK)
    {
        *pSak = rece_buff[0];
    }
    return result;
}

/****************************************************************************************/
/*���ƣ�TypeA_Halt																		*/
/*���ܣ�TypeA_Halt��Ƭֹͣ																*/
/*���룺																				*/
/*       			    			     												*/
/*	       								 												*/
/*�����																			 	*/
/*	       											                            		*/
/*       	OK: Ӧ����ȷ                                                              	*/
/*	 		ERROR: Ӧ�����																*/
/****************************************************************************************/
unsigned char TypeA_Halt(void)
{
    unsigned char send_buff[2], rece_buff[1];
    unsigned int rece_bitlen;
    send_buff[0] = 0x50;
    send_buff[1] = 0x00;

    Write_Reg(BitFramingReg, 0x00);
    Set_BitMask(TxModeReg, 0x80);
    Set_BitMask(RxModeReg, 0x80);
    Clear_BitMask(Status2Reg, 0x08);
    Pcd_SetTimer(1);
    Pcd_Comm(Transceive, send_buff, 2, rece_buff, &rece_bitlen);
    return OK;
}

/****************************************************************************************/
/*���ƣ�TypeA_CardActivate																*/
/*���ܣ�TypeA_CardActivate��Ƭ����														*/
/*���룺																				*/
/*       			    			     												*/
/*	       								 												*/
/*�����	pTagType[0],pTagType[1] =ATQA 											 	*/
/*	       	pSnr[0],pSnr[1],pSnr[2],pSnr[3]pSnr[4] =UID 		                   		*/
/*	       	pSak[0],pSak[1],pSak[2] =SAK			                            		*/
/*       	OK: Ӧ����ȷ                                                              	*/
/*	 		ERROR: Ӧ�����																*/
/****************************************************************************************/
unsigned char TypeA_CardActivate(struct PICC_A_STR *type_a_card)
{
    unsigned char result;

    result = TypeA_Request(type_a_card->ATQA); //Ѱ�� Standard	 send request command Standard mode
    if (result != OK)
    {
        return ERROR;
    }
    if (type_a_card->ATQA[1] != 0x00)
    {
        return ERROR;
    }
    if ((type_a_card->ATQA[0] & 0xC0) == 0x00)
    {
        result = TypeA_Anticollision(0x93, type_a_card->UID); //����ͻ��֤
        if (result != OK)
        {
            return ERROR;
        }
        result = TypeA_Select(0x93, type_a_card->UID, type_a_card->SAK); //ѡ������������ͻ��֤
        if (result != OK)
        {
            return ERROR;
        }
        type_a_card->UID_len = 4;
    }
    if ((type_a_card->ATQA[0] & 0xC0) == 0x40)	//����UID
    {
        result = TypeA_Anticollision(0x93, type_a_card->UID); //1������ͻ
        if (result != OK)
        {
            return ERROR;
        }
        result = TypeA_Select(0x93, type_a_card->UID, type_a_card->SAK); //1��ѡ��
        if (result != OK)
        {
            return ERROR;
        }
        result = TypeA_Anticollision(0x95, type_a_card->UID + 5); //2������ͻ
        if (result != OK)
        {
            return ERROR;
        }
        result = TypeA_Select(0x95, type_a_card->UID + 5, type_a_card->SAK + 1); //2��ѡ��
        if (result != OK)
        {
            return ERROR;
        }
        type_a_card->UID_len = 8;
    }
    if ((type_a_card->ATQA[0] & 0xC0) == 0x80)
    {
        result = TypeA_Anticollision(0x93, type_a_card->UID); //1������ͻ
        if (result != OK)
        {
            return ERROR;
        }
        result = TypeA_Select(0x93, type_a_card->UID, type_a_card->SAK); //1��ѡ��
        if (result != OK)
        {
            return ERROR;
        }
        result = TypeA_Anticollision(0x95, type_a_card->UID + 5); //2������ͻ
        if (result != OK)
        {
            return ERROR;
        }
        result = TypeA_Select(0x95, type_a_card->UID + 5, type_a_card->SAK + 1); //2��ѡ��
        if (result != OK)
        {
            return ERROR;
        }
        result = TypeA_Anticollision(0x97, type_a_card->UID + 10); //3������ͻ
        if (result != OK)
        {
            return ERROR;
        }
        result = TypeA_Select(0x97, type_a_card->UID + 10, type_a_card->SAK + 2); //3��ѡ��
        if (result != OK)
        {
            return ERROR;
        }
        type_a_card->UID_len = 12;
    }
    return result;
}

