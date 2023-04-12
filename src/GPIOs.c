
#include "includes.h"

//TaskHandle_t  TaskHandle_Morseptt;   //���morseptt��������ָ��


//PD7 
//static void Wxen_Control_Init(void)
//{
//	//1. ʱ��ʹ��
//	rcu_periph_clock_enable(RCU_GPIOD);
//		
//	//2.0 �ϵ��������
//	gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_7);  //�������	
//	//2. ��ʼ����Ĭ�������
//	gpio_bit_reset(GPIOD, GPIO_PIN_7);
//	
//}


//PD6  MicCtl
static void MicCtl_Control_Init(void)
{
	//1. ʱ��ʹ��
	rcu_periph_clock_enable(RCU_GPIOD);
		
	//2.0 �ϵ��������
	gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_6);  //�������	
	//2. ��ʼ����Ĭ�������
	gpio_bit_reset(GPIOD, GPIO_PIN_6);	
}





////PD6  MicCtl�����
//void MicCtl_Control_OutLow(void)
//{
//	gpio_bit_reset(GPIOD, GPIO_PIN_6);
//}



#if 0
//PD10  MorsePtt,��������
static void MorsePtt_Control_Init(void)
{
	//1. ʱ��ʹ��
	rcu_periph_clock_enable(RCU_GPIOD);
		
	//2.0 �ϵ��������
	gpio_init(GPIOD, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, GPIO_PIN_10);  //�������	
	//2. ��ʼ����Ĭ�������
	//gpio_bit_reset(GPIOD, GPIO_PIN_6);	
		
	//3 ����Ϊ�ⲿ�ж����ţ�
	gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOD, GPIO_PIN_SOURCE_10);
	//���ô�����ʽ��˫���ش���
	exti_init(EXTI_10, EXTI_INTERRUPT, EXTI_TRIG_BOTH);	
	exti_interrupt_flag_clear(EXTI_10);
	exti_interrupt_enable(EXTI_10);
	//3.1 nvic�����ж�
	//�жϿ�����ʹ�ܣ�ʹ�õ����ⲿ�ж�12
	//nvic_irq_enable(EXTI10_15_IRQn,  7, 0);   //�����жϣ����������ȼ�	
}



//�ⲿ�ж�10�Ĵ�����,�������º��ɿ����ᴥ���жϣ�������
void exint10_handle(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	
	if(gpio_input_bit_get(GPIOD, GPIO_PIN_10))  //�ߵ�ƽ�����ǰ����Ͽ�
	{
		V12_CTL_Control_SetOutVal(0);  //v12 ��ѹ��ֹ���
	}
	else  //�͵�ƽ������������
	{
		xTaskNotifyFromISR(TaskHandle_Morseptt, 0, eIncrement, &xHigherPriorityTaskWoken);  //�������ߵ�����	
	}
	//���ҽ�ֹ�ж�
	//exti_interrupt_disable(EXTI_10);   //ɨ�����֮����ʹ��
}




//morse ptt �������
void task_morse_ptt_scan(void* arg)
{	
	MorsePtt_Control_Init();
	while(1)
	{	
		//�ȴ����񱻻���
		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);   //��1��Ȼ�����޵ȴ�
		{	
			vTaskDelay(2000);    //��ʱ3s
			V12_CTL_Control_SetOutVal(1);  //v12 ��ѹʹ�����
		}		
	}
}


#endif








//PD8  lcd-reset ,��֪���Ǹߵ�ƽ��λ���ǵ͵�ƽ��λ
//�ȼ���͵�ƽ��λ�ɡ�
static void lcd_reset_control_init(void)
{
	//1. ʱ��ʹ��
	rcu_periph_clock_enable(RCU_GPIOD);
		
	//2.0 �ϵ��������
	gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_8);  //�������	
	//2. ��ʼ����Ĭ�������
	gpio_bit_set(GPIOD, GPIO_PIN_8);	
}



