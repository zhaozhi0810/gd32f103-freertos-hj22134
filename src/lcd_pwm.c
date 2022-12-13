

/*
	2021-09-26
	第十一个移植的文件。
	
	7寸LCDPWM，需要使用定时器0CH2N。  GPB15   （GD32定时器从0开始编号，stm32从1开始编号）

	主要是
	1.pwm控制LCD亮度  PB15
	2.背光电源使能引脚 PB14
	//似乎只能控制7寸屏，2022-07-28  对5寸屏无效

*/

#include "includes.h"

	
static uint16_t PWM_DEGREE_MAX = 100;   //PWM频率  1us计个数，4000计算的频率就是1000000/4000=250Hz 太高了影响电磁兼容实验？？？
uint8_t g_lcd_pwm = 100;
static int8_t gs_LcdType = -1;   //0 表示5inch，1表示7inch，-1屏类型未知，2022-12-13
//PB14 timer11 ch0,timer0 ch1_ON
//PB15 timer11 ch1,timer0 ch2_ON

#define LCD_PWM_POWER_VOL_PORT GPIO_PIN_14
#define LCD_PWM_PIN GPIO_PIN_15      //LCD_PWM1  7寸引脚
#define LCD_PWM_PORT GPIOB
#define LCD_PWM_PORT_RCU RCU_GPIOB
#define LCD_PWM_TIMER_RCU  RCU_TIMER0    //20220714 更正为timer0，示波器已经看到波形，串口调节正常
#define LCD_PWM_TIMER  TIMER0
#define LCD_PWM_TIMER_CH TIMER_CH_2

#define LCD_PWM_TIMER_USE_CHN   //使用互补通道？

//uint8_t g_Lcd_Power_status = 0;   //未加点
//uint8_t g_Lcd_Pd_N_status = 0;
//static uint8_t g_lcd_pwm_poweroff_save=0;


void lcd_pwm_init(uint8_t degree)
{
	rcu_periph_clock_enable(LCD_PWM_PORT_RCU);   //端口的时钟使能
#ifndef LCD_PWM	
	//uint32_t pin;
	//rcu_periph_clock_enable(RCU_GPIOB);
	//2. 设置为输出模式	
	//pin = BIT(GPIO_PIN_0);    //PB0和PB1 同时设置为输出
	
	gpio_init(LCD_PWM_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, LCD_PWM_PIN);
	
	if(degree > 0)
	{
		gpio_bit_set(LCD_PWM_PORT, LCD_PWM_PIN);
	}
	else
	{
		gpio_bit_reset(LCD_PWM_PORT, LCD_PWM_PIN);
	}
	
#else		
	//PB15 通道
	timer_parameter_struct initpara;
	timer_oc_parameter_struct ocpara;
	//1. io引脚设置复用功能	
	gpio_init(LCD_PWM_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_2MHZ, LCD_PWM_PIN);   //复用功能	
	//PB15 默认映射tim0 ch2的N通道（从0开始数）
	//gpio_pin_remap_config(GPIO_TIMER2_PARTIAL_REMAP, ENABLE);    //部分映射
	
	//2. 定时器时钟使能
	rcu_periph_clock_enable(LCD_PWM_TIMER_RCU);  //定时器模块时钟使能
		
	if(degree > 100)
	{
		degree = 100;
	}
	g_lcd_pwm = degree;   //保存到全局变量中。
	
	//3. 初始化定时器的数据结构  /* initialize TIMER init parameter struct */
	timer_struct_para_init(&initpara);
	initpara.period = PWM_DEGREE_MAX-1;  //重载的数字，频率20kHZ
	initpara.prescaler = (SystemCoreClock/1000000)-1;  //预分频数，得到是1Mhz的脉冲  
		
	//4. 初始化定时器      /* initialize TIMER counter */
	timer_init(LCD_PWM_TIMER, &initpara);
		
	//5. 初始化定时器通道的数据结构 /* initialize TIMER channel output parameter struct */
	timer_channel_output_struct_para_init(&ocpara);	
#ifndef LCD_PWM_TIMER_USE_CHN
	ocpara.outputstate  = TIMER_CCX_ENABLE;  //输出通道使能	
#else
    ocpara.outputnstate = TIMER_CCXN_ENABLE;//使能互补通道输出
#endif
	//6. 初始化定时器通道   /* configure TIMER channel output function */
	timer_channel_output_config(LCD_PWM_TIMER, LCD_PWM_TIMER_CH, &ocpara);
			
	//7. 初始化定时器通道输出方式设置   /* configure TIMER channel output compare mode */
	timer_channel_output_mode_config(LCD_PWM_TIMER, LCD_PWM_TIMER_CH, TIMER_OC_MODE_PWM1);
	/* configure TIMER channel output pulse value */
	
	//8. 初始化定时器通道输出脉冲宽带
	timer_channel_output_pulse_value_config(LCD_PWM_TIMER, LCD_PWM_TIMER_CH, (100-degree) * PWM_DEGREE_MAX/100);
//	timer_channel_complementary_output_state_config
//timer_channel_complementary_output_polarity_config
	//9. 初始化定时器通道输出使能
	//timer_channel_output_fast_config(TIMER2, TIMER_CH_0, TIMER_OC_FAST_ENABLE);
	timer_channel_output_shadow_config(LCD_PWM_TIMER, LCD_PWM_TIMER_CH, TIMER_OC_SHADOW_DISABLE);	  //stm32似乎用的是这个0x8
	//10.初始化，定时器不使能 2022-04-18	
	
	
	/* enable a TIMER */
	timer_primary_output_config(TIMER0, ENABLE);
	timer_auto_reload_shadow_enable(TIMER0);
//	timer_enable(TIMER0);
#endif

	//*******以下为背光使能控制。	
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, LCD_PWM_POWER_VOL_PORT);
	gpio_bit_reset(GPIOB, LCD_PWM_POWER_VOL_PORT);   //初始化后，输出高电平，默认点亮背光

	//禁止背光显示，通电前一段时间，屏幕显示条纹。
	//Disable_LcdLight();
}



