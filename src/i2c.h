

//i2c.h ����iic_app.c ���ţ���������c������������


#ifndef I2C_H
#define I2C_H



#include <gd32f10x.h>
//#include "sys.h"
#include "iic_app.h"


//typedef enum{
//	IIC1_INDEX = 1,
//	IIC2_INDEX = 2
//}iic_index_t;



 
//IO��������
//#define IIC1_SCL    PBout(6) //SCL
//#define IIC1_SDA    PBout(7) //SDA	 
//#define READ_SDA1   PBin(7)  //����SDA 

//#define IIC2_SCL    PBout(10) //SCL
//#define IIC2_SDA    PBout(11) //SDA	 
//#define READ_SDA2   PBin(11)  //����SDA 

 
//IIC���в�������
void IIC_Init(iic_index_t index);                //��ʼ��IIC��IO��				 
void IIC_Start(iic_index_t index);				//����IIC��ʼ�ź�
void IIC_Stop(iic_index_t index);	  			//����IICֹͣ�ź�
void IIC_Send_Byte(iic_index_t index,uint8_t txd);			//IIC����һ���ֽ�
uint8_t IIC_Read_Byte(iic_index_t index,unsigned char ack);//IIC��ȡһ���ֽ�
uint8_t IIC_Wait_Ack(iic_index_t index); 				//IIC�ȴ�ACK�ź�
void IIC_Ack(iic_index_t index);					//IIC����ACK�ź�
void IIC_NAck(iic_index_t index);				//IIC������ACK�ź�
 
void I2C_WriteByte(uint16_t addr,uint8_t data,uint8_t device_addr);
uint16_t I2C_ReadByte(uint16_t addr,uint8_t device_addr,uint8_t ByteNumToRead);//�Ĵ�����ַ��������ַ��Ҫ�����ֽ��� 

#endif

