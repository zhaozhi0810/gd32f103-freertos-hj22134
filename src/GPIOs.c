
#include "includes.h"

//TaskHandle_t  TaskHandle_Morseptt;   //存放morseptt按键任务指针


//PD7 
//static void Wxen_Control_Init(void)
//{
//	//1. 时钟使能
//	rcu_periph_clock_enable(RCU_GPIOD);
//		
//	//2.0 上电控制引脚
//	gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_7);  //控制输出	
//	//2. 初始化后，默认输出高
//	gpio_bit_reset(GPIOD, GPIO_PIN_7);
//	
//}


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



#if 0
//PD10  MorsePtt,输入引脚
static void MorsePtt_Control_Init(void)
{
	//1. 时钟使能
	rcu_periph_clock_enable(RCU_GPIOD);
		
	//2.0 上电控制引脚
	gpio_init(GPIOD, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, GPIO_PIN_10);  //控制输出	
	//2. 初始化后，默认输出低
	//gpio_bit_reset(GPIOD, GPIO_PIN_6);	
		
	//3 复用为外部中断引脚，
	gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOD, GPIO_PIN_SOURCE_10);
	//设置触发方式，双边沿触发
	exti_init(EXTI_10, EXTI_INTERRUPT, EXTI_TRIG_BOTH);	
	exti_interrupt_flag_clear(EXTI_10);
	exti_interrupt_enable(EXTI_10);
	//3.1 nvic允许中断
	//中断控制器使能，使用的是外部中断12
	//nvic_irq_enable(EXTI10_15_IRQn,  7, 0);   //允许中断，并设置优先级	
}



//外部中断10的处理函数,按键按下和松开都会触发中断！！！！
void exint10_handle(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	
	if(gpio_input_bit_get(GPIOD, GPIO_PIN_10))  //高电平，就是按键断开
	{
		V12_CTL_Control_SetOutVal(0);  //v12 电压禁止输出
	}
	else  //低电平，按键被按下
	{
		xTaskNotifyFromISR(TaskHandle_Morseptt, 0, eIncrement, &xHigherPriorityTaskWoken);  //唤醒休眠的任务	
	}
	//并且禁止中断
	//exti_interrupt_disable(EXTI_10);   //扫描完毕之后再使能
}




//morse ptt 按键检测
void task_morse_ptt_scan(void* arg)
{	
	MorsePtt_Control_Init();
	while(1)
	{	
		//等待任务被唤醒
		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);   //减1，然后无限等待
		{	
			vTaskDelay(2000);    //延时3s
			V12_CTL_Control_SetOutVal(1);  //v12 电压使能输出
		}		
	}
}


#endif








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



//static void Lcd7INCtrl_PwmPins_Init(void)
//{
//	//1. 时钟使能
//	rcu_periph_clock_enable(RCU_GPIOB);
//		
//	//2.0 上电控制引脚
//	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_14|GPIO_PIN_15);  //控制输出	
//	//2. 初始化后，默认输出高
//	gpio_bit_set(GPIOB, GPIO_PIN_14|GPIO_PIN_15);  //LcdCtrl 输出低

//}




//gpio 引脚初始化
void Gpios_init(void)
{
//	Wxen_Control_Init();   //2022-09-15 不初始化反而能正常使用
	if(Get_Lcd_Type())  //为非0时表示7寸屏
		lcd_pwm_init(70);
	
	MicCtl_Control_Init();   //MIC_CRL 引脚的初始化
	lcd_reset_control_init();  //lcd 复位引脚的初始化
	OePins_Control_Init();    //OE引脚的初始化

	
	if(Get_Lcd_Type())  //为非0时表示7寸屏
		LcdCtrl_Control_Init();   //7寸屏电源控制引脚  		

//	LcdCtrl_Enable();   //7寸屏lcd电源通电
//	Enable_LcdLight();    //对7寸屏的控制信号，背光使能和背光pwm控制,	

//	LcdCtrl_Control_Init();	    //
	//Lcd7INCtrl_PwmPins_Init();	
	
	LSPK_Control_Init();      //LSPK 继电器控制  PA7
	V12_CTL_Control_Init();   //MORSE  12V 输出控制
	LcdType_Control_Init();   //屏幕类型获取引脚初始化
	
	SHTDB_5IN_Control_Init();  //5寸屏的背光使能,2022-12-13
	
	//音频控制，低电平有效，高电平禁止，2022-12-13，可以不使用
	SPKEN_Control_Init();
	EAR_L_EN_Control_Init();
	EAR_R_EN_Control_Init();
	
}





