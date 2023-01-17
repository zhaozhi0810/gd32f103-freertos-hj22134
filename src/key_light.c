

/*
	�����ƵĲ���
	ʹ��local bus�ķ�ʽ
	
	��ַ  A0~A5
	����  D0
	CS    CS
7. PD11- 3V3_A0	
13.PE2 - 3V3_A1
14.PE3 - 3V3_A2
15.PE4 - 3V3_A3
16.PE5 - 3V3_A4
17.PE6 - 3V3_A5

9.PD14- 3V3_D0

19.PE8 - KEYBOARD_CS	
	
	״̬�ƣ���ɫ�ƣ������3399���ƣ����ɵ�Ƭ�����ƣ�
	ÿ�����ſ���һ����ɫ������


�����ȿ��ƣ�����ԭ��ͼ�����������lcd��pwm��ͻ���޷�ͬʱ(����)ʹ�ö�ʱ��
20.PE9-PWM_LED ��TIM0-CH0��//������io��ģ���


2022-09-14 freertos��ֲ��֧�ֵ������ȣ�������


*/

#include "includes.h"


#define ENABLE_KEYBOARD_CS 1
#define DISABLE_KEYBOARD_CS 0
#define LEDS_NUM_MAX 40   //���ֵ40������ֵ1-40��Ч 

//static uint8_t g_led_pwm = 100;
static uint64_t leds_status = 0;   //ÿһλ��ʾһ��led��״̬������45����1��ʾ����0��ʾ��
#define LEDS_PWM_HZ 20   //20��ʾled��pwmΪ50HZ����ʱ��ÿ1ms����һ��

#ifdef LEDS_FLASH_TASK
TaskHandle_t  TaskHandle_leds_Flash;   //���led_��˸����ָ��
#endif

static uint8_t g_leds_flash_time[LEDS_NUM_MAX] = {0};  //led ��Ҫ��˸��ʱ��ֵ��0��ʾ500ms��1��ʾ800ms��2��ʾ1s��3��ʾ2s��������ʾ����˸
static uint8_t g_leds_flash_time_already[4] = {0};   //led�Ѿ���˸��ʱ��,ֻ��¼ĳ����˸��ʱ�䣬����ĳһ��led
static uint8_t g_leds_flash_action=0;      // 0-3λ���ֱ��ʾ5��8��10��20�����������1��ʾ����0��ʾ��
//static uint8_t g_leds_flash_control = 0;	//0��ʾû�е���Ҫ��˸�ˣ���0��ʾ���е���Ҫ��˸

//2022-09-16 ����timer1 �����ж��źţ����޸�pwm�����ֵ
//ֻ�����ڼ�ʱ����ʹ��tim1�ˣ�
//static void TIM1_Led_Pwm_Init(uint16_t arr,uint16_t psc);
void Kleds_pwm_init(uint8_t degree);

//2022-12-12 ����timer1 �����ж��źţ������Ƽ�����˸
static void TIM1_Led_Pwm_Init(uint16_t arr,uint16_t psc);


void key_light_leds_init(void)
{
	uint32_t pin;
	uint8_t i;
	//�����������
	//1. ʱ��ʹ��
	rcu_periph_clock_enable(RCU_GPIOD);
		
	//2.0 �ϵ��������  (D11  -- A0   D14  -- D0)
	gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_11 | GPIO_PIN_14);  //�������	
	//2. ��ʼ����Ĭ�������
	gpio_bit_reset(GPIOD, GPIO_PIN_11 | GPIO_PIN_14);
	
	
	rcu_periph_clock_enable(RCU_GPIOE);
	pin = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;	
	//2.0 �ϵ��������
	gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, pin);  //�������	
	//2. ��ʼ����Ĭ�������
	gpio_bit_reset(GPIOE, pin);
	
	
	//2. cs���ų�ʼ����Ĭ�������
	gpio_bit_reset(GPIOE, GPIO_PIN_8);   //PE8 ��cs���ţ���Ҫ����Ϊ�ͣ�����2022-09-16	  | GPIO_PIN_9
	gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_8);  //�������	 | GPIO_PIN_9
	Kleds_pwm_init(100);   //����pwmֵΪ100
	
	//����led������˸���ƣ�2022-12-12
	TIM1_Led_Pwm_Init(1000-1,(SystemCoreClock/10000)-1);    //100ms��ʱ��ʼ��

	for(i=0;i<LEDS_NUM_MAX;i++)
	{
		g_leds_flash_time[i] = 5;
	}
}


