

#ifndef __KEY_LIGHT_H__
#define __KEY_LIGHT_H__

#include <gd32f10x.h>

void key_light_leds_init(void);

/*
	whichled 1-36 �ֱ��Ӧ�����ĵ�
			 
	status   0 ��ʾϨ��
			 ��0��ʾ����
*/
void key_light_leds_control(uint8_t whichled,uint8_t status);


//�������е�led
void key_light_allleds_control(uint8_t status);



//���ĳһ���Ƶ�״̬
uint8_t get_led_status(uint8_t whichled);



//����led������ [0-100]
void set_Led_Pwm(uint8_t pwm);
#endif

