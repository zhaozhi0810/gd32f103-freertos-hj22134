

#include <includes.h>


const char* g_build_time_str = "Buildtime :"__DATE__" "__TIME__;   //��ñ���ʱ��




static void BoardInit(void)
{
//	OePins_Control_Init();   //OE ����������ĻϨ��
//	LcdCtrl_Control_Init();  //lcd��Դ���ƣ���ʼ�������õ�
	//0. �жϷ����ʼ��
	//NVIC_SetPriorityGrouping(4);  //��Ϊ4���ȼ�
	nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);    //2022-09-09 ���ȼ������޸��ˣ�����ֻ����ռ���ȼ��ˣ���
	
	//0.1 ���ù���ģ��ͨ��
    rcu_periph_clock_enable(RCU_AF);
	
	//ֻ����sw�ӿڣ���������GPIO�˿�
	gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);
	
	//1.���ڳ�ʼ��
	//#define DEBUG_COM_NUM 0   //���Դ��ں�
	//#define TOCPU_COM_NUM 1   //��cpuͨ�ŵĴ���
	Com_Debug_init(115200);
//	gd_eval_com_init(DEBUG_COM_NUM);  //���ڵ���
//	gd_eval_com_init(TOCPU_COM_NUM);  //������cpu����ͨ��,�ĵ�cpu�ϵ���ٳ�ʼ��
	
	//2.systick ��ʼ��
//	SystickConfig();
	
	//3.��Ѷ1860�ĵ�Դ1.1vʹ�ܿ���
//	Wxen_Control_Init();
	
	//3.LT9211 mcu���ƶ����ų�ʼ����������LT9211��λ
//	LT9211_Mcu_ControlPort_Init();
//	LT9211_Reset();
	// LT9211 ��ʼ����
//	LT9211_Config();

	//5. ���󰴼�ɨ���ʼ��
//	matrix_keys_init();
	
	
	//6. ��Ƭ���ڲ��¶Ȳɼ�
//	ADC_Init();
	
//	lcd_pwm_init(70);    //����Ĭ��Ϊ70% ����ʱ��ʾ����������������
			
	//10. ��ʼ���ⲿӲ�����Ź���Ĭ�ϲ�����
//	hard_wtd_pins_init();
	
	// 11.led��ʼ��
	Led_Show_Work_init();
//	key_light_leds_init();
	
	//12. ��ѹ�������¶ȣ�iic�ĳ�ʼ��
//	lcd_reset_control_init();  //PD8 lcd��λ���ſ���
		
	//13.//PD6  MicCtl 	
//	MicCtl_Control_Init();


//	key_light_allleds_control(SET);
//	Delay1ms(5000);
	
		
	//15. ������Ƭ���ڲ����Ź�
//	iwdog_init();
//	Delay1ms(5);
//	Wxen_Control_Enable();    //1.1v wx1860ʹ��

//	LcdCtrl_Enable();   //lcd��Դͨ��
//	Enable_LcdLight();    //��7�����Ŀ����źţ�����ʹ�ܺͱ���pwm����,
	//9. lcd�������ų�ʼ��

//	key_light_allleds_control(RESET);  //��������еĵƶ�Ϩ��
	
//	OePins_Output_Hight(3);   //��Ļ���� ͨ��OE3���Ƶ�cpu������pwm
		
}






/*
	2022-09-09 �������������ʱ�򣬷�������ʱ������vTaskStartScheduler�����У���1������ֵerrCOULD_NOT_ALLOCATE_REQUIRED_MEMORY

	�����
	��freeRTOSConfig.h�� #define configTOTAL_HEAP_SIZE			( ( size_t ) ( 10 * 1024 ) )  ԭ����2k����Ϊ��10k
*/






int main(void)
{
	BoardInit();
	
	printf("init ok!!\r\n");
	printf("%s\r\n", g_build_time_str);   //��ʱ��û�д�ӡ���񣬲�Ҫ��ӡ̫������
	
	xTaskCreate(Task_Led_Show_Work,"TaskLed1",configMINIMAL_STACK_SIZE,NULL,2,NULL);

	xTaskCreate(Com_Debug_Print_Task,"debug",configMINIMAL_STACK_SIZE,NULL,1,NULL);
	xTaskCreate(Com_Debug_Recv_Task,"debugr",configMINIMAL_STACK_SIZE,NULL,1,NULL);
	
	
//	xTaskCreate(Task_Led1,"TaskLed1",configMINIMAL_STACK_SIZE,NULL,2,NULL);	
//	xTaskCreate(Task_Led2,"TaskLed2",configMINIMAL_STACK_SIZE,NULL,2,NULL);
	

	
	vTaskStartScheduler();
    
    while(1){}

}





