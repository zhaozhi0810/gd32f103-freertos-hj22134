

/*
	按键灯的操作
	使用local bus的方式
	
	地址  A0~A5
	数据  D0
	CS    CS
7. PD11- 3V3_A0	
13.PE2 - 3V3_A1
14.PE3 - 3V3_A2
15.PE4 - 3V3_A3
16.PE5 - 3V3_A4
17.PE6 - 3V3_A5

9.PD14- 3V3_D0

19.PE8 - KEYBOARD_CS	
	
	状态灯，三色灯（这个由3399控制，不由单片机控制）
	每个引脚控制一个颜色，共阳


灯亮度控制，根据原理图，这个引脚与lcd的pwm冲突，无法同时(复用)使用定时器
20.PE9-PWM_LED （TIM0-CH0）//可以用io口模拟吧


2022-09-14 freertos移植后不支持调节亮度！！！！


*/

#include "includes.h"


#define ENABLE_KEYBOARD_CS 1
#define DISABLE_KEYBOARD_CS 0
#define LEDS_NUM_MAX 40   //最大值40，键灯值1-40有效 

//static uint8_t g_led_pwm = 100;
static uint64_t leds_status = 0;   //每一位表示一个led的状态，共计45个。1表示亮，0表示灭
#define LEDS_PWM_HZ 20   //20表示led的pwm为50HZ，定时器每1ms进入一次

#ifdef LEDS_FLASH_TASK
TaskHandle_t  TaskHandle_leds_Flash;   //存放led_闪烁任务指针
#endif

static uint8_t g_leds_flash_time[LEDS_NUM_MAX] = {0};  //led 需要闪烁的时间值，0表示500ms，1表示800ms，2表示1s，3表示2s，其他表示不闪烁
static uint8_t g_leds_flash_time_already[4] = {0};   //led已经闪烁的时间,只记录某种闪烁的时间，不是某一个led
static uint8_t g_leds_flash_action=0;      // 0-3位，分别表示5，8，10，20的亮灭情况，1表示亮，0表示灭
//static uint8_t g_leds_flash_control = 0;	//0表示没有灯需要闪烁了，非0表示还有灯需要闪烁

//2022-09-16 改由timer1 产生中断信号，来修改pwm的输出值
//只是用于计时，就使用tim1了，
//static void TIM1_Led_Pwm_Init(uint16_t arr,uint16_t psc);
void Kleds_pwm_init(uint8_t degree);

//2022-12-12 改由timer1 产生中断信号，来控制键灯闪烁
static void TIM1_Led_Pwm_Init(uint16_t arr,uint16_t psc);


void key_light_leds_init(void)
{
	uint32_t pin;
	uint8_t i;
	//都是输出引脚
	//1. 时钟使能
	rcu_periph_clock_enable(RCU_GPIOD);
		
	//2.0 上电控制引脚  (D11  -- A0   D14  -- D0)
	gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_11 | GPIO_PIN_14);  //控制输出	
	//2. 初始化后，默认输出低
	gpio_bit_reset(GPIOD, GPIO_PIN_11 | GPIO_PIN_14);
	
	
	rcu_periph_clock_enable(RCU_GPIOE);
	pin = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;	
	//2.0 上电控制引脚
	gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, pin);  //控制输出	
	//2. 初始化后，默认输出低
	gpio_bit_reset(GPIOE, pin);
	
	
//	rcu_periph_clock_enable(RCU_GPIOE);	
	//2.0 上电控制引脚
	gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_8 | GPIO_PIN_9);  //控制输出	
	//2. 初始化后，默认输出高
	gpio_bit_set(GPIOE, GPIO_PIN_8 | GPIO_PIN_9);   //PE9 是pwm引脚，需要设置为高！！！2022-09-16	
	
	Kleds_pwm_init(100);   //键灯pwm值为100
	
	//用于led键灯闪烁控制，2022-12-12
	TIM1_Led_Pwm_Init(1000-1,(SystemCoreClock/10000)-1);    //100ms定时初始化

	for(i=0;i<LEDS_NUM_MAX;i++)
	{
		g_leds_flash_time[i] = 5;
	}
}


