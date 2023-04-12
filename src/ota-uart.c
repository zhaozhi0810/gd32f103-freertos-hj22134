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

#define ApplicationAddress    0x8006000
#define PAGE_SIZE             (0x400)    /* 1 Kbyte */
#define UPDATE_FLAG_START_ADDR   (ApplicationAddress-PAGE_SIZE)     //设置起始地址，0x805c00

uint8_t update_success_flag = 0;  //升级成功设置为1，否则为0
//static uint8_t mcu_update_flag = 0;    //升级标志


typedef struct update_flag
{
	uint16_t need_update;    //需要升级吗？0xff或者0x0都是不升级，其他值需要升级
	uint16_t update_success;  //升级成功了吗？0xff是未成功，其他值表示成功
	uint16_t update_where;    //升级的渠道，0xff表示调试串口，其他值表示与3399通信的串口
}update_flag_t;

//起始地址
static update_flag_t *g_updateflag = (void*)UPDATE_FLAG_START_ADDR;

void goto_ota_update(void)
{
	printf("goto_ota_update \r\n");
		
	if((g_updateflag->need_update!=0xff) || (g_updateflag->update_where!=0xff))
	{
		/* Flash unlock */
		fmc_unlock();
		fmc_page_erase(UPDATE_FLAG_START_ADDR);  //升级标志被清除，未成功标识被设置（需要app去清除）
	}
	vTaskDelay(50);
	/* Flash unlock */
	fmc_unlock();
	//g_updateflag->need_update = 0x0f;    //表示是需要升级
	//g_updateflag->update_where = 0x0f;   //表示是通信串口升级
	fmc_halfword_program(UPDATE_FLAG_START_ADDR+0, 0xf);
	fmc_halfword_program(UPDATE_FLAG_START_ADDR+4, 0xf);
	
	NVIC_SystemReset(); // 复位
}







//启动后，设置升级成功标志
void set_ota_update_success(void)
{
	if(g_updateflag->update_success == 0xff)
	{
		printf("set_ota_update_success \r\n");
		/* Flash unlock */
		fmc_unlock();
		//g_updateflag->update_success = 0xf; //升级成功了。
		fmc_halfword_program(UPDATE_FLAG_START_ADDR+2, 0xf);
		update_success_flag = 1;
	}
	else if (g_updateflag->update_success == 0xf)  //已经是升级成功的状态
	{
		update_success_flag = 1;
	}		
}






//void gd32_disable_phy(void)
//{
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
//	__disable_irq(); // 关闭总中断
	//__set_BASEPRI(WWDGT_IRQn);
//}





//void goto_ota_program(uint32_t ota_addr)
//{
//	pFunction Jump_To_Application;
//	uint32_t JumpAddress;
//	if (((*(__IO uint32_t*)ota_addr) & 0x2FFE0000 ) == 0x20000000)
//	{ 
//		gd32_disable_phy();
//		/* Jump to user application */
//		JumpAddress = *(__IO uint32_t*) (ota_addr + 4);
//		Jump_To_Application = (pFunction) JumpAddress;
//		/* 设置程序运行的栈 */
//		__set_MSP(*(__IO uint32_t*) ota_addr);
//		Jump_To_Application();  //
//	}
//	else
//	{
//		printf("ERROR:goto_ota_program,hasn't ota program!\n");
//	}
//}
















