#include "main.h"

/*************************************************************/
//*��������	Read_Reg
//*���ܣ�	    	���Ĵ�������
//*���������	reg_add���Ĵ�����ַ
//*����ֵ��	�Ĵ�����ֵ
/*************************************************************/
uint8_t Read_Reg(uint8_t reg_add)
{
    uint8_t reg_value;
    reg_value = SPIRead(reg_add);
    return reg_value;
}

/*************************************************************/
//*��������	Read_Reg All
//*���ܣ�	    	��64���Ĵ�������
//*���������	��
//*�������	*reg_value���Ĵ�����ֵָ��
//*����ֵ��	OK
/*************************************************************/
uint8_t Read_Reg_All(uint8_t *reg_value)
{
    uint8_t i;
    for (i = 0; i < 64; i++)
        *(reg_value + i) = Read_Reg(i);
    return OK;
}

/*************************************************************/
/*��������	Write_Reg									 */
/*���ܣ�	    	д�Ĵ�������								 */
/*															 */
/*���������	reg_add���Ĵ�����ַ��reg_value���Ĵ�����ֵ	 */
/*����ֵ��	OK											 */
/*			ERROR										 */
/*************************************************************/
uint8_t Write_Reg(uint8_t reg_add, uint8_t reg_value)
{
    SPIWrite(reg_add, reg_value);
    return OK;
}

/*************************************************************/
/*��������	Read_FIFO									 */
/*���ܣ�	    	��ȡFIFO  									 */
/*															 */
/*���������	length����ȡ���ݳ���						 */
/*���������	*fifo_data�����ݴ��ָ��					 */
/*����ֵ��	    											 */
/*															 */
/*************************************************************/
void Read_FIFO(uint8_t length, uint8_t *fifo_data)
{
    SPIRead_Sequence(length, FIFODataReg, fifo_data);
    return;
}

/*************************************************************/
/*��������	Write_FIFO									 */
/*���ܣ�	    	д��FIFO  									 */
/*															 */
/*���������	length����ȡ���ݳ��ȣ�*fifo_data�����ݴ��ָ��*/
/*���������												 */
/*����ֵ��	    											 */
/*															 */
/*************************************************************/
void Write_FIFO(uint8_t length, uint8_t *fifo_data)
{
    SPIWrite_Sequence(length, FIFODataReg, fifo_data);
    return;
}

/*************************************************************/
/*��������	Clear_FIFO									 */
/*���ܣ�	    	���FIFO  									 */
/*															 */
/*���������	��											*/
/*���������												 */
/*����ֵ��	OK											 */
/*			ERROR										 */
/*************************************************************/
uint8_t Clear_FIFO(void)
{
    Set_BitMask(FIFOLevelReg, 0x80); //���FIFO����
    if (Read_Reg(FIFOLevelReg) == 0)
        return OK;
    else
        return ERROR;
}

/*************************************************************/
/*��������	Set_BitMask									 */
/*���ܣ�	    	��λ�Ĵ�������								 */
/*���������	reg_add���Ĵ�����ַ��mask���Ĵ�����λ		 */
/*����ֵ��	OK											 */
/*			ERROR										 */
/*************************************************************/
uint8_t Set_BitMask(uint8_t reg_add, uint8_t mask)
{
    uint8_t result;
    result = Write_Reg(reg_add, Read_Reg(reg_add) | mask); // set bit mask
    return result;
}

/*********************************************/
//*��������	Clear_BitMask
//*���ܣ�	    	���λ�Ĵ�������
//
//*���������	reg_add���Ĵ�����ַ��mask���Ĵ������λ
//*����ֵ��	OK
//			ERROR
/*********************************************/
uint8_t Clear_BitMask(uint8_t reg_add, uint8_t mask)
{
    uint8_t result;
    result = Write_Reg(reg_add, Read_Reg(reg_add) & ~mask); // clear bit mask
    return result;
}