/*
	status   0 表示禁止，输出高电平
			 非0表示选中，输出低电平
*/
static void key_light_cs(uint8_t status)
{
	if(status)
		gpio_bit_set(GPIOE, GPIO_PIN_8);   //status = 1,表示数据输出
	else
		gpio_bit_reset(GPIOE, GPIO_PIN_8);   //status = 0,表示数据保持，对应cs引脚为0
}



//addr 1-40 对应40个灯。
static void key_light_send_addr(uint8_t addr)
{
#if 1
	uint16_t val;
	val = gpio_output_port_get(GPIOE);
		
	val &= ~(0x1f<<2);   //清零
	val |= ((addr&0x3e)<<1); 
	
	gpio_port_write(GPIOE, val);
#else	
	if(addr&2)
		gpio_bit_set(GPIOE, GPIO_PIN_2);   //A1
	else
		gpio_bit_reset(GPIOE, GPIO_PIN_2);
	
	if(addr&4)
		gpio_bit_set(GPIOE, GPIO_PIN_3);   //A2
	else
		gpio_bit_reset(GPIOE, GPIO_PIN_3);
	
	if(addr&8)
		gpio_bit_set(GPIOE, GPIO_PIN_4);   //A3
	else
		gpio_bit_reset(GPIOE, GPIO_PIN_4);
	
	if(addr&0x10)
		gpio_bit_set(GPIOE, GPIO_PIN_5);   //A4
	else
		gpio_bit_reset(GPIOE, GPIO_PIN_5);
	
	if(addr&0x20)
		gpio_bit_set(GPIOE, GPIO_PIN_6);   //A5
	else
		gpio_bit_reset(GPIOE, GPIO_PIN_6);
#endif
	if(addr&1)
		gpio_bit_set(GPIOD, GPIO_PIN_11);   //A0
	else
		gpio_bit_reset(GPIOD, GPIO_PIN_11);

}

//这个led控制主要是本文件内部使用，不对外提供控制接口
//这个接口用于闪烁和常亮灭控制，不会影响闪烁控制
//whichled: 0-39 ,其中39表示所有led
//status : 0表示熄灭，非零表示点亮
static void key_light_leds_control2(uint8_t whichled,uint8_t status)
{		
	if(whichled < LEDS_NUM_MAX)  //whichled>0 && 
	{
		key_light_send_addr(whichled+1);		
	}
	else
		return;
		
	if(status)	
	{
		gpio_bit_set(GPIOD, GPIO_PIN_14);
		
		if(whichled == 39)  //记录led的状态
			leds_status = ~0ULL;	 //全部开启
		else
			leds_status |= 1<<whichled;
	}
	else
	{
		gpio_bit_reset(GPIOD, GPIO_PIN_14);	
		
		if(whichled == 39)  //记录led的状态
			leds_status = 0;	 //全部开启
		else
			leds_status &= ~(1<<whichled);
	}
	
//	vTaskDelay(pdMS_TO_TICKS(1));
//	Delay1ms(1);
	
	//经过测试，感觉这个使能是一个边沿操作，不管是上升沿还是下降沿，都可以引起一次更新。，2022-12-20
	key_light_cs(ENABLE_KEYBOARD_CS);   //1，使能输出
	Delay1us(30);
	key_light_cs(DISABLE_KEYBOARD_CS);  //0，保持输出
	
	
}

/*
//这个led控制 对外提供控制接口，用于设置灯的亮和灭，同时会停止灯的闪烁控制。
	whichled  32表示所有灯
			1-40 分别对应按键的灯
			 
	status   0 表示熄灭
			 非0表示点亮
*/
void key_light_leds_control(uint8_t whichled,uint8_t status)
{		
	uint8_t i;

	if((whichled < 1) || (whichled > LEDS_NUM_MAX))  //whichled>0 && 	
		return;
	
	whichled -= 1;   //调整为0-39  !!!!!!!!

#ifdef LEDS_FLASH_TASK   //这一段主要是取消灯的闪烁控制
	if(whichled == 39)
	{
		timer_disable(TIMER1);
		for(i=0;i<LEDS_NUM_MAX;i++)
			g_leds_flash_time[i] = 5;  //全部取消闪烁
	//	g_leds_flash_control = 0;  //全部不需要闪烁了。
		
	}
	else
		g_leds_flash_time[whichled] = 5;  //0-3表示设置闪烁 
#endif	
	key_light_leds_control2(whichled,status);
}






