
#include "includes.h"


/*

Command Register
0         Input port 0   //�˿�0����������
1         Input port 1
2         Output port 0   //�˿�0���������
3         Output port 1
4         Polarity inversion port 0  //����������ݼ���ȡ��
5         Polarity inversion port 1
6         Configuration port 0     //�������ŵĹ��ܣ�1 ���루Ĭ�ϣ� 0 ���
7         Configuration port 1

*/


//const iic_index_t  NCA9555_Iic_Controler = IIC3_INDEX;   //PB8,PB9
#define DATA_LENGTH 2





void nca9555_init(iic_index_t index)
{
	//��������ʼ��
	IicApp_Init(index);
}





uint8_t nca9555_read_inport(iic_index_t index,uint8_t dev_addr,uint8_t port_offset,uint8_t dat[])
{	
	dev_addr &= 0xe;
	dev_addr |= PCA9555_DEVADDR;
	
	port_offset = !!port_offset;   //ֻȡ0��1
	
	return IicApp_Read_Bytes(index,dev_addr,INPORT0+port_offset,dat,1);
}


//�����������ɲ�������
/*
	����ֵ��0��ʾ����
		0��ʾ�ɹ�

	����dev_addr ֻ���ǵ�4λ�����λ����Ϊ0���ɡ�ֻ��Ҫ��7λ��������
*/
uint8_t nca9555_read_2inport(iic_index_t index,uint8_t dev_addr,uint8_t dat[])
{	
	dev_addr &= 0xe;
	dev_addr |= PCA9555_DEVADDR;
	return IicApp_Read_Bytes(index,dev_addr,INPORT0,dat,DATA_LENGTH);
}



/*
	����ֵ��0��ʾ����
		0��ʾ�ɹ�

	����dev_addr ֻ���ǵ�4λ�����λ����Ϊ0���ɡ�ֻ��Ҫ��7λ��������
*/
uint8_t nca9555_read_2outport(iic_index_t index,uint8_t dev_addr,uint8_t dat[])
{	
	dev_addr &= 0xe;
	dev_addr |= PCA9555_DEVADDR;
	return IicApp_Read_Bytes(index,dev_addr,OUTPORT0,dat,DATA_LENGTH);
}

/*
	����ֵ��0��ʾ����
		0��ʾ�ɹ�
*/
uint8_t nca9555_write_2config(iic_index_t index,uint8_t dev_addr,const uint8_t dat[])
{
	dev_addr &= 0xe;
	dev_addr |= PCA9555_DEVADDR;
	return IicApp_Write_Bytes(index,dev_addr,CFGPORT0,dat,DATA_LENGTH);
}

//2022-01-03 ��ȡ���üĴ�����ֵ�������жϵ�Ƭ����Ҫ�������Ǹ�λ
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

//2022-01-03���ӽ���

/*
	����ֵ��0��ʾ����
		0��ʾ�ɹ�
*/
uint8_t nca9555_write_outport(iic_index_t index,uint8_t dev_addr,uint8_t port_offset, uint8_t dat)
{
	dev_addr &= 0xe;
	dev_addr |= PCA9555_DEVADDR;
	
	port_offset = !!port_offset;   //ֻȡ0��1
	
	return IicApp_Write_Bytes(index,dev_addr,OUTPORT0+port_offset,&dat,1);
}



uint8_t nca9555_write_2outport(iic_index_t index,uint8_t dev_addr,const uint8_t dat[])
{
	dev_addr &= 0xe;
	dev_addr |= PCA9555_DEVADDR;
	return IicApp_Write_Bytes(index,dev_addr,OUTPORT0,dat,DATA_LENGTH);
}





/*
	����ֵ��0��ʾ����
		0��ʾ�ɹ�
*/
uint8_t nca9555_write_2inv_cfg(iic_index_t index,uint8_t dev_addr,const uint8_t dat[])
{
	dev_addr &= 0xe;
	dev_addr |= PCA9555_DEVADDR;
	return IicApp_Write_Bytes(index,dev_addr,INVPORT0,dat,DATA_LENGTH);
}

