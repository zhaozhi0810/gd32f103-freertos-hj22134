

/*
	2021-09-26
	��ʮһ����ֲ���ļ���
	
	7��LCDPWM����Ҫʹ�ö�ʱ��0CH2N��  GPB15   ��GD32��ʱ����0��ʼ��ţ�stm32��1��ʼ��ţ�

	��Ҫ��
	1.pwm����LCD����  PB15
	2.�����Դʹ������ PB14
	//�ƺ�ֻ�ܿ���7������2022-07-28  ��5������Ч

*/

#include "includes.h"

	
static uint16_t PWM_DEGREE_MAX = 100;   //PWMƵ��  1us�Ƹ�����4000�����Ƶ�ʾ���1000000/4000=250Hz ̫����Ӱ���ż���ʵ�飿����
uint8_t g_lcd_pwm = 100;
static int8_t gs_LcdType = -1;   //0 ��ʾ5inch��1��ʾ7inch��-1������δ֪��2022-12-13
//PB14 timer11 ch0,timer0 ch1_ON
//PB15 timer11 ch1,timer0 ch2_ON

#define LCD_PWM_POWER_VOL_PORT GPIO_PIN_14
#define LCD_PWM_PIN GPIO_PIN_15      //LCD_PWM1  7������
#define LCD_PWM_PORT GPIOB
#define LCD_PWM_PORT_RCU RCU_GPIOB
#define LCD_PWM_TIMER_RCU  RCU_TIMER0    //20220714 ����Ϊtimer0��ʾ�����Ѿ��������Σ����ڵ�������
#define LCD_PWM_TIMER  TIMER0
#define LCD_PWM_TIMER_CH TIMER_CH_2

#define LCD_PWM_TIMER_USE_CHN   //ʹ�û���ͨ����

//uint8_t g_Lcd_Power_status = 0;   //δ�ӵ�
//uint8_t g_Lcd_Pd_N_status = 0;
//static uint8_t g_lcd_pwm_poweroff_save=0;


void lcd_pwm_init(uint8_t degree)
{
	rcu_periph_clock_enable(LCD_PWM_PORT_RCU);   //�˿ڵ�ʱ��ʹ��
#ifndef LCD_PWM	
	//uint32_t pin;
	//rcu_periph_clock_enable(RCU_GPIOB);
	//2. ����Ϊ���ģʽ	
	//pin = BIT(GPIO_PIN_0);    //PB0��PB1 ͬʱ����Ϊ���
	
	gpio_init(LCD_PWM_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, LCD_PWM_PIN);
	
	if(degree > 0)
	{
		gpio_bit_set(LCD_PWM_PORT, LCD_PWM_PIN);
	}
	else
	{
		gpio_bit_reset(LCD_PWM_PORT, LCD_PWM_PIN);
	}
	
#else		
	//PB15 ͨ��
	timer_parameter_struct initpara;
	timer_oc_parameter_struct ocpara;
	//1. io�������ø��ù���	
	gpio_init(LCD_PWM_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_2MHZ, LCD_PWM_PIN);   //���ù���	
	//PB15 Ĭ��ӳ��tim0 ch2��Nͨ������0��ʼ����
	//gpio_pin_remap_config(GPIO_TIMER2_PARTIAL_REMAP, ENABLE);    //����ӳ��
	
	//2. ��ʱ��ʱ��ʹ��
	rcu_periph_clock_enable(LCD_PWM_TIMER_RCU);  //��ʱ��ģ��ʱ��ʹ��
		
	if(degree > 100)
	{
		degree = 100;
	}
	g_lcd_pwm = degree;   //���浽ȫ�ֱ����С�
	
	//3. ��ʼ����ʱ�������ݽṹ  /* initialize TIMER init parameter struct */
	timer_struct_para_init(&initpara);
	initpara.period = PWM_DEGREE_MAX-1;  //���ص����֣�Ƶ��20kHZ
	initpara.prescaler = (SystemCoreClock/1000000)-1;  //Ԥ��Ƶ�����õ���1Mhz������  
		
	//4. ��ʼ����ʱ��      /* initialize TIMER counter */
	timer_init(LCD_PWM_TIMER, &initpara);
		
	//5. ��ʼ����ʱ��ͨ�������ݽṹ /* initialize TIMER channel output parameter struct */
	timer_channel_output_struct_para_init(&ocpara);	
#ifndef LCD_PWM_TIMER_USE_CHN
	ocpara.outputstate  = TIMER_CCX_ENABLE;  //���ͨ��ʹ��	
#else
    ocpara.outputnstate = TIMER_CCXN_ENABLE;//ʹ�ܻ���ͨ�����
#endif
	//6. ��ʼ����ʱ��ͨ��   /* configure TIMER channel output function */
	timer_channel_output_config(LCD_PWM_TIMER, LCD_PWM_TIMER_CH, &ocpara);
			
	//7. ��ʼ����ʱ��ͨ�������ʽ����   /* configure TIMER channel output compare mode */
	timer_channel_output_mode_config(LCD_PWM_TIMER, LCD_PWM_TIMER_CH, TIMER_OC_MODE_PWM1);
	/* configure TIMER channel output pulse value */
	
	//8. ��ʼ����ʱ��ͨ�����������
	timer_channel_output_pulse_value_config(LCD_PWM_TIMER, LCD_PWM_TIMER_CH, (100-degree) * PWM_DEGREE_MAX/100);
//	timer_channel_complementary_output_state_config
//timer_channel_complementary_output_polarity_config
	//9. ��ʼ����ʱ��ͨ�����ʹ��
	//timer_channel_output_fast_config(TIMER2, TIMER_CH_0, TIMER_OC_FAST_ENABLE);
	timer_channel_output_shadow_config(LCD_PWM_TIMER, LCD_PWM_TIMER_CH, TIMER_OC_SHADOW_DISABLE);	  //stm32�ƺ��õ������0x8
	//10.��ʼ������ʱ����ʹ�� 2022-04-18	
	
	
	/* enable a TIMER */
	timer_primary_output_config(TIMER0, ENABLE);
	timer_auto_reload_shadow_enable(TIMER0);
//	timer_enable(TIMER0);
#endif

	//*******����Ϊ����ʹ�ܿ��ơ�	
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, LCD_PWM_POWER_VOL_PORT);
	gpio_bit_reset(GPIOB, LCD_PWM_POWER_VOL_PORT);   //��ʼ��������ߵ�ƽ��Ĭ�ϵ�������

	//��ֹ������ʾ��ͨ��ǰһ��ʱ�䣬��Ļ��ʾ���ơ�
	//Disable_LcdLight();
}



