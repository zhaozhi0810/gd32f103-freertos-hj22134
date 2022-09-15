
#include "includes.h"
#include "i2c.h"     //i2c.h不包含到includes.h中，因为它不对其他文件开发接口。只对iic_app使用


/*	
	这里是IIC的应用层，实现两个函数
		需要调用底层的hard_iiC.c中的实现，或者使用模拟的方式实现
		
	1. 任意位置读操作  IicApp_read_bytes
	2. 任意位置写操作  IicApp_write_bytes
	
*/

//考虑到有多个接口可能会调用初始化，设置全局防止多次初始化
static uint8_t iic_inited = 0;    //第0位和第1位 设置为1表示已经初始化了，0表示未初始化

void IicApp_Init(iic_index_t index)
{
//	iic_init(I2Cx);	
	if(iic_inited & (1<< index))  //已经初始化了就直接返回吧
		return;
	
	IIC_Init(index);
	
	iic_inited |= (1<< index);    //表示已经初始化了
}

/*
	当前位置读多个字节。最多一次读取256个字节！！！！
	返回    0 表示成功，非0表示失败
	!!!!!不对外提供接口
*/
uint8_t IicApp_Read_Byte_Cur(iic_index_t index,uint8_t dev_addr,uint8_t *dat,uint8_t len)
{
	uint8_t i;
	
	vTaskSuspendAll();//taskENTER_CRITICAL();  //进入临界区   2022-09-13
	
	IIC_Start(index);
			
	//2.发送设备地址
	IIC_Send_Byte(index,dev_addr | 1);	    //发器件地址
	if(IIC_Wait_Ack(index)!= 0) 
	//if(iic_put_devaddr(I2Cx,dev_addr | 1) != 0)   //最低位是1，表示读操作
	{
		xTaskResumeAll();//taskEXIT_CRITICAL(); //退出临界区   2022-09-13
		//printf("i2c  read_byte_cur send dev addr error!\r\n");
		DBG_PRINTF("ERROR:IIC_Wait_Ack(index)!= 0 index = %d\r\n",index);
		IIC_Stop(index);//产生一个停止条件
		
		return 2;
	}
	
	for(i=0;i<len;i++)
	{					
		//3.获得一个字节的数据
		//dat[i] = iic_get_byte_data(I2Cx);
	
		//4.只有最后一个数据发送非应答
		if(i == len -1)
		{
			dat[i]=IIC_Read_Byte(index,0);
		}
		else
		{
			dat[i]=IIC_Read_Byte(index,1);
		}
			
	}		
	//.发送stop时序
	IIC_Stop(index);//产生一个停止条件
	xTaskResumeAll();//taskEXIT_CRITICAL(); //退出临界区  2022-09-13
	return 0;
}

/*
	IicApp写多个数据，最多写入256个字节！！！
	参数：
		word_addr 空间地址，就是要把字节dat写到24c02的哪一个存储单元中去
		dat       实际数据的首地址，
		len       实际需要写入数据的个数
	返回值：
		0  ：     成功
		非0：     失败
*/
uint8_t IicApp_Write_Bytes(iic_index_t index,uint8_t dev_addr,uint8_t word_addr,const uint8_t *dat,uint8_t len)
{
	uint8_t i;
	
	vTaskSuspendAll();//taskENTER_CRITICAL();  //进入临界区   2022-09-13
	IIC_Start(index);
	
	//2.发送设备地址
	IIC_Send_Byte(index,dev_addr & 0xfe);	    //发器件地址
	if(IIC_Wait_Ack(index)!= 0) 
	{
		xTaskResumeAll();//taskEXIT_CRITICAL(); //退出临界区  2022-09-13
		//printf("i2c  read_byte_cur send dev addr error!\r\n");
		DBG_PRINTF("ERROR: send dev_addr IIC_Wait_Ack(index)!= 0 dev_addr = 0x%x index = %d\r\n",dev_addr,index);
		IIC_Stop(index);
		
		return 2;
	}

	//3.发送空间地址
	IIC_Send_Byte(index,word_addr);     //发送字节							    
	if(IIC_Wait_Ack(index))  //如果没有应答，直接退出
	{
		xTaskResumeAll();//taskEXIT_CRITICAL(); //退出临界区  2022-09-13
		//printf("send word addr error!\r\n");
		DBG_PRINTF("ERROR: send word_addr IIC_Wait_Ack(index)!= 0 index = %d\r\n",index);
		IIC_Stop(index); //iic_stop(I2Cx);     //发送停止信号，总线就空闲了
		
		return 3;
	}
	
	//len等于0的时候，我认为是随机读的一个有效操作
	if(len == 0)
	{
		xTaskResumeAll();//taskEXIT_CRITICAL(); //退出临界区  2022-09-13
		return 255;   //这是个特殊情况
	}
	
	for(i=0;i<len;i++)
	{		
		//4.发送内容
		IIC_Send_Byte(index,dat[i]);     //发送字节							    
		if(IIC_Wait_Ack(index)!= 0)  //如果没有应答，直接退出
		//if(iic_put_byte_data(I2Cx,dat[i]))  //如果没有应答，直接退出
		{
			xTaskResumeAll();//taskEXIT_CRITICAL(); //退出临界区  2022-09-13
			//printf("send data error!\r\n");
			DBG_PRINTF("ERROR: send data IIC_Wait_Ack(index)!= 0 i=%d index = %d\r\n",i,index);
			IIC_Stop(index); //iic_stop(I2Cx);     //发送停止信号，总线就空闲了
			
			return 4;
		}
	}
	
	//5.结束，结束总线的占用
	IIC_Stop(index); //iic_stop(I2Cx);
	xTaskResumeAll();//taskEXIT_CRITICAL(); //退出临界区  2022-09-13
	return 0;
}





/*
	随机读数据
	参数 ： word_addr 指定我要读取的位置
			dat 表示存储数据缓存的首地址
			len 表示需要读取数据的个数
	返回值： 0 表示成功
			非0 表示失败
*/
uint8_t IicApp_Read_Bytes(iic_index_t index,uint8_t dev_addr,uint8_t word_addr,uint8_t *dat,uint8_t len)
{
	uint8_t ret;
	
	ret = IicApp_Write_Bytes(index,dev_addr,word_addr,(void*)0,0);
	
	if(ret == 255)	//特殊情况的处理
	{
	//	printf("IicApp_Read_Byte_Curret == 255 \r\n");
		return IicApp_Read_Byte_Cur(index,dev_addr,dat,len);
	}
	else
		DBG_PRINTF("ERROR: IicApp_Write_Bytes ret = %d index = %d\r\n",ret,index);
	return ret;   //在这返回
}

