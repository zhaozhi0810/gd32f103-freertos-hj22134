

#include <includes.h>


const char* g_build_time_str = "Buildtime :"__DATE__" "__TIME__;   //获得编译时间




static void BoardInit(void)
{
	//0. 中断分组初始化
	nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);    //2022-09-09 优先级被我修改了，现在只有抢占优先级了！！
	
	//0.1 复用功能模块通电
    rcu_periph_clock_enable(RCU_AF);
	
	//只保留sw接口，其他用于GPIO端口
	gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);
	
	//gpios.c中的引脚初始化部分
	Gpios_init();
	
	//1.串口初始化
	Com_Debug_init(115200);
	
	//有看门狗硬件时需要初始化，默认不开启
	hard_wtd_pins_init(); //复位引脚需要初始化
	//7寸屏的控制？？
	//	lcd_pwm_init(70);    //亮度默认为70% ，此时显示屏不开启！！！！
			
	//键灯引脚控制
	key_light_leds_init();  
	
	//12. lcd复位引脚控制
	//lcd_reset_control_init();  //PD8 lcd复位引脚控制
		
//	key_light_allleds_control(SET);
	//vTaskDelay(1000);
			
	//15. 启动单片机内部看门狗
//	iwdog_init();
//	Delay1ms(5);

	LcdCtrl_Enable();   //lcd电源通电
	//Enable_LcdLight();    //对7寸屏的控制信号，背光使能和背光pwm控制,
	
	//9. lcd控制引脚初始化	
	OePins_Output_Hight(3);   //屏幕点亮 通过OE3控制的cpu发出的pwm
		
	//10.面板上所有的灯都熄灭
//	key_light_allleds_control(RESET);  //面板上所有的灯都熄灭	
}






/*
	2022-09-09 创建两个任务的时候，发现运行时报错。在vTaskStartScheduler函数中，有1个返回值errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY

	解决：
	在freeRTOSConfig.h中 #define configTOTAL_HEAP_SIZE			( ( size_t ) ( 10 * 1024 ) )  原来是2k，改为了10k
*/






int main(void)
{
	BoardInit();
	
	debug_printf_string("init ok!!\r\n");
	debug_printf_string((char*)g_build_time_str);   //此时还没有打印任务，不要打印太多数据
	debug_printf_string("\r\n");
	
	//1.工作灯的任务
	xTaskCreate(Task_Led_Show_Work,"TaskLed1",configMINIMAL_STACK_SIZE/2,NULL,2,NULL);
	//2.调试串口接收任务
	xTaskCreate(Com_Debug_Recv_Task,"debugr",configMINIMAL_STACK_SIZE+16,NULL,2,NULL);  //调试串口接收任务

	//3. 9211只需要初始化，创建任务后自己删除自己,，如果要打印数据的话，栈要设置大一点
	xTaskCreate(LT9211_Once_Task,"lt9211",configMINIMAL_STACK_SIZE+16,NULL,4,NULL);
	//4. 优先级要高一点，不然容易引起cpu端超时错误
	xTaskCreate(Com_ToCPU_Recv_Task,"ToCpu",configMINIMAL_STACK_SIZE+16,NULL,4,&TaskHandle_ToCpu_Com);  //cpu通信串口任务，优先级高一点
	//5. 矩阵键盘扫描任务
	xTaskCreate(task_matrix_keys_scan,"key_bod",configMINIMAL_STACK_SIZE,NULL,3,&TaskHandle_key_Matrix);  //矩阵键盘扫描任务
	
	//6.获取单片机内部温度的任务
	xTaskCreate(Inter_Temp_task,"temp",configMINIMAL_STACK_SIZE,NULL,2,NULL);

	
	vTaskStartScheduler();
    
    while(1){}

}