/*********************************************/
//*��������	Set_RF
//*���ܣ�	    	������Ƶ���
//
//*���������	mode����Ƶ���ģʽ
//			0���ر����
//			3��TX1��TX2�������TX2Ϊ�������
//*����ֵ��	OK
//			ERROR
/*********************************************/
uint8_t Set_Rf(uint8_t mode)
{
    uint8_t result;
    if ((Read_Reg(TxControlReg) & 0x03) == mode)
        return OK;
    if (mode == 0)
    {
        result = Clear_BitMask(TxControlReg, 0x03); //�ر�TX1��TX2���
    }
    else if (mode == 1)
    {
        result = Clear_BitMask(TxControlReg, 0x02);
        result = Set_BitMask(TxControlReg, 0x01); //����TX1���
    }
    else if (mode == 2)
    {
        result = Clear_BitMask(TxControlReg, 0x01);
        result = Set_BitMask(TxControlReg, 0x02); //����TX2���
    }
    else if (mode == 3)
    {
        result = Set_BitMask(TxControlReg, 0x03); //��TX1��TX2���
    }
    //Delay_100us(2000);//��TX�������Ҫ��ʱ�ȴ������ز��ź��ȶ�
    if(mode != 0){
    	Delay_100us(10);
    }
    return result;
}

/*********************************************/
//*��������	Pcd_Comm
//*���ܣ�	    	������ͨ��
//
//*���������	Command��ͨ�Ų������
//			pInData�������������飻
//			InLenByte���������������ֽڳ��ȣ�
//			pOutData�������������飻
//			pOutLenBit���������ݵ�λ����
//*����ֵ��	OK
//			ERROR
/*********************************************/
uint8_t Pcd_Comm(uint8_t Command,
                        uint8_t *pInData,
                        uint8_t InLenByte,
                        uint8_t *pOutData,
                        uint32_t *pOutLenBit)
{
    uint8_t result;
    uint8_t rx_temp = 0; //��ʱ�����ֽڳ���
    uint8_t rx_len = 0; //���������ֽڳ���
    uint8_t lastBits = 0; //��������λ����
    uint8_t irq;
    Clear_FIFO();
    //Write_Reg(CommandReg, Idle);
    Write_Reg(WaterLevelReg, 0x20); //����FIFOLevel=32�ֽ�
    Write_Reg(ComIrqReg, 0x7F); //���IRQ��־

    if(Command == MFAuthent)
    {
        Write_FIFO(InLenByte, pInData); //������֤��Կ
        Set_BitMask(BitFramingReg, 0x80); //��������
    }
    Set_BitMask(TModeReg, 0x80); //�Զ�������ʱ��
    Write_Reg(CommandReg, Command);

    while(1)//ѭ���ж��жϱ�ʶ
    {
        irq = Read_Reg(ComIrqReg);//��ѯ�жϱ�־
        if(irq & 0x01)		//TimerIRq  ��ʱ��ʱ���þ�
        {
            result=TIMEOUT_Err;		
            break;
        }
        if(Command == MFAuthent)
        {
            if(irq & 0x10)	//IdelIRq  command�Ĵ���Ϊ���У�ָ��������
            {
                result = OK;
                break;
            }
        }
        if(Command == Transmit)
        {
            if((irq & 0x04) && (InLenByte > 0)) //LoAlertIrq+�����ֽ�������0
            {
                if (InLenByte < 32)
                {
                    Write_FIFO(InLenByte, pInData);
                    InLenByte = 0;
                }
                else
                {
                    Write_FIFO(32, pInData);
                    InLenByte = InLenByte - 32;
                    pInData = pInData + 32;
                }
                Write_Reg(ComIrqReg, 0x04); //���LoAlertIrq
                Set_BitMask(BitFramingReg, 0x80); //��������

            }

            if((irq & 0x40) && (InLenByte == 0))		//TxIRq
            {
                result = OK;
                break;
            }
        }

        if(Command == Transceive)
        {
            if((irq & 0x04) && (InLenByte > 0)) //LoAlertIrq+�����ֽ�������0
            {
                if (InLenByte > 32)
                {
                    Write_FIFO(32, pInData);
                    InLenByte = InLenByte - 32;
                    pInData = pInData + 32;
                }
                else
                {
                    Write_FIFO(InLenByte, pInData);
                    InLenByte = 0;
                }
                Set_BitMask(BitFramingReg, 0x80); //��������
                Write_Reg(ComIrqReg, 0x04); //���LoAlertIrq
            }
            if(irq & 0x08)	//HiAlertIRq
            {
                if((irq & 0x40) && (InLenByte == 0) && (Read_Reg(FIFOLevelReg) > 32)) //TxIRq
                {
                    Read_FIFO(32, pOutData + rx_len); //����FIFO����
                    rx_len = rx_len + 32;
                    Write_Reg(ComIrqReg, 0x08);	//��� HiAlertIRq
                }
            }
            if((irq & 0x20) && (InLenByte == 0))	//RxIRq=1
            {
                result = OK;
                break;
            }
        }
    }

    {
        if (Command == Transceive)
        {
            rx_temp = Read_Reg(FIFOLevelReg);
            lastBits = Read_Reg(ControlReg) & 0x07;

            if ((rx_temp == 0) & (lastBits > 0)) //����յ�����δ��1���ֽڣ������ý��ճ���Ϊ1���ֽڡ�
                rx_temp = 1;
            Read_FIFO(rx_temp, pOutData + rx_len); //����FIFO����
            rx_len = rx_len + rx_temp; //���ճ����ۼ�

            if (lastBits > 0)
                *pOutLenBit = (rx_len - 1) * (unsigned int)8 + lastBits;
            else
                *pOutLenBit = rx_len * (unsigned int)8;
        }
    }

    if (result == OK)
        result = Read_Reg(ErrorReg);

    Set_BitMask(ControlReg, 0x80);          // stop timer now
    Write_Reg(CommandReg, Idle);
    Clear_BitMask(BitFramingReg, 0x80); //�رշ���
    return result;
}

