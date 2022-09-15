

#ifndef __GPIOS_H__
#define __GPIOS_H__

#include <gd32f10x.h>

//gpio ���ų�ʼ��
void Gpios_init(void);


//void Wxen_Control_Init(void);
void Wxen_Control_Enable(void);
//��ֹ�����ţ��ϵ�1.1v
void Wxen_Control_Disable(void);


//void OePins_Control_Init(void);
//which 1-4 �ֱ��ʾoe1-oe4
void OePins_Output_Hight(uint8_t which);
//which 1-4 �ֱ��ʾoe1-oe4
void OePins_Output_Low(uint8_t which);


//PE15����ʼ���������
//void LcdCtrl_Control_Init(void);
//ʹ�ܸ����ţ�ͨ��Lcd��Դ
void LcdCtrl_Enable(void);
//��ֹ�����ţ��ϵ�Lcd��Դ
void LcdCtrl_Disable(void);



//PD8  lcd-reset ,��֪���Ǹߵ�ƽ��λ���ǵ͵�ƽ��λ
//�ȼ���͵�ƽ��λ�ɡ�
//void lcd_reset_control_init(void);
//����һ��lcd�ĸ�λ�ź�
void lcd_reset_control(void);



//PD6  MicCtl �������(����status ��0����ߣ�0�����)
void MicCtl_Control_OutHigh(uint8_t status);

//PD6  MicCtl
//void MicCtl_Control_Init(void);

#endif