//OE���ŵĳ�ʼ�� 
/*
23.PC0 - OE1  ��·������ƣ�rk3399���Դ��ڣ�rk3399��gd32�Ĵ��ڣ�rk3399��Һ��ͨ�Ŵ��ڣ�SAI8159iic4�ӿ�
24.PC1 - OE2  ��·������ƣ�i2s��i2s0��CB_RESETn��WDT_OUT
25.PC2 - OE3  ��·������ƣ�iic3�ӿڣ�iic1�ӿڣ�GPO6��PCIE_WAKEn��LED_PWM_7IN
26.PC3 - OE4 ��·������ƣ�PTT��GPI5��MIC_DET��PTT1��CODEC_GPI2��GPO4

OE3 �͵�ƽ����3������
*/
static void OePins_Control_Init(void)
{
	//1. ʱ��ʹ��
	rcu_periph_clock_enable(RCU_GPIOC);
		
	//2.0 �ϵ��������
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);  //�������	
	//2. ��ʼ����Ĭ�������
	gpio_bit_reset(GPIOC, GPIO_PIN_2);  //OE3 �����
	
	gpio_bit_set(GPIOC, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_3);  //���� �����
}


//PE15����ʼ���������
static void LcdCtrl_Control_Init(void)
{
	//1. ʱ��ʹ��
	rcu_periph_clock_enable(RCU_GPIOE);
		
	//2.0 �ϵ��������
	gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_15);  //�������	
	//2. ��ʼ����Ĭ�������
	gpio_bit_reset(GPIOE, GPIO_PIN_15);  //LcdCtrl �����

}



//static void Lcd7INCtrl_PwmPins_Init(void)
//{
//	//1. ʱ��ʹ��
//	rcu_periph_clock_enable(RCU_GPIOB);
//		
//	//2.0 �ϵ��������
//	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_14|GPIO_PIN_15);  //�������	
//	//2. ��ʼ����Ĭ�������
//	gpio_bit_set(GPIOB, GPIO_PIN_14|GPIO_PIN_15);  //LcdCtrl �����

//}




//gpio ���ų�ʼ��
void Gpios_init(void)
{
//	Wxen_Control_Init();   //2022-09-15 ����ʼ������������ʹ��
	if(Get_Lcd_Type())  //Ϊ��0ʱ��ʾ7����
		lcd_pwm_init(70);
	
	MicCtl_Control_Init();   //MIC_CRL ���ŵĳ�ʼ��
	lcd_reset_control_init();  //lcd ��λ���ŵĳ�ʼ��
	OePins_Control_Init();    //OE���ŵĳ�ʼ��

	
	if(Get_Lcd_Type())  //Ϊ��0ʱ��ʾ7����
		LcdCtrl_Control_Init();   //7������Դ��������  		

//	LcdCtrl_Enable();   //7����lcd��Դͨ��
//	Enable_LcdLight();    //��7�����Ŀ����źţ�����ʹ�ܺͱ���pwm����,	

//	LcdCtrl_Control_Init();	    //
	//Lcd7INCtrl_PwmPins_Init();	
	
	LSPK_Control_Init();      //LSPK �̵�������  PA7
	V12_CTL_Control_Init();   //MORSE  12V �������
	LcdType_Control_Init();   //��Ļ���ͻ�ȡ���ų�ʼ��
	
	SHTDB_5IN_Control_Init();  //5�����ı���ʹ��,2022-12-13
	
	//��Ƶ���ƣ��͵�ƽ��Ч���ߵ�ƽ��ֹ��2022-12-13�����Բ�ʹ��
	SPKEN_Control_Init();
	EAR_L_EN_Control_Init();
	EAR_R_EN_Control_Init();
	
}





//PD6  MicCtl �������(����status ��0����ߣ�0�����)
void MicCtl_Control_SetOutVal(uint8_t status)
{
	//debug_printf_string("MicCtl_Control_OutHigh\r\n");
	MY_PRINTF("MicCtl_Control_SetOutVal status = %d\r\n",status);
	if(status)
		gpio_bit_set(GPIOD, GPIO_PIN_6);
	else
		gpio_bit_reset(GPIOD, GPIO_PIN_6);
}



//����һ��lcd�ĸ�λ�ź�
void lcd_reset_control(void)
{
	gpio_bit_reset(GPIOD, GPIO_PIN_8);
	vTaskDelay(100);
	gpio_bit_set(GPIOD, GPIO_PIN_8);
}







//ʹ�ܸ����ţ�ͨ��7��Lcd��Դ
void LcdCtrl_Enable(void)
{
	gpio_bit_set(GPIOE, GPIO_PIN_15);  //�ߵ�ƽʹ��
}

