

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


static uint8_t g_led_pwm = 100;
static uint64_t leds_status = 0;   //每一位表示一个led的状态，共计45个。1表示亮，0表示灭
#define LEDS_PWM_HZ 20   //20表示led的pwm为50HZ，定时器每1ms进入一次


//2022-09-16 改由timer1 产生中断信号，来修改pwm的输出值
//只是用于计时，就使用tim1了，
static void TIM1_Led_Pwm_Init(uint16_t arr,uint16_t psc);


void key_light_leds_init(void)
{
	uint32_t pin;
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
	//PE9 是pwm控制，现在输出高，全亮
	
	//用于控制PE9的PWM定时器初始化，并没有开启定时器！！
	TIM1_Led_Pwm_Init(1000-1,(SystemCoreClock/1000000)-1);  //1ms 计时	
}



/*
	status   0 表示禁止，输出高电平
			 非0表示选中，输出低电平
*/
static void key_light_cs(uint8_t status)
{
	if(status)
		gpio_bit_reset(GPIOE, GPIO_PIN_8 );
	else
		gpio_bit_set(GPIOE, GPIO_PIN_8 );
}




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




/*
	whichled  40表示所有灯
			0-39 分别对应按键的灯
			 
	status   0 表示熄灭
			 非0表示点亮
*/
void key_light_leds_control(uint8_t whichled,uint8_t status)
{	
	debug_printf_string("enter key_light_leds_control\r\n");
	if(whichled < 41)  //whichled>0 && 
	{
		key_light_send_addr(whichled);		
	}
	else
	{
		debug_printf_string("key_light_leds_control outof limit\r\n");
		return;
	}
	
	key_light_cs(ENABLE_KEYBOARD_CS);
	if(status)	
	{
		gpio_bit_set(GPIOD, GPIO_PIN_14);
		
		if(whichled == 40)  //记录led的状态
			leds_status = ~0ULL;	 //全部开启
		else
			leds_status |= 1<<whichled;
	}
	else
	{
		gpio_bit_reset(GPIOD, GPIO_PIN_14);	
		
		if(whichled == 40)  //记录led的状态
			leds_status = 0;	 //全部开启
		else
			leds_status &= ~(1<<whichled);
	}
	
	vTaskDelay(1);
	key_light_cs(DISABLE_KEYBOARD_CS);
}


//获得某一个灯的状态
//返回255表示错误，0，1表示正确
uint8_t get_led_status(uint8_t whichled)
{
	if(whichled > 50)  //由之前的40改为50，2022-09-16 
	{
		debug_printf_string("error:get_led_status whichled > 40!!\r\n");
		return 255;		
	}	
	return (leds_status>>whichled) & 1;	
}



//对按键面板上所有led的控制
void key_light_allleds_control(uint8_t status)
{
	key_light_leds_control(40,status);		
}



//-----------------------------------------------------------------------------------
//led的pwm控制

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








