#include "main.h"

struct CPU_CARD_STR CPU_CARD;

/****************************************************************/
/*����: Rats 													*/
/*����: �ú���ʵ��ת��APDU�����ʽ����	CPU card reset			*/
/*����: param1 byte :PCB 									    */
/*		param2 byte bit8-bit5��FSDI, bit4-bit0 ��CID  			*/
/*		param2 ͨ��ȡֵΪ0x51����FSDIΪ64�ֽڣ�CID=0x01			*/
/*���:															*/
/*		rece_len�����յ����ݳ���								*/
/*		buff�����յ�����ָ��									*/
/* OK: Ӧ����ȷ													*/
/* ERROR: Ӧ�����												*/
/****************************************************************/
unsigned char CPU_Rats(unsigned char param1, unsigned char param2, unsigned char *rece_len, unsigned char *buff)
{
    unsigned char ret, send_byte[2], point;
    unsigned int rece_bitlen;

    Write_Reg(TxModeReg, 0x80);
    Write_Reg(RxModeReg, 0x80);
    Write_Reg(BitFramingReg, 0x00);
    Pcd_SetTimer(100);
    send_byte[0] = 0xE0;//Start byte
    send_byte[1] = param2;//default=0x51
    CPU_CARD.CID = param2 & 0x0f;
    CPU_CARD.PCB = param1;
    Clear_FIFO();
    ret = Pcd_Comm(Transceive, send_byte, 2, buff, &rece_bitlen);
    if (ret == OK)
    {
        point = 0;
        *rece_len = rece_bitlen / 8;
        CPU_CARD.ATS.TL = *buff;
        CPU_CARD.ATS.T0 = *(buff + 1);
        CPU_CARD.ATS.FSCI = CPU_CARD.ATS.T0 & 0x0F;
        if(CPU_CARD.ATS.T0 & 0x40)
        {
            CPU_CARD.ATS.TA1 = *(buff + 2 + point);
            point++;
        }
        if(CPU_CARD.ATS.T0 & 0x20)
        {
            CPU_CARD.ATS.TB1 = *(buff + 2 + point);
            CPU_CARD.ATS.FWI = (CPU_CARD.ATS.TB1 & 0xF0) >> 8;
            CPU_CARD.ATS.SFGI = CPU_CARD.ATS.TB1 & 0x0F;
            point++;
        }
        if(CPU_CARD.ATS.T0 & 0x10)
        {
            CPU_CARD.ATS.TC1 = *(buff + 2 + point);
            point++;
        }
    }
    else
        *rece_len = 0;
    return ret;
}

/****************************************************************/
/*����: PPS 													*/
/*����: 														*/
/*����: PPSS byte, PPS0 byte, PPS1 byte							*/
/*���:															*/
/*		rece_len�����յ����ݳ���								*/
/*		buff�����յ�����ָ��									*/
/* OK: Ӧ����ȷ													*/
/* ERROR: Ӧ�����												*/
/****************************************************************/
unsigned char CPU_PPS(unsigned char pps0, unsigned char pps1, unsigned char *rece_len, unsigned char *buff)
{
    unsigned char ret, send_byte[3];
    unsigned int rece_bitlen;
    Pcd_SetTimer(50);
    send_byte[0] = 0xD0 | CPU_CARD.CID; //ppss byte
    send_byte[1] = pps0;	//pps0 byte
    //106K: pps0=0x11(pps1����),pps1=0x00(DSI:00,DRI:00)
    //212K: pps0=0x11(pps1����),pps1=0x05(DSI:01,DRI:01)
    //424K:	pps0=0x11(pps1����),pps1=0x0A(DSI:10,DRI:10)
    Clear_FIFO();
    if ((pps0 & 0x10) == 0x10)
    {
        send_byte[2] = pps1; 	//pps1 byte
        ret = Pcd_Comm(Transceive, send_byte, 3, buff, &rece_bitlen );
    }
    else
    {
        ret = Pcd_Comm(Transceive, send_byte, 2, buff, &rece_bitlen );
    }
    if (ret == OK)
        *rece_len = rece_bitlen / 8;
    else
        *rece_len = 0;
    return ret;
}

void CPU_PCB(void)
{
    //PCB bit0=~(bit0)
    CPU_CARD.PCB = CPU_CARD.PCB ^ 0x01; //���ÿ�ű任
    return;
}

/****************************************************************/
/*����: �����													*/
/*����: 														*/
/*����: N/A 													*/
/*���:															*/
/*		rece_len�����յ����ݳ���								*/
/*		buff�����յ�����ָ��									*/
/* 		OK: Ӧ����ȷ											*/
/* 		ERROR: Ӧ�����											*/
/****************************************************************/
unsigned char CPU_Random(unsigned char *rece_len, unsigned char *buff)
{
    unsigned char ret, send_byte[5];
    memcpy(send_byte, "\x00\x84\x00\x00\x08", 5);
    ret = CPU_I_Block(5, send_byte, &(*rece_len), buff);
    return ret;
}

