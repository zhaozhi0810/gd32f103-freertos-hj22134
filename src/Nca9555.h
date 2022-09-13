

#ifndef __NCA9555_H__
#define __NCA9555_H__

#include <gd32f10x.h>

#include "iic_app.h"


typedef enum  
{
	INPORT0 = 0,
	INPORT1 = 1,
	OUTPORT0 = 2,
	OUTPORT1 = 3,
	INVPORT0 = 4,
	INVPORT1 = 5,
	CFGPORT0 = 6,
	CFGPORT1 = 7
}Nca9555_Cmd_t;
	


#define PCA9555_DEVADDR 0x40    //iic地址，A0A1A2都为0，8位地址


//#define PCA9555_DEV1 (0<<1)
//#define PCA9555_DEV2 (4<<1)

void nca9555_init(iic_index_t index);


//读到的数据由参数返回
/*
	返回值非0表示错误
		0表示成功
*/
uint8_t nca9555_read_2inport(iic_index_t index,uint8_t dev_addr,uint8_t dat[]);


/*
	返回值非0表示错误
		0表示成功
*/
uint8_t nca9555_write_2config(iic_index_t index,uint8_t dev_addr,const uint8_t dat[]);



/*
	返回值非0表示错误
		0表示成功

	参数dev_addr 只考虑低4位，最低位保持为0即可。只需要高7位！！！！
*/
uint8_t nca9555_read_2outport(iic_index_t index,uint8_t dev_addr,uint8_t dat[]);


/*
	返回值非0表示错误
		0表示成功
*/
uint8_t nca9555_write_2outport(iic_index_t index,uint8_t dev_addr,const uint8_t dat[]);

/*
	返回值非0表示错误
		0表示成功
*/
uint8_t nca9555_write_2inv_cfg(iic_index_t index,uint8_t dev_addr,const uint8_t dat[]);



//2022-01-03 增加
uint8_t nca9555_read_2outport(iic_index_t index,uint8_t dev_addr,uint8_t dat[]);
//2022-01-03 读取配置寄存器的值，用于判断单片机需要重启还是复位
uint8_t nca9555_read_2config(iic_index_t index,uint8_t dev_addr,uint8_t dat[]);


//2022-07-04 增加 针对其中某一个P0的读写操作
uint8_t nca9555_read_inport(iic_index_t index,uint8_t dev_addr,uint8_t port_offset,uint8_t dat[]);
uint8_t nca9555_write_outport(iic_index_t index,uint8_t dev_addr,uint8_t port_offset, uint8_t dat);
#endif
