


#include "includes.h"


/*
	获取单片机内部温度
*/


static short g_int_temp = 0;  //温度值，扩大了100倍





//初始化ADC
//这里我们仅以规则通道为例
//我们默认将开启通道0~3	
static void ADC_Init(void)  //ADC通道初始化
{
    // ADC时钟使能
    rcu_periph_clock_enable(RCU_ADC0);
    // ADC时钟8分频，最大14MHz
    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV8);
    
  
    /*------------------ADC工作模式配置------------------*/
    // 只使用一个ADC,属于独立模式
    adc_mode_config(ADC_MODE_FREE);
    // 多通道用扫描模式
    adc_special_function_config(ADC0, ADC_SCAN_MODE, DISABLE);
    // 单通道用连续转换模式
    adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, DISABLE);
    
	adc_discontinuous_mode_config(ADC0, ADC_REGULAR_CHANNEL, 1);   //规则模式下1个通道
	
    // 结果转换右对齐
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
    // 转换通道1个
    adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 1);
    
	//内部温度通道
	adc_tempsensor_vrefint_enable();  //温度通道开启
	adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_16, ADC_SAMPLETIME_55POINT5);
//	pc3_adc_test_init();
	// 配置ADC通道转换顺序，采样时间为55.5个时钟周期
//	adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_13, ADC_SAMPLETIME_55POINT5);
	
    // 不用外部触发转换，软件开启即可
    adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE);
//    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);
    
    // 使能ADC
    adc_enable(ADC0);
    vTaskDelay(1);      // 等待1ms
    // 使能ADC校准
    adc_calibration_enable(ADC0);
  
    // 由于没有采用外部触发，所以使用软件触发ADC转换
    adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL); 	
}



static uint16_t T_Get_Adc(uint8_t ch)   
{
    uint16_t adcValue = 0;

    adc_flag_clear(ADC0, ADC_FLAG_EOC);                             // 清除结束标志
    
    adcValue = adc_regular_data_read(ADC0);                         // 读取ADC数据

     // 配置ADC通道转换顺序，采样时间为55.5个时钟周期
    adc_regular_channel_config(ADC0, 0, ch, ADC_SAMPLETIME_55POINT5);
    // 由于没有采用外部触发，所以使用软件触发ADC转换
    adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);  

	return adcValue;

}

//得到ADC采样内部温度传感器的值
//取10次,然后平均
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

 //获取通道ch的转换值
//取times次,然后平均
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

//得到温度值
//返回值:温度值(扩大了100倍,单位:℃.)
static short Get_Temprate(void)	//获取内部温度传感器温度值
{
	uint32_t adcx;
	short result;
 	double temperate;
	adcx=T_Get_Adc_Average(ADC_CHANNEL_16,2);	//读取通道16,20次取平均
	temperate=(float)adcx*(3.3/4096);		//电压值 
	temperate=(1.43-temperate)/0.0043+10;	//转换为温度值 	 
	result=temperate*=100;					//扩大100倍.
	return result;
}




//对外接口，获取温度值
short get_internal_temp(void)
{
	MY_PRINTF("get_internal_temp  g_int_temp = %d\r\n",g_int_temp);	
	return g_int_temp;
}






//电池电压检测任务,1000ms
void Inter_Temp_task(void* arg)
{
	ADC_Init();  //初始化
		
	while(1)
	{
		g_int_temp = Get_Temprate();   //获得温度值(扩大了100倍,单位:℃.)
		vTaskDelay(1000);
	}
}



