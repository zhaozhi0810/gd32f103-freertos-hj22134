

#ifndef __KEY_LIGHT_H__
#define __KEY_LIGHT_H__

#include <gd32f10x.h>
#include "task.h"

extern TaskHandle_t  TaskHandle_leds_Flash;   //存放led_闪烁任务指针


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
//void set_Led_Pwm(uint8_t pwm);



/*
2022-10-10
设置lcd亮度占空比
//degree 修改为0-100
*/
void set_Kleds_pwm_out(uint8_t degree);


//pwm增大减小，方便串口调试控制
void kLedPWM_ToggleOut(void);


void light_leds_add_flash(uint8_t whichled);
void keyLeds_Flash_task(void* arg);

#endif

