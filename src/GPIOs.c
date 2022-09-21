
#include "includes.h"

//PD7 
static void Wxen_Control_Init(void)
{
	//1. 时钟使能
	rcu_periph_clock_enable(RCU_GPIOD);
		
	//2.0 上电控制引脚
	gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_7);  //控制输出	
	//2. 初始化后，默认输出高
	gpio_bit_reset(GPIOD, GPIO_PIN_7);
	
}


//PD6  MicCtl
static void MicCtl_Control_Init(void)
{
	//1. 时钟使能
	rcu_periph_clock_enable(RCU_GPIOD);
		
	//2.0 上电控制引脚
	gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_6);  //控制输出	
	//2. 初始化后，默认输出低
	gpio_bit_reset(GPIOD, GPIO_PIN_6);	
}





////PD6  MicCtl输出低
//void MicCtl_Control_OutLow(void)
//{
//	gpio_bit_reset(GPIOD, GPIO_PIN_6);
//}







//PD8  lcd-reset ,不知道是高电平复位还是低电平复位
//先假设低电平复位吧。
static void lcd_reset_control_init(void)
{
	//1. 时钟使能
	rcu_periph_clock_enable(RCU_GPIOD);
		
	//2.0 上电控制引脚
	gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_8);  //控制输出	
	//2. 初始化后，默认输出高
	gpio_bit_set(GPIOD, GPIO_PIN_8);	
}



//OE引脚的初始化 
/*
23.PC0 - OE1  电路输出控制，rk3399调试串口，rk3399与gd32的串口，rk3399与液晶通信串口，SAI8159iic4接口
24.PC1 - OE2  电路输出控制，i2s，i2s0，CB_RESETn，WDT_OUT
25.PC2 - OE3  电路输出控制，iic3接口，iic1接口，GPO6，PCIE_WAKEn，LED_PWM_7IN
26.PC3 - OE4 电路输出控制，PTT，GPI5，MIC_DET，PTT1，CODEC_GPI2，GPO4

OE3 低电平保持3秒左右
*/
static void OePins_Control_Init(void)
{
	//1. 时钟使能
	rcu_periph_clock_enable(RCU_GPIOC);
		
	//2.0 上电控制引脚
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);  //控制输出	
	//2. 初始化后，默认输出高
	gpio_bit_reset(GPIOC, GPIO_PIN_2);  //OE3 输出低
	
	gpio_bit_set(GPIOC, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_3);  //其他 输出高
}


//PE15，初始化后输出低
static void LcdCtrl_Control_Init(void)
{
	//1. 时钟使能
	rcu_periph_clock_enable(RCU_GPIOE);
		
	//2.0 上电控制引脚
	gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_15);  //控制输出	
	//2. 初始化后，默认输出高
	gpio_bit_reset(GPIOE, GPIO_PIN_15);  //LcdCtrl 输出低

}



static void Lcd7INCtrl_PwmPins_Init(void)
{
	//1. 时钟使能
	rcu_periph_clock_enable(RCU_GPIOB);
		
	//2.0 上电控制引脚
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_14|GPIO_PIN_15);  //控制输出	
	//2. 初始化后，默认输出高
	gpio_bit_set(GPIOB, GPIO_PIN_14|GPIO_PIN_15);  //LcdCtrl 输出低

}




//gpio 引脚初始化
void Gpios_init(void)
{
//	Wxen_Control_Init();   //2022-09-15 不初始化反而能正常使用
	lcd_pwm_init(70);
	
	MicCtl_Control_Init();   //MIC_CRL 引脚的初始化
	lcd_reset_control_init();  //lcd 复位引脚的初始化
	OePins_Control_Init();    //OE引脚的初始化

	LcdCtrl_Control_Init();   //7寸屏电源控制引脚  		
//	LcdCtrl_Enable();   //7寸屏lcd电源通电
//	Enable_LcdLight();    //对7寸屏的控制信号，背光使能和背光pwm控制,	

//	LcdCtrl_Control_Init();	    //
	//Lcd7INCtrl_PwmPins_Init();	
}





//PD6  MicCtl 输出控制(参数status 非0输出高，0输出低)
void MicCtl_Control_OutHigh(uint8_t status)
{
	debug_printf_string("MicCtl_Control_OutHigh\r\n");
	if(status)
		gpio_bit_set(GPIOD, GPIO_PIN_6);
	else
		gpio_bit_reset(GPIOD, GPIO_PIN_6);
}



//触发一次lcd的复位信号
void lcd_reset_control(void)
{
	gpio_bit_reset(GPIOD, GPIO_PIN_8);
	vTaskDelay(100);
	gpio_bit_set(GPIOD, GPIO_PIN_8);
}







//使能该引脚，通电7寸Lcd电源
void LcdCtrl_Enable(void)
{
	gpio_bit_set(GPIOE, GPIO_PIN_15);  //高电平使能
}

//禁止该引脚，断电Lcd电源
void LcdCtrl_Disable(void)
{
	gpio_bit_reset(GPIOE, GPIO_PIN_15);  //LcdCtrl 输出低
}


//使能该引脚，通电1.1v
void Wxen_Control_Enable(void)
{
	gpio_bit_set(GPIOD, GPIO_PIN_7);  //高电平使能
}


//禁止该引脚，断电1.1v
void Wxen_Control_Disable(void)
{
	gpio_bit_reset(GPIOD, GPIO_PIN_7);   //低电平无效
}


//which 1-4 分别表示oe1-oe4
void OePins_Output_Hight(uint8_t which)
{
	if(which >0 && which < 5)
	{
		gpio_bit_set(GPIOC, BIT(which-1));  //输出高
	}	
}


//which 1-4 分别表示oe1-oe4
void OePins_Output_Low(uint8_t which)
{
	if(which >0 && which < 5)
	{
		gpio_bit_reset(GPIOC, BIT(which-1));  //输出低
	}
}