/*********************************************/
//*��������	Pcd_SetTimer
//*���ܣ�	     ���ý�����ʱ
//*���������	delaytime����ʱʱ�䣨��λΪ���룩
//*����ֵ��	OK
/*********************************************/
uint8_t Pcd_SetTimer(unsigned long delaytime)//�趨��ʱʱ�䣨ms��
{
    unsigned long TimeReload;
    uint32_t Prescaler;

    Prescaler = 0;
    TimeReload = 0;
    while(Prescaler < 0xfff)
    {
        TimeReload = ((delaytime * (long)13560) - 1) / (Prescaler * 2 + 1);

        if( TimeReload < 0xffff)
            break;
        Prescaler++;
    }
    TimeReload = TimeReload & 0xFFFF;
    Set_BitMask(TModeReg, Prescaler >> 8);
    Write_Reg(TPrescalerReg, Prescaler & 0xFF);
    Write_Reg(TReloadMSBReg, TimeReload >> 8);
    Write_Reg(TReloadLSBReg, TimeReload & 0xFF);
    return OK;
}

/*********************************************/
//*��������	Pcd_ConfigISOType
//*���ܣ�	    	���ò���Э��
//*���������	type 0��ISO14443AЭ�飻
//*			1��ISO14443BЭ�飻
//*����ֵ��	OK
/*********************************************/
uint8_t Pcd_ConfigISOType(uint8_t type)
{
    if (type == 0)                     //ISO14443_A
    {
    #if 1
        Set_BitMask(ControlReg, 0x10); //�����ֽ���Чλ���������ֽ���Ч
        Set_BitMask(TxAutoReg, 0x40); //TxASKReg 0x15 ����100%ASK��Ч		..
        Write_Reg(TxModeReg, 0x00);  //TxModeReg 0x12 ����TX CRC��Ч��TX FRAMING =TYPE A  106kbit	..
        Write_Reg(RxModeReg, 0x00); //RxModeReg 0x13 ����RX CRC��Ч��RX FRAMING =TYPE A	..


        Set_BitMask(0x39, 0x80);	  //TestDAC1Reg�Ĵ���ΪTestDAC1�������ֵ
        Clear_BitMask(0x3C, 0x01);  //��ղ��ԼĴ�������
        Clear_BitMask(0x3D, 0x07);   //��ղ��ԼĴ�������
        Clear_BitMask(0x3E, 0x03);    //��ղ��ԼĴ�������
        Write_Reg(0x33, 0xFF); //TestPinEnReg�Ĵ���D1~D7�����������ʹ��		???
        Write_Reg(0x32, 0x07);
			

        Write_Reg(GsNOnReg, 0xFF); //ѡ��������������TX1��TX2�絼�ʣ�N����,������ƺ��޵��ƽ׶�ʱ�ĵ絼
        Write_Reg(CWGsPReg, 0x3F); //P��������������ʣ���Ӧ�ĵ��������Լ���������
        Write_Reg(ModGsPReg, 0x3F); //���������
		Write_Reg(RFCfgReg, 0x70);	//����Bit6~Bit4Ϊ111 ��������48db   --new
        Write_Reg(DemodReg, 0x0D);
        Write_Reg(RxThresholdReg, 0x84); //0x18�Ĵ���	Bit7~Bit4 MinLevel Bit2~Bit0 CollLevel
        Write_Reg(AutoTestReg, 0x40); //AmpRcv=1		��1������106kbit�µĲ�������
	#else
		Set_BitMask(ControlReg, 0x10); //ControlReg 0x0C ����readerģʽ
        Set_BitMask(TxAutoReg, 0x40); //TxASKReg 0x15 ����100%ASK��Ч
        Write_Reg(TxModeReg, 0x00);  //TxModeReg 0x12 ����TX CRC��Ч��TX FRAMING =TYPE A
        Write_Reg(RxModeReg, 0x00); //RxModeReg 0x13 ����RX CRC��Ч��RX FRAMING =TYPE A

        Set_BitMask(0x39, 0x80);	  //TestDAC1Reg�Ĵ���ΪTestDAC1�������ֵ
        Clear_BitMask(0x3C, 0x01);  //��ղ��ԼĴ�������
        Clear_BitMask(0x3D, 0x07);   //��ղ��ԼĴ�������
        Clear_BitMask(0x3E, 0x03);    //��ղ��ԼĴ�������
        Write_Reg(0x33, 0xFF); //TestPinEnReg�Ĵ���D1~D7�����������ʹ��
        Write_Reg(0x32, 0x07);

        Write_Reg(GsNOnReg, 0xF1); //ѡ��������������TX1��TX2�絼��
        Write_Reg(CWGsPReg, 0x3F); //ѡ��������������TX1��TX2�絼��
        Write_Reg(ModGsPReg, 0x01); //ѡ��������������TX1��TX2�絼��
        //Write_Reg(RFCfgReg, 0x40);	//����Bit6~Bit4Ϊ100 ��������33db
        Write_Reg(RFCfgReg, 0x70);	//48db
        Write_Reg(DemodReg, 0x0D);	
        Write_Reg(RxThresholdReg, 0x84); //0x18�Ĵ���	Bit7~Bit4 MinLevel Bit2~Bit0 CollLevel

        //Write_Reg(TestSel1Reg,);
        //Write_Reg(AnalogTestReg,0x20); //AUX1=Corr1
        //Write_Reg(AnalogTestReg,0x50); //AUX1=ADC_I
        Write_Reg(AutoTestReg, 0x40); //AmpRcv=1
        //Write_Reg(AutoTestReg,0x00); //AmpRcv=0
	#endif
	
	}
    else if (type == 1)                     //ISO14443_B
    {
        Write_Reg(ControlReg, 0x10); //ControlReg 0x0C ����readerģʽ
        Write_Reg(TxModeReg, 0x83); //TxModeReg 0x12 ����TX CRC��Ч��TX FRAMING =TYPE B
        Write_Reg(RxModeReg, 0x83); //RxModeReg 0x13 ����RX CRC��Ч��RX FRAMING =TYPE B
        Write_Reg(GsNOnReg, 0xF4); //GsNReg 0x27 ����ON�絼
        Write_Reg(GsNOffReg, 0xF4); //GsNOffReg 0x23 ����OFF�絼
        Write_Reg(TxAutoReg, 0x00);// TxASKReg 0x15 ����100%ASK��Ч
    }
    else if (type == 2)                     //Felica
    {
        Write_Reg(ControlReg, 0x10); //ControlReg 0x0C ����readerģʽ
        Write_Reg(TxModeReg, 0x92); //TxModeReg 0x12 ����TX CRC��Ч��212kbps,TX FRAMING =Felica
        Write_Reg(RxModeReg, 0x96); //RxModeReg 0x13 ����RX CRC��Ч��212kbps,Rx Multiple Enable,RX FRAMING =Felica
        Write_Reg(GsNOnReg, 0xF4); //GsNReg 0x27 ����ON�絼
        Write_Reg(CWGsPReg, 0x20); //
        Write_Reg(GsNOffReg, 0x4F); //GsNOffReg 0x23 ����OFF�絼
        Write_Reg(ModGsPReg, 0x20);
        Write_Reg(TxAutoReg, 0x07);// TxASKReg 0x15 ����100%ASK��Ч
    }

    return OK;
}