/****************************************************************/
/*����: �ⲿ��֤												*/
/*����: 														*/
/*����: N/A 													*/
/*���:															*/
/* OK: Ӧ����ȷ													*/
/* ERROR: Ӧ�����												*/
/****************************************************************/
//unsigned char CPU_auth(unsigned char *key_buff,unsigned char *rece_len,unsigned char *buff)
//{
//	unsigned char i,ret,send_byte[15],rece_byte[16],Random_data[8],des_buff[8],temp;
//
//	ret=CPU_random(&temp,rece_byte);
//	if (ret==ERROR)
//		return ret;
//	for (i=0;i<4;i++)
//	Random_data[i]=rece_byte[i+2];
//	for (i=4;i<8;i++)
//	Random_data[i]=0;
//	Tdes(ENCRY,Random_data,key_buff,des_buff);
//	//CPU_PCB();
//	//send_byte[0] = PCB;
//	//send_byte[1] = CID;
//	send_byte[0] = 0x00;
//	send_byte[1] = 0x82;
//	send_byte[2] = 0x00;
//	send_byte[3] = 0x00;
//	send_byte[4] = 0x08;
//	for(i=0;i<8;i++)
//	send_byte[i+5]= des_buff[i];
//	ret=CPU_command(1,13,send_byte,&(*rece_len),buff);
//	return ret;
//
//}

/****************************************************************/
/*����: CPU_command 	cpu card operation						*/
/*����: CPU�������												*/
/*����:															*/
/*																*/
/*		send_len:���͵�ָ��� 								*/
/*		send_buff:���͵�ָ������ָ��	 						*/
/*																*/
/*���:��Ӧ����													*/
/*		rece_len�����յ����ݳ���								*/
/*		rece_buff�����յ�����ָ��								*/
/* 		OK: Ӧ����ȷ											*/
/* 		ERROR: Ӧ�����											*/
/****************************************************************/
unsigned char CPU_TPDU(unsigned char send_len, unsigned char *send_buff, unsigned char *rece_len, unsigned char *rece_buff)
{
    unsigned char ret;
    unsigned int rece_bitlen;

    Pcd_SetTimer(100);
    ret = Pcd_Comm(Transceive, send_buff, send_len, rece_buff, &rece_bitlen);
    if (ret == OK)
        *rece_len = rece_bitlen / 8;
    else
        *rece_len = 0;
    return ret;
}

/****************************************************************/
/*����: I Block 	cpu card operation							*/
/*����: CPU�������(I block)									*/
/*����:															*/
/*																*/
/*		send_len:���͵�ָ��� 								*/
/*		send_buff:���͵�ָ������ָ��	 						*/
/*																*/
/*���:��Ӧ����													*/
/*		rece_len�����յ����ݳ���								*/
/*		rece_buff�����յ�����ָ��								*/
/* 		OK: Ӧ����ȷ											*/
/* 		ERROR: Ӧ�����											*/
/****************************************************************/
unsigned char CPU_I_Block(unsigned char send_len, unsigned char *send_buff, unsigned char *rece_len, unsigned char *rece_buff)
{
    unsigned char ret;
    unsigned int rece_bitlen;
    Pcd_SetTimer(500);

    if (CPU_CARD.PCB & 0x08) //�ж��Ƿ���CID�ֽ�
    {
        memmove(send_buff + 2, send_buff, send_len);
        *send_buff = CPU_CARD.PCB;
        *(send_buff + 1) = CPU_CARD.CID;
        send_len = send_len + 2;
    }
    else
    {
        memmove(send_buff + 1, send_buff, send_len);
        *send_buff = CPU_CARD.PCB;
        send_len = send_len + 1;
    }
    ret = Pcd_Comm(Transceive, send_buff, send_len, rece_buff, &rece_bitlen);
    if (ret == OK)
    {
        CPU_CARD.PCB = *rece_buff;
        if((CPU_CARD.PCB & 0xF0) == 0xF0) //WTX
        {
            if(CPU_CARD.PCB & 0x08)
                CPU_CARD.WTXM = *(rece_buff + 2);
            else
                CPU_CARD.WTXM = *(rece_buff + 1);
        }
        CPU_PCB();
        *rece_len = rece_bitlen / (unsigned int)8;
    }
    else
        *rece_len = 0;
    return ret;
}

/****************************************************************/
/*����: R Block 	cpu card operation							*/
/*����: CPU�������(R block)									*/
/*����:															*/
/*																*/
/*																*/
/*���:��Ӧ����													*/
/*		rece_len�����յ����ݳ���								*/
/*		rece_buff�����յ�����ָ��								*/
/* 		OK: Ӧ����ȷ											*/
/* 		ERROR: Ӧ�����											*/
/****************************************************************/

