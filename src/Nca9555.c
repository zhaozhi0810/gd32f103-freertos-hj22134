
#include "includes.h"


/*

Command Register
0         Input port 0   //端口0的输入数据
1         Input port 1
2         Output port 0   //端口0的输出数据
3         Output port 1
4         Polarity inversion port 0  //将输入的数据极性取反
5         Polarity inversion port 1
6         Configuration port 0     //配置引脚的功能，1 输入（默认） 0 输出
7         Configuration port 1

*/


//const iic_index_t  NCA9555_Iic_Controler = IIC3_INDEX;   //PB8,PB9
#define DATA_LENGTH 2





void nca9555_init(iic_index_t index)
{
	//控制器初始化
	IicApp_Init(index);
}





uint8_t nca9555_read_inport(iic_index_t index,uint8_t dev_addr,uint8_t port_offset,uint8_t dat[])
{	
	dev_addr &= 0xe;
	dev_addr |= PCA9555_DEVADDR;
	
	port_offset = !!port_offset;   //只取0和1
	
	return IicApp_Read_Bytes(index,dev_addr,INPORT0+port_offset,dat,1);
}


//读到的数据由参数返回
/*
	返回值非0表示错误
		0表示成功

	参数dev_addr 只考虑低4位，最低位保持为0即可。只需要高7位！！！！
*/
uint8_t nca9555_read_2inport(iic_index_t index,uint8_t dev_addr,uint8_t dat[])
{	
	dev_addr &= 0xe;
	dev_addr |= PCA9555_DEVADDR;
	return IicApp_Read_Bytes(index,dev_addr,INPORT0,dat,DATA_LENGTH);
}



/*
	返回值非0表示错误
		0表示成功

	参数dev_addr 只考虑低4位，最低位保持为0即可。只需要高7位！！！！
*/
uint8_t nca9555_read_2outport(iic_index_t index,uint8_t dev_addr,uint8_t dat[])
{	
	dev_addr &= 0xe;
	dev_addr |= PCA9555_DEVADDR;
	return IicApp_Read_Bytes(index,dev_addr,OUTPORT0,dat,DATA_LENGTH);
}

/*
	返回值非0表示错误
		0表示成功
*/
uint8_t nca9555_write_2config(iic_index_t index,uint8_t dev_addr,const uint8_t dat[])
{
	dev_addr &= 0xe;
	dev_addr |= PCA9555_DEVADDR;
	return IicApp_Write_Bytes(index,dev_addr,CFGPORT0,dat,DATA_LENGTH);
}

//2022-01-03 读取配置寄存器的值，用于判断单片机需要重启还是复位
//uint8_t nca9555_read_2config(iic_index_t index,uint8_t dev_addr,uint8_t dat[])
//{
//	dev_addr &= 0xe;
//	dev_addr |= PCA9555_DEVADDR;
//	return IICapp_read_bytes(index,dev_addr,CFGPORT0,dat,DATA_LENGTH);
//}

//uint8_t nca9555_read_2outport(iic_index_t index,uint8_t dev_addr,uint8_t dat[])
//{
//	dev_addr &= 0xe;
//	dev_addr |= PCA9555_DEVADDR;
//	return IICapp_read_bytes(index,dev_addr,OUTPORT0,dat,DATA_LENGTH);
//}

//2022-01-03增加结束

/*
	返回值非0表示错误
		0表示成功
*/
uint8_t nca9555_write_outport(iic_index_t index,uint8_t dev_addr,uint8_t port_offset, uint8_t dat)
{
	dev_addr &= 0xe;
	dev_addr |= PCA9555_DEVADDR;
	
	port_offset = !!port_offset;   //只取0和1
	
	return IicApp_Write_Bytes(index,dev_addr,OUTPORT0+port_offset,&dat,1);
}



uint8_t nca9555_write_2outport(iic_index_t index,uint8_t dev_addr,const uint8_t dat[])
{
	dev_addr &= 0xe;
	dev_addr |= PCA9555_DEVADDR;
	return IicApp_Write_Bytes(index,dev_addr,OUTPORT0,dat,DATA_LENGTH);
}





/*
	返回值非0表示错误
		0表示成功
*/
uint8_t nca9555_write_2inv_cfg(iic_index_t index,uint8_t dev_addr,const uint8_t dat[])
{
	dev_addr &= 0xe;
	dev_addr |= PCA9555_DEVADDR;
	return IicApp_Write_Bytes(index,dev_addr,INVPORT0,dat,DATA_LENGTH);
}