/*********************************************/
//*��������	FM175X_SoftReset
//*���ܣ�	    	��λ����
//
//*���������
//
//
//*����ֵ��	OK
//			ERROR
/*********************************************/
uint8_t  FM175XX_SoftReset(void)
{	
	Write_Reg(CommandReg,SoftReset);//
	Set_BitMask(ControlReg,0x10);//17520����ΪREADER
	if (Read_Reg(ControlReg)&0x10)
		return OK;
	else
		return	ERROR;
}

uint8_t FM175XX_HardReset(void)
{
    NPD = 0;
    Delay_100us(10);
    NPD = 1;
    Delay_100us(10);
    return OK;
}

/*********************************************/
//*��������	FM175XX_SoftPowerdown
//*���ܣ�	    	����͹��Ĳ���
//*���������
//*����ֵ��	OK������͹���ģʽ��
//*			ERROR���˳��͹���ģʽ��
/*********************************************/
uint8_t FM175XX_SoftPowerdown(void)
{
    if (Read_Reg(CommandReg) & 0x10)
    {
        Clear_BitMask(CommandReg, 0x10); //�˳��͹���ģʽ
        return ERROR;
    }
    else
        Set_BitMask(CommandReg, 0x10); //����͹���ģʽ
    return OK;
}

/*********************************************/
//*��������	FM175XX_HardPowerdown
//*���ܣ�	    	Ӳ���͹��Ĳ���
//*���������
//*����ֵ��	OK������͹���ģʽ(mode=1)��
//*			ERROR���˳��͹���ģʽ(mode=0)��
/*********************************************/
uint8_t FM175XX_HardPowerdown(uint8_t mode)
{
    if (mode == 0)
    {
        NPD = 1;
        Delay_100us(20);
        return ERROR;
    }
    else if (mode == 1)
    {
    	Set_Ext_BitMask(0x03, 0x20);
        NPD = 0;
        Delay_100us(20);
    }
    return OK; //����͹���ģʽ
}
/*********************************************/
//*��������	FM175XX_DeepPowerdown
//*���ܣ�	    	��ȵ͹��Ĳ���
//*���������
//*����ֵ��	OK������͹���ģʽ(mode=1)��
//*			ERROR���˳��͹���ģʽ(mode=0)��
/*********************************************/