//获得某一个灯的状态，whichled ： 1 - 40
//返回255表示错误，0，1表示正确
uint8_t get_led_status(uint8_t whichled)
{
	if(whichled > LEDS_NUM_MAX)  //whichled>0 && 
	{
		return 255;		
	}	
	whichled -= 1;   //调整为0-39  !!!!!!!!
	return (leds_status>>whichled) & 1;	
}



//对按键面板上所有led的控制
void key_light_allleds_control(uint8_t status)
{
	key_light_leds_control(40,status);	  //全部用40表示	
}







//-----------------------------------------------------------------------------------
//led的pwm控制

// 1us计个数，4000计算的频率就是1000000/4000=250Hz 太高了影响电磁兼容实验？？？
static uint16_t KLEDS_PWM_HZ = 4000;   //led的pwm为250HZ，
static uint8_t g_kleds_pwm = 100;

//timer2 ch2

#define KLEDS_PWM_PIN GPIO_PIN_0
#define KLEDS_PWM_PORT GPIOB
#define KLEDS_PWM_PORT_RCU RCU_GPIOB
#define KLEDS_PWM_TIMER_RCU  RCU_TIMER2    //20221010 timer2
#define KLEDS_PWM_TIMER  TIMER2
#define KLEDS_PWM_TIMER_CH TIMER_CH_2

//#define KLEDS_PWM_TIMER_USE_CHN   //使用互补通道？


//键灯的pwm的初始化，degree表示占空比，0-100
void Kleds_pwm_init(uint8_t degree)
{
	rcu_periph_clock_enable(KLEDS_PWM_PORT_RCU);   //端口的时钟使能
#ifndef KLEDS_PWM	
	//uint32_t pin;
	//rcu_periph_clock_enable(RCU_GPIOB);
	//2. 设置为输出模式	
	//pin = BIT(GPIO_PIN_0);    //PB0和PB1 同时设置为输出
	
	gpio_init(KLEDS_PWM_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, KLEDS_PWM_PIN);
	
	if(degree > 0)
	{
		gpio_bit_set(KLEDS_PWM_PORT, KLEDS_PWM_PIN);
	}
	else
	{
		gpio_bit_reset(KLEDS_PWM_PORT, KLEDS_PWM_PIN);
	}
	
#else		
	//PB15 通道
	timer_parameter_struct initpara;
	timer_oc_parameter_struct ocpara;
	//1. io引脚设置复用功能	
	gpio_init(KLEDS_PWM_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_2MHZ, KLEDS_PWM_PIN);   //复用功能	
	//PB15 默认映射tim0 ch2的N通道（从0开始数）
	//gpio_pin_remap_config(GPIO_TIMER2_PARTIAL_REMAP, ENABLE);    //部分映射
	
	//2. 定时器时钟使能
	rcu_periph_clock_enable(KLEDS_PWM_TIMER_RCU);  //定时器模块时钟使能
		
	if(degree > 100)
	{
		degree = 100;
	}
	g_kleds_pwm = degree;   //保存到全局变量中。
	
	//3. 初始化定时器的数据结构  /* initialize TIMER init parameter struct */
	timer_struct_para_init(&initpara);
	initpara.period = KLEDS_PWM_HZ-1;  //重载的数字，频率20kHZ
	initpara.prescaler = (SystemCoreClock/1000000)-1;  //预分频数，得到是1Mhz的脉冲  
		
	//4. 初始化定时器      /* initialize TIMER counter */
	timer_init(KLEDS_PWM_TIMER, &initpara);
		
	//5. 初始化定时器通道的数据结构 /* initialize TIMER channel output parameter struct */
	timer_channel_output_struct_para_init(&ocpara);	
#ifndef KLEDS_PWM_TIMER_USE_CHN
	ocpara.outputstate  = TIMER_CCX_ENABLE;  //输出通道使能	
#else
    ocpara.outputnstate = TIMER_CCXN_ENABLE;//使能互补通道输出
#endif
	//6. 初始化定时器通道   /* configure TIMER channel output function */
	timer_channel_output_config(KLEDS_PWM_TIMER, KLEDS_PWM_TIMER_CH, &ocpara);
			
	//7. 初始化定时器通道输出方式设置   /* configure TIMER channel output compare mode */
	timer_channel_output_mode_config(KLEDS_PWM_TIMER, KLEDS_PWM_TIMER_CH, TIMER_OC_MODE_PWM1);
	/* configure TIMER channel output pulse value */
	
	//8. 初始化定时器通道输出脉冲宽带
	timer_channel_output_pulse_value_config(KLEDS_PWM_TIMER, KLEDS_PWM_TIMER_CH, (100-degree) * KLEDS_PWM_HZ/100);
//	timer_channel_complementary_output_state_config
//timer_channel_complementary_output_polarity_config
	//9. 初始化定时器通道输出使能
	//timer_channel_output_fast_config(TIMER2, TIMER_CH_0, TIMER_OC_FAST_ENABLE);
	timer_channel_output_shadow_config(KLEDS_PWM_TIMER, KLEDS_PWM_TIMER_CH, TIMER_OC_SHADOW_DISABLE);	  //stm32似乎用的是这个0x8
	//10.初始化，定时器不使能 2022-04-18	
		
	/* enable a TIMER */
	timer_primary_output_config(KLEDS_PWM_TIMER, ENABLE);
	timer_auto_reload_shadow_enable(KLEDS_PWM_TIMER);
	timer_enable(KLEDS_PWM_TIMER);
#endif

	//*******以下为背光使能控制。	
//	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_14);
//	gpio_bit_reset(GPIOB, GPIO_PIN_14);   //初始化后，输出高电平，默认点亮背光


}





