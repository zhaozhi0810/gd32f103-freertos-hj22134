

#include "includes.h"


const char* g_build_time_str = "Buildtime :"__DATE__" "__TIME__;   //��ñ���ʱ��
static uint8_t g_McuVersion = 103;   //1.03,2023-03-29����103

/*
	102   2023-02-02  ��Ƭ��֧�ִ����������ܣ����Դ�������y ��Ҫ��дota�ĳ���
*/



uint8_t GetMcuVersion(void)
{
	return g_McuVersion;
}

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
	
//	LT9211_ResetPort_Init();	 //9211��λ���ų�ʼ��.2023-03-29
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
	nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x6000);   //ע��仯������2023-02-01
	
	BoardInit();

	debug_printf_string("freertos , init ok!!\r\n");
	debug_printf_string((char*)g_build_time_str);   //��ʱ��û�д�ӡ���񣬲�Ҫ��ӡ̫������
	debug_printf_string("\r\n");
	
//	debug_printf_string("\r\n");
	
	//1.�����Ƶ�����
	xTaskCreate(Task_Led_Show_Work,"TaskLed1",configMINIMAL_STACK_SIZE/2,NULL,2,NULL);
	//2.���Դ��ڽ�������
	xTaskCreate(Com_Debug_Recv_Task,"debugr",configMINIMAL_STACK_SIZE*2-16,NULL,2,NULL);  //���Դ��ڽ�������

	//3. 9211ֻ��Ҫ��ʼ��������������Լ�ɾ���Լ�,�����Ҫ��ӡ���ݵĻ���ջҪ���ô�һ��
	xTaskCreate(LT9211_Once_Task,"lt9211",configMINIMAL_STACK_SIZE+16,NULL,4,NULL);
	//4. ���ȼ�Ҫ��һ�㣬��Ȼ��������cpu�˳�ʱ����
	xTaskCreate(Com_ToCPU_Recv_Task,"ToCpu",configMINIMAL_STACK_SIZE*2,NULL,4,&TaskHandle_ToCpu_Com);  //cpuͨ�Ŵ����������ȼ���һ��
	//5. �������ɨ������
	xTaskCreate(task_matrix_keys_scan,"key_bod",configMINIMAL_STACK_SIZE*2,NULL,3,&TaskHandle_key_Matrix);  //�������ɨ������
	
	//6.��ȡ��Ƭ���ڲ��¶ȵ�����
	xTaskCreate(Inter_Temp_task,"temp",configMINIMAL_STACK_SIZE,NULL,2,NULL);

	//7.led��˸���Ƶ�����2022-12-12 ��Ϊ��ʱ��1����
	//xTaskCreate(keyLeds_Flash_task,"led_flash",configMINIMAL_STACK_SIZE,NULL,2,&TaskHandle_leds_Flash);
	
	//morse ptt ��ʱû�����ϣ�2022-12-19��gpios.c
	//xTaskCreate(task_morse_ptt_scan,"morseptt",configMINIMAL_STACK_SIZE,NULL,2,&TaskHandle_Morseptt);
	
	
	vTaskStartScheduler();
    
    while(1){}

}





