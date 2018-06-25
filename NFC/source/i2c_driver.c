#include "main.h"

#define COMM_SIMPLE_CIPHER

//�ߵͰ�λ���0x044D��ת��ʮ���ƾ���1101����λ���������İ汾�ž���V1.1.01
#define VERSION_MASTER 0x04		//�߰�λ
#define VERSION_SLAVE 0x4D		//�Ͱ�λ

uint8_t send_buf[I2C_SEND_BUFF_LEN] = {0};
uint8_t send_idx = 0;
uint8_t recv_buf[I2C_RECV_BUFF_LEN];
uint8_t recv_idx = 0;

volatile uint8_t is_send = 0;
volatile uint8_t is_recv = 0;

static RECV_STATE_E s_recv_state = RECV_STATE_INIT;
static uint8_t s_data_len = 0;
static uint8_t s_cur_data_len = 0;

static FRAME_INFO_T s_frame_info = {ENCRYPTION_TYPE_NONE, 0, 0};


void I2C_Init(void)
{
	/* Open I2C module and set bus clock */
    I2C_Open(I2C0, 100000);
    /* Set I2C 4 Slave Addresses */
    I2C_SetSlaveAddr(I2C0, 0, 0x7E, 0);   /* Slave Address : 0x7E */
    //I2C_SetSlaveAddr(I2C0, 1, 0x7E, 0);   /* Slave Address : 0x35 */
    //I2C_SetSlaveAddr(I2C0, 2, 0x7E, 0);   /* Slave Address : 0x55 */
    //I2C_SetSlaveAddr(I2C0, 3, 0x7E, 0);   /* Slave Address : 0x75 */

	I2C_SetSlaveAddrMask(I2C0, 0, 0x01);

	I2C_EnableWakeup(I2C0);		//����I2C���ѹ���
    /* Enable I2C interrupt */
    I2C_EnableInt(I2C0);
    NVIC_EnableIRQ(I2C0_IRQn);

    /* I2C as slave */
    I2C_SET_CONTROL_REG(I2C0, I2C_SI | I2C_AA);
}

int parse_frame(FRAME_INFO_T *pinfo, const uint8_t *pframe, uint8_t len)
{
    uint8_t data_len;
    FRAME_CMD_T cmd;
    uint8_t cipher;
    uint8_t crpt_type;
    uint8_t cc = 0x00;

    if (len == 0) {
        return -1;
    }

    while (*pframe != I2C_FRAME_HEAD && len > 0) {
        pframe++;
        len--;
    }

    if (len > 0) {
        pframe++;
        data_len = *pframe;
        cc ^= *pframe++;
        len--;
    } else {
        return -1;
    }

    if (len >= data_len) {
        cmd.cmd = *pframe;
        cc ^= *pframe++;
        cipher = *pframe;
        cc ^= *pframe++;
        crpt_type = *pframe;
        cc ^= *pframe++;
    } else {
        return -1;
    }

    if (cc == *pframe) {
        pinfo->crpt_type = (ENCRYPTION_TYPE_E)crpt_type;
        pinfo->cipher = cipher;
        pinfo->tag_idx = cmd.cmd_t.tag_idx;
        return 0;

    } else {
        return -2;
    }
}