/*
设置lcd亮度占空比
//degree 修改为0-100
*/
void set_Kleds_pwm_out(int8_t degree)
{
#ifdef KLEDS_PWM
	uint32_t value;

//	if(PWM_DEGREE_MIN > degree) degree = PWM_DEGREE_MIN;
//	if(PWM_DEGREE_MAX < degree) degree = PWM_DEGREE_MAX;
	
	if(degree > 100)	
		degree = 100;	
	else if(degree < 0)
		degree = 0;
		
	g_kleds_pwm = degree; 
	MY_PRINTF("Kleds_pwm_out g_kleds_pwm = %d\r\n",g_kleds_pwm);   //打印一般信息，可以关闭
	
	value = (100-degree) * KLEDS_PWM_HZ / 100;
	/* CH configuration in PWM mode1,duty cycle  */
	//timer_channel_output_pulse_value_config(TIMER1,TIMER_CH_0,value);
	//TIM_SetCompare4(TIM3, value);
	timer_channel_output_pulse_value_config(KLEDS_PWM_TIMER, KLEDS_PWM_TIMER_CH, value);
#else
	if(degree > 0)
	{
		g_kleds_pwm = 10;   //10比较好处理
		gpio_bit_set(KLEDS_PWM_PORT, KLEDS_PWM_PIN);
	}
	else
	{
		g_kleds_pwm = 0;
		gpio_bit_reset(KLEDS_PWM_PORT, KLEDS_PWM_PIN);
	}
#endif	
}


/*
调整lcd亮度占空比
//val 为需要调整的值，可正负，正表示亮度增加，负表示亮度减小
*/
void Kleds_pwm_change(int8_t val)
{
	int8_t degree ;
		
	if((val > 0) && (g_kleds_pwm<100))  //大于0就是增加亮度，而且亮度值也小于100
		degree = val + g_kleds_pwm;    //这里不做溢出判断了
	else if((val < 0) && (g_kleds_pwm > 0))
		degree = val + g_kleds_pwm;   //这里不做溢出判断了
	else
		return;   //保持不变
	
	set_Kleds_pwm_out(degree);	
}

void kLedPWM_ToggleOut(void)
{
	static int8_t dir = 1; 
//	uint8_t data = 10;
	
	if(g_kleds_pwm >= 100)  //减小方向
		dir = -1;
	else if(g_kleds_pwm ==0)  //增大方向
		dir = 1;
	
	Kleds_pwm_change(10*dir);	

}


/*
	设置lcdPWM的频率
*/
void setKleds_pwm_freq(uint16_t freq)
{
#ifdef KLEDS_PWM
	if(freq > 20)
	{
		KLEDS_PWM_HZ = freq;
		//TIM_SetAutoreload(TIM3,freq);   
		timer_autoreload_value_config(KLEDS_PWM_TIMER, freq);  //设置频率
		
		MY_PRINTF("setKleds_pwm_freq freq = %d\r\n",freq);
	}
#endif	
}






