

#include "includes.h"
#include "i2c.h"


#define IIC_DELAY_CONT 2   //������ʱ����,GD32�ﵽ��108M����������Ϊ2����������Ĭ��72M��������Ϊ0


static void Sda_In(iic_index_t index)
{
	if(index == IIC2_INDEX)
		gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ,GPIO_PIN_11);
	else if(index == IIC3_INDEX)
		gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ,GPIO_PIN_9);
	else 
		gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ,GPIO_PIN_7);
}


static void Sda_Out(iic_index_t index)
{
	if(index == IIC2_INDEX)
		gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ,GPIO_PIN_11); 
	else if(index == IIC3_INDEX)
		gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ,GPIO_PIN_9);		
	else 
		gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ,GPIO_PIN_7);
}



static void Iic_Sda_Set(iic_index_t index,uint8_t val)
{
	if(index == IIC2_INDEX)
	{
		if(val)
			gpio_bit_set(GPIOB, GPIO_PIN_11);
		else
			gpio_bit_reset(GPIOB, GPIO_PIN_11);		
	}
	else if(index == IIC3_INDEX)
	{
		if(val)
			gpio_bit_set(GPIOB, GPIO_PIN_9);
		else
			gpio_bit_reset(GPIOB, GPIO_PIN_9);		
	}
	else
	{
		if(val)
			gpio_bit_set(GPIOB, GPIO_PIN_7);
		else
			gpio_bit_reset(GPIOB, GPIO_PIN_7);
	}
}

static void Iic_Scl_Set(iic_index_t index,uint8_t val)
{
	if(index == IIC2_INDEX)
	{
		if(val)
			gpio_bit_set(GPIOB, GPIO_PIN_10);
		else
			gpio_bit_reset(GPIOB, GPIO_PIN_10);
	}
	else if(index == IIC3_INDEX)
	{
		if(val)
			gpio_bit_set(GPIOB, GPIO_PIN_8);
		else
			gpio_bit_reset(GPIOB, GPIO_PIN_8);
	}
	else
	{		
		if(val)
			gpio_bit_set(GPIOB, GPIO_PIN_6);
		else
			gpio_bit_reset(GPIOB, GPIO_PIN_6);
	}
}



static uint8_t Read_Sda(iic_index_t index)
{
	if(index == IIC2_INDEX) 
		return gpio_input_bit_get(GPIOB,GPIO_PIN_11); 
	else if(index == IIC3_INDEX) 
		return gpio_input_bit_get(GPIOB,GPIO_PIN_9); 
	else 
		return gpio_input_bit_get(GPIOB,GPIO_PIN_7);
}



void IIC_Init(iic_index_t index)
{					     
	uint32_t pin;
	//1. ʱ��ʹ��
	rcu_periph_clock_enable(RCU_GPIOB);
		
	//2. ����Ϊ����ģʽ	
	if(index == IIC2_INDEX)
		pin = BITS(10,11); 
	else if(index == IIC3_INDEX)
		pin = BITS(8,9); 
	else
		pin = BITS(6,7); 
	
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, pin);

	Iic_Scl_Set(index,1);
	Iic_Sda_Set(index,1);
}



//����IIC��ʼ�ź�
void IIC_Start(iic_index_t index)
{
	Sda_Out(index);     //sda�����
	Iic_Sda_Set(index,1);	  	  
	Iic_Scl_Set(index,1);
	Delay1us(IIC_DELAY_CONT+4);
 	Iic_Sda_Set(index,0);//START:when CLK is high,DATA change form high to low 
	Delay1us(IIC_DELAY_CONT+4);
	Iic_Scl_Set(index,0);//ǯסI2C���ߣ�׼�����ͻ�������� 
}


