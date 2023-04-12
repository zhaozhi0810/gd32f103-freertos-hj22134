

#include "includes.h"
//#include <task.h>


/*
	工作指示灯   PB4 低电平点亮
*/
//ota-uart.c 启动后，设置升级成功标志
void set_ota_update_success(void);
extern uint8_t update_success_flag;  //升级成功设置为1，否则为0

void Led_Show_Work_init(void)
{
		//时钟使能
	rcu_periph_clock_enable(RCU_GPIOB);	
		
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_4);	
	//熄灭
	gpio_bit_reset(GPIOB, GPIO_PIN_4);
}




void Led_Show_Work_On(void)
{
	gpio_bit_reset(GPIOB, GPIO_PIN_4);
}

void Led_Show_Work_Off(void)
{
	gpio_bit_set(GPIOB, GPIO_PIN_4);
}



//工作灯闪烁
void Task_Led_Show_Work(void* arg)
{
//	static uint8_t n = 0;
	Led_Show_Work_init();
	arg = arg;

	while(1)
	{	
		Led_Show_Work_On();
		vTaskDelay(500);
		
		Led_Show_Work_Off();		
		vTaskDelay(500);
		
		if(!update_success_flag)
		{
			//启动后，设置升级成功标志
			set_ota_update_success();
		}
	}
	
}