unsigned char CPU_R_Block(unsigned char *rece_len, unsigned char *rece_buff)
{
    unsigned char send_buff[2], ret;
    unsigned int rece_bitlen;
    Pcd_SetTimer(100);

    if (CPU_CARD.PCB & 0x08) //�ж��Ƿ���CID�ֽ�
    {
        send_buff[0] = (CPU_CARD.PCB & 0x0F) | 0xA0;
        send_buff[1] = CPU_CARD.CID;
        ret = Pcd_Comm(Transceive, send_buff, 2, rece_buff, &rece_bitlen);
    }
    else
    {
        send_buff[0] = (CPU_CARD.PCB & 0x0F) | 0xA0;
        ret = Pcd_Comm(Transceive, send_buff, 1, rece_buff, &rece_bitlen);
    }
    if (ret == OK)
    {
        CPU_CARD.PCB = *rece_buff;
        if((CPU_CARD.PCB & 0xF0) == 0xF0) //WTX
        {
            if(CPU_CARD.PCB & 0x08)
                CPU_CARD.WTXM = *(rece_buff + 2);
            else
                CPU_CARD.WTXM = *(rece_buff + 1);
        }
        CPU_PCB();
        *rece_len = rece_bitlen / 8;
    }
    else
        *rece_len = 0;
    return ret;
}

/****************************************************************/
/*����: S Block 	cpu card operation							*/
/*����: CPU�������(S block)									*/
/*����:															*/
/*																*/
/*		pcb_byte:����PCB�ֽ�									*/
/*																*/
/*���:��Ӧ����													*/
/*		rece_len�����յ����ݳ���								*/
/*		rece_buff�����յ�����ָ��								*/
/* 		OK: Ӧ����ȷ											*/
/* 		ERROR: Ӧ�����											*/
/****************************************************************/
unsigned char CPU_S_Block(unsigned char pcb_byte, unsigned char *rece_len, unsigned char *rece_buff)
{
    unsigned char send_buff[3], ret;
    unsigned int rece_bitlen;

    Pcd_SetTimer(500);

    send_buff[0] = pcb_byte | 0xC0;

    if ((CPU_CARD.PCB & 0x08) == 0x08) //�ж��Ƿ���CID�ֽ�
    {
        if ((pcb_byte & 0xF0) == 0xF0) //WTX
        {
            send_buff[1] = CPU_CARD.CID;
            send_buff[2] = CPU_CARD.WTXM;
            ret = Pcd_Comm(Transceive, send_buff, 3, rece_buff, &rece_bitlen);
        }
        else  //DESELECT
        {
            send_buff[1] = CPU_CARD.CID;
            ret = Pcd_Comm(Transceive, send_buff, 2, rece_buff, &rece_bitlen);
        }
    }
    else
    {
        if ((pcb_byte & 0xF0) == 0xF0) //WTX
        {
            send_buff[1] = CPU_CARD.WTXM;
            ret = Pcd_Comm(Transceive, send_buff, 2, rece_buff, &rece_bitlen);
        }
        else   //DESELECT
        {
            send_buff[1] = 0x00;
            ret = Pcd_Comm(Transceive, send_buff, 2, rece_buff, &rece_bitlen);
        }
    }
    if (ret == OK)
    {
        CPU_CARD.PCB = *rece_buff;
        if((CPU_CARD.PCB & 0xF0) == 0xF0) //WTX
        {
            if(CPU_CARD.PCB & 0x08)
                CPU_CARD.WTXM = *(rece_buff + 2);
            else
                CPU_CARD.WTXM = *(rece_buff + 1);
        }
        CPU_PCB();
        *rece_len = rece_bitlen / 8;
    }
    else
        *rece_len = 0;
    return ret;
}

/****************************************************************/
/*����: CPU_baud_rate_change									*/
/*����: ����ͨ�Ų�����											*/
/*����:															*/
/*		mode=0 ���շ��Ͳ�����106kbps							*/
/*		mode=1 ���շ��Ͳ�����212kbps							*/
/*		mode=2 ���շ��Ͳ�����424kbps	 						*/
/*																*/
/*���:��Ӧ����													*/
/* 		OK: Ӧ����ȷ											*/
/* 		ERROR: Ӧ�����											*/
/****************************************************************/
unsigned char CPU_BaudRate_Set(unsigned char mode) //mode=0:106Kbps;1:212Kpbs;2:424Kpbs;3:848Kpbs
{
    Clear_BitMask(TxModeReg, 0x70);
    Clear_BitMask(RxModeReg, 0x70);
    switch (mode)
    {
    case 0:	  //106kbps
        Set_BitMask(TxModeReg, 0x00);
        Set_BitMask(RxModeReg, 0x00);
        break;
    case 1:	  //212kbps
        Set_BitMask(TxModeReg, 0x10);
        Set_BitMask(RxModeReg, 0x10);
        break;
    case 2:	  //424kbps
        Set_BitMask(TxModeReg, 0x20);
        Set_BitMask(RxModeReg, 0x20);
        break;
    case 3:	  //848kbps
        Set_BitMask(TxModeReg, 0x30);
        Set_BitMask(RxModeReg, 0x30);
        break;
    }
    return OK;
}