//����IICֹͣ�ź�
void IIC_Stop(iic_index_t index)
{
	Iic_Scl_Set(index,0);
	Sda_Out(index);//sda�����	
	Iic_Sda_Set(index,0);//STOP:when CLK is high DATA change form low to high
 	Delay1us(IIC_DELAY_CONT+4);
	Iic_Scl_Set(index,1); 
	Delay1us(IIC_DELAY_CONT+1);
	Iic_Sda_Set(index,1);//����I2C���߽����ź�
	Delay1us(IIC_DELAY_CONT+4);	

	//����֮��sda��scl�����Ϊ�ߣ���
}



//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
uint8_t IIC_Wait_Ack(iic_index_t index)
{
	uint8_t ucErrTime=0;
	
	Iic_Scl_Set(index,0);
	Delay1us(IIC_DELAY_CONT+1);	 
	Iic_Sda_Set(index,1);
	Delay1us(IIC_DELAY_CONT+1);	
	
	Sda_In(index);      //SDA����Ϊ���� 	
	Iic_Scl_Set(index,1);
	
	Delay1us(IIC_DELAY_CONT+1);	 
	while(Read_Sda(index))
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop(index);
			return 1;
		}
		Delay1us(1);   //��ʱһ��
	}
	Iic_Scl_Set(index,0);//ʱ�����0
	Delay1us(IIC_DELAY_CONT+1);	
	return 0;  
} 



//����ACKӦ��
void IIC_Ack(iic_index_t index)
{
	Iic_Scl_Set(index,0);
	Sda_Out(index);
	Iic_Sda_Set(index,0);
	Delay1us(IIC_DELAY_CONT+2);
	Iic_Scl_Set(index,1);
	Delay1us(IIC_DELAY_CONT+4);
	Iic_Scl_Set(index,0);
	Delay1us(IIC_DELAY_CONT+1);
}


//������ACKӦ��		    
void IIC_NAck(iic_index_t index)
{
	Iic_Scl_Set(index,0);
	Sda_Out(index);
	Iic_Sda_Set(index,1);
	Delay1us(IIC_DELAY_CONT+2);
	Iic_Scl_Set(index,1);
	Delay1us(IIC_DELAY_CONT+4);
	Iic_Scl_Set(index,0);
	Delay1us(IIC_DELAY_CONT+1);
}


//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��			  
void IIC_Send_Byte(iic_index_t index,uint8_t txd)
{                        
    uint8_t t;  
	Iic_Scl_Set(index,0);//����ʱ�ӿ�ʼ���ݴ���
	Sda_Out(index); 	    
    
    for(t=0;t<8;t++)
    {              
        Delay1us(IIC_DELAY_CONT+1);   //��TEA5767��������ʱ���Ǳ����
		Iic_Sda_Set(index,txd & 0x80);//IIC_SDA(index)=((txd>>(7-t))& 1);
		txd<<=1; 	//�ȷ������λ��Ȼ�������ƶ�һλ  
		Delay1us(IIC_DELAY_CONT+2);   //��TEA5767��������ʱ���Ǳ����
		Iic_Scl_Set(index,1);
		Delay1us(IIC_DELAY_CONT+4); 
		Iic_Scl_Set(index,0);	
		Delay1us(IIC_DELAY_CONT+1);
    }	 
} 	    
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
uint8_t IIC_Read_Byte(iic_index_t index,unsigned char ack)
{
	unsigned char i,receive=0;
	Iic_Scl_Set(index,0); 
	Sda_In(index);//SDA����Ϊ����
	Delay1us(IIC_DELAY_CONT+1);
	for(i=0;i<8;i++ )
	{
		Iic_Scl_Set(index,0); 
		Delay1us(IIC_DELAY_CONT+3);
		Iic_Scl_Set(index,1);
		receive<<=1;
		Delay1us(IIC_DELAY_CONT+2);
		if(Read_Sda(index))
			receive++;   
		Delay1us(IIC_DELAY_CONT+2);
		Iic_Scl_Set(index,0); 	
	}					 
	if (!ack)  //0 ����ҪӦ��һ�������һ���ֽ��ˡ�
		IIC_NAck(index);//����nACK
	else   //����Ӧ��
		IIC_Ack(index); //����ACK 

	
	return receive;
}
