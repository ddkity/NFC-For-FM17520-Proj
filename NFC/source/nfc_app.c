#include "main.h"

static uint8_t s_cur_tag_id = 0;
static TAG_INFO_T s_tag_info[TOTAL_TAG_NUM] = {0};
//static TAG_INFO_T s_tag_info_temp[16] = {0};	//一个临时存卡信息的变量，用于容错
//static uint8_t tag_cnt = 0;
unsigned long long findtagcnt = 0;	//寻卡次数，测试寻卡多少次会有一次寻不到卡的


#define NtagCare

#define TAG_UID_START_BLOCK    (0x00)
#define TAG_INFO_START_BLOCK    (0x10)
#define TAG_AUTH_START_BLOCK    (0x14)
#define ENC_KEY_LEN      (16)


const uint8_t s_static_info_key[ENC_KEY_LEN] = {8, 87, 110, 234, 60, 43, 70, 213};
const uint8_t s_static_auth_key[ENC_KEY_LEN] = {61, 203, 134, 185, 243, 207, 38, 24};

void set_cur_tag_id(uint8_t id)
{
    s_cur_tag_id = id;
}

const TAG_INFO_T * get_tag_info(uint8_t id)
{
    if (id >= TOTAL_TAG_NUM) {
        return NULL;
    } else {
        return &s_tag_info[id];
    }
}



uint8_t clear_tag_info(uint8_t id)
{
    TAG_INFO_T *pinfo;

    if (id >= TOTAL_TAG_NUM) {
        return ERROR;
    } else {
        pinfo = &s_tag_info[id];
        memset(pinfo, 0x00, sizeof(TAG_INFO_T));
        pinfo->filter_type = 0xFF; // no tag found
    }

    return OK;
}

//因为主程序天线读取了四次卡片，去除错误的信息
/*
tag:	t0	t1	t2	t3
	0	1	2	3
	4	5	6	7
	8	9	10	11
	12	13	14	15

*/
#if 0
void NFC_Adjustment(void)
{
	uint8_t i;

	i = 0;
	tag_cnt = 0;
	if(s_tag_info[0].filter_type == 0x04){
		while(i <= 12){
			if(s_tag_info_temp[i].filter_type == 0x01){
				memcpy(&s_tag_info[0], &s_tag_info_temp[i], sizeof(TAG_INFO_T));
				break;
			}else{
				i = i + 4;
			}
		}
	}

	i = 1;
	if(s_tag_info[1].filter_type == 0x04){
		while(i <= 13){
			if(s_tag_info_temp[i].filter_type == 0x01){
				memcpy(&s_tag_info[1], &s_tag_info_temp[i], sizeof(TAG_INFO_T));
				break;
			}else{
				i = i + 4;
			}
		}
	}

	i = 2;
	if(s_tag_info[2].filter_type == 0x04){
		while(i <= 14){
			if(s_tag_info_temp[i].filter_type == 0x01){
				memcpy(&s_tag_info[2], &s_tag_info_temp[i], sizeof(TAG_INFO_T));
				break;
			}else{
				i = i + 4;
			}
		}
	}

	i = 3;
	if(s_tag_info[3].filter_type == 0x04){
		while(i <= 15){
			if(s_tag_info_temp[i].filter_type == 0x01){
				memcpy(&s_tag_info[3], &s_tag_info_temp[i], sizeof(TAG_INFO_T));
				break;
			}else{
				i = i + 4;
			}
		}
	}
}
#endif
#if 0
/* 检测不稳定的测试是否能纠正过来 */
void NFC_test(void)
{
	uint8_t i;
	volatile  uint8_t tag0, tag1, tag2, tag3;
	volatile uint8_t tagtype[16] = {0};

	tag0 = s_tag_info[0].filter_type;
	tag1 = s_tag_info[1].filter_type;
	tag2 = s_tag_info[2].filter_type;
	tag3 = s_tag_info[3].filter_type;

	for(i = 0; i < 16; i++)
	{
		tagtype[i] = s_tag_info_temp[i].filter_type;
	}

	return;
}
#endif