/*
	status   0 ��ʾ��ֹ������ߵ�ƽ
			 ��0��ʾѡ�У�����͵�ƽ
*/
static void key_light_cs(uint8_t status)
{
	if(status)
		gpio_bit_set(GPIOE, GPIO_PIN_8);   //status = 1,��ʾ�������
	else
		gpio_bit_reset(GPIOE, GPIO_PIN_8);   //status = 0,��ʾ���ݱ��֣���Ӧcs����Ϊ0
}



//addr 1-40 ��Ӧ40���ơ�
static void key_light_send_addr(uint8_t addr)
{
#if 1
	uint16_t val;
	val = gpio_output_port_get(GPIOE);
		
	val &= ~(0x1f<<2);   //����
	val |= ((addr&0x3e)<<1); 
	
	gpio_port_write(GPIOE, val);
#else	
	if(addr&2)
		gpio_bit_set(GPIOE, GPIO_PIN_2);   //A1
	else
		gpio_bit_reset(GPIOE, GPIO_PIN_2);
	
	if(addr&4)
		gpio_bit_set(GPIOE, GPIO_PIN_3);   //A2
	else
		gpio_bit_reset(GPIOE, GPIO_PIN_3);
	
	if(addr&8)
		gpio_bit_set(GPIOE, GPIO_PIN_4);   //A3
	else
		gpio_bit_reset(GPIOE, GPIO_PIN_4);
	
	if(addr&0x10)
		gpio_bit_set(GPIOE, GPIO_PIN_5);   //A4
	else
		gpio_bit_reset(GPIOE, GPIO_PIN_5);
	
	if(addr&0x20)
		gpio_bit_set(GPIOE, GPIO_PIN_6);   //A5
	else
		gpio_bit_reset(GPIOE, GPIO_PIN_6);
#endif
	if(addr&1)
		gpio_bit_set(GPIOD, GPIO_PIN_11);   //A0
	else
		gpio_bit_reset(GPIOD, GPIO_PIN_11);

}

//���led������Ҫ�Ǳ��ļ��ڲ�ʹ�ã��������ṩ���ƽӿ�
//����ӿ�������˸�ͳ�������ƣ�����Ӱ����˸����
//whichled: 0-39 ,����39��ʾ����led
//status : 0��ʾϨ�𣬷����ʾ����
static void key_light_leds_control2(uint8_t whichled,uint8_t status)
{		
//	key_light_cs(DISABLE_KEYBOARD_CS);  //0���������
	if(whichled < LEDS_NUM_MAX)  //whichled>0 && 
	{
		key_light_send_addr(whichled+1);		
	}
	else
		return;
		
	if(status)	
	{
		gpio_bit_set(GPIOD, GPIO_PIN_14);
		
		if(whichled == 39)  //��¼led��״̬
			leds_status = ~0ULL;	 //ȫ������
		else
			leds_status |= 1<<whichled;
	}
	else
	{
		gpio_bit_reset(GPIOD, GPIO_PIN_14);	
		
		if(whichled == 39)  //��¼led��״̬
			leds_status = 0;	 //ȫ������
		else
			leds_status &= ~(1<<whichled);
	}
	
//	vTaskDelay(pdMS_TO_TICKS(1));
//	Delay1ms(1);
//	Delay1us(30);
	//�������ԣ��о����ʹ����һ�����ز����������������ػ����½��أ�����������һ�θ��¡���2022-12-20
	key_light_cs(ENABLE_KEYBOARD_CS);   //1��ʹ�����
	Delay1us(30);
	key_light_cs(DISABLE_KEYBOARD_CS);  //0���������
		
}

