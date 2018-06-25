#include "main.h"

/*************************************************************/
//*函数名：	Read_Reg
//*功能：	    	读寄存器操作
//*输入参数：	reg_add，寄存器地址
//*返回值：	寄存器数值
/*************************************************************/
uint8_t Read_Reg(uint8_t reg_add)
{
    uint8_t reg_value;
    reg_value = SPIRead(reg_add);
    return reg_value;
}

/*************************************************************/
//*函数名：	Read_Reg All
//*功能：	    	读64个寄存器操作
//*输入参数：	无
//*输出参数	*reg_value，寄存器数值指针
//*返回值：	OK
/*************************************************************/
uint8_t Read_Reg_All(uint8_t *reg_value)
{
    uint8_t i;
    for (i = 0; i < 64; i++)
        *(reg_value + i) = Read_Reg(i);
    return OK;
}

/*************************************************************/
/*函数名：	Write_Reg									 */
/*功能：	    	写寄存器操作								 */
/*															 */
/*输入参数：	reg_add，寄存器地址；reg_value，寄存器数值	 */
/*返回值：	OK											 */
/*			ERROR										 */
/*************************************************************/
uint8_t Write_Reg(uint8_t reg_add, uint8_t reg_value)
{
    SPIWrite(reg_add, reg_value);
    return OK;
}

/*************************************************************/
/*函数名：	Read_FIFO									 */
/*功能：	    	读取FIFO  									 */
/*															 */
/*输入参数：	length，读取数据长度						 */
/*输出参数：	*fifo_data，数据存放指针					 */
/*返回值：	    											 */
/*															 */
/*************************************************************/
void Read_FIFO(uint8_t length, uint8_t *fifo_data)
{
    SPIRead_Sequence(length, FIFODataReg, fifo_data);
    return;
}

/*************************************************************/
/*函数名：	Write_FIFO									 */
/*功能：	    	写入FIFO  									 */
/*															 */
/*输入参数：	length，读取数据长度；*fifo_data，数据存放指针*/
/*输出参数：												 */
/*返回值：	    											 */
/*															 */
/*************************************************************/
void Write_FIFO(uint8_t length, uint8_t *fifo_data)
{
    SPIWrite_Sequence(length, FIFODataReg, fifo_data);
    return;
}

/*************************************************************/
/*函数名：	Clear_FIFO									 */
/*功能：	    	清空FIFO  									 */
/*															 */
/*输入参数：	无											*/
/*输出参数：												 */
/*返回值：	OK											 */
/*			ERROR										 */
/*************************************************************/
uint8_t Clear_FIFO(void)
{
    Set_BitMask(FIFOLevelReg, 0x80); //清除FIFO缓冲
    if (Read_Reg(FIFOLevelReg) == 0)
        return OK;
    else
        return ERROR;
}

/*************************************************************/
/*函数名：	Set_BitMask									 */
/*功能：	    	置位寄存器操作								 */
/*输入参数：	reg_add，寄存器地址；mask，寄存器置位		 */
/*返回值：	OK											 */
/*			ERROR										 */
/*************************************************************/
uint8_t Set_BitMask(uint8_t reg_add, uint8_t mask)
{
    uint8_t result;
    result = Write_Reg(reg_add, Read_Reg(reg_add) | mask); // set bit mask
    return result;
}

/*********************************************/
//*函数名：	Clear_BitMask
//*功能：	    	清除位寄存器操作
//
//*输入参数：	reg_add，寄存器地址；mask，寄存器清除位
//*返回值：	OK
//			ERROR
/*********************************************/
uint8_t Clear_BitMask(uint8_t reg_add, uint8_t mask)
{
    uint8_t result;
    result = Write_Reg(reg_add, Read_Reg(reg_add) & ~mask); // clear bit mask
    return result;
}

