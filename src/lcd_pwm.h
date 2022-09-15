
#ifndef __LCD_PWM_H__
#define __LCD_PWM_H__

#include <gd32f10x.h>

void lcd_pwm_init(uint8_t degree);

void Lcd_pwm_out(int8_t degree);

//用于上报设置值
extern uint8_t g_lcd_pwm;



//关闭背光
void Disable_LcdLight(void);

//开启背光
void Enable_LcdLight(void);

//获得lcd电源引脚的状态
uint8_t Get_Lcd_Power_Status(void);
//获得ttl转换使能的状态
uint8_t Get_Lcd_PdN_Status(void);


/*
调整lcd亮度占空比
//val 为需要调整的值，可正负，正表示亮度增加，负表示亮度减小
*/
void Lcd_pwm_change(int8_t val);

#endif