//����LCD��Դ
void Enable_Lcd_Power(void)
{
	LcdCtrl_Enable();  //pe15
	gpio_bit_set(GPIOB, LCD_PWM_POWER_VOL_PORT);  //PB14
}

//�ر�LCD��Դ
void Disable_Lcd_Power(void)
{
	LcdCtrl_Disable();  //pe15
	gpio_bit_reset(GPIOB, LCD_PWM_POWER_VOL_PORT);   //PB14
}


uint8_t Get_Lcd_Power_Status(void)
{
	return gpio_output_bit_get(GPIOB, LCD_PWM_POWER_VOL_PORT);	
}


////����ttlת��ʹ��
//void Enable_Lcd_PdN(void)
//{
//	gpio_bit_set(GPIOC, GPIO_PIN_14);
//}

////�ر�ttlת��ʹ��
//void Disable_Lcd_PdN(void)
//{
//	gpio_bit_reset(GPIOC, GPIO_PIN_14);
//}

//uint8_t Get_Lcd_PdN_Status(void)
//{
//	return gpio_output_bit_get(GPIOC, GPIO_PIN_14);	
//}




//�������⣨������Դ��ttlת�������⣬��ʱ��ȫ��������
void Enable_LcdLight(void)
{	
	if(gs_LcdType < 0)
	{
		gs_LcdType = Get_Lcd_Type(); //�����Ļ����1 ��7�磬0��5��
	}
	
	if(gs_LcdType == 0)  //5��Ŀ���
	{
		SHTDB_5IN_Control_SetOutVal(1);  //�����
		debug_printf_string("Enable_LcdLight 5inch\r\n");
	}	
	else  //7��Ŀ���
	{
	//	Enable_Lcd_PdN();     //lvds ת�����ܿ���	
		Enable_Lcd_Power();   //lcd��Դ����
		vTaskDelay(100);
	#ifndef LCD_PWM
		gpio_bit_set(LCD_PWM_PORT, LCD_PWM_PIN);  //����PWM����
	#else	
		Lcd_pwm_out(g_lcd_pwm==0?70:g_lcd_pwm);   //����ռ�ձ�70
		vTaskDelay(100);
		timer_enable(LCD_PWM_TIMER);   //������ʱ��
	#endif		
		vTaskDelay(100);
		//gpio_bit_set(GPIOB, GPIO_PIN_14);   //��������ʹ��
		Enable_Lcd_Power();
		debug_printf_string("Enable_LcdLight\r\n");
	}
}


