


#ifndef __HARD_WTD_H__
#define __HARD_WTD_H__

#include <gd32f10x.h>


void hard_wtd_enable(void);
void hard_wtd_disable(void);

//喂狗
void hard_wtd_feed(void);

//初始化
void hard_wtd_pins_init(void);

//#ifdef 	HWTD_USE_INT
//外部中断12的处理函数,按键按下和松开都会触发中断！！！！
void exint4_handle(void);
//#endif



//获得看门狗的状态 1表示开启，0表示关闭
uint8_t get_hard_wtd_status(void);

//3399重启控制
void hard_wtd_reset_3399board(void);


//100ms进入一次就好 SGM706是1.6秒没有喂狗就会复位
//为了解决喂狗时间可以设置的问题，增加喂狗任务
//喂狗任务根据设置的时间喂狗，
//void hard_wtd_feed_task(void);
void hard_wtd_feed_task(void* arg);

//获得看门狗超时时间，单位100ms
uint8_t  hard_wtd_get_timeout(void);


//设置看门狗超时时间，单位100ms
void hard_wtd_set_timeout(uint8_t timeout);
#endif