uint8_t FM175XX_DeepPowerdown(uint8_t mode)
{
	if (mode == 0)
    {
        NPD = 1;
        Delay_100us(20);
        return ERROR;
    }
    else if (mode == 1)
    {
    	Clear_Ext_BitMask(0x03, 0x20);
        NPD = 0;
        Delay_100us(20);
    }
    return OK; //����͹���ģʽ
}

/*********************************************/
//*��������	Read_Ext_Reg
//*���ܣ�	     ��ȡ��չ�Ĵ���
//*���������	reg_add���Ĵ�����ַ
//*����ֵ��	�Ĵ�����ֵ
/*********************************************/
uint8_t Read_Ext_Reg(uint8_t reg_add)
{
    Write_Reg(0x0F, 0x80 + reg_add);
    return Read_Reg(0x0F);
}

/*********************************************/
//*��������	Write_Ext_Reg
//*���ܣ�	     д����չ�Ĵ���
//*���������	reg_add���Ĵ�����ַ��reg_value���Ĵ�����ֵ
//*����ֵ��	OK
//			ERROR
/*********************************************/
uint8_t Write_Ext_Reg(uint8_t reg_add, uint8_t reg_value)
{
    Write_Reg(0x0F, 0x40 + reg_add);
    Write_Reg(0x0F, 0xC0 + reg_value);
    return OK;
}

uint8_t Set_Ext_BitMask(uint8_t reg_add, uint8_t mask)
{
    uint8_t result;
    result = Write_Ext_Reg(reg_add, Read_Ext_Reg(reg_add) | mask); // set bit mask
    return result;
}

uint8_t Clear_Ext_BitMask(uint8_t reg_add, uint8_t mask)
{
    uint8_t result;
    result = Write_Ext_Reg(reg_add, Read_Ext_Reg(reg_add) & (~mask)); // clear bit mask
    return result;
}

