
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
	IICappд������ݣ����д��256���ֽڣ�����
	������
		word_addr �ռ��ַ������Ҫ���ֽ�datд��24c02����һ���洢��Ԫ��ȥ
		dat       ʵ�����ݵ��׵�ַ��
		len       ʵ����Ҫд�����ݵĸ���
	����ֵ��
		0  ��     �ɹ�
		��0��     ʧ��
*/
uint8_t IicApp_Write_Bytes(iic_index_t index,uint8_t dev_addr,uint8_t word_addr,const uint8_t *dat,uint8_t len);



/*
	���������
	���� �� word_addr ָ����Ҫ��ȡ��λ��
			dat ��ʾ�洢���ݻ�����׵�ַ
			len ��ʾ��Ҫ��ȡ���ݵĸ���
	����ֵ�� 0 ��ʾ�ɹ�
			��0 ��ʾʧ��
*/
uint8_t IicApp_Read_Bytes(iic_index_t index,uint8_t dev_addr,uint8_t word_addr,uint8_t *dat,uint8_t len);

//��ǰ��ַ������
uint8_t IicApp_Read_Byte_Cur(iic_index_t index,uint8_t dev_addr,uint8_t *dat,uint8_t len);

#endif