/*********************************************/
//*函数名：	Set_RF
//*功能：	    	设置射频输出
//
//*输入参数：	mode，射频输出模式
//			0，关闭输出
//			3，TX1，TX2打开输出，TX2为反向输出
//*返回值：	OK
//			ERROR
/*********************************************/
uint8_t Set_Rf(uint8_t mode)
{
    uint8_t result;
    if ((Read_Reg(TxControlReg) & 0x03) == mode)
        return OK;
    if (mode == 0)
    {
        result = Clear_BitMask(TxControlReg, 0x03); //关闭TX1，TX2输出
    }
    else if (mode == 1)
    {
        result = Clear_BitMask(TxControlReg, 0x02);
        result = Set_BitMask(TxControlReg, 0x01); //仅打开TX1输出
    }
    else if (mode == 2)
    {
        result = Clear_BitMask(TxControlReg, 0x01);
        result = Set_BitMask(TxControlReg, 0x02); //仅打开TX2输出
    }
    else if (mode == 3)
    {
        result = Set_BitMask(TxControlReg, 0x03); //打开TX1，TX2输出
    }
    //Delay_100us(2000);//打开TX输出后需要延时等待天线载波信号稳定
    if(mode != 0){
    	Delay_100us(10);
    }
    return result;
}

/*********************************************/
//*函数名：	Pcd_Comm
//*功能：	    	读卡器通信
//
//*输入参数：	Command，通信操作命令；
//			pInData，发送数据数组；
//			InLenByte，发送数据数组字节长度；
//			pOutData，接收数据数组；
//			pOutLenBit，接收数据的位长度
//*返回值：	OK
//			ERROR
/*********************************************/
uint8_t Pcd_Comm(uint8_t Command,
                        uint8_t *pInData,
                        uint8_t InLenByte,
                        uint8_t *pOutData,
                        uint32_t *pOutLenBit)
{
    uint8_t result;
    uint8_t rx_temp = 0; //临时数据字节长度
    uint8_t rx_len = 0; //接收数据字节长度
    uint8_t lastBits = 0; //接收数据位长度
    uint8_t irq;
    Clear_FIFO();
    //Write_Reg(CommandReg, Idle);
    Write_Reg(WaterLevelReg, 0x20); //设置FIFOLevel=32字节
    Write_Reg(ComIrqReg, 0x7F); //清除IRQ标志

    if(Command == MFAuthent)
    {
        Write_FIFO(InLenByte, pInData); //填入认证密钥
        Set_BitMask(BitFramingReg, 0x80); //启动发送
    }
    Set_BitMask(TModeReg, 0x80); //自动启动定时器
    Write_Reg(CommandReg, Command);

    while(1)//循环判断中断标识
    {
        irq = Read_Reg(ComIrqReg);//查询中断标志
        if(irq & 0x01)		//TimerIRq  定时器时间用尽
        {
            result=TIMEOUT_Err;		
            break;
        }
        if(Command == MFAuthent)
        {
            if(irq & 0x10)	//IdelIRq  command寄存器为空闲，指令操作完成
            {
                result = OK;
                break;
            }
        }
        if(Command == Transmit)
        {
            if((irq & 0x04) && (InLenByte > 0)) //LoAlertIrq+发送字节数大于0
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
                Write_Reg(ComIrqReg, 0x04); //清除LoAlertIrq
                Set_BitMask(BitFramingReg, 0x80); //启动发送

            }

            if((irq & 0x40) && (InLenByte == 0))		//TxIRq
            {
                result = OK;
                break;
            }
        }

        if(Command == Transceive)
        {
            if((irq & 0x04) && (InLenByte > 0)) //LoAlertIrq+发送字节数大于0
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
                Set_BitMask(BitFramingReg, 0x80); //启动发送
                Write_Reg(ComIrqReg, 0x04); //清除LoAlertIrq
            }
            if(irq & 0x08)	//HiAlertIRq
            {
                if((irq & 0x40) && (InLenByte == 0) && (Read_Reg(FIFOLevelReg) > 32)) //TxIRq
                {
                    Read_FIFO(32, pOutData + rx_len); //读出FIFO内容
                    rx_len = rx_len + 32;
                    Write_Reg(ComIrqReg, 0x08);	//清除 HiAlertIRq
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

            if ((rx_temp == 0) & (lastBits > 0)) //如果收到长度未满1个字节，则设置接收长度为1个字节。
                rx_temp = 1;
            Read_FIFO(rx_temp, pOutData + rx_len); //读出FIFO内容
            rx_len = rx_len + rx_temp; //接收长度累加

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
    Clear_BitMask(BitFramingReg, 0x80); //关闭发送
    return result;
}

/*********************************************/
//*函数名：	Pcd_SetTimer
//*功能：	     设置接收延时
//*输入参数：	delaytime，延时时间（单位为毫秒）
//*返回值：	OK
/*********************************************/
uint8_t Pcd_SetTimer(unsigned long delaytime)//设定超时时间（ms）
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
//*函数名：	Pcd_ConfigISOType
//*功能：	    	设置操作协议
//*输入参数：	type 0，ISO14443A协议；
//*			1，ISO14443B协议；
//*返回值：	OK
/*********************************************/
uint8_t Pcd_ConfigISOType(uint8_t type)
{
    if (type == 0)                     //ISO14443_A
    {
    #if 1
        Set_BitMask(ControlReg, 0x10); //接收字节有效位数，整个字节有效
        Set_BitMask(TxAutoReg, 0x40); //TxASKReg 0x15 设置100%ASK有效		..
        Write_Reg(TxModeReg, 0x00);  //TxModeReg 0x12 设置TX CRC无效，TX FRAMING =TYPE A  106kbit	..
        Write_Reg(RxModeReg, 0x00); //RxModeReg 0x13 设置RX CRC无效，RX FRAMING =TYPE A	..


        Set_BitMask(0x39, 0x80);	  //TestDAC1Reg寄存器为TestDAC1定义测试值
        Clear_BitMask(0x3C, 0x01);  //清空测试寄存器设置
        Clear_BitMask(0x3D, 0x07);   //清空测试寄存器设置
        Clear_BitMask(0x3E, 0x03);    //清空测试寄存器设置
        Write_Reg(0x33, 0xFF); //TestPinEnReg寄存器D1~D7引脚输出驱动使能		???
        Write_Reg(0x32, 0x07);
			

        Write_Reg(GsNOnReg, 0xFF); //选择天线驱动引脚TX1，TX2电导率，N驱动,定义调制和无调制阶段时的电导
        Write_Reg(CWGsPReg, 0x3F); //P驱动控制输出功率，相应的电流功耗以及操作距离
        Write_Reg(ModGsPReg, 0x3F); //解调器设置
		Write_Reg(RFCfgReg, 0x70);	//设置Bit6~Bit4为111 接收增益48db   --new
        Write_Reg(DemodReg, 0x0D);
        Write_Reg(RxThresholdReg, 0x84); //0x18寄存器	Bit7~Bit4 MinLevel Bit2~Bit0 CollLevel
        Write_Reg(AutoTestReg, 0x40); //AmpRcv=1		置1可增加106kbit下的操作距离
	#else
		Set_BitMask(ControlReg, 0x10); //ControlReg 0x0C 设置reader模式
        Set_BitMask(TxAutoReg, 0x40); //TxASKReg 0x15 设置100%ASK有效
        Write_Reg(TxModeReg, 0x00);  //TxModeReg 0x12 设置TX CRC无效，TX FRAMING =TYPE A
        Write_Reg(RxModeReg, 0x00); //RxModeReg 0x13 设置RX CRC无效，RX FRAMING =TYPE A

        Set_BitMask(0x39, 0x80);	  //TestDAC1Reg寄存器为TestDAC1定义测试值
        Clear_BitMask(0x3C, 0x01);  //清空测试寄存器设置
        Clear_BitMask(0x3D, 0x07);   //清空测试寄存器设置
        Clear_BitMask(0x3E, 0x03);    //清空测试寄存器设置
        Write_Reg(0x33, 0xFF); //TestPinEnReg寄存器D1~D7引脚输出驱动使能
        Write_Reg(0x32, 0x07);

        Write_Reg(GsNOnReg, 0xF1); //选择天线驱动引脚TX1，TX2电导率
        Write_Reg(CWGsPReg, 0x3F); //选择天线驱动引脚TX1，TX2电导率
        Write_Reg(ModGsPReg, 0x01); //选择天线驱动引脚TX1，TX2电导率
        //Write_Reg(RFCfgReg, 0x40);	//设置Bit6~Bit4为100 接收增益33db
        Write_Reg(RFCfgReg, 0x70);	//48db
        Write_Reg(DemodReg, 0x0D);	
        Write_Reg(RxThresholdReg, 0x84); //0x18寄存器	Bit7~Bit4 MinLevel Bit2~Bit0 CollLevel

        //Write_Reg(TestSel1Reg,);
        //Write_Reg(AnalogTestReg,0x20); //AUX1=Corr1
        //Write_Reg(AnalogTestReg,0x50); //AUX1=ADC_I
        Write_Reg(AutoTestReg, 0x40); //AmpRcv=1
        //Write_Reg(AutoTestReg,0x00); //AmpRcv=0
	#endif
	
	}
    else if (type == 1)                     //ISO14443_B
    {
        Write_Reg(ControlReg, 0x10); //ControlReg 0x0C 设置reader模式
        Write_Reg(TxModeReg, 0x83); //TxModeReg 0x12 设置TX CRC有效，TX FRAMING =TYPE B
        Write_Reg(RxModeReg, 0x83); //RxModeReg 0x13 设置RX CRC有效，RX FRAMING =TYPE B
        Write_Reg(GsNOnReg, 0xF4); //GsNReg 0x27 设置ON电导
        Write_Reg(GsNOffReg, 0xF4); //GsNOffReg 0x23 设置OFF电导
        Write_Reg(TxAutoReg, 0x00);// TxASKReg 0x15 设置100%ASK无效
    }
    else if (type == 2)                     //Felica
    {
        Write_Reg(ControlReg, 0x10); //ControlReg 0x0C 设置reader模式
        Write_Reg(TxModeReg, 0x92); //TxModeReg 0x12 设置TX CRC有效，212kbps,TX FRAMING =Felica
        Write_Reg(RxModeReg, 0x96); //RxModeReg 0x13 设置RX CRC有效，212kbps,Rx Multiple Enable,RX FRAMING =Felica
        Write_Reg(GsNOnReg, 0xF4); //GsNReg 0x27 设置ON电导
        Write_Reg(CWGsPReg, 0x20); //
        Write_Reg(GsNOffReg, 0x4F); //GsNOffReg 0x23 设置OFF电导
        Write_Reg(ModGsPReg, 0x20);
        Write_Reg(TxAutoReg, 0x07);// TxASKReg 0x15 设置100%ASK无效
    }

    return OK;
}

/*********************************************/
//*函数名：	FM175X_SoftReset
//*功能：	    	软复位操作
//
//*输入参数：
//
//
//*返回值：	OK
//			ERROR
/*********************************************/
uint8_t  FM175XX_SoftReset(void)
{	
	Write_Reg(CommandReg,SoftReset);//
	Set_BitMask(ControlReg,0x10);//17520配置为READER
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
//*函数名：	FM175XX_SoftPowerdown
//*功能：	    	软件低功耗操作
//*输入参数：
//*返回值：	OK，进入低功耗模式；
//*			ERROR，退出低功耗模式；
/*********************************************/
uint8_t FM175XX_SoftPowerdown(void)
{
    if (Read_Reg(CommandReg) & 0x10)
    {
        Clear_BitMask(CommandReg, 0x10); //退出低功耗模式
        return ERROR;
    }
    else
        Set_BitMask(CommandReg, 0x10); //进入低功耗模式
    return OK;
}

/*********************************************/
//*函数名：	FM175XX_HardPowerdown
//*功能：	    	硬件低功耗操作
//*输入参数：
//*返回值：	OK，进入低功耗模式(mode=1)；
//*			ERROR，退出低功耗模式(mode=0)；
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
    return OK; //进入低功耗模式
}
/*********************************************/
//*函数名：	FM175XX_DeepPowerdown
//*功能：	    	深度低功耗操作
//*输入参数：
//*返回值：	OK，进入低功耗模式(mode=1)；
//*			ERROR，退出低功耗模式(mode=0)；
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
    return OK; //进入低功耗模式
}

/*********************************************/
//*函数名：	Read_Ext_Reg
//*功能：	     读取扩展寄存器
//*输入参数：	reg_add，寄存器地址
//*返回值：	寄存器数值
/*********************************************/
uint8_t Read_Ext_Reg(uint8_t reg_add)
{
    Write_Reg(0x0F, 0x80 + reg_add);
    return Read_Reg(0x0F);
}

/*********************************************/
//*函数名：	Write_Ext_Reg
//*功能：	     写入扩展寄存器
//*输入参数：	reg_add，寄存器地址；reg_value，寄存器数值
//*返回值：	OK
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