//��ֹ�����ţ��ϵ�Lcd��Դ
void LcdCtrl_Disable(void)
{
	gpio_bit_reset(GPIOE, GPIO_PIN_15);  //LcdCtrl �����
}


//ʹ�ܸ����ţ�ͨ��1.1v
void Wxen_Control_Enable(void)
{
	gpio_bit_set(GPIOD, GPIO_PIN_7);  //�ߵ�ƽʹ��
}


//��ֹ�����ţ��ϵ�1.1v
void Wxen_Control_Disable(void)
{
	gpio_bit_reset(GPIOD, GPIO_PIN_7);   //�͵�ƽ��Ч
}


//which 1-4 �ֱ��ʾoe1-oe4
void OePins_Output_Hight(uint8_t which)
{
	if(which >0 && which < 5)
	{
		gpio_bit_set(GPIOC, BIT(which-1));  //�����
	}	
}


//which 1-4 �ֱ��ʾoe1-oe4
void OePins_Output_Low(uint8_t which)
{
	if(which >0 && which < 5)
	{
		gpio_bit_reset(GPIOC, BIT(which-1));  //�����
	}
}

//PC5  5�米��ʹ�ܣ�
//2022-12-13 ����
void SHTDB_5IN_Control_Init(void)
{
	//1. ʱ��ʹ��
	rcu_periph_clock_enable(RCU_GPIOC);
		
	//2. ��ʼ����Ĭ�������
	gpio_bit_reset(GPIOC, GPIO_PIN_5);	
	
	//3 �ϵ��������
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_5);  //�������	
}

//�ߵ�ƽ���� 5inch lcd
void SHTDB_5IN_Enable(void)
{	
	gpio_bit_set(GPIOC, GPIO_PIN_5);		
}

//�͵�ƽϨ�� 5inch lcd
void SHTDB_5IN_Disable(void)
{	
	gpio_bit_reset(GPIOC, GPIO_PIN_5);		
}


////PC5  SHTDB_5IN �������(����status ��0����ߵ�ƽ����5inch��0�����Ϩ�� 5inch lcd)
void SHTDB_5IN_Control_SetOutVal(uint8_t status)
{
	MY_PRINTF("SHTDB_5IN_Control_SetOutVal status = %d\r\n",status);
	if(status)
		SHTDB_5IN_Enable();
	else
		SHTDB_5IN_Disable();
}

//��ת
void SHTDB_5IN_Control_ToggleOut(void)
{
	uint8_t status = gpio_output_bit_get(GPIOC, GPIO_PIN_5);
	SHTDB_5IN_Control_SetOutVal(!status);
	
//	MY_PRINTF("V12_CTL_Control(MORSE) output status = %d\r\n",!status);
}




//PC6  V12_CTL
void V12_CTL_Control_Init(void)
{
	//1. ʱ��ʹ��
	rcu_periph_clock_enable(RCU_GPIOC);
		
	//2. ��ʼ����Ĭ������ͣ�2022-12-19�ĵġ�
	gpio_bit_reset(GPIOC, GPIO_PIN_6);	
	
	//3 �ϵ��������
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_6);  //�������		
}


void V12_CTL_Enable(void)
{	
	//1. ��ʼ����Ĭ������ͣ��̵�������
	gpio_bit_set(GPIOC, GPIO_PIN_6);		
}

void V12_CTL_Disable(void)
{	
	//1. ��ʼ����Ĭ�������
	gpio_bit_reset(GPIOC, GPIO_PIN_6);		
}


////PC6  V12_CTL �������(����status ��0����ߣ�0�����)
void V12_CTL_Control_SetOutVal(uint8_t status)
{
	MY_PRINTF("V12_CTL_Control(MORSE)_OutHigh status = %d\r\n",status);
	if(status)
		gpio_bit_set(GPIOC, GPIO_PIN_6);
	else
		gpio_bit_reset(GPIOC, GPIO_PIN_6);
}

//��ת
void V12_CTL_Control_ToggleOut(void)
{
	uint8_t status = gpio_output_bit_get(GPIOC, GPIO_PIN_6);
	V12_CTL_Control_SetOutVal(!status);
	
//	MY_PRINTF("V12_CTL_Control(MORSE) output status = %d\r\n",!status);
}