/*
//���led���� �����ṩ���ƽӿڣ��������õƵ�������ͬʱ��ֹͣ�Ƶ���˸���ơ�
	whichled  32��ʾ���е�
			1-40 �ֱ��Ӧ�����ĵ�
			 
	status   0 ��ʾϨ��
			 ��0��ʾ����
*/
void key_light_leds_control(uint8_t whichled,uint8_t status)
{		
	uint8_t i;

	if((whichled < 1) || (whichled > LEDS_NUM_MAX))  //whichled>0 && 	
		return;
	
	whichled -= 1;   //����Ϊ0-39  !!!!!!!!

#ifdef LEDS_FLASH_TASK   //��һ����Ҫ��ȡ���Ƶ���˸����
	if(whichled == 39)
	{
		timer_disable(TIMER1);
		for(i=0;i<LEDS_NUM_MAX;i++)
			g_leds_flash_time[i] = 5;  //ȫ��ȡ����˸
	//	g_leds_flash_control = 0;  //ȫ������Ҫ��˸�ˡ�
		
	}
	else
		g_leds_flash_time[whichled] = 5;  //0-3��ʾ������˸ 
#endif	
	key_light_leds_control2(whichled,status);
}






//���ĳһ���Ƶ�״̬��whichled �� 1 - 40
//����255��ʾ����0��1��ʾ��ȷ
uint8_t get_led_status(uint8_t whichled)
{
	if(whichled > LEDS_NUM_MAX)  //whichled>0 && 
	{
		return 255;		
	}	
	whichled -= 1;   //����Ϊ0-39  !!!!!!!!
	return (leds_status>>whichled) & 1;	
}



//�԰������������led�Ŀ���
void key_light_allleds_control(uint8_t status)
{
//	uint8_t i;
//	
//	for(i=0;i<40;i++)   //��������
//	{
//		key_light_leds_control(i,status);
//		Delay1us(50);
//	}
	
	key_light_leds_control(40,status);	  //ȫ����40��ʾ	
}







//-----------------------------------------------------------------------------------
//led��pwm����

// 1us�Ƹ�����4000�����Ƶ�ʾ���1000000/4000=250Hz ̫����Ӱ���ż���ʵ�飿����
static uint16_t KLEDS_PWM_HZ = 4000;   //led��pwmΪ250HZ��
static uint8_t g_kleds_pwm = 100;

//timer2 ch2

#define KLEDS_PWM_PIN GPIO_PIN_0
#define KLEDS_PWM_PORT GPIOB
#define KLEDS_PWM_PORT_RCU RCU_GPIOB
#define KLEDS_PWM_TIMER_RCU  RCU_TIMER2    //20221010 timer2
#define KLEDS_PWM_TIMER  TIMER2
#define KLEDS_PWM_TIMER_CH TIMER_CH_2

//#define KLEDS_PWM_TIMER_USE_CHN   //ʹ�û���ͨ����


