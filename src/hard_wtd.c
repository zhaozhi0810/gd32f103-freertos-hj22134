
/*
	�ⲿ�Ŀ��Ź�оƬ�Ĵ���  (�ⲿ���Ź�оƬδ����)

	WDO --�� PA4  ���룬�͵�ƽ��ʾ��ʱ
	WDI --�� PA5  			�����ι��
	RESET --�� PA6  		������͵�ƽ��λ3399
	SYS_RESET_IN --�� PA7  	���������ߣ�3399��λ

	APP���õ�ι������
	
	ι���������APP���õ�ι������ι��
	app����ι������ι������ȴ�ʱ�䵽֮�󽫽���ι��
	
	�������õ�Ƭ��������һ�����Ź���cpu������2022-09-14
	

*/
#include "includes.h"



static uint8_t is_hwtd_enable = 0;   //Ĭ�Ͽ��Ź�������ʹ��

static TaskHandle_t  TaskHandle_Hard_Wtd;  


//ι������������õ�ʱ��ι����
static void hard_wtd_feed_task(void* arg);


//���25�� == ���ֵ255������
volatile static uint16_t hwtd_timeout = 220;    //APP���õ�ι�����ڣ� ��С��λ100ms
static uint16_t hwtd_timeout_count = 0;  //ι����ʱֵ��

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

//�̶�ι����1sι��һ��
static void hard_wtd_feed_internel(void)
{
	if(gpio_output_bit_get(GPIOA, GPIO_PIN_5))  //ԭ���Ǹ�
	{
		gpio_bit_reset(GPIOA, GPIO_PIN_5); //����
	}
	else
		gpio_bit_set(GPIOA, GPIO_PIN_5);  //����
	
//	MY_PRINTF("hard_wtd_feed_internel\r\n");
}


//cpuι��һ�Σ����ǰ�ʱ�����¸�ֵΪ���ֵ
void hard_wtd_feed(void)
{
	hwtd_timeout_count = hwtd_timeout;  //
	
	MY_PRINTF("hard_wtd_feed\r\n");
}


//����������Ź���״̬ 1��ʾ������0��ʾ�ر�
uint8_t get_hard_wtd_status(void)
{
	return is_hwtd_enable;
}



//���ÿ��Ź���ʱʱ�䣬��λ100ms
void hard_wtd_set_timeout(uint8_t timeout)
{
	MY_PRINTF("hard_wtd_set_timeout timeout = %d(*100ms)\r\n",timeout);
	hwtd_timeout = timeout;
}

//��ÿ��Ź���ʱʱ�䣬��λ100ms
uint8_t  hard_wtd_get_timeout(void)
{
	MY_PRINTF("hard_wtd_get_timeout timeout = %d(*100ms)\r\n",hwtd_timeout);
	return hwtd_timeout;
}


//3399��������
void hard_wtd_reset_3399board(void)
{	
	gpio_bit_reset(GPIOA, GPIO_PIN_6);
	vTaskDelay(200);
	gpio_bit_set(GPIOA, GPIO_PIN_6);
}






//��ʼ����Ӳ�����Ź��Ĳ��֣�����Ӳ��оƬδ����
void hard_wtd_pins_init(void)
{
	//ʱ��ʹ��
	rcu_periph_clock_enable(RCU_GPIOA);	
	

	gpio_bit_reset(GPIOA, GPIO_PIN_7);  //7 �Ǹ���Ч
	gpio_bit_set(GPIOA, GPIO_PIN_6);  //6�ǵ���Ч
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);	

	//4����WDO������ţ�����Ч������Ϊ�ж�ģʽ�ɣ���
	gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, GPIO_PIN_4);	

#ifdef 	HWTD_USE_INT   //����includes.h�ж���
	//�ж����ų�ʼ��
	//2. �ж����ŵĳ�ʼ�� PA4���ⲿ�ж�4
	rcu_periph_clock_enable(RCU_AF);		
	
	//2.2 ����Ϊ�ⲿ�ж����ţ�
	gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOA, GPIO_PIN_SOURCE_4);
	
	//���ô�����ʽ���½��ش���
	exti_init(EXTI_4, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
	exti_interrupt_enable(EXTI_4);
	exti_interrupt_flag_clear(EXTI_4);
	//2.3 nvic�����ж�
	//�жϿ�����ʹ�ܣ�ʹ�õ����ⲿ�ж�12
	nvic_irq_enable(EXTI4_IRQn,  1, 2);   //�����жϣ����������ȼ�

#endif	

}



#ifdef 	HWTD_USE_INT
//�ⲿ�ж�12�Ĵ�������,�������º��ɿ����ᴥ���жϣ�������
void exint4_handle(void)
{
	printf("exint4_handle reset core\r\n");
	if(is_hwtd_enable)   //�������Ź�������£�����
		hard_wtd_reset_3399board();
}

#endif




//ֻ�ǵ�Ƭ��ģ��һ�����Ź�
static void hard_wtd_feed_task(void* arg)
{
	while(1)
	{
		if(is_hwtd_enable) //
		{
	//		printf(".");
			if(hwtd_timeout_count)  //ι������ʱʱ��δ����
			{
				hwtd_timeout_count--;		
				if(!hwtd_timeout_count) //��ֵ������0
				{
					printf("hard_wtd_feed_task timeout\r\n");
					hard_wtd_reset_3399board();  
					hard_wtd_disable();   //���������󣬿��Ź��ر�
				}
			}
		}
		vTaskDelay(100);
	}

}


//100ms����һ�ξͺ� SGM706��1.6��û��ι���ͻḴλ
//Ϊ�˽��ι��ʱ��������õ����⣬����ι������
//ι������������õ�ʱ��ι����
//static void hard_wtd_feed_task(void* arg)
//{
//	static uint16_t count = 0;
//	
//	count++;

//	if(count > 10)  //1s
//	{
//		count = 0;
//		hard_wtd_feed_internel();   //��Ƭ������1s����ι��
//	}
////	printf(",");
//	if(is_hwtd_enable)
//	{
////		printf(".");
//		if(hwtd_timeout_count)  //ι������ʱʱ��δ����
//		{
//			hwtd_timeout_count--;		
//			if(!hwtd_timeout_count) //��ֵ������0
//			{
//				printf("hard_wtd_feed_task timeout\r\n");
//				hard_wtd_reset_3399board();  
//				hard_wtd_disable();   //���������󣬿��Ź��ر�
//			}
//		}
//	}
//}