


#ifndef __INTENAL_TEMP_H__
#define __INTENAL_TEMP_H__

#include <gd32f10x.h>


//void ADC_Init(void);
//uint16_t ADC_Read(uint8_t channel);


/*
	��ȡ��Ƭ���¶�
*/
//uint16_t ADCgetIntTemp(void);


//��Ƭ���¶ȼ������,500ms����һ��
//void Int_Temp_task(void);


short get_internal_temp(void);


//��ص�ѹ�������,1000ms
void Inter_Temp_task(void* arg);
#endif


