

#include <includes.h>


const char* g_build_time_str = "Buildtime :"__DATE__" "__TIME__;   //获得编译时间




static void BoardInit(void)
{
//	OePins_Control_Init();   //OE 可以用于屏幕熄灭
//	LcdCtrl_Control_Init();  //lcd电源控制，初始化后设置低
	//0. 中断分组初始化
	//NVIC_SetPriorityGrouping(4);  //均为4个等级
	nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);    //2022-09-09 优先级被我修改了，现在只有抢占优先级了！！
	
	//0.1 复用功能模块通电
    rcu_periph_clock_enable(RCU_AF);
	
	//只保留sw接口，其他用于GPIO端口
	gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);
	
	//1.串口初始化
	//#define DEBUG_COM_NUM 0   //调试串口号
	//#define TOCPU_COM_NUM 1   //与cpu通信的串口
	Com_Debug_init(115200);
//	gd_eval_com_init(DEBUG_COM_NUM);  //用于调试
//	gd_eval_com_init(TOCPU_COM_NUM);  //用于与cpu数据通信,改到cpu上电后再初始化
	
	//2.systick 初始化
//	SystickConfig();
	
	//3.网讯1860的电源1.1v使能控制
//	Wxen_Control_Init();
	
	//3.LT9211 mcu控制端引脚初始化，并控制LT9211复位
//	LT9211_Mcu_ControlPort_Init();
//	LT9211_Reset();
	// LT9211 开始工作
//	LT9211_Config();

	//5. 矩阵按键扫描初始化
//	matrix_keys_init();
	
	
	//6. 单片机内部温度采集
//	ADC_Init();
	
//	lcd_pwm_init(70);    //亮度默认为70% ，此时显示屏不开启！！！！
			
	//10. 初始化外部硬件看门狗，默认不开启
//	hard_wtd_pins_init();
	
	// 11.led初始化
	Led_Show_Work_init();
//	key_light_leds_init();
	
	//12. 电压电流，温度，iic的初始化
//	lcd_reset_control_init();  //PD8 lcd复位引脚控制
		
	//13.//PD6  MicCtl 	
//	MicCtl_Control_Init();


//	key_light_allleds_control(SET);
//	Delay1ms(5000);
	
		
	//15. 启动单片机内部看门狗
//	iwdog_init();
//	Delay1ms(5);
//	Wxen_Control_Enable();    //1.1v wx1860使能

//	LcdCtrl_Enable();   //lcd电源通电
//	Enable_LcdLight();    //对7寸屏的控制信号，背光使能和背光pwm控制,
	//9. lcd控制引脚初始化

//	key_light_allleds_control(RESET);  //面板上所有的灯都熄灭
	
//	OePins_Output_Hight(3);   //屏幕点亮 通过OE3控制的cpu发出的pwm
		
}






/*
	2022-09-09 创建两个任务的时候，发现运行时报错。在vTaskStartScheduler函数中，有1个返回值errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY

	解决：
	在freeRTOSConfig.h中 #define configTOTAL_HEAP_SIZE			( ( size_t ) ( 10 * 1024 ) )  原来是2k，改为了10k
*/






int main(void)
{
	BoardInit();
	
	printf("init ok!!\r\n");
	printf("%s\r\n", g_build_time_str);   //此时还没有打印任务，不要打印太多数据
	
	xTaskCreate(Task_Led_Show_Work,"TaskLed1",configMINIMAL_STACK_SIZE,NULL,2,NULL);

	xTaskCreate(Com_Debug_Print_Task,"debug",configMINIMAL_STACK_SIZE,NULL,1,NULL);
	xTaskCreate(Com_Debug_Recv_Task,"debugr",configMINIMAL_STACK_SIZE,NULL,1,NULL);
	
	
//	xTaskCreate(Task_Led1,"TaskLed1",configMINIMAL_STACK_SIZE,NULL,2,NULL);	
//	xTaskCreate(Task_Led2,"TaskLed2",configMINIMAL_STACK_SIZE,NULL,2,NULL);
	

	
	vTaskStartScheduler();
    
    while(1){}

}