//���Ƶ�pwm�ĳ�ʼ����degree��ʾռ�ձȣ�0-100
void Kleds_pwm_init(uint8_t degree)
{
	rcu_periph_clock_enable(KLEDS_PWM_PORT_RCU);   //�˿ڵ�ʱ��ʹ��
#ifndef KLEDS_PWM	
	//uint32_t pin;
	//rcu_periph_clock_enable(RCU_GPIOB);
	//2. ����Ϊ���ģʽ	
	//pin = BIT(GPIO_PIN_0);    //PB0��PB1 ͬʱ����Ϊ���
	
	gpio_init(KLEDS_PWM_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, KLEDS_PWM_PIN);
	
	if(degree > 0)
	{
		gpio_bit_set(KLEDS_PWM_PORT, KLEDS_PWM_PIN);
	}
	else
	{
		gpio_bit_reset(KLEDS_PWM_PORT, KLEDS_PWM_PIN);
	}
	
#else		
	//PB15 ͨ��
	timer_parameter_struct initpara;
	timer_oc_parameter_struct ocpara;
	//1. io�������ø��ù���	
	gpio_init(KLEDS_PWM_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_2MHZ, KLEDS_PWM_PIN);   //���ù���	
	//PB15 Ĭ��ӳ��tim0 ch2��Nͨ������0��ʼ����
	//gpio_pin_remap_config(GPIO_TIMER2_PARTIAL_REMAP, ENABLE);    //����ӳ��
	
	//2. ��ʱ��ʱ��ʹ��
	rcu_periph_clock_enable(KLEDS_PWM_TIMER_RCU);  //��ʱ��ģ��ʱ��ʹ��
		
	if(degree > 100)
	{
		degree = 100;
	}
	g_kleds_pwm = degree;   //���浽ȫ�ֱ����С�
	
	//3. ��ʼ����ʱ�������ݽṹ  /* initialize TIMER init parameter struct */
	timer_struct_para_init(&initpara);
	initpara.period = KLEDS_PWM_HZ-1;  //���ص����֣�Ƶ��20kHZ
	initpara.prescaler = (SystemCoreClock/1000000)-1;  //Ԥ��Ƶ�����õ���1Mhz������  
		
	//4. ��ʼ����ʱ��      /* initialize TIMER counter */
	timer_init(KLEDS_PWM_TIMER, &initpara);
		
	//5. ��ʼ����ʱ��ͨ�������ݽṹ /* initialize TIMER channel output parameter struct */
	timer_channel_output_struct_para_init(&ocpara);	
#ifndef KLEDS_PWM_TIMER_USE_CHN
	ocpara.outputstate  = TIMER_CCX_ENABLE;  //���ͨ��ʹ��	
#else
    ocpara.outputnstate = TIMER_CCXN_ENABLE;//ʹ�ܻ���ͨ�����
#endif
	//6. ��ʼ����ʱ��ͨ��   /* configure TIMER channel output function */
	timer_channel_output_config(KLEDS_PWM_TIMER, KLEDS_PWM_TIMER_CH, &ocpara);
			
	//7. ��ʼ����ʱ��ͨ�������ʽ����   /* configure TIMER channel output compare mode */
	timer_channel_output_mode_config(KLEDS_PWM_TIMER, KLEDS_PWM_TIMER_CH, TIMER_OC_MODE_PWM1);
	/* configure TIMER channel output pulse value */
	
	//8. ��ʼ����ʱ��ͨ�����������
	timer_channel_output_pulse_value_config(KLEDS_PWM_TIMER, KLEDS_PWM_TIMER_CH, (100-degree) * KLEDS_PWM_HZ/100);
//	timer_channel_complementary_output_state_config
//timer_channel_complementary_output_polarity_config
	//9. ��ʼ����ʱ��ͨ�����ʹ��
	//timer_channel_output_fast_config(TIMER2, TIMER_CH_0, TIMER_OC_FAST_ENABLE);
	timer_channel_output_shadow_config(KLEDS_PWM_TIMER, KLEDS_PWM_TIMER_CH, TIMER_OC_SHADOW_DISABLE);	  //stm32�ƺ��õ������0x8
	//10.��ʼ������ʱ����ʹ�� 2022-04-18	
		
	/* enable a TIMER */
	timer_primary_output_config(KLEDS_PWM_TIMER, ENABLE);
	timer_auto_reload_shadow_enable(KLEDS_PWM_TIMER);
	timer_enable(KLEDS_PWM_TIMER);
#endif

	//*******����Ϊ����ʹ�ܿ��ơ�	
//	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_14);
//	gpio_bit_reset(GPIOB, GPIO_PIN_14);   //��ʼ��������ߵ�ƽ��Ĭ�ϵ�������


}





