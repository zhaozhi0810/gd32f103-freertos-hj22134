
#include "includes.h"
#include "i2c.h"     //i2c.h��������includes.h�У���Ϊ�����������ļ������ӿڡ�ֻ��iic_appʹ��


/*	
	������IIC��Ӧ�ò㣬ʵ����������
		��Ҫ���õײ��hard_iiC.c�е�ʵ�֣�����ʹ��ģ��ķ�ʽʵ��
		
	1. ����λ�ö�����  IicApp_read_bytes
	2. ����λ��д����  IicApp_write_bytes
	
*/

//���ǵ��ж���ӿڿ��ܻ���ó�ʼ��������ȫ�ַ�ֹ��γ�ʼ��
static uint8_t iic_inited = 0;    //��0λ�͵�1λ ����Ϊ1��ʾ�Ѿ���ʼ���ˣ�0��ʾδ��ʼ��

void IicApp_Init(iic_index_t index)
{
//	iic_init(I2Cx);	
	if(iic_inited & (1<< index))  //�Ѿ���ʼ���˾�ֱ�ӷ��ذ�
		return;
	
	IIC_Init(index);
	
	iic_inited |= (1<< index);    //��ʾ�Ѿ���ʼ����
}

/*
	��ǰλ�ö�����ֽڡ����һ�ζ�ȡ256���ֽڣ�������
	����    0 ��ʾ�ɹ�����0��ʾʧ��
	!!!!!�������ṩ�ӿ�
*/
uint8_t IicApp_Read_Byte_Cur(iic_index_t index,uint8_t dev_addr,uint8_t *dat,uint8_t len)
{
	uint8_t i;
	
	vTaskSuspendAll();//taskENTER_CRITICAL();  //�����ٽ���   2022-09-13
	
	IIC_Start(index);
			
	//2.�����豸��ַ
	IIC_Send_Byte(index,dev_addr | 1);	    //��������ַ
	if(IIC_Wait_Ack(index)!= 0) 
	//if(iic_put_devaddr(I2Cx,dev_addr | 1) != 0)   //���λ��1����ʾ������
	{
		xTaskResumeAll();//taskEXIT_CRITICAL(); //�˳��ٽ���   2022-09-13
		//debug_printf_string("i2c  read_byte_cur send dev addr error!\r\n");
		debug_printf_string("ERROR:IIC_Wait_Ack(index)!= 0\r\n");
		IIC_Stop(index);//����һ��ֹͣ����
		
		return 2;
	}
	
	for(i=0;i<len;i++)
	{					
		//3.���һ���ֽڵ�����
		//dat[i] = iic_get_byte_data(I2Cx);
	
		//4.ֻ�����һ�����ݷ��ͷ�Ӧ��
		if(i == len -1)
		{
			dat[i]=IIC_Read_Byte(index,0);
		}
		else
		{
			dat[i]=IIC_Read_Byte(index,1);
		}
			
	}		
	//.����stopʱ��	
	xTaskResumeAll();//taskEXIT_CRITICAL(); //�˳��ٽ���  2022-09-13
	IIC_Stop(index);//����һ��ֹͣ����
	return 0;
}

/*
	IicAppд������ݣ����д��256���ֽڣ�����
	������
		word_addr �ռ��ַ������Ҫ���ֽ�datд��24c02����һ���洢��Ԫ��ȥ
		dat       ʵ�����ݵ��׵�ַ��
		len       ʵ����Ҫд�����ݵĸ���
	����ֵ��
		0  ��     �ɹ�
		��0��     ʧ��
*/
uint8_t IicApp_Write_Bytes(iic_index_t index,uint8_t dev_addr,uint8_t word_addr,const uint8_t *dat,uint8_t len)
{
	uint8_t i;
	
	vTaskSuspendAll();//taskENTER_CRITICAL();  //�����ٽ���   2022-09-13
	IIC_Start(index);
	
	//2.�����豸��ַ
	IIC_Send_Byte(index,dev_addr & 0xfe);	    //��������ַ
	if(IIC_Wait_Ack(index)!= 0) 
	{
		xTaskResumeAll();//taskEXIT_CRITICAL(); //�˳��ٽ���  2022-09-13
		//debug_printf_string("i2c  read_byte_cur send dev addr error!\r\n");
		debug_printf_string("ERROR: send dev_addr IIC_Wait_Ack(index)!= 0\r\n");
		IIC_Stop(index);
		
		return 2;
	}

	//3.���Ϳռ��ַ
	IIC_Send_Byte(index,word_addr);     //�����ֽ�							    
	if(IIC_Wait_Ack(index))  //���û��Ӧ��ֱ���˳�
	{
		xTaskResumeAll();//taskEXIT_CRITICAL(); //�˳��ٽ���  2022-09-13
		//debug_printf_string("send word addr error!\r\n");
		debug_printf_string("ERROR: send word_addr IIC_Wait_Ack(index)!= 0\r\n");
		IIC_Stop(index); //iic_stop(I2Cx);     //����ֹͣ�źţ����߾Ϳ�����
		
		return 3;
	}
	
	//len����0��ʱ������Ϊ���������һ����Ч����
	if(len == 0)
	{
		xTaskResumeAll();//taskEXIT_CRITICAL(); //�˳��ٽ���  2022-09-13
		return 255;   //���Ǹ��������
	}
	
	for(i=0;i<len;i++)
	{		
		//4.��������
		IIC_Send_Byte(index,dat[i]);     //�����ֽ�							    
		if(IIC_Wait_Ack(index)!= 0)  //���û��Ӧ��ֱ���˳�
		//if(iic_put_byte_data(I2Cx,dat[i]))  //���û��Ӧ��ֱ���˳�
		{
			xTaskResumeAll();//taskEXIT_CRITICAL(); //�˳��ٽ���  2022-09-13
			//debug_printf_string("send data error!\r\n");
			debug_printf_string("ERROR: send data IIC_Wait_Ack(index)!= 0\r\n");
			IIC_Stop(index); //iic_stop(I2Cx);     //����ֹͣ�źţ����߾Ϳ�����
			
			return 4;
		}
	}
	xTaskResumeAll();//taskEXIT_CRITICAL(); //�˳��ٽ���  2022-09-13
	//5.�������������ߵ�ռ��
	IIC_Stop(index); //iic_stop(I2Cx);	
	return 0;
}





/*
	���������
	���� �� word_addr ָ����Ҫ��ȡ��λ��
			dat ��ʾ�洢���ݻ�����׵�ַ
			len ��ʾ��Ҫ��ȡ���ݵĸ���
	����ֵ�� 0 ��ʾ�ɹ�
			��0 ��ʾʧ��
*/
uint8_t IicApp_Read_Bytes(iic_index_t index,uint8_t dev_addr,uint8_t word_addr,uint8_t *dat,uint8_t len)
{
	uint8_t ret;
	
	ret = IicApp_Write_Bytes(index,dev_addr,word_addr,(void*)0,0);
	
	if(ret == 255)	//��������Ĵ���
	{
	//	debug_printf_string("IicApp_Read_Byte_Curret == 255 \r\n");
		return IicApp_Read_Byte_Cur(index,dev_addr,dat,len);
	}
	else
		debug_printf_string("ERROR: IicApp_Write_Bytes\r\n");
	return ret;   //���ⷵ��
}

