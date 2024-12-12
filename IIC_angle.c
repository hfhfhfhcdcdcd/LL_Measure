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
  
	uint8_t RecvData = 0;//��ʱ�洢��һ���յ���8λ���ݣ�����ȥ���ٽ��ڶ������ݴ��ȥ
	uint16_t MT6701_Data;//�洢���յ�������
  
/*------------------------��ʼ��ȡ����------------------------*/
/*-----------------------��һ�ζ�ȡ����-----------------------*/
	MT6701_Start();								                    //�����ź�                                �½���
	MT6701_SendByte(MT_Adress_Write);				        //���͵�ַ��дָ��                         0000_1100
	MT6701_SendByte(ReadAddress1);		              //����0x03�Ĵ�����ַ                       0000_0011
	MT6701_Start();								                  //���͵ڶ��������źſ�ʼ����               �½���
	MT6701_SendByte(MT_Adress_Read);		            //���͵�ַ������                       0000_1101
	RecvData = MT6701_RecvByte();			              //��ȡ0x03�Ĵ��������ݣ���Recv_data ��ֵ�� RecvData
	MT6701_Stop();								                  //ֹͣ�ź�
	MT6701_Data = RecvData;					                //�����ݸ�ֵ
/*-----------------------�ڶ��ζ�ȡ����-----------------------*/
  uint16_t Temp = MT6701_Data<<6;
	MT6701_Start();								                  //�����ź�                                 �½���
	MT6701_SendByte(MT_Adress_Write);				        //���͵�ַ��дָ��                          0000_1100
	MT6701_SendByte(ReadAddress2);		              //����0x04�Ĵ�����ַ                        0000_0100
	MT6701_Start();								                  //���͵ڶ��������źſ�ʼ����                �½��� 
	MT6701_SendByte(MT_Adress_Read);		            //���͵�ַ�������                          0000_1101                     
	RecvData = MT6701_RecvByte();			              //��ȡ0x04�Ĵ���������                                                
	MT6701_Stop();								                  //ֹͣ�ź�                                                
	MT6701_Data = (MT6701_Data << 8) + RecvData;  //����λ�����洢�ռ�                         
  MT6701_Data = Temp + RecvData;						                //ȡ���ݸ�14λ                                 
	return (MT6701_Data*360.0)/16384.0;
}

/*==================΢����ʱ����====================*/
void My_Delay_us(uint32_t us)//��ʱʱ�� = us* 72 ; 1s = 72_000_000hz;
{
   for(uint16_t i=0;i<us;i++)//us
  {
    for(uint16_t j=0;j<72;j++);//72
  }
}

/*==================IIC��ʼ�ź�====================*/
void MT6701_Start(void)
{
  SDA_H;//����SDA
  SCL_H;//����SLC
  My_Delay_us(10);  
  SDA_L;//����SDA
  My_Delay_us(10);
  SCL_L;//����SCL
  My_Delay_us(10); 
}

/*==================IICֹͣ�ź�====================*/
void MT6701_Stop(void)
{
  SDA_L;//����SDA
  SCL_H;//����SCL
  My_Delay_us(10);
  SDA_H;//����SDA,����������
  My_Delay_us(10);
}

/*==================����Ӧ���ź�====================*/
void MT6701_SendACK(uint8_t ack)
{
  SDA_Output;//дģʽ
  
    if((ack)&&(!READ_SCL) )          //ack==1ʱ�������ǵ͵�ƽʱ����Ӧ���ź�
      SDA_H;//����SDA
    else if((!ack)&&(!READ_SCL) )    //ack==0ʱ�������ǵ͵�ƽʱ��Ӧ���ź�
      SDA_L;//����SDA
	My_Delay_us(5);
  SCL_H;//����SCL 
  My_Delay_us(10);
  SCL_L;//����SCL
  My_Delay_us(5);
}

/*==================����Ӧ���ź�====================*/
uint8_t MT6701_RecvACK(void)
{
  int8_t ACK = 0;//Ӧ���ź�
  My_Delay_us(5);
  SDA_Input;//stm32��SDA����Ϊ����ģʽ��MT6701��stm32���룩
  SCL_H;//����SCL
  My_Delay_us(10);
  SCL_L;//����SCL
  My_Delay_us(5);
  
    if(READ_SDA == 1)//��ȡSDA�ϵ�Ӧ���ź�״̬
      ACK = 1 ;
    else
      ACK = 0 ;
    
	My_Delay_us(5); 
  SCL_H;//����SCL   
  SDA_Output;//дģʽ
  return ACK;
}

/*==================����һ���ֽ�����====================*/
void MT6701_SendByte(uint8_t send_data)
{ 
  int mask = 0x7F;
  for(uint8_t i=0;i<8;i++)
  {
    if(!READ_SCL)
    {
      if((0x80 & send_data))//1000_000&data �ж�����ߵ�һλ�Ƿ�Ϊ1
        SDA_H;//SDA��1
      else if((send_data & mask) == send_data)//send_data & 0111_1111;
        SDA_L;//SDA��0
      My_Delay_us(5);
    }
    send_data  <<= 1;//����ѭ���ж�data
    SCL_H;//����SCL  
    My_Delay_us(10);
    SCL_L;//����SCL
    My_Delay_us(5);
  }
  MT6701_RecvACK();
}

/*==================��ȡһ���ֽ�����====================*/
uint8_t MT6701_RecvByte(void)
{
  uint8_t Recv_data = 0;//�洢���ն�ȡ�����ֽ�����
  uint8_t bit;//��ʱ�洢ÿ�δ� IIC �����������ţ�SDA����ȡ����һλ����
  
  SDA_Input;//�л��ɶ�����ģʽ
  
  for(uint8_t i=0;i<8;i++)
  {
    Recv_data <<= 1;
    SCL_H;//����SCL
    My_Delay_us(5);
    if(READ_SDA == 1)
      bit = 0x01;
    else
      bit = 0x00;
		My_Delay_us(5);
    Recv_data|=bit;
    SCL_L;//����SCL
    My_Delay_us(10);
  }
  MT6701_SendACK(1);					//�����ڲ�SDA_Output;//дģʽ;
  return Recv_data;           //��ȡ����һ���ֽ�����
}