/*
����lcd����ռ�ձ�
//degree �޸�Ϊ0-100
*/
void set_Kleds_pwm_out(int8_t degree)
{
#ifdef KLEDS_PWM
	uint32_t value;

//	if(PWM_DEGREE_MIN > degree) degree = PWM_DEGREE_MIN;
//	if(PWM_DEGREE_MAX < degree) degree = PWM_DEGREE_MAX;
	
	if(degree > 100)	
		degree = 100;	
	else if(degree < 0)
		degree = 0;
		
	g_kleds_pwm = degree; 
	MY_PRINTF("Kleds_pwm_out g_kleds_pwm = %d\r\n",g_kleds_pwm);   //��ӡһ����Ϣ�����Թر�
	
	value = (100-degree) * KLEDS_PWM_HZ / 100;
	/* CH configuration in PWM mode1,duty cycle  */
	//timer_channel_output_pulse_value_config(TIMER1,TIMER_CH_0,value);
	//TIM_SetCompare4(TIM3, value);
	timer_channel_output_pulse_value_config(KLEDS_PWM_TIMER, KLEDS_PWM_TIMER_CH, value);
#else
	if(degree > 0)
	{
		g_kleds_pwm = 10;   //10�ȽϺô���
		gpio_bit_set(KLEDS_PWM_PORT, KLEDS_PWM_PIN);
	}
	else
	{
		g_kleds_pwm = 0;
		gpio_bit_reset(KLEDS_PWM_PORT, KLEDS_PWM_PIN);
	}
#endif	
}


/*
����lcd����ռ�ձ�
//val Ϊ��Ҫ������ֵ��������������ʾ�������ӣ�����ʾ���ȼ�С
*/
void Kleds_pwm_change(int8_t val)
{
	int8_t degree ;
		
	if((val > 0) && (g_kleds_pwm<100))  //����0�����������ȣ���������ֵҲС��100
		degree = val + g_kleds_pwm;    //���ﲻ������ж���
	else if((val < 0) && (g_kleds_pwm > 0))
		degree = val + g_kleds_pwm;   //���ﲻ������ж���
	else
		return;   //���ֲ���
	
	set_Kleds_pwm_out(degree);	
}

void kLedPWM_ToggleOut(void)
{
	static int8_t dir = 1; 
//	uint8_t data = 10;
	
	if(g_kleds_pwm >= 100)  //��С����
		dir = -1;
	else if(g_kleds_pwm ==0)  //������
		dir = 1;
	
	Kleds_pwm_change(10*dir);	

}


/*
	����lcdPWM��Ƶ��
*/
void setKleds_pwm_freq(uint16_t freq)
{
#ifdef KLEDS_PWM
	if(freq > 20)
	{
		KLEDS_PWM_HZ = freq;
		//TIM_SetAutoreload(TIM3,freq);   
		timer_autoreload_value_config(KLEDS_PWM_TIMER, freq);  //����Ƶ��
		
		MY_PRINTF("setKleds_pwm_freq freq = %d\r\n",freq);
	}
#endif	
}






/*********************************************************************************/


#ifdef LEDS_FLASH_TASK

static const uint8_t g_const_led_flash_time[4] = {5,8,10,20};

//����ĳ��led����˸
/*
	whichled ����λ��ʾ��˸������
			 ��6λ��ʾ�ĸ�����˸��[1-40]  ,ʵ����ЧֵΪ0-39�������м�������
*/
void light_leds_add_flash(uint8_t whichled)
{
//	printf("light_leds_add_flash whichled = %d\r\n",whichled);
	uint8_t flash_freq = whichled >> 6;  
	uint8_t i;
	whichled = (whichled & 0x3f) - 1;   //����λ��ʾĳ���ƣ�ԭ����ȡֵ��1-40������Ϊ0-39
		
	if(whichled < LEDS_NUM_MAX)
	{	
		timer_enable(TIMER1);	//������˸��ʱ��	
		
		if(whichled == 39)  //40-1��ʾȫ���ĵ�
		{		
			for(i=0;i<LEDS_NUM_MAX;i++)
			{
				//led ��Ҫ��˸��ʱ��ֵ��flash_freq:0��ʾ500ms��1��ʾ800ms��2��ʾ1s��3��ʾ2s��������ʾ����˸
				g_leds_flash_time[i] = flash_freq;//g_const_led_flash_time[flash_freq];
			}
			
		}
		else{	
			//led ��Ҫ��˸��ʱ��ֵ��flash_freq:0��ʾ500ms��1��ʾ800ms��2��ʾ1s��3��ʾ2s��������ʾ����˸			
			g_leds_flash_time[whichled] = flash_freq;//g_const_led_flash_time[flash_freq];
		}
	}
}
#endif





