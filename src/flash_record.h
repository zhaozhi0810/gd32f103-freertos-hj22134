
#ifndef __FLASH_RECORD_H__
#define __FLASH_RECORD_H__


#include <gd32f10x.h>

//4�ֽڶ��룬flashд���ʱ��Ҳ��4�ֽڶ���
typedef struct flash_record
{
	uint32_t lcd_pwm;
	
}flash_rcd_t;


//�������ݼ�¼
uint8_t read_flash_config(flash_rcd_t* config);

//�����ݼ�¼��flash��
uint8_t write_flash_config(flash_rcd_t * data);
#endif