//PC8,9,10,��������
void LcdType_Control_Init(void)
{
	//1. ʱ��ʹ��
	rcu_periph_clock_enable(RCU_GPIOC);		
	//2.0 ����ģʽ
	gpio_init(GPIOC, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 );  //�������	

}


//��ȡ��������
/*����ֵ
1�������ն˼�������
2���ڹҢ����ն˼������ͣ������ģ�����
4��Ƕ��ʽ��/��/���͡��������͡��ڹҢ����ն˼�������
6���๦�����ն˼�������(ֻ�������7��������������5��)
*/
uint8_t get_LcdType_val(void)
{
	uint8_t val = (gpio_input_port_get(GPIOC) >> 8) & 0x7;  //ֻҪPC8��9��10
//	MY_PRINTF("get_LcdType_val status = %d\r\n",val);
	return val;   //ֻҪPC8��9��10
}


//���lcd��Ļ�����ͣ�2022-09-21֮�������ĵװ壬ͨ���밴�����������������
//��ȡ���ŵĵ�ƽ���ж�lcd�����ͣ�֮ǰ����3399�������жϵġ�
//����ֵ0��ʾ5��������0��ʾ7����
//2022-09-21 Ŀǰ��û���µĵװ������жϣ�Ĭ�Ϸ���0����ʾ5������
uint8_t Get_Lcd_Type(void)
{
	//get_LcdType_val() ����6 ��ʾ�Ƕ๦����壬��7������������Ϊ5�磬2022-10-10
	return (get_LcdType_val() == 6);   //2022-09-21 Ŀǰ��û���µĵװ������жϣ�Ĭ�Ϸ���0����ʾ5������
}


#if 1
//PA7  LSPK_CRL
//2022-12-13  ��PD5
void LSPK_Control_Init(void)
{
	//1. ʱ��ʹ��
	rcu_periph_clock_enable(RCU_GPIOD);
		
	//2. ��ʼ����Ĭ�������
	gpio_bit_reset(GPIOD, GPIO_PIN_5);	
	
	//3 �ϵ��������
	gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_5);  //�������		
}


void LSPK_Enable(void)
{	
	//1. ��ʼ����Ĭ������ͣ��̵�������
	gpio_bit_set(GPIOD, GPIO_PIN_5);		
}

void LSPK_Disable(void)
{	
	//1. ��ʼ����Ĭ�������
	gpio_bit_reset(GPIOD, GPIO_PIN_5);		
}


////PA7  LSPK �������(����status ��0����ߣ�0�����)
void LSPK_Control_SetOutVal(uint8_t status)
{
	MY_PRINTF("LSPK_Control_SetOutVal status = %d\r\n",status);
	if(status)
		gpio_bit_set(GPIOD, GPIO_PIN_5);
	else
		gpio_bit_reset(GPIOD, GPIO_PIN_5);
}

//��ת
void LSPK_Control_ToggleOut(void)
{
	uint8_t status = gpio_output_bit_get(GPIOD, GPIO_PIN_5);
	LSPK_Control_SetOutVal(!status);
	
//	MY_PRINTF("LSPK_Control output status = %d\r\n",!status);
}
#else
//PA7  LSPK_CRL
void LSPK_Control_Init(void)
{
	//1. ʱ��ʹ��
	rcu_periph_clock_enable(RCU_GPIOA);
		
	//2. ��ʼ����Ĭ�������
	gpio_bit_set(GPIOA, GPIO_PIN_7);	
	
	//3 �ϵ��������
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_7);  //�������		
}


void LSPK_Enable(void)
{	
	//1. ��ʼ����Ĭ������ͣ��̵�������
	gpio_bit_set(GPIOA, GPIO_PIN_7);		
}

void LSPK_Disable(void)
{	
	//1. ��ʼ����Ĭ�������
	gpio_bit_reset(GPIOA, GPIO_PIN_7);		
}


////PA7  LSPK �������(����status ��0����ߣ�0�����)
void LSPK_Control_SetOutVal(uint8_t status)
{
	MY_PRINTF("LSPK_Control_SetOutVal status = %d\r\n",status);
	if(status)
		gpio_bit_set(GPIOA, GPIO_PIN_7);
	else
		gpio_bit_reset(GPIOA, GPIO_PIN_7);
}

//��ת
void LSPK_Control_ToggleOut(void)
{
	uint8_t status = gpio_output_bit_get(GPIOA, GPIO_PIN_7);
	LSPK_Control_SetOutVal(!status);
	
//	MY_PRINTF("LSPK_Control output status = %d\r\n",!status);
}
#endif


