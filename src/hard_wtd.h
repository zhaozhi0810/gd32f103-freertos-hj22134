


#ifndef __HARD_WTD_H__
#define __HARD_WTD_H__

#include <gd32f10x.h>


void hard_wtd_enable(void);
void hard_wtd_disable(void);

//ι��
void hard_wtd_feed(void);

//��ʼ��
void hard_wtd_pins_init(void);

//#ifdef 	HWTD_USE_INT
//�ⲿ�ж�12�Ĵ�����,�������º��ɿ����ᴥ���жϣ�������
void exint4_handle(void);
//#endif



//��ÿ��Ź���״̬ 1��ʾ������0��ʾ�ر�
uint8_t get_hard_wtd_status(void);

//3399��������
void hard_wtd_reset_3399board(void);


//100ms����һ�ξͺ� SGM706��1.6��û��ι���ͻḴλ
//Ϊ�˽��ι��ʱ��������õ����⣬����ι������
//ι������������õ�ʱ��ι����
//void hard_wtd_feed_task(void);
void hard_wtd_feed_task(void* arg);

//��ÿ��Ź���ʱʱ�䣬��λ100ms
uint8_t  hard_wtd_get_timeout(void);


//���ÿ��Ź���ʱʱ�䣬��λ100ms
void hard_wtd_set_timeout(uint8_t timeout);

//��Ƭ�������Ź���ʱ����λ��2022-12-19����
void my_mcu_retart(void);
#endif









