#ifndef TYPE_A_H
#define TYPE_A_H

struct PICC_A_STR
{
    unsigned char ATQA[2];
    unsigned char UID_len;
    unsigned char UID[12];
    unsigned char SAK[3];
};
extern struct PICC_A_STR PICC_A;

unsigned char TypeA_Request(unsigned char *pTagType);
unsigned char TypeA_WakeUp(unsigned char *pTagType);
unsigned char TypeA_Anticollision(unsigned char selcode, unsigned char *pSnr);
unsigned char TypeA_Select(unsigned char selcode, unsigned char *pSnr, unsigned char *pSak);
unsigned char TypeA_Halt(void);
unsigned char TypeA_CardActivate(struct PICC_A_STR *type_a_card);
void TypeA_Set_NVB(unsigned char collpos, unsigned char *nvb, unsigned char *row, unsigned char *col);
void TypeA_Set_Bit_Framing(unsigned char collpos, unsigned char *bit_framing);
#endif

