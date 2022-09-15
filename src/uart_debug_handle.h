

#ifndef UART_DEBUG_HANDLE_H
#define UART_DEBUG_HANDLE_H

#include <gd32f10x.h>
#include <task.h>


extern TaskHandle_t  TaskHandle_Debug_Com;   //存放调试串口任务指针



void Com_Debug_init(uint32_t bandrate);

//void Com_Debug_Rne_Int_Handle(void);

//void Com_Debug_Idle_Int_Handle(void);


//调试串口的打印任务
//void Com_Debug_Print_Task(void * parameter);

//调试串口的接收任务
void Com_Debug_Recv_Task(void * parameter);

//用于调试串口打印字符串,替换printf函数 2022-09-15
void debug_printf_string(char* str);
//int _write(int file, char *ptr, int len);
//base 表示进制，10为10进制，其他均为16进制
void debug_printf_u32(uint32_t dat,uint8_t base);
//base 表示进制，10为10进制，其他均为16进制
void debug_printf_string_u32(char* str,uint32_t dat,uint8_t base);


//调试串口的打印任务，把打印和接收的任务放在一起
void Com_Debug_Task(void * parameter);
#endif
