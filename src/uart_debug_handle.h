

#ifndef UART_DEBUG_HANDLE_H
#define UART_DEBUG_HANDLE_H

#include <gd32f10x.h>

void Com_Debug_init(uint32_t bandrate);

//void Com_Debug_Rne_Int_Handle(void);

//void Com_Debug_Idle_Int_Handle(void);


//调试串口的打印任务
void Com_Debug_Print_Task(void * parameter);

//调试串口的接收任务
void Com_Debug_Recv_Task(void * parameter);
#endif