/*
	0: fail
	1: success
	2: not auth
*/
uint8_t NFC_findTag(void)
{
	uint8_t ret;
	uint8_t RecvBuff[16];
	TAG_INFO_T *pinfo;
	uint8_t raw_data[16] = {0};
	uint8_t *ptemp;
	uint8_t i, j;

	if(s_cur_tag_id >= TOTAL_TAG_NUM) 
	{
        return 0;
    }
	findtagcnt++;
	FM175XX_SoftReset();
	
	//1、寻卡并选定卡片
	Set_Rf(3);
    Pcd_ConfigISOType(0);
    ret = TypeA_CardActivate(&PICC_A);
	if(ret != OK)
	{
		Set_Rf(0);
		//memset(&s_tag_info_temp[tag_cnt], 0, sizeof(TAG_INFO_T));
		//tag_cnt++;
		return 0;
	}
	pinfo = &s_tag_info[s_cur_tag_id];
	
	//same uid and the tag is authed last time, discard this loop.
    if(pinfo->is_auth)
    {
		NFC_LED = 1;
		return 1;
    }

	//1.读卡的UID，在第0和第1块上
	Ultra_Read(TAG_UID_START_BLOCK, &RecvBuff[0]);
	pinfo->tag_uid[0] = 0;
    memcpy(&pinfo->tag_uid[1], &RecvBuff[0], 3);
    memcpy(&pinfo->tag_uid[4], &RecvBuff[4], 4);

	//2.PWD认证
	memset(raw_data, 0x00, sizeof(raw_data));
	raw_data[0] = PWD0;
	raw_data[1] = PWD1;
	raw_data[2] = PWD2;
	raw_data[3] = PWD3;
	DesEncrypt8(&raw_data[0], pinfo->tag_uid, &raw_data[8]);
	raw_data[0] = PACK0;
    raw_data[1] = PACK1;
    Ultra_PwdAuth(&raw_data[8], &raw_data[0]);

	//3.读标签的信息，第10/11/12/13上
	Ultra_Read(TAG_INFO_START_BLOCK, &RecvBuff[0]);
	DesDecrypt16(s_static_info_key, &RecvBuff[0], raw_data);

	memcpy(pinfo->filter_uid, raw_data, FILTER_UID_LEN);
	ptemp = raw_data + FILTER_UID_LEN;
	pinfo->year = (*ptemp << 8) | *(ptemp+1);
    ptemp += 2;
    pinfo->month = *ptemp++;
    pinfo->day = *ptemp++;
    pinfo->manufacturer = *ptemp++;
    pinfo->filter_type = *ptemp++;

	if((((pinfo->filter_uid[0] >> 4) + 2017) != pinfo->year) || ((pinfo->filter_uid[0] & 0x0F) != pinfo->month))
	{
		pinfo->filter_type = 0x04; //0xFF; // invalid filter
		pinfo->is_auth = 0x00;
		NFC_LED = 0;
		return 2;
	}

	//4.读认证信息
	Ultra_Read(TAG_AUTH_START_BLOCK, &RecvBuff[0]);
	DesDecrypt16(s_static_auth_key, &RecvBuff[0], raw_data);
	
	pinfo->is_auth = 1;
    for (i = 0, j = 0; i < 8; i++, j += 2) {

        if (pinfo->tag_uid[i] != (raw_data[j] ^ raw_data[j+1])) {
            pinfo->is_auth = 0;
            break;
        }
    }

	//5.结果判断
	memset(&PICC_A, 0x00, sizeof(PICC_A));
	if (pinfo->is_auth) {
        NFC_LED = 1;
		Set_Rf(0);
		//memcpy(&s_tag_info_temp[tag_cnt], &s_tag_info[s_cur_tag_id], sizeof(TAG_INFO_T));
		//tag_cnt++;
        return 1;
    } else {
        pinfo->filter_type = 0x04; //0xFF; // invalid filter
        NFC_LED = 0;
    }

	Set_Rf(0);
	//memcpy(&s_tag_info_temp[tag_cnt], &s_tag_info[s_cur_tag_id], sizeof(TAG_INFO_T));
	//tag_cnt++;
    return 2;
}



