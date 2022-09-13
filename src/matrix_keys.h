


#ifndef __MATRIX_KEYS_H__
#define __MATRIX_KEYS_H__

#include "gd32f10x.h"
#include "task.h"
#include <string.h>

#define KEY_MAX 36

#define ROW_NUM 6  //行数
#define COL_NUM 6  //列数

//#define KEY_BYTES_RECORD (KEY_MAX/8+(!!(KEY_MAX%8)))


typedef struct btn_info{
	uint8_t  value[KEY_MAX];	     //0表示松开，1表示按下
	uint8_t  reportEn[KEY_MAX];   //1，需要上报，0不需要上报
	uint16_t  pressCnt[KEY_MAX];     //消抖，长按区分
}BTN_INFO;





void matrix_keys_init(void);
char matrix_keys_scan(void);
uint8_t matrix_keys_row_scan(void);
//void matrix_keys_function(void);
void task_matrix_keys_scan(void* arg);

//#define KEY_CLO0_OUT_LOW  GPIO_WriteBit(GPIOB,GPIO_Pin_12,Bit_RESET)
//#define KEY_CLO1_OUT_LOW  GPIO_WriteBit(GPIOB,GPIO_Pin_13,Bit_RESET)
//#define KEY_CLO2_OUT_LOW  GPIO_WriteBit(GPIOB,GPIO_Pin_14,Bit_RESET)
//#define KEY_CLO3_OUT_LOW  GPIO_WriteBit(GPIOB,GPIO_Pin_15,Bit_RESET)

//#define KEY_CLO0_OUT_HIGH  GPIO_WriteBit(GPIOB,GPIO_Pin_12,Bit_SET) 
//#define KEY_CLO1_OUT_HIGH  GPIO_WriteBit(GPIOB,GPIO_Pin_13,Bit_SET)
//#define KEY_CLO2_OUT_HIGH  GPIO_WriteBit(GPIOB,GPIO_Pin_14,Bit_SET)
//#define KEY_CLO3_OUT_HIGH  GPIO_WriteBit(GPIOB,GPIO_Pin_15,Bit_SET)

extern TaskHandle_t  TaskHandle_key_Matrix;   //存放按键任务指针
#endif