//100HZ��Ƶ�ʣ�10ms����һ��
//void leds_run_pwm_task(void)
//{
//	static uint16_t count = 0;
////	uint8_t i;
//		
//	
//	if(count <= PWM_HZ)
//	{		
//		//ֻ��ĳһ�����������������
//		if(g_led_pwm == count) //����ֵcount���趨ֵled_pwmҪ�󣬹ر�
//		{  //
//			led_pwm_pin_control(0);   //�����
//		}
//		else if(0 == count)  //��0�����
//			led_pwm_pin_control(1);  //�����
//	}
////	else
////	{
////		count = 0;   //һ�����ڽ��������¿�ʼ��һ������
////		return;   //�ո�����Ͳ���ȥ����
////	}
//	count++;
//	if(count>PWM_HZ)
//		count = 0;
//}






#if 0 //def LEDS_FLASH_TASK



//������˸���� //100ms����һ��
//void leds_flash_task(void)
//{
//	uint8_t i;
//	uint8_t c = 0;
//	
//	uint8_t stat[4] = {0};
//	
//	if(!g_leds_flash_control)
//		return;
//	
//	stat[0] = g_leds_flash_action & 1;
//	stat[1] = g_leds_flash_action & 2;   //0 ���߷�0 
//	stat[2] = g_leds_flash_action & 4;
//	stat[3] = g_leds_flash_action & 8;
//	
//	for(i=0;i<LEDS_NUM_MAX;i++)
//	{
//		if(g_leds_flash_time[i] < 4)  //ĳ������Ҫ��˸
//		{
//			if( g_leds_flash_time_already[g_leds_flash_time[i]] == 0)  //��תһ��
//			{
//				key_light_leds_control2(i,stat[ g_leds_flash_time[i] ]); //���� or Ϩ��		
//			}
//		}
//		else
//			c ++;   //��һ�»��ж��ٵƲ���Ҫ��˸
//	}
//	
//	if(c == LEDS_NUM_MAX)
//		g_leds_flash_control = 0;   //û�е���Ҫ��˸�ˡ�
//	
//	for(i=0;i<4;i++)
//	{
//		g_leds_flash_time_already[i]++;
//		if(g_leds_flash_time_already[i] >= g_const_led_flash_time[i])
//		{
//			g_leds_flash_time_already[i] = 0;   //�Ѿ���˸��ʱ��
//			g_leds_flash_action ^= 1<<i;    //״̬ȡ������ʾled״̬��ת
//		}
//	}
//}


void keyLeds_Flash_task(void* arg)
{
	uint8_t i;
	uint8_t c = 0;	
	uint8_t stat[4] = {0};
	
	while(1)
	{	
		//����һ����������������������
		if(!g_leds_flash_control)
			ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
		//	TaskHandle_leds_Flash
	//		return;
	
		stat[0] = g_leds_flash_action & 1;
		stat[1] = g_leds_flash_action & 2;   //0 ���߷�0 
		stat[2] = g_leds_flash_action & 4;
		stat[3] = g_leds_flash_action & 8;
		
		for(i=0;i<LEDS_NUM_MAX;i++)
		{
			if(g_leds_flash_time[i] < 4)  //ĳ������Ҫ��˸
			{
				if( g_leds_flash_time_already[g_leds_flash_time[i]] == 0)  //��תһ��
				{
					key_light_leds_control2(i,stat[ g_leds_flash_time[i] ]); //���� or Ϩ��		
				}
			}
			else
				c ++;   //��һ�»��ж��ٵƲ���Ҫ��˸
		}
		
		
		
		for(i=0;i<4;i++)
		{
			g_leds_flash_time_already[i]++;
			if(g_leds_flash_time_already[i] >= g_const_led_flash_time[i])
			{
				g_leds_flash_time_already[i] = 0;   //�Ѿ���˸��ʱ��
				g_leds_flash_action ^= 1<<i;    //״̬ȡ������ʾled״̬��ת
			}
		}
		vTaskDelay(pdMS_TO_TICKS(100));  //��ʱ  pdMS_TO_TICKS(10)
		
		if(c == LEDS_NUM_MAX)
			g_leds_flash_control = 0;   //û�е���Ҫ��˸�ˡ�
	}
}


