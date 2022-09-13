

//i2c.h 仅对iic_app.c 开放，不对其他c开发！！！！


#ifndef I2C_H
#define I2C_H



#include <gd32f10x.h>
//#include "sys.h"
#include "iic_app.h"


//typedef enum{
//	IIC1_INDEX = 1,
//	IIC2_INDEX = 2
//}iic_index_t;



 
//IO操作函数
//#define IIC1_SCL    PBout(6) //SCL
//#define IIC1_SDA    PBout(7) //SDA	 
//#define READ_SDA1   PBin(7)  //输入SDA 

//#define IIC2_SCL    PBout(10) //SCL
//#define IIC2_SDA    PBout(11) //SDA	 
//#define READ_SDA2   PBin(11)  //输入SDA 

 
//IIC所有操作函数
void IIC_Init(iic_index_t index);                //初始化IIC的IO口				 
void IIC_Start(iic_index_t index);				//发送IIC开始信号
void IIC_Stop(iic_index_t index);	  			//发送IIC停止信号
void IIC_Send_Byte(iic_index_t index,uint8_t txd);			//IIC发送一个字节
uint8_t IIC_Read_Byte(iic_index_t index,unsigned char ack);//IIC读取一个字节
uint8_t IIC_Wait_Ack(iic_index_t index); 				//IIC等待ACK信号
void IIC_Ack(iic_index_t index);					//IIC发送ACK信号
void IIC_NAck(iic_index_t index);				//IIC不发送ACK信号
 
void I2C_WriteByte(uint16_t addr,uint8_t data,uint8_t device_addr);
uint16_t I2C_ReadByte(uint16_t addr,uint8_t device_addr,uint8_t ByteNumToRead);//寄存器地址，器件地址，要读的字节数 

#endif