int form_frame(FRAME_INFO_T *pinfo, uint8_t *pframe)
{
    FRAME_CMD_T cmd = {0};
    uint8_t i;
    uint8_t index;
    uint8_t *temp = pframe;
    uint8_t cc = 0;
    uint8_t cipher;
    const TAG_INFO_T *ptaginfo;
    uint16_t year;

    *temp++ = I2C_FRAME_HEAD;
    *temp = TAG_INFO_DATA_LEN;
    cc ^= *temp++;
    cmd.cmd_t.direction = 1;	//NFCģ�鷢����Ӧ����Ϣ
    cmd.cmd_t.tag_idx = pinfo->tag_idx;
    *temp = cmd.cmd;
    cc ^= *temp++;

    index = cmd.cmd_t.tag_idx - 1;		//��Ƭ���������ı�ǩ��1��ʼ������ȡ�õı�ǩ��0��ʼ��

	//���������һ���汾��ѯ�����ֻ���ذ汾��Ϣ
	if(index == 4)
	{
		*temp = VERSION_MASTER;
		cc ^= *temp++;

		*temp = VERSION_SLAVE;
		cc ^= *temp++;

		*temp = 0x05;
		cc ^= *temp++;

		for(i = 0; i < 14; i++)
		{
			*temp = 0x00;
        	cc ^= *temp++;
		}

		//*temp = cc;
		//return 0;
		goto Cipher;
	}

	//���������һ�����Ź����Ե�����򷵻�I2C�Ƿ������ı�־
	if(index == 5)
	{
		*temp = WDT_ResetFlag;
		cc ^= *temp++;

		*temp = 0x06;
		cc ^= *temp++;

		//��λ��־
		*temp = (SYS_GetResetSrc()) >> 8;	//��ȡǰһ�εĸ�λԴ�����Ÿ�λ����Ϣ�ϴ�
		cc ^= *temp++;

		*temp = SYS_GetResetSrc();	//��ȡǰһ�εĸ�λԴ�����Ÿ�λ����Ϣ�ϴ�
		cc ^= *temp++;
		
		SYS_ClearResetSrc(0xFFFF);	//�����λԴ��־

		for(i = 0; i < 13; i++)
		{
			*temp = 0x00;
        	cc ^= *temp++;
		}
		//*temp = cc;
		
		WDT_ResetFlag = 0x01;	//��ȡ��λ��Ϣ֮����һ��byte���Ǳ����0x01����һ�ζ���ʱ����0x00������ڶ��ζ��Ļ���0x00������ģ���Ѿ���������
		//return 0;
		goto Cipher;
	}

	//�����������I2C���豸���͵������������ģ���˳��͹���ģʽ�������ģʽ������������Ѱ��ģʽ����ʱi2c���豸�Ѿ���ʼ��ȡ������Ϣ�ˣ�2s��ģ���Զ��˳�����ģʽ
	if(index == 6)
	{
		CurrentTestMode = 1;	//��־�Ž����˲���ģʽ���ڶ�ʱ�ж����˳��͹���ģʽ���������ʱ����2s���ڴ��ڼ������򲻽���͹���ģʽ��һֱѰ�����4s���˳������1����˳�

		*temp = 0x07;
		cc ^= *temp++;
		
		//buffer����
		for(i = 0; i < 16; i++)
		{
			*temp = 0x00;
        	cc ^= *temp++;
		}
		//*temp = cc;
		
		//return 0;
		goto Cipher;
	}

    ptaginfo = get_tag_info(index);

    if (ptaginfo == NULL) {
        return -1;
    }

    for (i = 0; i < 8; i++) {
        *temp = ptaginfo->tag_uid[i];
        cc ^= *temp++;
    }

    for (i = 0; i < 4; i++) {
        *temp = ptaginfo->filter_uid[i];
        cc ^= *temp++;
    }

    // year
    year = (uint16_t)(ptaginfo->year % 1000);	//ȡ����������
    *temp = (uint8_t)year;
    cc ^= *temp++;

    // month
    *temp = ptaginfo->month;
    cc ^= *temp++;

    // day
    *temp = ptaginfo->day;
    cc ^= *temp++;

    // manufacturer
    *temp = ptaginfo->manufacturer;
    cc ^= *temp++;

    // filter type
    *temp = ptaginfo->filter_type;
    cc ^= *temp++;

Cipher:

#ifdef COMM_SIMPLE_CIPHER
    cipher = pinfo->cipher;
#else
    switch (pinfo->crpt_type) {
    case ENCRYPTION_TYPE_XOR:
        cipher = pinfo->cipher ^ cc;
        break;
    case ENCRYPTION_TYPE_AND:
        cipher = pinfo->cipher & cc;
        break;
    case ENCRYPTION_TYPE_OR:
        cipher = pinfo->cipher | cc;
        break;
    default:
        cipher = pinfo->cipher;
        break;
    }
#endif

    temp = pframe + 2;		//ָ������ָ�������

    // encrypt cmd & data domain, length not encrypt	��������,У�����Ǽ���ǰ��������������������ڼ���ǰ�Ѿ�������
    for (i = 0; i < TAG_INFO_DATA_LEN-1; i++) {
        *temp++ ^= cipher;
    }
    *temp = cc;

    return (TAG_INFO_DATA_LEN + 2);
}




