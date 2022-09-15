

#ifndef __KEY_LIGHT_H__
#define __KEY_LIGHT_H__

#include <gd32f10x.h>

void key_light_leds_init(void);

/*
	whichled 1-36 分别对应按键的灯
			 
	status   0 表示熄灭
			 非0表示点亮
*/
void key_light_leds_control(uint8_t whichled,uint8_t status);


//控制所有的led
void key_light_allleds_control(uint8_t status);



//获得某一个灯的状态
uint8_t get_led_status(uint8_t whichled);



//设置led的亮度 [0-100]
void set_Led_Pwm(uint8_t pwm);
#endif