#if 1
//PC11  ������ʹ��
//2022-12-13 ����
void SPKEN_Control_Init(void)
{
	//1. ʱ��ʹ��
	rcu_periph_clock_enable(RCU_GPIOC);
		
	//2. ��ʼ����Ĭ������ͣ���ʹ��
	gpio_bit_reset(GPIOC, GPIO_PIN_11);	
	
	//3 �ϵ��������
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_11);  //�������	
}

//�ߵ�ƽ���� 5inch lcd
void SPKEN_Enable(void)
{	
	gpio_bit_reset(GPIOC, GPIO_PIN_11);		
}

//�͵�ƽϨ�� 5inch lcd
void SPKEN_Disable(void)
{	
	gpio_bit_set(GPIOC, GPIO_PIN_11);		
}


////PC11  ������ʹ��
void SPKEN_Control_SetOutVal(uint8_t status)
{
	MY_PRINTF("SPKEN_Control_SetOutVal status = %d\r\n",status);
	if(status)
		SPKEN_Enable();
	else
		SPKEN_Disable();
}

//��ת
void SPKEN_Control_ToggleOut(void)
{
	uint8_t status = gpio_output_bit_get(GPIOC, GPIO_PIN_11);
	SPKEN_Control_SetOutVal(status);
}



//PC12  �����ʹ��
//2022-12-13 ����
void EAR_L_EN_Control_Init(void)
{
	//1. ʱ��ʹ��
	rcu_periph_clock_enable(RCU_GPIOC);
		
	//2. ��ʼ����Ĭ������ͣ���ʹ��
	gpio_bit_reset(GPIOC, GPIO_PIN_12);	
	
	//3 �ϵ��������
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_12);  //�������	
}

//�ߵ�ƽ���� 5inch lcd
void EAR_L_EN_Enable(void)
{	
	gpio_bit_reset(GPIOC, GPIO_PIN_12);		
}

//�͵�ƽϨ�� 5inch lcd
void EAR_L_EN_Disable(void)
{	
	gpio_bit_set(GPIOC, GPIO_PIN_12);		
}


////PC5  SHTDB_5IN �������(����status ��0����ߵ�ƽ����5inch��0�����Ϩ�� 5inch lcd)
void EAR_L_EN_Control_SetOutVal(uint8_t status)
{
	MY_PRINTF("EAR_L_EN_Control_SetOutVal status = %d\r\n",status);
	if(status)
		EAR_L_EN_Enable();
	else
		EAR_L_EN_Disable();
}

//��ת
void EAR_L_EN_Control_ToggleOut(void)
{
	uint8_t status = gpio_output_bit_get(GPIOC, GPIO_PIN_12);
	EAR_L_EN_Control_SetOutVal(status);
}


//PC13  �Ҷ���ʹ��
//2022-12-13 ����
void EAR_R_EN_Control_Init(void)
{
	//1. ʱ��ʹ��
	rcu_periph_clock_enable(RCU_GPIOC);
		
	//2. ��ʼ����Ĭ������ͣ���ʹ��
	gpio_bit_reset(GPIOC, GPIO_PIN_13);	
	
	//3 �ϵ��������
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_13);  //�������	
}

//�ߵ�ƽ���� 5inch lcd
void EAR_R_EN_Enable(void)
{	
	gpio_bit_reset(GPIOC, GPIO_PIN_13);		
}

//�͵�ƽϨ�� 5inch lcd
void EAR_R_EN_Disable(void)
{	
	gpio_bit_set(GPIOC, GPIO_PIN_13);		
}


////PC5  SHTDB_5IN �������(����status ��0����ߵ�ƽ����5inch��0�����Ϩ�� 5inch lcd)
void EAR_R_EN_Control_SetOutVal(uint8_t status)
{
	MY_PRINTF("EAR_R_EN_Control_SetOutVal status = %d\r\n",status);
	if(status)
		EAR_R_EN_Enable();
	else
		EAR_R_EN_Disable();
}

//��ת
void EAR_R_EN_Control_ToggleOut(void)
{
	uint8_t status = gpio_output_bit_get(GPIOC, GPIO_PIN_13);
	EAR_R_EN_Control_SetOutVal(status);
}

#endif

