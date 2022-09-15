
#ifndef __FLASH_RECORD_H__
#define __FLASH_RECORD_H__


#include <gd32f10x.h>

//4字节对齐，flash写入的时候也是4字节对齐
typedef struct flash_record
{
	uint32_t lcd_pwm;
	
}flash_rcd_t;


//用于数据记录
uint8_t read_flash_config(flash_rcd_t* config);

//把数据记录到flash中
uint8_t write_flash_config(flash_rcd_t * data);
#endif