//开启LCD电源
void Enable_Lcd_Power(void)
{
	LcdCtrl_Enable();  //pe15
	gpio_bit_set(GPIOB, LCD_PWM_POWER_VOL_PORT);  //PB14
}

//关闭LCD电源
void Disable_Lcd_Power(void)
{
	LcdCtrl_Disable();  //pe15
	gpio_bit_reset(GPIOB, LCD_PWM_POWER_VOL_PORT);   //PB14
}


uint8_t Get_Lcd_Power_Status(void)
{
	return gpio_output_bit_get(GPIOB, LCD_PWM_POWER_VOL_PORT);	
}


////开启ttl转换使能
//void Enable_Lcd_PdN(void)
//{
//	gpio_bit_set(GPIOC, GPIO_PIN_14);
//}

////关闭ttl转换使能
//void Disable_Lcd_PdN(void)
//{
//	gpio_bit_reset(GPIOC, GPIO_PIN_14);
//}

//uint8_t Get_Lcd_PdN_Status(void)
//{
//	return gpio_output_bit_get(GPIOC, GPIO_PIN_14);	
//}




//开启背光（包括电源，ttl转换，背光，定时器全部开启）
void Enable_LcdLight(void)
{	
	if(gs_LcdType < 0)
	{
		gs_LcdType = Get_Lcd_Type(); //获得屏幕类型1 是7寸，0是5寸
	}
	
	if(gs_LcdType == 0)  //5寸的控制
	{
		SHTDB_5IN_Control_SetOutVal(1);  //背光打开
		debug_printf_string("Enable_LcdLight 5inch\r\n");
	}	
	else  //7寸的控制
	{
	//	Enable_Lcd_PdN();     //lvds 转换功能开启	
		Enable_Lcd_Power();   //lcd电源开启
		vTaskDelay(100);
	#ifndef LCD_PWM
		gpio_bit_set(LCD_PWM_PORT, LCD_PWM_PIN);  //拉高PWM引脚
	#else	
		Lcd_pwm_out(g_lcd_pwm==0?70:g_lcd_pwm);   //设置占空比70
		vTaskDelay(100);
		timer_enable(LCD_PWM_TIMER);   //开启定时器
	#endif		
		vTaskDelay(100);
		//gpio_bit_set(GPIOB, GPIO_PIN_14);   //开启背光使能
		Enable_Lcd_Power();
		debug_printf_string("Enable_LcdLight\r\n");
	}
}