/*********************************************************************************/


#ifdef LEDS_FLASH_TASK

static const uint8_t g_const_led_flash_time[4] = {5,8,10,20};

//增加某个led灯闪烁
/*
	whichled 高两位表示闪烁的速率
			 低6位表示哪个灯闪烁，[1-40]  ,实际有效值为0-39，函数中间做调整
*/
void light_leds_add_flash(uint8_t whichled)
{
//	printf("light_leds_add_flash whichled = %d\r\n",whichled);
	uint8_t flash_freq = whichled >> 6;  
	uint8_t i;
	whichled = (whichled & 0x3f) - 1;   //低六位表示某个灯，原来的取值是1-40，调整为0-39
		
	if(whichled < LEDS_NUM_MAX)
	{	
		timer_enable(TIMER1);	//开启闪烁定时器	
		
		if(whichled == 39)  //40-1表示全部的灯
		{		
			for(i=0;i<LEDS_NUM_MAX;i++)
			{
				//led 需要闪烁的时间值，flash_freq:0表示500ms，1表示800ms，2表示1s，3表示2s，其他表示不闪烁
				g_leds_flash_time[i] = flash_freq;//g_const_led_flash_time[flash_freq];
			}
			
		}
		else{	
			//led 需要闪烁的时间值，flash_freq:0表示500ms，1表示800ms，2表示1s，3表示2s，其他表示不闪烁			
			g_leds_flash_time[whichled] = flash_freq;//g_const_led_flash_time[flash_freq];
		}
	}
}
#endif





//100HZ的频率，10ms进入一次
//void leds_run_pwm_task(void)
//{
//	static uint16_t count = 0;
////	uint8_t i;
//		
//	
//	if(count <= PWM_HZ)
//	{		
//		//只在某一点控制引脚拉高拉低
//		if(g_led_pwm == count) //计数值count比设定值led_pwm要大，关闭
//		{  //
//			led_pwm_pin_control(0);   //输出低
//		}
//		else if(0 == count)  //在0点点亮
//			led_pwm_pin_control(1);  //输出高
//	}
////	else
////	{
////		count = 0;   //一个周期结束，重新开始下一个周期
////		return;   //刚刚清零就不用去加了
////	}
//	count++;
//	if(count>PWM_HZ)
//		count = 0;
//}






#if 0 //def LEDS_FLASH_TASK



//键灯闪烁任务 //100ms进入一次
//void leds_flash_task(void)
//{
//	uint8_t i;
//	uint8_t c = 0;
//	
//	uint8_t stat[4] = {0};
//	
//	if(!g_leds_flash_control)
//		return;
//	
//	stat[0] = g_leds_flash_action & 1;
//	stat[1] = g_leds_flash_action & 2;   //0 或者非0 
//	stat[2] = g_leds_flash_action & 4;
//	stat[3] = g_leds_flash_action & 8;
//	
//	for(i=0;i<LEDS_NUM_MAX;i++)
//	{
//		if(g_leds_flash_time[i] < 4)  //某个灯需要闪烁
//		{
//			if( g_leds_flash_time_already[g_leds_flash_time[i]] == 0)  //翻转一次
//			{
//				key_light_leds_control2(i,stat[ g_leds_flash_time[i] ]); //点亮 or 熄灭		
//			}
//		}
//		else
//			c ++;   //数一下还有多少灯不需要闪烁
//	}
//	
//	if(c == LEDS_NUM_MAX)
//		g_leds_flash_control = 0;   //没有灯需要闪烁了。
//	
//	for(i=0;i<4;i++)
//	{
//		g_leds_flash_time_already[i]++;
//		if(g_leds_flash_time_already[i] >= g_const_led_flash_time[i])
//		{
//			g_leds_flash_time_already[i] = 0;   //已经闪烁的时间
//			g_leds_flash_action ^= 1<<i;    //状态取反，表示led状态翻转
//		}
//	}
//}


