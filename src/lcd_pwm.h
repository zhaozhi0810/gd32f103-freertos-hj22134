
#ifndef __LCD_PWM_H__
#define __LCD_PWM_H__

#include <gd32f10x.h>

void lcd_pwm_init(uint8_t degree);

void Lcd_pwm_out(int8_t degree);

//�����ϱ�����ֵ
extern uint8_t g_lcd_pwm;



//�رձ���
void Disable_LcdLight(void);

//��������
void Enable_LcdLight(void);

//���lcd��Դ���ŵ�״̬
uint8_t Get_Lcd_Power_Status(void);
//���ttlת��ʹ�ܵ�״̬
uint8_t Get_Lcd_PdN_Status(void);


/*
����lcd����ռ�ձ�
//val Ϊ��Ҫ������ֵ��������������ʾ�������ӣ�����ʾ���ȼ�С
*/
void Lcd_pwm_change(int8_t val);

#endif