//关闭背光（包括电源，ttl转换，背光，定时器全部关闭）
void Disable_LcdLight(void)
{
	if(gs_LcdType < 0)
	{
		gs_LcdType = Get_Lcd_Type(); //获得屏幕类型1 是7寸，0是5寸
	}
	
	if(gs_LcdType == 0)  //5寸的控制
	{
		SHTDB_5IN_Control_SetOutVal(0);  //背光关闭
		debug_printf_string("Disable_LcdLight 5inch\r\n");
	}	
	else  //7寸的控制
	{	
		
	//	gpio_bit_reset(GPIOB, GPIO_PIN_14);  //关闭背光使能
	#ifndef LCD_PWM
		gpio_bit_reset(LCD_PWM_PORT, LCD_PWM_PIN);
	#else
		timer_channel_output_pulse_value_config(LCD_PWM_TIMER, LCD_PWM_TIMER_CH, PWM_DEGREE_MAX);   //关闭背光pwm
		vTaskDelay(100);
		timer_disable(LCD_PWM_TIMER);  //关闭定时器
	#endif	
		
	//	Disable_Lcd_PdN();     //lvds 转换功能关闭
		Disable_Lcd_Power();   //lcd电源关闭
			
		debug_printf_string("Disable_LcdLight\r\n");
	}
}





/*
设置lcd亮度占空比
//degree 修改为可正可负
*/
void Lcd_pwm_out(int8_t degree)
{
#ifdef LCD_PWM
	uint32_t value;

//	if(PWM_DEGREE_MIN > degree) degree = PWM_DEGREE_MIN;
//	if(PWM_DEGREE_MAX < degree) degree = PWM_DEGREE_MAX;
	
	if(degree > 100)	
		degree = 100;	
	else if(degree < 0)
		degree = 0;
		
	g_lcd_pwm = degree; 
	MY_PRINTF("Lcd_pwm_out g_lcd_pwm = %d\r\n",g_lcd_pwm);   //打印一般信息，可以关闭
	
	value = (100-degree) * PWM_DEGREE_MAX / 100;
	/* CH configuration in PWM mode1,duty cycle  */
	//timer_channel_output_pulse_value_config(TIMER1,TIMER_CH_0,value);
	//TIM_SetCompare4(TIM3, value);
	timer_channel_output_pulse_value_config(LCD_PWM_TIMER, LCD_PWM_TIMER_CH, value);
#else
	if(degree > 0)
	{
		g_lcd_pwm = 10;   //10比较好处理
		gpio_bit_set(LCD_PWM_PORT, LCD_PWM_PIN);
	}
	else
	{
		g_lcd_pwm = 0;
		gpio_bit_reset(LCD_PWM_PORT, LCD_PWM_PIN);
	}
#endif	
}


/*
调整lcd亮度占空比
//val 为需要调整的值，可正负，正表示亮度增加，负表示亮度减小
*/
void Lcd_pwm_change(int8_t val)
{
	int8_t degree ;
		
	if((val > 0) && (g_lcd_pwm<100))  //大于0就是增加亮度，而且亮度值也小于100
		degree = val + g_lcd_pwm;    //这里不做溢出判断了
	else if((val < 0) && (g_lcd_pwm > 0))
		degree = val + g_lcd_pwm;   //这里不做溢出判断了
	else
		return;   //保持不变
	
	Lcd_pwm_out(degree);	
}




/*
	设置lcdPWM的频率
*/
void setLcd_pwm_freq(uint16_t freq)
{
#ifdef LCD_PWM
	if(freq > 20)
	{
		PWM_DEGREE_MAX = freq;
		//TIM_SetAutoreload(TIM3,freq);   
		timer_autoreload_value_config(LCD_PWM_TIMER, freq);  //设置频率
		
		MY_PRINTF("setLcd_pwm_freq freq = %d\r\n",freq);
	}
#endif	
}