void keyLeds_Flash_task(void* arg)
{
	uint8_t i;
	uint8_t c = 0;	
	uint8_t stat[4] = {0};
	
	while(1)
	{	
		//设置一个条件变量，让任务阻塞
		if(!g_leds_flash_control)
			ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
		//	TaskHandle_leds_Flash
	//		return;
	
		stat[0] = g_leds_flash_action & 1;
		stat[1] = g_leds_flash_action & 2;   //0 或者非0 
		stat[2] = g_leds_flash_action & 4;
		stat[3] = g_leds_flash_action & 8;
		
		for(i=0;i<LEDS_NUM_MAX;i++)
		{
			if(g_leds_flash_time[i] < 4)  //某个灯需要闪烁
			{
				if( g_leds_flash_time_already[g_leds_flash_time[i]] == 0)  //翻转一次
				{
					key_light_leds_control2(i,stat[ g_leds_flash_time[i] ]); //点亮 or 熄灭		
				}
			}
			else
				c ++;   //数一下还有多少灯不需要闪烁
		}
		
		
		
		for(i=0;i<4;i++)
		{
			g_leds_flash_time_already[i]++;
			if(g_leds_flash_time_already[i] >= g_const_led_flash_time[i])
			{
				g_leds_flash_time_already[i] = 0;   //已经闪烁的时间
				g_leds_flash_action ^= 1<<i;    //状态取反，表示led状态翻转
			}
		}
		vTaskDelay(pdMS_TO_TICKS(100));  //延时  pdMS_TO_TICKS(10)
		
		if(c == LEDS_NUM_MAX)
			g_leds_flash_control = 0;   //没有灯需要闪烁了。
	}
}


#endif



#if 1

//2022-12-12 改由timer1 产生中断信号，来控制键灯闪烁
static void TIM1_Led_Pwm_Init(uint16_t arr,uint16_t psc)
{
	timer_parameter_struct initpara;
	//接收部分
	rcu_periph_clock_enable(RCU_TIMER1);  //定时器模块时钟使能

	//3. 初始化定时器的数据结构  /* initialize TIMER init parameter struct */
	timer_struct_para_init(&initpara);
	initpara.period = arr;  //重载的数字，
	initpara.prescaler = psc;  //预分频数，得到是1Mhz的脉
	//4. 初始化定时器      /* initialize TIMER counter */
	timer_init(TIMER1, &initpara);

	timer_disable(TIMER1);
	timer_interrupt_flag_clear(TIMER1,TIMER_INT_FLAG_UP);	
	
	nvic_irq_enable(TIMER1_IRQn, 7U, 0U);  //现在只有抢占优先级了。
	timer_interrupt_enable(TIMER1, TIMER_INT_UP);   //定时中断		
}


//定时器1的中断处理，每进入1次100ms
void TIMER1_IRQHandler(void)
{
//	static uint8_t count = 0;
	uint8_t i;
	uint8_t c = 0;	
	uint8_t stat[4] = {0};
	
	if(timer_interrupt_flag_get(TIMER1,TIMER_INT_FLAG_UP)!=RESET)
	{		
//		count++;  //100ms 已过去			
		
		//设置一个条件变量，让任务阻塞
//		if(!g_leds_flash_control)			
//		{
//			
//			return;
//		}
	
		stat[0] = g_leds_flash_action & 1;
		stat[1] = g_leds_flash_action & 2;   //0 或者非0 
		stat[2] = g_leds_flash_action & 4;
		stat[3] = g_leds_flash_action & 8;
		
		for(i=0;i<LEDS_NUM_MAX;i++)
		{
			if(g_leds_flash_time[i] < 4)  //某个灯需要闪烁
			{
				if( g_leds_flash_time_already[g_leds_flash_time[i]] == 0)  //翻转一次
				{
					key_light_leds_control2(i,stat[ g_leds_flash_time[i] ]); //点亮 or 熄灭		
				}
			}
			else
				c ++;   //数一下还有多少灯不需要闪烁
		}
				
		for(i=0;i<4;i++)
		{
			g_leds_flash_time_already[i]++;
			if(g_leds_flash_time_already[i] >= g_const_led_flash_time[i])
			{
				g_leds_flash_time_already[i] = 0;   //已经闪烁的时间
				g_leds_flash_action ^= 1<<i;    //状态取反，表示led状态翻转
			}
		}
		//vTaskDelay(pdMS_TO_TICKS(100));  //延时  pdMS_TO_TICKS(10)
		
		if(c == LEDS_NUM_MAX)
			timer_disable(TIMER1);  //关闭定时器//g_leds_flash_control = 0;   //没有灯需要闪烁了。

		
	}
	timer_interrupt_flag_clear(TIMER1,TIMER_INT_FLAG_UP);	
}
#endif




