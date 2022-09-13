

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
	


#define PCA9555_DEVADDR 0x40    //iic��ַ��A0A1A2��Ϊ0��8λ��ַ


//#define PCA9555_DEV1 (0<<1)
//#define PCA9555_DEV2 (4<<1)

void nca9555_init(iic_index_t index);


//�����������ɲ�������
/*
	����ֵ��0��ʾ����
		0��ʾ�ɹ�
*/
uint8_t nca9555_read_2inport(iic_index_t index,uint8_t dev_addr,uint8_t dat[]);


/*
	����ֵ��0��ʾ����
		0��ʾ�ɹ�
*/
uint8_t nca9555_write_2config(iic_index_t index,uint8_t dev_addr,const uint8_t dat[]);



/*
	����ֵ��0��ʾ����
		0��ʾ�ɹ�

	����dev_addr ֻ���ǵ�4λ�����λ����Ϊ0���ɡ�ֻ��Ҫ��7λ��������
*/
uint8_t nca9555_read_2outport(iic_index_t index,uint8_t dev_addr,uint8_t dat[]);


/*
	����ֵ��0��ʾ����
		0��ʾ�ɹ�
*/
uint8_t nca9555_write_2outport(iic_index_t index,uint8_t dev_addr,const uint8_t dat[]);

/*
	����ֵ��0��ʾ����
		0��ʾ�ɹ�
*/
uint8_t nca9555_write_2inv_cfg(iic_index_t index,uint8_t dev_addr,const uint8_t dat[]);



//2022-01-03 ����
uint8_t nca9555_read_2outport(iic_index_t index,uint8_t dev_addr,uint8_t dat[]);
//2022-01-03 ��ȡ���üĴ�����ֵ�������жϵ�Ƭ����Ҫ�������Ǹ�λ
uint8_t nca9555_read_2config(iic_index_t index,uint8_t dev_addr,uint8_t dat[]);


//2022-07-04 ���� �������ĳһ��P0�Ķ�д����
uint8_t nca9555_read_inport(iic_index_t index,uint8_t dev_addr,uint8_t port_offset,uint8_t dat[]);
uint8_t nca9555_write_outport(iic_index_t index,uint8_t dev_addr,uint8_t port_offset, uint8_t dat);
#endif
