

#include <includes.h>


const char* g_build_time_str = "Buildtime :"__DATE__" "__TIME__;   //��ñ���ʱ��




static void BoardInit(void)
{
	//0. �жϷ����ʼ��
	nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);    //2022-09-09 ���ȼ������޸��ˣ�����ֻ����ռ���ȼ��ˣ���
	
	//0.1 ���ù���ģ��ͨ��
    rcu_periph_clock_enable(RCU_AF);
	
	//ֻ����sw�ӿڣ���������GPIO�˿�
	gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);
	
	//gpios.c�е����ų�ʼ������
	Gpios_init();
	
	//1.���Դ��ڳ�ʼ��
	Com_Debug_init(115200);
	
	//���Ź�Ӳ�� �� ��Ƭ�����Ź� ʱ��Ҫ��ʼ����Ĭ�Ͽ���
	hard_wtd_pins_init(); //��Ƭ����λ���İ�������Ҫ��ʼ��
	//7�����Ŀ��ƣ���
	//	lcd_pwm_init(70);    //����Ĭ��Ϊ70% ����ʱ��ʾ����������������
			
	//�������ſ���
	key_light_leds_init();  
	
		
	//15. ������Ƭ���ڲ����Ź�
//	iwdog_init();
//	Delay1ms(5);

}






/*
	2022-09-09 �������������ʱ�򣬷�������ʱ������vTaskStartScheduler�����У���1������ֵerrCOULD_NOT_ALLOCATE_REQUIRED_MEMORY

	�����
	��freeRTOSConfig.h�� #define configTOTAL_HEAP_SIZE			( ( size_t ) ( 10 * 1024 ) )  ԭ����2k����Ϊ��10k
*/






int main(void)
{
	BoardInit();
	
	debug_printf_string("freertos , init ok!!\r\n");
	debug_printf_string((char*)g_build_time_str);   //��ʱ��û�д�ӡ���񣬲�Ҫ��ӡ̫������
	debug_printf_string("\r\n");
	
	//1.�����Ƶ�����
	xTaskCreate(Task_Led_Show_Work,"TaskLed1",configMINIMAL_STACK_SIZE/2,NULL,2,NULL);
	//2.���Դ��ڽ�������
	xTaskCreate(Com_Debug_Recv_Task,"debugr",configMINIMAL_STACK_SIZE+16,NULL,2,NULL);  //���Դ��ڽ�������

	//3. 9211ֻ��Ҫ��ʼ��������������Լ�ɾ���Լ�,�����Ҫ��ӡ���ݵĻ���ջҪ���ô�һ��
	xTaskCreate(LT9211_Once_Task,"lt9211",configMINIMAL_STACK_SIZE+16,NULL,4,NULL);
	//4. ���ȼ�Ҫ��һ�㣬��Ȼ��������cpu�˳�ʱ����
	xTaskCreate(Com_ToCPU_Recv_Task,"ToCpu",configMINIMAL_STACK_SIZE+16,NULL,4,&TaskHandle_ToCpu_Com);  //cpuͨ�Ŵ����������ȼ���һ��
	//5. �������ɨ������
	xTaskCreate(task_matrix_keys_scan,"key_bod",configMINIMAL_STACK_SIZE,NULL,3,&TaskHandle_key_Matrix);  //�������ɨ������
	
	//6.��ȡ��Ƭ���ڲ��¶ȵ�����
	xTaskCreate(Inter_Temp_task,"temp",configMINIMAL_STACK_SIZE,NULL,2,NULL);

	
	vTaskStartScheduler();
    
    while(1){}

}





