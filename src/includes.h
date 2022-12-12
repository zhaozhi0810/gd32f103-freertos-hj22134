
#ifndef INCLUDES_H
#define INCLUDES_H

//用于总的头文件包含

#include <gd32f10x.h>
#include <stdio.h>
//#include <stdarg.h>
#include <limits.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <atomic.h>
#include <event_groups.h>
#include <stream_buffer.h>

//功能控制宏定义：
#define LCD_PWM   //lcd亮度控制，使用pwm的方式，主要针对7寸屏的控制，5寸屏不受gd32单片机控制
//#define LCD_PWM_HEAT   //LCD使用pwm加热，注释该宏表示不使用pwm
#define BTNS_USE_INT   //按键扫描使用中断方式
//#define HWTD_USE_INT   //硬件看门狗使用外部中断，下降沿电平触发
#define KLEDS_PWM  //键灯亮度控制使用pwm方式，2022-10-10

#define HWTD_USE_INT   //硬件看门狗使用外部中断，下降沿电平触发
#define LEDS_FLASH_TASK  //led键灯的闪烁由单片机控制,改为定时器控制

extern const char* g_build_time_str;

#define DEBUG_COM_NUM 0   //调试串口号
#define TOCPU_COM_NUM 1   //与cpu通信的串口


#define UNUSED(x)  

#include "iic_app.h"
//#include "sys.h"
#include "systick.h"     //延时函数
#include "uart.h"        //串口处理
#include "gpios.h"       //高低电平控制的
//#include "gd32f10x_it.h"      //中断配置


#include "lcd_pwm.h"     //液晶屏亮度控制，主要针对7寸屏的控制，5寸屏不受gd32单片机控制

// #include "cpu_run_states.h"  //cpu上电运行状态的监控
// #include "power_btn.h"    //cpu上电复位，及开机按键处理
// #include "fan_control.h"   //风扇控制部分
// #include "vol_temp_control.h"   //电压温度读取，lcd加热控制等

//#include "task_cfg.h"    //任务相关的宏定义
#include "led_show_work.h"    //调试灯
// #include "di_4ttl.h"     //4路开关量输入
#include "uart_conect_cpu_handler.h"   //与cpu通信的串口接收发送处理
#include "uart_debug_handle.h"        //调试串口的接收处理
// #include "optica_switch_status.h"        //调试串口的接收处理
// #include "hard_wtg.h"     //硬件看门狗
// #include "msata_destroy.h"     //硬件销毁
// #include "power_data_iic.h"     //获取电源iic数据

#include "lt9211.h"
#include "matrix_keys.h"
#include "nca9555.h"     //沿用之前的项目中的。
#include "key_light.h"   //按键灯的控制

//#include "flash_record.h"   //内部flash操作
#include "internal_temp.h"   //单片机温度采集
#include "hard_wtd.h"     //外部硬件看门狗

#include "my_task_info.h"
#endif

