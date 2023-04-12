

#include "includes.h"


const char* g_build_time_str = "Buildtime :"__DATE__" "__TIME__;   //获得编译时间
static uint8_t g_McuVersion = 103;   //1.03,2023-03-29升级103

/*
	102   2023-02-02  单片机支持串口升级功能，调试串口命令y 需要烧写ota的程序
*/



uint8_t GetMcuVersion(void)
{
	return g_McuVersion;
}

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
	
	//1.调试串口初始化
	Com_Debug_init(115200);
	
	//看门狗硬件 和 单片机软看门狗 时需要初始化，默认开启
	hard_wtd_pins_init(); //单片机复位核心板引脚需要初始化
	//7寸屏的控制？？
	//	lcd_pwm_init(70);    //亮度默认为70% ，此时显示屏不开启！！！！
			
	//键灯引脚控制
	key_light_leds_init();  
	
//	LT9211_ResetPort_Init();	 //9211复位引脚初始化.2023-03-29
	//15. 启动单片机内部看门狗
//	iwdog_init();
//	Delay1ms(5);

}






/*
	2022-09-09 创建两个任务的时候，发现运行时报错。在vTaskStartScheduler函数中，有1个返回值errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY

	解决：
	在freeRTOSConfig.h中 #define configTOTAL_HEAP_SIZE			( ( size_t ) ( 10 * 1024 ) )  原来是2k，改为了10k
*/






int main(void)
{
	nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x6000);   //注意变化！！！2023-02-01
	
	BoardInit();

	debug_printf_string("freertos , init ok!!\r\n");
	debug_printf_string((char*)g_build_time_str);   //此时还没有打印任务，不要打印太多数据
	debug_printf_string("\r\n");
	
//	debug_printf_string("\r\n");
	
	//1.工作灯的任务
	xTaskCreate(Task_Led_Show_Work,"TaskLed1",configMINIMAL_STACK_SIZE/2,NULL,2,NULL);
	//2.调试串口接收任务
	xTaskCreate(Com_Debug_Recv_Task,"debugr",configMINIMAL_STACK_SIZE*2-16,NULL,2,NULL);  //调试串口接收任务

	//3. 9211只需要初始化，创建任务后自己删除自己,，如果要打印数据的话，栈要设置大一点
	xTaskCreate(LT9211_Once_Task,"lt9211",configMINIMAL_STACK_SIZE+16,NULL,4,NULL);
	//4. 优先级要高一点，不然容易引起cpu端超时错误
	xTaskCreate(Com_ToCPU_Recv_Task,"ToCpu",configMINIMAL_STACK_SIZE*2,NULL,4,&TaskHandle_ToCpu_Com);  //cpu通信串口任务，优先级高一点
	//5. 矩阵键盘扫描任务
	xTaskCreate(task_matrix_keys_scan,"key_bod",configMINIMAL_STACK_SIZE*2,NULL,3,&TaskHandle_key_Matrix);  //矩阵键盘扫描任务
	
	//6.获取单片机内部温度的任务
	xTaskCreate(Inter_Temp_task,"temp",configMINIMAL_STACK_SIZE,NULL,2,NULL);

	//7.led闪烁控制的任务，2022-12-12 改为定时器1控制
	//xTaskCreate(keyLeds_Flash_task,"led_flash",configMINIMAL_STACK_SIZE,NULL,2,&TaskHandle_leds_Flash);
	
	//morse ptt 暂时没有用上，2022-12-19，gpios.c
	//xTaskCreate(task_morse_ptt_scan,"morseptt",configMINIMAL_STACK_SIZE,NULL,2,&TaskHandle_Morseptt);
	
	
	vTaskStartScheduler();
    
    while(1){}

}





