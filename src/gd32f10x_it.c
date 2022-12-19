/*!
    \file  gd32f10x_it.c
    \brief interrupt service routines
    
    \version 2015-11-16, V1.0.0, demo for GD32F10x
    \version 2017-06-30, V2.0.0, demo for GD32F10x
    \version 2018-07-31, V2.1.0, demo for GD32F10x
*/

/*
    Copyright (c) 2018, GigaDevice Semiconductor Inc.

    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include "gd32f10x_it.h"
#include "includes.h"


//void work_led_toggle(void);   //main.c中

/*!
    \brief      this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
}

/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void HardFault_Handler(void)
{
    /* if Hard Fault exception occurs, go to infinite loop */
    while (1);
}

/*!
    \brief      this function handles MemManage exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void MemManage_Handler(void)
{
    /* if Memory Manage exception occurs, go to infinite loop */
    while (1);
}

/*!
    \brief      this function handles BusFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void BusFault_Handler(void)
{
    /* if Bus Fault exception occurs, go to infinite loop */
    while (1);
}

/*!
    \brief      this function handles UsageFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UsageFault_Handler(void)
{
    /* if Usage Fault exception occurs, go to infinite loop */
    while (1);
}

/*!
    \brief      this function handles SVC exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
//void SVC_Handler(void)
//{
//}


/*!
    \brief      this function handles SysTick exception 10ms中断一次
    \param[in]  none
    \param[out] none
    \retval     none
*/
//void SysTick_Handler(void)
//{
//	Systick_Int_Update();
//}



/*!
    \brief      this function handles DebugMon exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void DebugMon_Handler(void)
{
}

/*!
    \brief      this function handles PendSV exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
//void PendSV_Handler(void)
//{
//}


//串口1 用于与cpu进行通信
//void USART1_IRQHandler(void)
//{		
//	static uint8_t ide_int_enable = 0;
//	
//	if(usart_interrupt_flag_get(USART1, USART_INT_FLAG_RBNE))
//	{
//		Com_Cpu_Rne_Int_Handle();
//		usart_interrupt_flag_clear(USART1, USART_INT_FLAG_RBNE);   //清中断标志
//		if(ide_int_enable == 0)
//		{
//			usart_interrupt_enable(USART1, USART_INT_IDLE);    //允许空闲中断
//			ide_int_enable = 1;
//		}
//	}
//	else if(usart_interrupt_flag_get(USART1, USART_INT_FLAG_IDLE))  //空闲中断，表示一帧数据已结束
//	{
//		//解析命令，并处理。
//	//	Com_Cpu_Idle_Int_Handle();
//		usart_interrupt_flag_clear(USART1, USART_INT_FLAG_IDLE);//清中断标志
//		if(ide_int_enable)
//		{
//			usart_interrupt_disable(USART1, USART_INT_IDLE);    //禁止空闲中断
//			ide_int_enable = 0;
//		}
//		
//	}
//}


//串口0 用于与程序员进行调试通信使用
//void USART0_IRQHandler(void)
//{		
//	if(usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE))
//	{
//		Com_Debug_Rne_Int_Handle();
//		usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RBNE);   //清中断标志
//	}
//	else if(usart_interrupt_flag_get(USART0, USART_INT_FLAG_IDLE))  //空闲中断，表示一帧数据已结束
//	{
//		//解析命令，并处理。
//	//	Com_Debug_Idle_Int_Handle();
//		usart_interrupt_flag_clear(USART0, USART_INT_FLAG_IDLE);//清中断标志		
//	}
//}


//中断处理函数声明
void exint12_handle(void);
//void exint4_handle(void);

//外部中断4，5，6 用于监控cpu的运行状态
//void EXTI4_IRQHandler(void)
//{
//	if(exti_interrupt_flag_get(EXTI_4))
//	{
//		exti_interrupt_flag_clear(EXTI_4);  //清冲断标志
//		exint4_handle();
//	}
//}




//中断处理函数
//void exint456_handle(void);
//void ir_irq9_handle(void);



//void EXTI5_9_IRQHandler(void)
//{
//	if(exti_interrupt_flag_get(EXTI_9))
//	{		
////#ifdef IR_DETECT_USE_IRQ
////		ir_irq9_handle();
////#endif		
//		exti_interrupt_flag_clear(EXTI_9);  //清冲断标志
//	}
//	else if(exti_interrupt_flag_get(EXTI_5))  //用于按键唤醒cpu，啥也不干
//	{
//		exti_interrupt_flag_clear(EXTI_5);  //清冲断标志
//	//	exint456_handle();
//	}
//}




#ifdef 	BTNS_USE_INT
//外部中断12的处理函数,按键按下和松开都会触发中断！！！！
void exint12_handle(void);
#endif	

//void exint10_handle(void);


void EXTI10_15_IRQHandler(void)
{
#ifdef 	BTNS_USE_INT		
	if(exti_interrupt_flag_get(EXTI_12))
	{
		
		exint12_handle();
	}
	exti_interrupt_flag_clear(EXTI_12);  //清冲断标志
#endif	
	
	//morse ptt  暂时没有用上 2022-12-19
//	if(exti_interrupt_flag_get(EXTI_10))
//	{		
//		exint10_handle();   //morseptt 按键 的处理
//		exti_interrupt_flag_clear(EXTI_10);  //清冲断标志
//	}	
}






