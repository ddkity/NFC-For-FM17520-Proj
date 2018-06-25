#ifndef __NFC_APP_H__
#define __NFC_APP_H__

#define TAG_UID_LEN      (8)
#define FILTER_UID_LEN   (4)

#define TOTAL_TAG_NUM   (4)

#define PWD0   (0x56)
#define PWD1   (0x9D)
#define PWD2   (0xF5)
#define PWD3   (0xC9)

#define PACK0       (0x66)
#define PACK1       (0x88)



typedef struct _TAG_INFO_T {
    uint8_t tag_uid[TAG_UID_LEN];
    uint8_t filter_uid[FILTER_UID_LEN];
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t manufacturer;
    uint8_t filter_type;
    uint8_t is_auth;
}TAG_INFO_T;

uint8_t NFC_findTag(void);
uint8_t clear_tag_info(uint8_t id);
const TAG_INFO_T *get_tag_info(uint8_t id);
void set_cur_tag_id(uint8_t id);
//void NFC_Adjustment(void);
void NFC_test(void);




#endif