//PD6  MicCtl 输出控制(参数status 非0输出高，0输出低)
void MicCtl_Control_SetOutVal(uint8_t status)
{
	//debug_printf_string("MicCtl_Control_OutHigh\r\n");
	MY_PRINTF("MicCtl_Control_SetOutVal status = %d\r\n",status);
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

//PC5  5寸背光使能？
//2022-12-13 增加
void SHTDB_5IN_Control_Init(void)
{
	//1. 时钟使能
	rcu_periph_clock_enable(RCU_GPIOC);
		
	//2. 初始化后，默认输出高
	gpio_bit_reset(GPIOC, GPIO_PIN_5);	
	
	//3 上电控制引脚
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_5);  //控制输出	
}

//高电平点亮 5inch lcd
void SHTDB_5IN_Enable(void)
{	
	gpio_bit_set(GPIOC, GPIO_PIN_5);		
}

//低电平熄灭 5inch lcd
void SHTDB_5IN_Disable(void)
{	
	gpio_bit_reset(GPIOC, GPIO_PIN_5);		
}


////PC5  SHTDB_5IN 输出控制(参数status 非0输出高电平点亮5inch，0输出低熄灭 5inch lcd)
void SHTDB_5IN_Control_SetOutVal(uint8_t status)
{
	MY_PRINTF("SHTDB_5IN_Control_SetOutVal status = %d\r\n",status);
	if(status)
		SHTDB_5IN_Enable();
	else
		SHTDB_5IN_Disable();
}

//翻转
void SHTDB_5IN_Control_ToggleOut(void)
{
	uint8_t status = gpio_output_bit_get(GPIOC, GPIO_PIN_5);
	SHTDB_5IN_Control_SetOutVal(!status);
	
//	MY_PRINTF("V12_CTL_Control(MORSE) output status = %d\r\n",!status);
}




//PC6  V12_CTL
void V12_CTL_Control_Init(void)
{
	//1. 时钟使能
	rcu_periph_clock_enable(RCU_GPIOC);
		
	//2. 初始化后，默认输出低，2022-12-19改的。
	gpio_bit_reset(GPIOC, GPIO_PIN_6);	
	
	//3 上电控制引脚
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_6);  //控制输出		
}


void V12_CTL_Enable(void)
{	
	//1. 初始化后，默认输出低，继电器吸合
	gpio_bit_set(GPIOC, GPIO_PIN_6);		
}

void V12_CTL_Disable(void)
{	
	//1. 初始化后，默认输出高
	gpio_bit_reset(GPIOC, GPIO_PIN_6);		
}


////PC6  V12_CTL 输出控制(参数status 非0输出高，0输出低)
void V12_CTL_Control_SetOutVal(uint8_t status)
{
	MY_PRINTF("V12_CTL_Control(MORSE)_OutHigh status = %d\r\n",status);
	if(status)
		gpio_bit_set(GPIOC, GPIO_PIN_6);
	else
		gpio_bit_reset(GPIOC, GPIO_PIN_6);
}

//翻转
void V12_CTL_Control_ToggleOut(void)
{
	uint8_t status = gpio_output_bit_get(GPIOC, GPIO_PIN_6);
	V12_CTL_Control_SetOutVal(!status);
	
//	MY_PRINTF("V12_CTL_Control(MORSE) output status = %d\r\n",!status);
}



//PC8,9,10,输入引脚
void LcdType_Control_Init(void)
{
	//1. 时钟使能
	rcu_periph_clock_enable(RCU_GPIOC);		
	//2.0 输入模式
	gpio_init(GPIOC, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 );  //控制输出	

}


