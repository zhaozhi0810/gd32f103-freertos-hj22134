/*
* @Author: dazhi
* @Date:   2023-02-01 19:42:13
* @Last Modified by:   dazhi
* @Last Modified time: 2023-02-01 20:02:03
*/

/*
	适用于跳转到ota程序，进行串口ota升级

	1. 之前OTA程序的弊端是OTA放到flash的前端，这时单片机的应用程序不可调试，也不能使用keil直接下载。
	2. 解决1这个问题，就想着把OTA的部分放到Flash的后面去，单片机的程序还是前面。
	3. 单片机增加一个串口指令，跳转到OTA去执行

 */
#include "includes.h"

typedef  void (*pFunction)(void);


void gd32_disable_phy(void)
{
//	SysTick->CTRL  = 0;  //关闭systick
//	rcu_periph_clock_disable(RCU_GPIOA);
//	rcu_periph_clock_disable(RCU_GPIOB);
//	rcu_periph_clock_disable(RCU_GPIOC);
//	rcu_periph_clock_disable(RCU_GPIOD);
//	rcu_periph_clock_disable(RCU_GPIOE);
//	rcu_periph_clock_disable(RCU_GPIOF);
//	rcu_periph_clock_disable(RCU_GPIOG);
//	rcu_periph_clock_disable(RCU_USART0);
//	rcu_periph_clock_disable(RCU_USART1);
//	
//	rcu_periph_clock_disable(RCU_TIMER1);
//	rcu_periph_clock_disable(RCU_TIMER0);
//	rcu_periph_clock_disable(RCU_TIMER2);
//	rcu_periph_clock_disable(RCU_TIMER3);
//	rcu_periph_clock_disable(RCU_TIMER4);
//	rcu_periph_clock_disable(RCU_TIMER5);
//	rcu_periph_clock_disable(RCU_TIMER6);
//	rcu_periph_clock_disable(RCU_TIMER7);

//	
//	fwdgt_write_disable();
//	nvic_irq_disable(USART0_IRQn);
//	nvic_irq_disable(USART1_IRQn);
//	nvic_irq_disable(TAMPER_IRQn);
//	nvic_irq_disable(FMC_IRQn);
//	nvic_irq_disable(EXTI0_IRQn);
//	nvic_irq_disable(EXTI1_IRQn);
//	nvic_irq_disable(EXTI2_IRQn);
//	nvic_irq_disable(EXTI3_IRQn);
//	nvic_irq_disable(EXTI4_IRQn);
//	nvic_irq_disable(EXTI5_9_IRQn);
//	nvic_irq_disable(EXTI10_15_IRQn);
//	nvic_irq_disable(TIMER1_IRQn);
//	nvic_irq_disable(TIMER2_IRQn);
//	nvic_irq_disable(TIMER3_IRQn);
//	nvic_irq_disable(TIMER4_IRQn);
	__disable_irq(); // 关闭总中断
	//__set_BASEPRI(WWDGT_IRQn);
}





void goto_ota_program(uint32_t ota_addr)
{
	pFunction Jump_To_Application;
	uint32_t JumpAddress;
	if (((*(__IO uint32_t*)ota_addr) & 0x2FFE0000 ) == 0x20000000)
	{ 
		gd32_disable_phy();
		/* Jump to user application */
		JumpAddress = *(__IO uint32_t*) (ota_addr + 4);
		Jump_To_Application = (pFunction) JumpAddress;
		/* 设置程序运行的栈 */
		__set_MSP(*(__IO uint32_t*) ota_addr);
		Jump_To_Application();  //
	}
	else
	{
		printf("ERROR:goto_ota_program,hasn't ota program!\n");
	}
}
