#endif



#if 1

//2022-12-12 ����timer1 �����ж��źţ������Ƽ�����˸
static void TIM1_Led_Pwm_Init(uint16_t arr,uint16_t psc)
{
	timer_parameter_struct initpara;
	//���ղ���
	rcu_periph_clock_enable(RCU_TIMER1);  //��ʱ��ģ��ʱ��ʹ��

	//3. ��ʼ����ʱ�������ݽṹ  /* initialize TIMER init parameter struct */
	timer_struct_para_init(&initpara);
	initpara.period = arr;  //���ص����֣�
	initpara.prescaler = psc;  //Ԥ��Ƶ�����õ���1Mhz����
	//4. ��ʼ����ʱ��      /* initialize TIMER counter */
	timer_init(TIMER1, &initpara);

	timer_disable(TIMER1);
	timer_interrupt_flag_clear(TIMER1,TIMER_INT_FLAG_UP);	
	
	nvic_irq_enable(TIMER1_IRQn, 7U, 0U);  //����ֻ����ռ���ȼ��ˡ�
	timer_interrupt_enable(TIMER1, TIMER_INT_UP);   //��ʱ�ж�		
}


//��ʱ��1���жϴ���ÿ����1��100ms
void TIMER1_IRQHandler(void)
{
//	static uint8_t count = 0;
	uint8_t i;
	uint8_t c = 0;	
	uint8_t stat[4] = {0};

		
	if(timer_interrupt_flag_get(TIMER1,TIMER_INT_FLAG_UP)!=RESET)
	{	
		timer_interrupt_flag_clear(TIMER1,TIMER_INT_FLAG_UP);
		
//		count++;  //100ms �ѹ�ȥ			
		
		//����һ����������������������
//		if(!g_leds_flash_control)			
//		{
//			
//			return;
//		}
	
		//ledҪ��Ҫ��ת
		stat[0] = g_leds_flash_action & 1;  //500ms����˸��Ҫ��ת
		stat[1] = g_leds_flash_action & 2;   //0 ���߷�0 ��800ms����˸��Ҫ��ת
		stat[2] = g_leds_flash_action & 4;   //1000ms����Ҫ��ת
		stat[3] = g_leds_flash_action & 8;   //2000ms����Ҫ��ת
		
		//ѭ��ɨ�����е�led,ѭ��0-38
		for(i=0;i<LEDS_NUM_MAX-1;i++)
		{
			if(g_leds_flash_time[i] < 4)  //ĳ������Ҫ��˸�����ڵ���5�Ͳ���Ҫ��˸
			{
				//ĳ����˸��ʱ����Ҫ��ת��
				if( g_leds_flash_time_already[g_leds_flash_time[i]] == 0)  //���˾���Ҫ��תһ��
				{
					//��ĳ��led���е�������Ϩ�����
					key_light_leds_control2(i,stat[g_leds_flash_time[i]]); //���� or Ϩ��	
				//	Delay1us(50);					
				}
			}
			else
				c ++;   //��һ�»��ж��ٵƲ���Ҫ��˸
		}
				
		for(i=0;i<4;i++)
		{
			g_leds_flash_time_already[i]++;
			if(g_leds_flash_time_already[i] >= g_const_led_flash_time[i])
			{
				g_leds_flash_time_already[i] = 0;   //�Ѿ���˸��ʱ��
				g_leds_flash_action ^= 1<<i;    //״̬ȡ������ʾled״̬��ת
			}
		}
		//vTaskDelay(pdMS_TO_TICKS(100));  //��ʱ  pdMS_TO_TICKS(10)
		
		if(c == LEDS_NUM_MAX-1)
			timer_disable(TIMER1);  //�رն�ʱ��//g_leds_flash_control = 0;   //û�е���Ҫ��˸�ˡ�

		
	}
//	timer_interrupt_flag_clear(TIMER1,TIMER_INT_FLAG_UP);	
}
#endif