//获取键盘类型
/*返回值
1：防爆终端键盘类型
2：壁挂Ⅲ型终端键盘类型（不关心！！）
4：嵌入式Ⅰ/Ⅱ/Ⅲ型、防风雨型、壁挂Ⅱ型终端键盘类型
6：多功能型终端键盘类型(只有这个是7寸屏，其他都是5寸)
*/
uint8_t get_LcdType_val(void)
{
	uint8_t val = (gpio_input_port_get(GPIOC) >> 8) & 0x7;  //只要PC8，9，10
//	MY_PRINTF("get_LcdType_val status = %d\r\n",val);
	return val;   //只要PC8，9，10
}


//获得lcd屏幕的类型，2022-09-21之后新做的底板，通过与按键板的三个引脚相连
//读取引脚的电平，判断lcd的类型，之前是由3399的引脚判断的。
//返回值0表示5寸屏，非0表示7寸屏
//2022-09-21 目前还没有新的底板用于判断，默认返回0（表示5寸屏）
uint8_t Get_Lcd_Type(void)
{
	//get_LcdType_val() 返回6 表示是多功能面板，是7寸屏，其他均为5寸，2022-10-10
	return (get_LcdType_val() == 6);   //2022-09-21 目前还没有新的底板用于判断，默认返回0（表示5寸屏）
}


#if 1
//PA7  LSPK_CRL
//2022-12-13  该PD5
void LSPK_Control_Init(void)
{
	//1. 时钟使能
	rcu_periph_clock_enable(RCU_GPIOD);
		
	//2. 初始化后，默认输出高
	gpio_bit_reset(GPIOD, GPIO_PIN_5);	
	
	//3 上电控制引脚
	gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_5);  //控制输出		
}


void LSPK_Enable(void)
{	
	//1. 初始化后，默认输出低，继电器吸合
	gpio_bit_set(GPIOD, GPIO_PIN_5);		
}

void LSPK_Disable(void)
{	
	//1. 初始化后，默认输出高
	gpio_bit_reset(GPIOD, GPIO_PIN_5);		
}


////PA7  LSPK 输出控制(参数status 非0输出高，0输出低)
void LSPK_Control_SetOutVal(uint8_t status)
{
	MY_PRINTF("LSPK_Control_SetOutVal status = %d\r\n",status);
	if(status)
		gpio_bit_set(GPIOD, GPIO_PIN_5);
	else
		gpio_bit_reset(GPIOD, GPIO_PIN_5);
}

//翻转
void LSPK_Control_ToggleOut(void)
{
	uint8_t status = gpio_output_bit_get(GPIOD, GPIO_PIN_5);
	LSPK_Control_SetOutVal(!status);
	
//	MY_PRINTF("LSPK_Control output status = %d\r\n",!status);
}
#else
//PA7  LSPK_CRL
void LSPK_Control_Init(void)
{
	//1. 时钟使能
	rcu_periph_clock_enable(RCU_GPIOA);
		
	//2. 初始化后，默认输出高
	gpio_bit_set(GPIOA, GPIO_PIN_7);	
	
	//3 上电控制引脚
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_7);  //控制输出		
}


void LSPK_Enable(void)
{	
	//1. 初始化后，默认输出低，继电器吸合
	gpio_bit_set(GPIOA, GPIO_PIN_7);		
}

void LSPK_Disable(void)
{	
	//1. 初始化后，默认输出高
	gpio_bit_reset(GPIOA, GPIO_PIN_7);		
}


////PA7  LSPK 输出控制(参数status 非0输出高，0输出低)
void LSPK_Control_SetOutVal(uint8_t status)
{
	MY_PRINTF("LSPK_Control_SetOutVal status = %d\r\n",status);
	if(status)
		gpio_bit_set(GPIOA, GPIO_PIN_7);
	else
		gpio_bit_reset(GPIOA, GPIO_PIN_7);
}

//翻转
void LSPK_Control_ToggleOut(void)
{
	uint8_t status = gpio_output_bit_get(GPIOA, GPIO_PIN_7);
	LSPK_Control_SetOutVal(!status);
	
//	MY_PRINTF("LSPK_Control output status = %d\r\n",!status);
}
#endif


