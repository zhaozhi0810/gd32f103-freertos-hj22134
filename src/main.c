

#include <includes.h>






int main(void)
{
	nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
	//0.1 复用功能模块通电
    rcu_periph_clock_enable(RCU_AF);
	//只保留sw接口，其他用于GPIO端口
	gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);	
//    LED_Init();
	Led_Show_Work_init();
	
	xTaskCreate(Task_Led_Show_Work,"TaskLed1",configMINIMAL_STACK_SIZE,NULL,2,NULL);	
	
//	xTaskCreate(Task_Led1,"TaskLed1",configMINIMAL_STACK_SIZE,NULL,2,NULL);	
//	xTaskCreate(Task_Led2,"TaskLed2",configMINIMAL_STACK_SIZE,NULL,2,NULL);
	vTaskStartScheduler();
    
    while(1){}

}





