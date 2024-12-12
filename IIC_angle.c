#include "IIC_angle.h"

#define SCL_H       LL_GPIO_SetOutputPin(IIC_SCL_GPIO_Port,IIC_SCL_Pin)
#define SCL_L       LL_GPIO_ResetOutputPin(IIC_SCL_GPIO_Port,IIC_SCL_Pin)
#define SDA_H       LL_GPIO_SetOutputPin(IIC_SDA_GPIO_Port,IIC_SDA_Pin)
#define SDA_L       LL_GPIO_ResetOutputPin(IIC_SDA_GPIO_Port,IIC_SDA_Pin)
#define SDA_Input   LL_GPIO_SetPinMode(IIC_SDA_GPIO_Port,IIC_SDA_Pin,LL_GPIO_MODE_INPUT)
#define SDA_Output  LL_GPIO_SetPinMode(IIC_SDA_GPIO_Port,IIC_SDA_Pin,LL_GPIO_MODE_OUTPUT)
#define READ_SDA    LL_GPIO_IsInputPinSet(IIC_SDA_GPIO_Port,IIC_SDA_Pin)
#define READ_SCL    LL_GPIO_IsInputPinSet(IIC_SCL_GPIO_Port,IIC_SCL_Pin)

double MT6701_RecvData(GPIO_TypeDef *sda_GPIO, uint16_t sda_Pin, GPIO_TypeDef *scl_GPIO, uint16_t scl_Pin)
{
  
	uint8_t RecvData = 0;//暂时存储第一次收到的8位数据，给出去后，再将第二波数据存进去
	uint16_t MT6701_Data;//存储接收的总数据
  
/*------------------------开始读取数据------------------------*/
/*-----------------------第一次读取数据-----------------------*/
	MT6701_Start();								                    //启动信号                                下降沿
	MT6701_SendByte(MT_Adress_Write);				        //发送地址与写指令                         0000_1100
	MT6701_SendByte(ReadAddress1);		              //发送0x03寄存器地址                       0000_0011
	MT6701_Start();								                  //发送第二个启动信号开始读数               下降沿
	MT6701_SendByte(MT_Adress_Read);		            //发送地址与读命令。                       0000_1101
	RecvData = MT6701_RecvByte();			              //读取0x03寄存器的数据，将Recv_data 赋值给 RecvData
	MT6701_Stop();								                  //停止信号
	MT6701_Data = RecvData;					                //将数据赋值
/*-----------------------第二次读取数据-----------------------*/
  uint16_t Temp = MT6701_Data<<6;
	MT6701_Start();								                  //启动信号                                 下降沿
	MT6701_SendByte(MT_Adress_Write);				        //发送地址与写指令                          0000_1100
	MT6701_SendByte(ReadAddress2);		              //发送0x04寄存器地址                        0000_0100
	MT6701_Start();								                  //发送第二个启动信号开始读数                下降沿 
	MT6701_SendByte(MT_Adress_Read);		            //发送地址与读命令                          0000_1101                     
	RecvData = MT6701_RecvByte();			              //读取0x04寄存器的数据                                                
	MT6701_Stop();								                  //停止信号                                                
	MT6701_Data = (MT6701_Data << 8) + RecvData;  //给低位留出存储空间                         
  MT6701_Data = Temp + RecvData;						                //取数据高14位                                 
	return (MT6701_Data*360.0)/16384.0;
}

/*==================微秒延时函数====================*/
void My_Delay_us(uint32_t us)//延时时间 = us* 72 ; 1s = 72_000_000hz;
{
   for(uint16_t i=0;i<us;i++)//us
  {
    for(uint16_t j=0;j<72;j++);//72
  }
}

/*==================IIC开始信号====================*/
void MT6701_Start(void)
{
  SDA_H;//拉高SDA
  SCL_H;//拉高SLC
  My_Delay_us(10);  
  SDA_L;//拉低SDA
  My_Delay_us(10);
  SCL_L;//拉低SCL
  My_Delay_us(10); 
}

/*==================IIC停止信号====================*/
void MT6701_Stop(void)
{
  SDA_L;//拉低SDA
  SCL_H;//拉高SCL
  My_Delay_us(10);
  SDA_H;//拉高SDA,产生上升沿
  My_Delay_us(10);
}

/*==================发送应答信号====================*/
void MT6701_SendACK(uint8_t ack)
{
  SDA_Output;//写模式
  
    if((ack)&&(!READ_SCL) )          //ack==1时，并且是低电平时，非应答信号
      SDA_H;//拉高SDA
    else if((!ack)&&(!READ_SCL) )    //ack==0时，并且是低电平时，应答信号
      SDA_L;//拉低SDA
	My_Delay_us(5);
  SCL_H;//拉高SCL 
  My_Delay_us(10);
  SCL_L;//拉低SCL
  My_Delay_us(5);
}

/*==================接收应答信号====================*/
uint8_t MT6701_RecvACK(void)
{
  int8_t ACK = 0;//应答信号
  My_Delay_us(5);
  SDA_Input;//stm32的SDA引脚为输入模式（MT6701向stm32输入）
  SCL_H;//拉高SCL
  My_Delay_us(10);
  SCL_L;//拉低SCL
  My_Delay_us(5);
  
    if(READ_SDA == 1)//读取SDA上的应答信号状态
      ACK = 1 ;
    else
      ACK = 0 ;
    
	My_Delay_us(5); 
  SCL_H;//拉高SCL   
  SDA_Output;//写模式
  return ACK;
}

/*==================发送一个字节数据====================*/
void MT6701_SendByte(uint8_t send_data)
{ 
  int mask = 0x7F;
  for(uint8_t i=0;i<8;i++)
  {
    if(!READ_SCL)
    {
      if((0x80 & send_data))//1000_000&data 判断最左边的一位是否为1
        SDA_H;//SDA置1
      else if((send_data & mask) == send_data)//send_data & 0111_1111;
        SDA_L;//SDA置0
      My_Delay_us(5);
    }
    send_data  <<= 1;//左移循环判断data
    SCL_H;//拉高SCL  
    My_Delay_us(10);
    SCL_L;//拉低SCL
    My_Delay_us(5);
  }
  MT6701_RecvACK();
}

/*==================读取一个字节数据====================*/
uint8_t MT6701_RecvByte(void)
{
  uint8_t Recv_data = 0;//存储最终读取到的字节数据
  uint8_t bit;//临时存储每次从 IIC 总线数据引脚（SDA）读取到的一位数据
  
  SDA_Input;//切换成读数据模式
  
  for(uint8_t i=0;i<8;i++)
  {
    Recv_data <<= 1;
    SCL_H;//拉高SCL
    My_Delay_us(5);
    if(READ_SDA == 1)
      bit = 0x01;
    else
      bit = 0x00;
		My_Delay_us(5);
    Recv_data|=bit;
    SCL_L;//拉低SCL
    My_Delay_us(10);
  }
  MT6701_SendACK(1);					//函数内部SDA_Output;//写模式;
  return Recv_data;           //读取到的一个字节数据
}

