

#ifndef __KEY_LIGHT_H__
#define __KEY_LIGHT_H__

#include <gd32f10x.h>
#include "task.h"

extern TaskHandle_t  TaskHandle_leds_Flash;   //���led_��˸����ָ��


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
//void set_Led_Pwm(uint8_t pwm);



/*
2022-10-10
����lcd����ռ�ձ�
//degree �޸�Ϊ0-100
*/
void set_Kleds_pwm_out(int8_t degree);


//pwm�����С�����㴮�ڵ��Կ���
void kLedPWM_ToggleOut(void);


void light_leds_add_flash(uint8_t whichled);
void keyLeds_Flash_task(void* arg);

#endif

