#ifndef __I2C_DRIVER_H__
#define __I2C_DRIVER_H__

#define I2C_SLAVE_ADDR    (0x7E)
#define I2C_FRAME_HEAD    (0xAA)



// Head + TAG_INFO_DATA_LEN + CC
#define MAX_SEND_FRAME_LEN (21)
#define I2C_SEND_BUFF_LEN  (MAX_SEND_FRAME_LEN+1)

// MAX_SEND_FRAME_LEN + 1
#define MAX_RECV_FRAME_LEN      (6)
#define I2C_RECV_BUFF_LEN       (MAX_RECV_FRAME_LEN+1)

// length of Len + CMD + DATA，不包括帧头和校验位
#define TAG_INFO_DATA_LEN  (19)

typedef union __FRAME_CMD_T {
  uint8_t cmd;
  struct {
    uint8_t tag_idx:6;
    uint8_t is_continue:1;
    uint8_t direction:1;
  }cmd_t;
}FRAME_CMD_T;



typedef enum __RECV_STATE_E {
  RECV_STATE_INIT,
  RECV_STATE_HEAD,
  RECV_STATE_LENGTH,
  RECV_STATE_COMPLETE,
  RECV_STATE_MAX
}RECV_STATE_E;

typedef enum __ENCRYPTION_TYPE_E {
  ENCRYPTION_TYPE_NONE,
  ENCRYPTION_TYPE_XOR = 0x01,
  ENCRYPTION_TYPE_AND = 0x02,
  ENCRYPTION_TYPE_OR = 0x03,
  ENCRYPTION_TYPE_RAW = 0x04,
  ENCRYPTION_TYPE_MAX
}ENCRYPTION_TYPE_E;


typedef struct __FRAME_INFO_T {
  ENCRYPTION_TYPE_E crpt_type;
  uint8_t cipher;
  uint8_t tag_idx;
}FRAME_INFO_T;

void I2C_Init(void);


#endif

