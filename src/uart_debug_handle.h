

#ifndef UART_DEBUG_HANDLE_H
#define UART_DEBUG_HANDLE_H

#include <gd32f10x.h>
#include <task.h>


extern TaskHandle_t  TaskHandle_Debug_Com;   //��ŵ��Դ�������ָ��



void Com_Debug_init(uint32_t bandrate);

//void Com_Debug_Rne_Int_Handle(void);

//void Com_Debug_Idle_Int_Handle(void);


//���Դ��ڵĴ�ӡ����
void Com_Debug_Print_Task(void * parameter);

//���Դ��ڵĽ�������
void Com_Debug_Recv_Task(void * parameter);



//���Դ��ڵĴ�ӡ���񣬰Ѵ�ӡ�ͽ��յ��������һ��
void Com_Debug_Task(void * parameter);
#endif
