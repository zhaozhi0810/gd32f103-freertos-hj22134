

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
static uint32_t leds_status = 0;   //每一位表示一个led的状态，共计32个。1表示亮，0表示灭
#define PWM_HZ 100   //led的pwm为100HZ，，定时器每10ms进入一次，即为100HZ 如果要改频率得改定时器进入的时间


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
	gpio_bit_set(GPIOE, GPIO_PIN_8 | GPIO_PIN_9);
	
	//PE9 是pwm控制，现在输出高，全亮
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
			leds_status = 0xffffffff;	 //全部开启
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
	if(whichled > 40)  //whichled>0 && 
	{
		return 255;		
	}	
	return (leds_status>>whichled) & 1;	
}



//对按键面板上所有led的控制
void key_light_allleds_control(uint8_t status)
{
	key_light_leds_control(40,status);		
}


//设置led的亮度 [0-100]
void set_Led_Pwm(uint8_t pwm)
{
	if(pwm > 100)
		pwm = 100;
	
	g_led_pwm = pwm;
	
	MY_PRINTF("set_Led_Pwm  g_led_pwm = %d\r\n",g_led_pwm);
}



//对按键面板上 led_PWM 引脚的控制
static void led_pwm_pin_control(uint8_t status)
{
	if(status)
		gpio_bit_set(GPIOE, GPIO_PIN_9);   //输出高电平
	else
		gpio_bit_reset(GPIOE, GPIO_PIN_9);  //输出低电平
}



//100HZ的频率，10ms进入一次
void laser_run_pwm_task(void)
{
	static uint16_t count = 0;
//	uint8_t i;
		
	
	if(count <= PWM_HZ)
	{		
		//只在某一点控制引脚拉高拉低
		if(g_led_pwm == count) //计数值count比设定值led_pwm要大，关闭
		{  //
			led_pwm_pin_control(0);   //输出低
		}
		else if(0 == count)  //在0点点亮
			led_pwm_pin_control(1);  //输出高
	}
//	else
//	{
//		count = 0;   //一个周期结束，重新开始下一个周期
//		return;   //刚刚清零就不用去加了
//	}
	count++;
	if(count>PWM_HZ)
		count = 0;
}








