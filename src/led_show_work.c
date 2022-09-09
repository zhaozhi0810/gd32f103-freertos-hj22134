

#include "includes.h"
//#include <task.h>


/*
	����ָʾ��   PB4 �͵�ƽ����
*/



void Led_Show_Work_init(void)
{
		//ʱ��ʹ��
	rcu_periph_clock_enable(RCU_GPIOB);	
		
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_4);	
	//Ϩ��
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



//��������˸
void Task_Led_Show_Work(void* arg)
{
//	static uint8_t n = 0;
	
	arg = arg;

	while(1)
	{	
		Led_Show_Work_On();
		vTaskDelay(500);
		
		Led_Show_Work_Off();
		
		vTaskDelay(500);
	}
	
}

