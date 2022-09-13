
#ifndef IIC_APP_H
#define IIC_APP_H

#include <gd32f10x.h>

typedef enum{
	IIC1_INDEX = 0    //PB6(scl),PB7
	,IIC2_INDEX = 1     //PB10(scl),PB11
	,IIC3_INDEX = 2    //PB8(scl),PB9
}iic_index_t;


void IicApp_Init(iic_index_t index);

/*
	IICapp写多个数据，最多写入256个字节！！！
	参数：
		word_addr 空间地址，就是要把字节dat写到24c02的哪一个存储单元中去
		dat       实际数据的首地址，
		len       实际需要写入数据的个数
	返回值：
		0  ：     成功
		非0：     失败
*/
uint8_t IicApp_Write_Bytes(iic_index_t index,uint8_t dev_addr,uint8_t word_addr,const uint8_t *dat,uint8_t len);



/*
	随机读数据
	参数 ： word_addr 指定我要读取的位置
			dat 表示存储数据缓存的首地址
			len 表示需要读取数据的个数
	返回值： 0 表示成功
			非0 表示失败
*/
uint8_t IicApp_Read_Bytes(iic_index_t index,uint8_t dev_addr,uint8_t word_addr,uint8_t *dat,uint8_t len);

//当前地址读！！
uint8_t IicApp_Read_Byte_Cur(iic_index_t index,uint8_t dev_addr,uint8_t *dat,uint8_t len);

#endif