//�رձ��⣨������Դ��ttlת�������⣬��ʱ��ȫ���رգ�
void Disable_LcdLight(void)
{
	if(gs_LcdType < 0)
	{
		gs_LcdType = Get_Lcd_Type(); //�����Ļ����1 ��7�磬0��5��
	}
	
	if(gs_LcdType == 0)  //5��Ŀ���
	{
		SHTDB_5IN_Control_SetOutVal(0);  //����ر�
		debug_printf_string("Disable_LcdLight 5inch\r\n");
	}	
	else  //7��Ŀ���
	{	
		
	//	gpio_bit_reset(GPIOB, GPIO_PIN_14);  //�رձ���ʹ��
	#ifndef LCD_PWM
		gpio_bit_reset(LCD_PWM_PORT, LCD_PWM_PIN);
	#else
		timer_channel_output_pulse_value_config(LCD_PWM_TIMER, LCD_PWM_TIMER_CH, PWM_DEGREE_MAX);   //�رձ���pwm
		vTaskDelay(100);
		timer_disable(LCD_PWM_TIMER);  //�رն�ʱ��
	#endif	
		
	//	Disable_Lcd_PdN();     //lvds ת�����ܹر�
		Disable_Lcd_Power();   //lcd��Դ�ر�
			
		debug_printf_string("Disable_LcdLight\r\n");
	}
}





/*
����lcd����ռ�ձ�
//degree �޸�Ϊ�����ɸ�
*/
void Lcd_pwm_out(int8_t degree)
{
#ifdef LCD_PWM
	uint32_t value;

//	if(PWM_DEGREE_MIN > degree) degree = PWM_DEGREE_MIN;
//	if(PWM_DEGREE_MAX < degree) degree = PWM_DEGREE_MAX;
	
	if(degree > 100)	
		degree = 100;	
	else if(degree < 0)
		degree = 0;
		
	g_lcd_pwm = degree; 
	MY_PRINTF("Lcd_pwm_out g_lcd_pwm = %d\r\n",g_lcd_pwm);   //��ӡһ����Ϣ�����Թر�
	
	value = (100-degree) * PWM_DEGREE_MAX / 100;
	/* CH configuration in PWM mode1,duty cycle  */
	//timer_channel_output_pulse_value_config(TIMER1,TIMER_CH_0,value);
	//TIM_SetCompare4(TIM3, value);
	timer_channel_output_pulse_value_config(LCD_PWM_TIMER, LCD_PWM_TIMER_CH, value);
#else
	if(degree > 0)
	{
		g_lcd_pwm = 10;   //10�ȽϺô���
		gpio_bit_set(LCD_PWM_PORT, LCD_PWM_PIN);
	}
	else
	{
		g_lcd_pwm = 0;
		gpio_bit_reset(LCD_PWM_PORT, LCD_PWM_PIN);
	}
#endif	
}


/*
����lcd����ռ�ձ�
//val Ϊ��Ҫ������ֵ��������������ʾ�������ӣ�����ʾ���ȼ�С
*/
void Lcd_pwm_change(int8_t val)
{
	int8_t degree ;
		
	if((val > 0) && (g_lcd_pwm<100))  //����0�����������ȣ���������ֵҲС��100
		degree = val + g_lcd_pwm;    //���ﲻ������ж���
	else if((val < 0) && (g_lcd_pwm > 0))
		degree = val + g_lcd_pwm;   //���ﲻ������ж���
	else
		return;   //���ֲ���
	
	Lcd_pwm_out(degree);	
}




/*
	����lcdPWM��Ƶ��
*/
void setLcd_pwm_freq(uint16_t freq)
{
#ifdef LCD_PWM
	if(freq > 20)
	{
		PWM_DEGREE_MAX = freq;
		//TIM_SetAutoreload(TIM3,freq);   
		timer_autoreload_value_config(LCD_PWM_TIMER, freq);  //����Ƶ��
		
		MY_PRINTF("setLcd_pwm_freq freq = %d\r\n",freq);
	}
#endif	
}