#if 1
//PC11  扬声器使能
//2022-12-13 增加
void SPKEN_Control_Init(void)
{
	//1. 时钟使能
	rcu_periph_clock_enable(RCU_GPIOC);
		
	//2. 初始化后，默认输出低，低使能
	gpio_bit_reset(GPIOC, GPIO_PIN_11);	
	
	//3 上电控制引脚
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_11);  //控制输出	
}

//高电平点亮 5inch lcd
void SPKEN_Enable(void)
{	
	gpio_bit_reset(GPIOC, GPIO_PIN_11);		
}

//低电平熄灭 5inch lcd
void SPKEN_Disable(void)
{	
	gpio_bit_set(GPIOC, GPIO_PIN_11);		
}


////PC11  扬声器使能
void SPKEN_Control_SetOutVal(uint8_t status)
{
	MY_PRINTF("SPKEN_Control_SetOutVal status = %d\r\n",status);
	if(status)
		SPKEN_Enable();
	else
		SPKEN_Disable();
}

//翻转
void SPKEN_Control_ToggleOut(void)
{
	uint8_t status = gpio_output_bit_get(GPIOC, GPIO_PIN_11);
	SPKEN_Control_SetOutVal(status);
}



//PC12  左耳机使能
//2022-12-13 增加
void EAR_L_EN_Control_Init(void)
{
	//1. 时钟使能
	rcu_periph_clock_enable(RCU_GPIOC);
		
	//2. 初始化后，默认输出低，低使能
	gpio_bit_reset(GPIOC, GPIO_PIN_12);	
	
	//3 上电控制引脚
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_12);  //控制输出	
}

//高电平点亮 5inch lcd
void EAR_L_EN_Enable(void)
{	
	gpio_bit_reset(GPIOC, GPIO_PIN_12);		
}

//低电平熄灭 5inch lcd
void EAR_L_EN_Disable(void)
{	
	gpio_bit_set(GPIOC, GPIO_PIN_12);		
}


////PC5  SHTDB_5IN 输出控制(参数status 非0输出高电平点亮5inch，0输出低熄灭 5inch lcd)
void EAR_L_EN_Control_SetOutVal(uint8_t status)
{
	MY_PRINTF("EAR_L_EN_Control_SetOutVal status = %d\r\n",status);
	if(status)
		EAR_L_EN_Enable();
	else
		EAR_L_EN_Disable();
}

//翻转
void EAR_L_EN_Control_ToggleOut(void)
{
	uint8_t status = gpio_output_bit_get(GPIOC, GPIO_PIN_12);
	EAR_L_EN_Control_SetOutVal(status);
}


//PC13  右耳机使能
//2022-12-13 增加
void EAR_R_EN_Control_Init(void)
{
	//1. 时钟使能
	rcu_periph_clock_enable(RCU_GPIOC);
		
	//2. 初始化后，默认输出低，低使能
	gpio_bit_reset(GPIOC, GPIO_PIN_13);	
	
	//3 上电控制引脚
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_13);  //控制输出	
}

//高电平点亮 5inch lcd
void EAR_R_EN_Enable(void)
{	
	gpio_bit_reset(GPIOC, GPIO_PIN_13);		
}

//低电平熄灭 5inch lcd
void EAR_R_EN_Disable(void)
{	
	gpio_bit_set(GPIOC, GPIO_PIN_13);		
}


////PC5  SHTDB_5IN 输出控制(参数status 非0输出高电平点亮5inch，0输出低熄灭 5inch lcd)
void EAR_R_EN_Control_SetOutVal(uint8_t status)
{
	MY_PRINTF("EAR_R_EN_Control_SetOutVal status = %d\r\n",status);
	if(status)
		EAR_R_EN_Enable();
	else
		EAR_R_EN_Disable();
}

//翻转
void EAR_R_EN_Control_ToggleOut(void)
{
	uint8_t status = gpio_output_bit_get(GPIOC, GPIO_PIN_13);
	EAR_R_EN_Control_SetOutVal(status);
}

#endif

