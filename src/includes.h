
#ifndef INCLUDES_H
#define INCLUDES_H

//�����ܵ�ͷ�ļ�����

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

//���ܿ��ƺ궨�壺
#define LCD_PWM   //lcd���ȿ��ƣ�ʹ��pwm�ķ�ʽ����Ҫ���7�����Ŀ��ƣ�5��������gd32��Ƭ������
//#define LCD_PWM_HEAT   //LCDʹ��pwm���ȣ�ע�͸ú��ʾ��ʹ��pwm
#define BTNS_USE_INT   //����ɨ��ʹ���жϷ�ʽ
//#define HWTD_USE_INT   //Ӳ�����Ź�ʹ���ⲿ�жϣ��½��ص�ƽ����
#define KLEDS_PWM  //�������ȿ���ʹ��pwm��ʽ��2022-10-10

#define HWTD_USE_INT   //Ӳ�����Ź�ʹ���ⲿ�жϣ��½��ص�ƽ����
#define LEDS_FLASH_TASK  //led���Ƶ���˸�ɵ�Ƭ������,��Ϊ��ʱ������

extern const char* g_build_time_str;

#define DEBUG_COM_NUM 0   //���Դ��ں�
#define TOCPU_COM_NUM 1   //��cpuͨ�ŵĴ���


#define UNUSED(x)  

#include "iic_app.h"
//#include "sys.h"
#include "systick.h"     //��ʱ����
#include "uart.h"        //���ڴ���
#include "gpios.h"       //�ߵ͵�ƽ���Ƶ�
//#include "gd32f10x_it.h"      //�ж�����


#include "lcd_pwm.h"     //Һ�������ȿ��ƣ���Ҫ���7�����Ŀ��ƣ�5��������gd32��Ƭ������

// #include "cpu_run_states.h"  //cpu�ϵ�����״̬�ļ��
// #include "power_btn.h"    //cpu�ϵ縴λ����������������
// #include "fan_control.h"   //���ȿ��Ʋ���
// #include "vol_temp_control.h"   //��ѹ�¶ȶ�ȡ��lcd���ȿ��Ƶ�

//#include "task_cfg.h"    //������صĺ궨��
#include "led_show_work.h"    //���Ե�
// #include "di_4ttl.h"     //4·����������
#include "uart_conect_cpu_handler.h"   //��cpuͨ�ŵĴ��ڽ��շ��ʹ���
#include "uart_debug_handle.h"        //���Դ��ڵĽ��մ���
// #include "optica_switch_status.h"        //���Դ��ڵĽ��մ���
// #include "hard_wtg.h"     //Ӳ�����Ź�
// #include "msata_destroy.h"     //Ӳ������
// #include "power_data_iic.h"     //��ȡ��Դiic����

#include "lt9211.h"
#include "matrix_keys.h"
#include "nca9555.h"     //����֮ǰ����Ŀ�еġ�
#include "key_light.h"   //�����ƵĿ���

//#include "flash_record.h"   //�ڲ�flash����
#include "internal_temp.h"   //��Ƭ���¶Ȳɼ�
#include "hard_wtd.h"     //�ⲿӲ�����Ź�

#include "my_task_info.h"
#endif

