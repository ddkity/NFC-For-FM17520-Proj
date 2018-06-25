#ifndef TYPE_B_H
#define TYPE_B_H

struct PICC_B_STR
{
unsigned char PUPI[4];
unsigned char APPLICATION_DATA[4];
unsigned char PROTOCOL_INF[3];
};

extern struct PICC_B_STR PICC_B;

unsigned char TypeB_Init(void);			//TYPE B ≥ı ºªØ
unsigned char TypeB_Request(unsigned char *rece_len,unsigned char *buff,struct PICC_B_STR *type_b_card);
unsigned char TypeB_Select(unsigned char *card_sn,unsigned char *rece_len,unsigned char *buff);
unsigned char TypeB_GetUID(unsigned char *rece_len,unsigned char *buff);
unsigned char TypeB_Halt(unsigned char *card_sn,unsigned char *rece_len,unsigned char *rece_buff);
unsigned char TypeB_WUP(unsigned char *rece_len,unsigned char *buff,struct PICC_B_STR *type_b_card);
#endif

