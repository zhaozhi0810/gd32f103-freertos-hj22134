


#include "includes.h"


/*
	��ȡ��Ƭ���ڲ��¶�
*/


static short g_int_temp = 0;  //�¶�ֵ��������100��





//��ʼ��ADC
//�������ǽ��Թ���ͨ��Ϊ��
//����Ĭ�Ͻ�����ͨ��0~3	
static void ADC_Init(void)  //ADCͨ����ʼ��
{
    // ADCʱ��ʹ��
    rcu_periph_clock_enable(RCU_ADC0);
    // ADCʱ��8��Ƶ�����14MHz
    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV8);
    
  
    /*------------------ADC����ģʽ����------------------*/
    // ֻʹ��һ��ADC,���ڶ���ģʽ
    adc_mode_config(ADC_MODE_FREE);
    // ��ͨ����ɨ��ģʽ
    adc_special_function_config(ADC0, ADC_SCAN_MODE, DISABLE);
    // ��ͨ��������ת��ģʽ
    adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, DISABLE);
    
	adc_discontinuous_mode_config(ADC0, ADC_REGULAR_CHANNEL, 1);   //����ģʽ��1��ͨ��
	
    // ���ת���Ҷ���
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
    // ת��ͨ��1��
    adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 1);
    
	//�ڲ��¶�ͨ��
	adc_tempsensor_vrefint_enable();  //�¶�ͨ������
	adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_16, ADC_SAMPLETIME_55POINT5);
//	pc3_adc_test_init();
	// ����ADCͨ��ת��˳�򣬲���ʱ��Ϊ55.5��ʱ������
//	adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_13, ADC_SAMPLETIME_55POINT5);
	
    // �����ⲿ����ת���������������
    adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE);
//    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);
    
    // ʹ��ADC
    adc_enable(ADC0);
    vTaskDelay(1);      // �ȴ�1ms
    // ʹ��ADCУ׼
    adc_calibration_enable(ADC0);
  
    // ����û�в����ⲿ����������ʹ���������ADCת��
    adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL); 	
}



static uint16_t T_Get_Adc(uint8_t ch)   
{
    uint16_t adcValue = 0;

    adc_flag_clear(ADC0, ADC_FLAG_EOC);                             // ���������־
    
    adcValue = adc_regular_data_read(ADC0);                         // ��ȡADC����

     // ����ADCͨ��ת��˳�򣬲���ʱ��Ϊ55.5��ʱ������
    adc_regular_channel_config(ADC0, 0, ch, ADC_SAMPLETIME_55POINT5);
    // ����û�в����ⲿ����������ʹ���������ADCת��
    adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);  

	return adcValue;

}

//�õ�ADC�����ڲ��¶ȴ�������ֵ
//ȡ10��,Ȼ��ƽ��
//static uint16_t T_Get_Temp(void)
//{
//	uint16_t temp_val=0;
//	uint8_t t;
//	for(t=0;t<10;t++)
//	{
//		temp_val+=T_Get_Adc(ADC_CHANNEL_16);	  //TampSensor
//		Delay1ms(5);
//	}
//	return temp_val/10;
//}

 //��ȡͨ��ch��ת��ֵ
//ȡtimes��,Ȼ��ƽ��
static uint16_t T_Get_Adc_Average(uint8_t ch,uint8_t times)
{
	uint32_t temp_val=0;
	uint8_t t;
	for(t=0;t<times;t++)
	{
		temp_val+=T_Get_Adc(ch);
		vTaskDelay(5);
	}
	return temp_val/times;
} 	   

//�õ��¶�ֵ
//����ֵ:�¶�ֵ(������100��,��λ:��.)
static short Get_Temprate(void)	//��ȡ�ڲ��¶ȴ������¶�ֵ
{
	uint32_t adcx;
	short result;
 	double temperate;
	adcx=T_Get_Adc_Average(ADC_CHANNEL_16,2);	//��ȡͨ��16,20��ȡƽ��
	temperate=(float)adcx*(3.3/4096);		//��ѹֵ 
	temperate=(1.43-temperate)/0.0043+10;	//ת��Ϊ�¶�ֵ 	 
	result=temperate*=100;					//����100��.
	return result;
}




//����ӿڣ���ȡ�¶�ֵ
short get_internal_temp(void)
{
	MY_PRINTF("get_internal_temp  g_int_temp = %d\r\n",g_int_temp);	
	return g_int_temp;
}






//��ص�ѹ�������,1000ms
void Inter_Temp_task(void* arg)
{
	ADC_Init();  //��ʼ��
		
	while(1)
	{
		g_int_temp = Get_Temprate();   //����¶�ֵ(������100��,��λ:��.)
		vTaskDelay(1000);
	}
}



