
/*
	外部的看门狗芯片的处理  (外部看门狗芯片未连接)

	WDO --》 PA4  输入，低电平表示超时
	WDI --》 PA5  			输出，喂狗
	RESET --》 PA6  		输出，低电平复位3399
	SYS_RESET_IN --》 PA7  	输出，输出高，3399复位

	APP设置的喂狗周期
	
	喂狗任务根据APP设置的喂狗周期喂狗
	app不再喂狗，则喂狗任务等待时间到之后将结束喂狗
	
	现在是用单片机仿真了一个看门狗给cpu！！！2022-09-14
	

*/
#include "includes.h"



static uint8_t is_hwtd_enable = 0;   //默认看门狗不允许使能

static TaskHandle_t  TaskHandle_Hard_Wtd;  


//喂狗任务根据设置的时间喂狗，
static void hard_wtd_feed_task(void* arg);


//最大25秒 == 最大值255！！！
volatile static uint16_t hwtd_timeout = 220;    //APP设置的喂狗周期， 最小单位100ms
static uint16_t hwtd_timeout_count = 0;  //喂狗计时值。

void hard_wtd_enable(void)
{
	is_hwtd_enable = 1;
	hwtd_timeout_count = hwtd_timeout;
	MY_PRINTF("hard_wtd_enable\r\n");
	
	xTaskCreate(hard_wtd_feed_task,"hwtd",configMINIMAL_STACK_SIZE,NULL,2,&TaskHandle_Hard_Wtd);
}


void hard_wtd_disable(void)
{
	is_hwtd_enable = 0;
	MY_PRINTF("hard_wtd_disable\r\n");
	vTaskDelete(TaskHandle_Hard_Wtd);
}

//固定喂狗，1s喂狗一次
static void hard_wtd_feed_internel(void)
{
	if(gpio_output_bit_get(GPIOA, GPIO_PIN_5))  //原来是高
	{
		gpio_bit_reset(GPIOA, GPIO_PIN_5); //拉低
	}
	else
		gpio_bit_set(GPIOA, GPIO_PIN_5);  //拉高
	
//	MY_PRINTF("hard_wtd_feed_internel\r\n");
}


//cpu喂狗一次，就是把时间重新赋值为最大值
void hard_wtd_feed(void)
{
	hwtd_timeout_count = hwtd_timeout;  //
	
	MY_PRINTF("hard_wtd_feed\r\n");
}


//获得软件看门狗的状态 1表示开启，0表示关闭
uint8_t get_hard_wtd_status(void)
{
	return is_hwtd_enable;
}



//设置看门狗超时时间，单位100ms
void hard_wtd_set_timeout(uint8_t timeout)
{
	MY_PRINTF("hard_wtd_set_timeout timeout = %d(*100ms)\r\n",timeout);
	hwtd_timeout = timeout;
}

//获得看门狗超时时间，单位100ms
uint8_t  hard_wtd_get_timeout(void)
{
	MY_PRINTF("hard_wtd_get_timeout timeout = %d(*100ms)\r\n",hwtd_timeout);
	return hwtd_timeout;
}


//3399重启控制
void hard_wtd_reset_3399board(void)
{	
	gpio_bit_reset(GPIOA, GPIO_PIN_6);
	vTaskDelay(200);
	gpio_bit_set(GPIOA, GPIO_PIN_6);
}






//初始化，硬件看门狗的部分，但是硬件芯片未连接
void hard_wtd_pins_init(void)
{
	//时钟使能
	rcu_periph_clock_enable(RCU_GPIOA);	
	

	gpio_bit_reset(GPIOA, GPIO_PIN_7);  //7 是高有效
	gpio_bit_set(GPIOA, GPIO_PIN_6);  //6是低有效
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);	

	//4脚是WDO的输出脚，低有效，设置为中断模式吧？？
	gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, GPIO_PIN_4);	

#ifdef 	HWTD_USE_INT   //宏在includes.h中定义
	//中断引脚初始化
	//2. 中断引脚的初始化 PA4，外部中断4
	rcu_periph_clock_enable(RCU_AF);		
	
	//2.2 复用为外部中断引脚，
	gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOA, GPIO_PIN_SOURCE_4);
	
	//设置触发方式，下降沿触发
	exti_init(EXTI_4, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
	exti_interrupt_enable(EXTI_4);
	exti_interrupt_flag_clear(EXTI_4);
	//2.3 nvic允许中断
	//中断控制器使能，使用的是外部中断12
	nvic_irq_enable(EXTI4_IRQn,  1, 2);   //允许中断，并设置优先级

#endif	

}



#ifdef 	HWTD_USE_INT
//外部中断12的处理函数,按键按下和松开都会触发中断！！！！
void exint4_handle(void)
{
	printf("exint4_handle reset core\r\n");
	if(is_hwtd_enable)   //允许看门狗的情况下，重启
		hard_wtd_reset_3399board();
}

#endif




//只是单片机模拟一个看门狗
static void hard_wtd_feed_task(void* arg)
{
	while(1)
	{
		if(is_hwtd_enable) //
		{
	//		printf(".");
			if(hwtd_timeout_count)  //喂狗倒计时时间未清零
			{
				hwtd_timeout_count--;		
				if(!hwtd_timeout_count) //数值被减到0
				{
					printf("hard_wtd_feed_task timeout\r\n");
					hard_wtd_reset_3399board();  
					hard_wtd_disable();   //主板重启后，看门狗关闭
				}
			}
		}
		vTaskDelay(100);
	}

}


//100ms进入一次就好 SGM706是1.6秒没有喂狗就会复位
//为了解决喂狗时间可以设置的问题，增加喂狗任务
//喂狗任务根据设置的时间喂狗，
//static void hard_wtd_feed_task(void* arg)
//{
//	static uint16_t count = 0;
//	
//	count++;

//	if(count > 10)  //1s
//	{
//		count = 0;
//		hard_wtd_feed_internel();   //单片机自身1s周期喂狗
//	}
////	printf(",");
//	if(is_hwtd_enable)
//	{
////		printf(".");
//		if(hwtd_timeout_count)  //喂狗倒计时时间未清零
//		{
//			hwtd_timeout_count--;		
//			if(!hwtd_timeout_count) //数值被减到0
//			{
//				printf("hard_wtd_feed_task timeout\r\n");
//				hard_wtd_reset_3399board();  
//				hard_wtd_disable();   //主板重启后，看门狗关闭
//			}
//		}
//	}
//}
