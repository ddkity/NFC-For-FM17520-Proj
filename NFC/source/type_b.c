/*************************************************************/
//2014.03.06�޸İ�
/*************************************************************/
#include "main.h"


struct PICC_B_STR PICC_B;

/*****************************************************************************************/
/*���ƣ�TypeB_Halt																		 */
/*���ܣ�����TYPE B��Ƭ����ֹͣ״̬														 */
/*���룺card_sn����Ƭ��PUPI																 */
/*�����																				 */
/*	   	OK�������ɹ�																	 */
/*		ERROR������ʧ��																	 */
/*****************************************************************************************/
unsigned char TypeB_Halt(unsigned char *card_sn,unsigned char *rece_len,unsigned char *rece_buff)
{
	unsigned char result,send_buff[5];
	unsigned int rece_bitlen;
		Pcd_SetTimer(5);
		send_buff[0] = 0x50;
		memcpy(	send_buff+1, card_sn,4);
					
		result = Pcd_Comm(Transceive,send_buff,5,rece_buff,&rece_bitlen);
		if (result==OK)
			*rece_len=rece_bitlen/8;
		else
			*rece_len=0;
		return result;
}
/*****************************************************************************************/
/*���ƣ�TypeB_WUP																		 */
/*���ܣ�TYPE B��Ƭ����																	 */
/*���룺N/A																				 */
/*�����																				 */
/*		rece_len:��ƬӦ������ݳ��ȣ�buff����ƬӦ�������ָ��							 */
/*		card_sn:��Ƭ��PUPI�ֽ�															 */
/*	   	OK�������ɹ�																	 */
/*		ERROR������ʧ��																	 */
/*****************************************************************************************/
unsigned char TypeB_WUP(unsigned char *rece_len,unsigned char *buff, struct PICC_B_STR *type_b_card)
{						
	unsigned char result,send_buff[3];
	unsigned int rece_bitlen;
		Pcd_SetTimer(5);
		send_buff[0]=0x05;//APf
		send_buff[1]=0x00;//AFI (00:for all cards)
		send_buff[2]=0x08;//PARAM(WUP,Number of slots =0)
		
		result = Pcd_Comm(Transceive,send_buff,3,buff,&rece_bitlen);
		if (result==OK)
			{
			*rece_len=rece_bitlen/8;
			memcpy(type_b_card->PUPI,buff+1,4);
			memcpy(type_b_card->APPLICATION_DATA,buff+6,4);
			memcpy(type_b_card->PROTOCOL_INF,buff+10,3);
			}
		else
			*rece_len=0;		
		return (result);
}
/*****************************************************************************************/
/*���ƣ�TypeB_Request																	 */
/*���ܣ�TYPE B��Ƭѡ��																	 */
/*���룺																				 */
/*�����																				 */
/*	   	OK�������ɹ�																	 */
/*		ERROR������ʧ��																	 */
/*****************************************************************************************/
unsigned char TypeB_Request(unsigned char *rece_len,unsigned char *buff,struct PICC_B_STR *type_b_card)
{			
	unsigned char result,send_buff[3];
	unsigned int rece_bitlen;
		Pcd_SetTimer(5);
		send_buff[0]=0x05;	//APf
		send_buff[1]=0x00;	//AFI (00:for all cards)
		send_buff[2]=0x00;	//PARAM(REQB,Number of slots =0)
	
		result = Pcd_Comm(Transceive,send_buff,3,buff,&rece_bitlen);
		if (result==OK)
			{	
			*rece_len=rece_bitlen/8;
			memcpy(type_b_card->PUPI,buff+1,4);
			memcpy(type_b_card->APPLICATION_DATA,buff+6,4);
			memcpy(type_b_card->PROTOCOL_INF,buff+10,3);
			}
		else
			*rece_len=0;
		return result;
}		
/*****************************************************************************************/
/*���ƣ�TypeB_Select																	 */
/*���ܣ�TYPE B��Ƭѡ��																	 */
/*���룺card_sn����ƬPUPI�ֽڣ�4�ֽڣ�													 */
/*�����																				 */
/*		rece_len��Ӧ�����ݵĳ���														 */
/*		buff��Ӧ�����ݵ�ָ��															 */
/*	   	OK�������ɹ�																	 */
/*		ERROR������ʧ��																	 */
/*****************************************************************************************/
unsigned char TypeB_Select(unsigned char *card_sn,unsigned char *rece_len,unsigned char *buff)
{
	unsigned char result,send_buff[9];
	unsigned int rece_bitlen;
		Pcd_SetTimer(5);
		send_buff[0] = 0x1d;
				
		memcpy(send_buff+ 1,card_sn,4);
			
		send_buff[5] = 0x00;
		send_buff[6] = 0x08;  //01--24��08--256
		send_buff[7] = 0x01;  //COMPATIBLE WITH 14443-4
		send_buff[8] = 0x01;  //CID��01 
        		
		result = Pcd_Comm(Transceive,send_buff,9,buff,&rece_bitlen);
		if (result==OK)
			*rece_len=rece_bitlen/8;
		else
			*rece_len=0;
		return result;
}	
/*****************************************************************************************/
/*���ƣ�TypeB_GetUID																	 */
/*���ܣ����֤ר��ָ��																	 */
/*���룺N/A																				 */
/*�����rece_len���������ݵĳ���														 */
/*		buff���������ݵ�ָ��															 */
/*	   	OK�������ɹ�																	 */
/*		ERROR������ʧ��																	 */
/*****************************************************************************************/
unsigned char TypeB_GetUID(unsigned char *rece_len,unsigned char *buff)
{
	unsigned char result,send_buff[5];
	unsigned int rece_bitlen;
		Pcd_SetTimer(5);
		send_buff[0]=0x00;
		send_buff[1]=0x36;
		send_buff[2]=0x00;
		send_buff[3]=0x00;
		send_buff[4]=0x08;
		
		result = Pcd_Comm(Transceive,send_buff,5,buff,&rece_bitlen);
		if (result==OK)
			*rece_len=rece_bitlen/8;
		else
			*rece_len=0;
		return (result);
}	