#if 0
//定时器开启或者关闭 1为开启，0为关闭
static void Pwm_Timer_Control(uint8_t enable)
{
	if(enable)
	{
		//启动定时器1
		timer_enable(TIMER1);
	}
	else
		timer_disable(TIMER1);
}


//对按键面板上 led_PWM 引脚的控制
static void led_pwm_pin_control(uint8_t status)
{
	if(status)
		gpio_bit_set(GPIOE, GPIO_PIN_9);   //输出高电平
	else
		gpio_bit_reset(GPIOE, GPIO_PIN_9);  //输出低电平
}



//设置led的亮度 [0-100]
void set_Led_Pwm(uint8_t pwm)
{
	if(pwm > 100)
		pwm = 100;
	
	g_led_pwm = pwm/5;  //限制在0-20

	if(pwm == 100) //不用开定时器
	{
		Pwm_Timer_Control(RESET);  //定时器关闭
		led_pwm_pin_control(SET);  //引脚输出高
	}
	else //开启定时器
	{
		Pwm_Timer_Control(SET);  //定时器开启
	}
	
//	MY_PRINTF("set_Led_Pwm  g_led_pwm = %d\r\n",pwm);
}







//2022-09-16 改由timer1 产生中断信号，来修改pwm的输出值
//只是用于计时，就使用tim1了，
static void TIM1_Led_Pwm_Init(uint16_t arr,uint16_t psc)
{
	timer_parameter_struct initpara;
	//接收部分
	rcu_periph_clock_enable(RCU_TIMER1);  //定时器模块时钟使能

	//3. 初始化定时器的数据结构  /* initialize TIMER init parameter struct */
	timer_struct_para_init(&initpara);
	initpara.period = arr;  //重载的数字，
	initpara.prescaler = psc;  //预分频数，得到是1Mhz的脉
	//4. 初始化定时器      /* initialize TIMER counter */
	timer_init(TIMER1, &initpara);

	nvic_irq_enable(TIMER1_IRQn, 7U, 0U);  //现在只有抢占优先级了。
	timer_interrupt_enable(TIMER1, TIMER_INT_UP);   //定时中断	
}


//定时器1的中断处理，目前是pwm
void TIMER1_IRQHandler(void)
{
	static uint8_t count = 0;
	
	if(timer_interrupt_flag_get(TIMER1,TIMER_INT_FLAG_UP)!=RESET)
	{		
		count++;  //1ms 已过去			
		if(count <= LEDS_PWM_HZ)
		{
			//只在某一点控制引脚拉高拉低
			if(g_led_pwm == count) //计数值count比设定值led_pwm要大，关闭
			{  //
				led_pwm_pin_control(0);   //输出低
			}
			else if(1 == count)  //在0点点亮
				led_pwm_pin_control(1);  //输出高
			}
		else 
		{
			count = 0;   //一个周期结束，重新开始下一个周期
		//	return;   //刚刚清零就不用去加了
		}				
	}
	timer_interrupt_flag_clear(TIMER1,TIMER_INT_FLAG_UP);	
}






//100HZ的频率，10ms进入一次
//void laser_run_pwm_task(void)
//{
//	static uint16_t count = 0;
////	uint8_t i;
//		
//	
//	if(count <= PWM_HZ)
//	{		
//		//只在某一点控制引脚拉高拉低
//		if(g_led_pwm == count) //计数值count比设定值led_pwm要大，关闭
//		{  //
//			led_pwm_pin_control(0);   //输出低
//		}
//		else if(0 == count)  //在0点点亮
//			led_pwm_pin_control(1);  //输出高
//	}
////	else
////	{
////		count = 0;   //一个周期结束，重新开始下一个周期
////		return;   //刚刚清零就不用去加了
////	}
//	count++;
//	if(count>PWM_HZ)
//		count = 0;
//}


#endif