#if 0
//��ʱ���������߹ر� 1Ϊ������0Ϊ�ر�
static void Pwm_Timer_Control(uint8_t enable)
{
	if(enable)
	{
		//������ʱ��1
		timer_enable(TIMER1);
	}
	else
		timer_disable(TIMER1);
}


//�԰�������� led_PWM ���ŵĿ���
static void led_pwm_pin_control(uint8_t status)
{
	if(status)
		gpio_bit_set(GPIOE, GPIO_PIN_9);   //����ߵ�ƽ
	else
		gpio_bit_reset(GPIOE, GPIO_PIN_9);  //����͵�ƽ
}



//����led������ [0-100]
void set_Led_Pwm(uint8_t pwm)
{
	if(pwm > 100)
		pwm = 100;
	
	g_led_pwm = pwm/5;  //������0-20

	if(pwm == 100) //���ÿ���ʱ��
	{
		Pwm_Timer_Control(RESET);  //��ʱ���ر�
		led_pwm_pin_control(SET);  //���������
	}
	else //������ʱ��
	{
		Pwm_Timer_Control(SET);  //��ʱ������
	}
	
//	MY_PRINTF("set_Led_Pwm  g_led_pwm = %d\r\n",pwm);
}







//2022-09-16 ����timer1 �����ж��źţ����޸�pwm�����ֵ
//ֻ�����ڼ�ʱ����ʹ��tim1�ˣ�
static void TIM1_Led_Pwm_Init(uint16_t arr,uint16_t psc)
{
	timer_parameter_struct initpara;
	//���ղ���
	rcu_periph_clock_enable(RCU_TIMER1);  //��ʱ��ģ��ʱ��ʹ��

	//3. ��ʼ����ʱ�������ݽṹ  /* initialize TIMER init parameter struct */
	timer_struct_para_init(&initpara);
	initpara.period = arr;  //���ص����֣�
	initpara.prescaler = psc;  //Ԥ��Ƶ�����õ���1Mhz����
	//4. ��ʼ����ʱ��      /* initialize TIMER counter */
	timer_init(TIMER1, &initpara);

	nvic_irq_enable(TIMER1_IRQn, 7U, 0U);  //����ֻ����ռ���ȼ��ˡ�
	timer_interrupt_enable(TIMER1, TIMER_INT_UP);   //��ʱ�ж�	
}


//��ʱ��1���жϴ���Ŀǰ��pwm
void TIMER1_IRQHandler(void)
{
	static uint8_t count = 0;
	
	if(timer_interrupt_flag_get(TIMER1,TIMER_INT_FLAG_UP)!=RESET)
	{		
		count++;  //1ms �ѹ�ȥ			
		if(count <= LEDS_PWM_HZ)
		{
			//ֻ��ĳһ�����������������
			if(g_led_pwm == count) //����ֵcount���趨ֵled_pwmҪ�󣬹ر�
			{  //
				led_pwm_pin_control(0);   //�����
			}
			else if(1 == count)  //��0�����
				led_pwm_pin_control(1);  //�����
			}
		else 
		{
			count = 0;   //һ�����ڽ��������¿�ʼ��һ������
		//	return;   //�ո�����Ͳ���ȥ����
		}				
	}
	timer_interrupt_flag_clear(TIMER1,TIMER_INT_FLAG_UP);	
}






//100HZ��Ƶ�ʣ�10ms����һ��
//void laser_run_pwm_task(void)
//{
//	static uint16_t count = 0;
////	uint8_t i;
//		
//	
//	if(count <= PWM_HZ)
//	{		
//		//ֻ��ĳһ�����������������
//		if(g_led_pwm == count) //����ֵcount���趨ֵled_pwmҪ�󣬹ر�
//		{  //
//			led_pwm_pin_control(0);   //�����
//		}
//		else if(0 == count)  //��0�����
//			led_pwm_pin_control(1);  //�����
//	}
////	else
////	{
////		count = 0;   //һ�����ڽ��������¿�ʼ��һ������
////		return;   //�ո�����Ͳ���ȥ����
////	}
//	count++;
//	if(count>PWM_HZ)
//		count = 0;
//}


#endif





