


#ifndef __INTENAL_TEMP_H__
#define __INTENAL_TEMP_H__

#include <gd32f10x.h>


//void ADC_Init(void);
//uint16_t ADC_Read(uint8_t channel);


/*
	获取单片机温度
*/
//uint16_t ADCgetIntTemp(void);


//单片机温度检测任务,500ms进入一次
//void Int_Temp_task(void);


short get_internal_temp(void);


//电池电压检测任务,1000ms
void Inter_Temp_task(void* arg);
#endif