/*---------------------------------------------------------------------------------------------------------*/
/*  I2C Tx Callback Function                                                                            */
/*---------------------------------------------------------------------------------------------------------*/
void I2C0_IRQHandler(void)
{
    	uint32_t u32Status;
	
		I2C_HaveInterrupt = 1;
		u32Status = I2C_GET_STATUS(I2C0);
		I2C_CLEAR_WAKEUP_FLAG(I2C0);	//�����־�Ա��ٴν���͹���ģʽ
	    switch(u32Status) {
	    /* Slave Transmitter Mode */
	    case 0xC0:                        /* DATA has been transmitted and NACK has been returned */
	    case 0xC8:                        /* DATA has been transmitted and ACK has been returned */
	        I2C_SET_CONTROL_REG(I2C0, I2C_SI | I2C_AA);
	        break;

	    case 0xA8:                        /* SLA+R has been received and ACK has been returned */
	    case 0xB0:
	        if (is_recv == 1) {
	            //I2C_Init();
	        }

	        if(is_send == 0) {
	            send_idx = 0;
	            is_send = 1;
	        }

			I2C_SET_DATA(I2C0, send_buf[send_idx++]);
			I2C_SET_CONTROL_REG(I2C0, I2C_SI | I2C_AA);
			break;
	    case 0xB8:                        /* DATA has been transmitted and ACK has been returned */
	        I2C_SET_DATA(I2C0, send_buf[send_idx++]);
			if (send_idx >= MAX_SEND_FRAME_LEN) {
	            send_idx = 0;
	            is_send = 0;
				I2C_Count_Flag = 0;		//�ж�I2C���ݣ���־��0
				I2C_SET_CONTROL_REG(I2C0, I2C_SI);
	        }
			else{
				I2C_SET_CONTROL_REG(I2C0, I2C_SI | I2C_AA);
			}   
	        break;

	    /* Slave Receiver Mode*/
	    case 0x68:                        /* SLA+W has been received and ACK has been returned */
	    case 0x60:
			if((g_nfc_running == 1) && (CurrentTestMode == 0)){
				recv_idx = 0;
	            is_recv = 0;
				send_idx = 0;
	            is_send = 0;
				I2C_SET_CONTROL_REG(I2C0, I2C_SI);
				return;
			}
			else
			{
		        if(is_send == 1)
		        {
					//I2C_Init();
				}
				if(is_recv == 0)
				{
					recv_idx = 0;
					is_recv = 1;
				}
		        I2C_SET_CONTROL_REG(I2C0, I2C_SI | I2C_AA);
			}
	        break;
	    case 0x80:                        /* DATA has been received and ACK has been returned */
			recv_buf[recv_idx] = I2C_GET_DATA(I2C0);

			switch(s_recv_state){
			case RECV_STATE_INIT:
				if(recv_buf[recv_idx] == I2C_FRAME_HEAD){
					s_recv_state = RECV_STATE_HEAD;
				}
				break;
			case RECV_STATE_HEAD:
	            s_data_len = recv_buf[recv_idx];
	            s_recv_state = RECV_STATE_LENGTH;
	            break;
	        case RECV_STATE_LENGTH:
	            s_cur_data_len++;
	            if (s_cur_data_len >= s_data_len) {
	                s_recv_state = RECV_STATE_COMPLETE;
	            }
	            break; 
			}

			recv_idx++;
	        if (recv_idx >= I2C_RECV_BUFF_LEN) {
	            recv_idx = 0;
	        }

			// Received enough bytes to switch to TX
	        if (recv_idx >= MAX_RECV_FRAME_LEN || s_recv_state == RECV_STATE_COMPLETE) {
	            s_recv_state = RECV_STATE_INIT;
	            s_cur_data_len = 0;
	            if (parse_frame(&s_frame_info, recv_buf, recv_idx) == 0) {
	                if(form_frame(&s_frame_info, send_buf) == -1){		//��ֹ�������͵���һ���Ϸ���I2C���󣬵����������󲢲�����Ҫ�����ݣ��᷵��-1
						memset(send_buf, 0xF0, sizeof(send_buf));
					}
					I2C_Count_Flag = 0;		//�ж�I2C���ݣ���־��0
	            } else {
	                memset(send_buf, 0xF0, sizeof(send_buf));
	            }
	            
	            recv_idx = 0;
	            is_recv = 0;
				I2C_SET_CONTROL_REG(I2C0, I2C_SI);
	        }
			else{
				I2C_SET_CONTROL_REG(I2C0, I2C_SI | I2C_AA);
			}
	        break;
			
	    case 0x88:                        /* DATA has been received and NACK has been returned */
	        //Rx_Data[DataLen++] = I2C_GET_DATA(I2C);
	        I2C_SET_CONTROL_REG(I2C0, I2C_SI | I2C_AA);
	        break;

	    case 0xA0:                      /* STOP or Repeat START has been received */
	        I2C_SET_CONTROL_REG(I2C0, I2C_SI | I2C_AA);
	        //printf("Slave Receiver Success\n");
	        break;

		case 0x00:
			I2C_SET_CONTROL_REG(I2C0, I2C_STO | I2C_SI);
			I2C_SET_CONTROL_REG(I2C0, I2C_SI | I2C_AA);
			break;
	    }
}


