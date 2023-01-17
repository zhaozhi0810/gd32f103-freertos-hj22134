
/*
用于调试的串口通信
	串口为GD32的串口1，115200，8N1

不能识别的命令也是打印提示和编译时间字符串

2022-09-09 关于串口的freertos的调试过程
1.打印部分，printf调用fputc，fputc写到发送缓存中，
2.建立发送任务，每次缓存有数据，就执行打印任务。
3.接收部分，接收中断，调用读串口指令，读出的数据写入（使用fromisr的方法）到接收缓存。
4.建立接收任务，每次缓存有数据就解析并完成其他工作。

注意：
这个程序一直卡在进入临界区的代码中。port.c  configASSERT( ( portNVIC_INT_CTRL_REG & portVECTACTIVE_MASK ) == 0 );

解决：
后来发现，我是在接收中断中，然后又调用了进入临界区的代码（printf中调用的），在百度中看到说不能在中断中进入临界区。
所以重新更换了方法，就是中断只负责接收数据，写入缓存，之后数据解析再使用别的任务完成。

终于解决了卡死在进入临界区的代码中的问题。！！！！

*/



#include "includes.h"



#define DEBUG_UART_BAUD_RATE 115200


static StreamBufferHandle_t _uart_tx_StreamBuffer_Handle;
static StreamBufferHandle_t _uart_rx_StreamBuffer_Handle;


void Com_Debug_init(uint32_t bandrate)
{
	gd_eval_com_init(DEBUG_COM_NUM,DEBUG_UART_BAUD_RATE);  //用于调试
	
	_uart_tx_StreamBuffer_Handle = xStreamBufferCreate(64, 1);  //缓冲大小，触发字节数
	_uart_rx_StreamBuffer_Handle = xStreamBufferCreate(64,  1);

}



// Make printf thread safe via linker wrapping and protecting the internals with a mutex
//int __wrap_printf(char *fmt, ...)
//{
//    int result = 0;
//	/* 通过二值信号量实现资源互斥访问，永久等待直到资源可用 */
//    if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdPASS)
//    {
//        va_list args;
//        va_start(args, fmt);
//        result = vprintf(fmt, args);
//        va_end(args);
//        xSemaphoreGive(xSemaphore);/* vTaskDelayUntil 是绝对延迟，vTaskDelay 是相对延迟。*/
//    }
//    return result;
//}

static int __io_putchar(int c)
{
	const TickType_t x10ms = pdMS_TO_TICKS( 10 );
	
	xStreamBufferSend(_uart_tx_StreamBuffer_Handle,&c,1,x10ms);  //一直等待，直到成功，感觉可能有点问题

	usart_interrupt_enable(EVAL_COM0, USART_INT_TBE);    //发送缓存空的中断
    return c;
}

int __io_getchar(void)
{
    // Dequeue received char, block if queue empty
    uint8_t dat;

	dat = (uint8_t)usart_data_receive(EVAL_COM0);//(USART3); 

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;  //要不要切换任务，属于返回参数

	xStreamBufferSendFromISR(_uart_rx_StreamBuffer_Handle,&dat,1,&xHigherPriorityTaskWoken);  //读出串口的数据，写入buf

    return dat;
}

//int _write(int file, char *ptr, int len)
//{
//    UNUSED(file); 
//    int i;

//    for (i = 0; i < len; i++)
//    {
//        __io_putchar(*ptr++);
//    }

//    return i;
//}

void debug_printf_string(char* str)
{
//    int i;
	if(str == NULL)
		return;
	
    while (*str)
    {
        __io_putchar(*str++);
    }
}

//base 表示进制，10为10进制，其他均为16进制
void debug_printf_u32(uint32_t dat,uint8_t base)
{
	char arr[16] = {0};
	uint8_t i;
	
	arr[15] = 0;
	if(base == 10)  //10进制输出
	{
		for(i=14;(i<16) && dat;i--)
		{
			arr[i] = dat%10 + '0';
			dat /=10;
		}
	}
	else  //16进制输出
	{
		for(i=14;(i<16) && dat;i--)
		{
			arr[i] = dat&0xf;
			if(arr[i]<10)
				arr[i] += '0';
			else
				arr[i] += 'a' - 10;
			dat >>=4;
		}
		debug_printf_string(" 0x");
	}
	
	debug_printf_string(arr+i+1);
	debug_printf_string(" ");  //加一个空格
}


//base 表示进制，10为10进制，其他均为16进制
void debug_printf_string_u32(char* str,uint32_t dat,uint8_t base)
{
	debug_printf_string(str);
	debug_printf_u32(dat,base);
	debug_printf_string("\r\n");
}




//int _read(int file, char *ptr, int len)
//{
//    UNUSED(file); 
//    // printf("len=%d, ", len);
//    
//    // KNOWN ISSUE: when using newlib-nano, setvbuf(stdin,  NULL, _IONBF, 0) has no effect, 
//    // and we keep getting len == 1024.
//    // Alternatively, if we remove the libc.a reference from the linker script, we
//    // get len == 1 with newlib-nano.
//    //
//    // newlib-nano is apparently built using --disable-newlib-fvwrite-in-streamio, which
//    // disables the setvbuf functions.
//    //
//    // see also 
//    //  https://answers.launchpad.net/gcc-arm-embedded/+question/246038
//    //  https://forum.43oh.com/topic/6989-newlib-problems-with-printf/

//    len = 1; // workaround

//    int i;

//    for (i = 0; i < len; i++)
//    {
//        int c;
//        if ((c = __io_getchar()) == EOF) break;
//        *ptr++ = c;
//    }

//    return i;
//}





static void  Com_Debug_Print_Help(void)
{
	printf("\r\nDebug cmd:\r\n");
	printf("0. print Program build time\r\n");
	if(Get_Lcd_Type()){  //返回1表示7寸屏，0表示5寸屏
		printf("1. 7 inch lcd PWM increace(5inch lcd has no effect)\r\n");
		printf("2. 7 inch lcd PWM decreace(5inch lcd has no effect)\r\n");
	}
	printf("3. freeRtos task stats!!\r\n");
	printf("4. reset LCD & 9211\r\n");
	printf("5. print Hard Watch Dog Status\r\n");
	printf("6. print Mcu internal_temp\r\n");
	printf("7. reset core board!!\r\n");
	printf("8. LSPK_Control test\r\n");
	printf("9. V12_CTL(MORSE) test\r\n");
	printf("a. keyLEDS pwm test\r\n");
	printf("b. 5inch lcd blacklight control\r\n");
	printf("c. SPKEN control\r\n");
	printf("d. EAR_L_EN control\r\n");
	printf("e. EAR_R_EN control\r\n");
	printf("f. key_leds pwm increace(10%%)\r\n");
	printf("g. key_leds all on\r\n");
	printf("h. key_leds all off\r\n");
#if 0
	printf("j. key_leds some one on\r\n");
	printf("k. key_leds some one off\r\n");
	printf("z. key_leds 1-10 on\r\n");
	printf("x. key_leds 1-10 off\r\n");
	printf("n. key_leds 11-20 on\r\n");
	printf("m. key_leds 11-20 off\r\n");
	printf("q. key_leds 21-30 on\r\n");
	printf("w. key_leds 21-30 off\r\n");
	printf("r. key_leds 31-39 on\r\n");
	printf("t. key_leds 31-39 off\r\n");
	printf("o. key_leds all on -2\r\n");
	printf("p. key_leds all off -2\r\n");
#endif	
	printf("other. print help\r\n");
}




//这个函数用来处理调试串口接收到的简单的调试命令
static void Com_Debug_Message_Handle1(uint8_t buf)
{
//	uint8_t t;
	switch(buf)
	{
		default:   //cmd打印的时候，可能超出了可显示字符的区间
			debug_printf_string("ERROR: Command Unknow \r\n");   //不能识别的命令
			Com_Debug_Print_Help();
		case '0':
			debug_printf_string("*FreeRTOS* ");
			debug_printf_string((char*)g_build_time_str);  //打印编译的时间
			debug_printf_string("\r\n");
			debug_printf_string("Author:JC&DaZhi <vx:285408136>\r\n"); 
		break;
		
		case '1':
			if(Get_Lcd_Type()){
				if(g_lcd_pwm < 100)
				{
					Lcd_pwm_out(g_lcd_pwm + 10);   //屏幕亮度加10
					debug_printf_string("increase 7 inch lcd PWM\r\n");
				}
				else
					debug_printf_string("g_lcd_pwm = 100\r\n");
			}
			break;
		case '2':
			if(Get_Lcd_Type()){
				if(g_lcd_pwm >= 10)
				{
					Lcd_pwm_out(g_lcd_pwm - 10);   //屏幕亮度加10
					debug_printf_string("decrease 7 inch lcd PWM\r\n");
				}
				else
					debug_printf_string("g_lcd_pwm = 0\r\n");
			}
			break;
		case '3':
			query_task();
			
			break;
		case '4':  //9211重启
			debug_printf_string("reset LCD & 9211\r\n");  //lcd加电状态
			LT9211_Config();
			break;
		case '5':  //看门狗状态
			debug_printf_string("Watch Dog Status : ");
			if(get_hard_wtd_status())
				debug_printf_string("on\r\n");   //开启
			else
				debug_printf_string("off\r\n");   //关闭
			break;
		case '6':  //mcu内部温度
			debug_printf_string("Mcu internal_temp = ");
			debug_printf_u32(get_internal_temp(),10);
			debug_printf_string("\r\n");
//			debug_printf_string("Mcu internal_temp = %d\r\n",get_internal_temp());
			break;
		case '7':   
			debug_printf_string("reset core board!!\r\n");  //lcd加电状态
			hard_wtd_reset_3399board();
			break;
		case '8':  //LSPK 继电器控制 改为PA7
			LSPK_Control_ToggleOut();
			break;
		case '9':   //mores 12 电压输出控制
			V12_CTL_Control_ToggleOut();
			break;
		case 'a':   //键灯亮度调节
			kLedPWM_ToggleOut();
			break;
		case 'b':   //5寸lcd背光控制
			SHTDB_5IN_Control_ToggleOut();
			break;
		case 'c':   //SPEN控制
			SPKEN_Control_ToggleOut();
			break;
		case 'd':   //EAR_L_EN控制
			EAR_L_EN_Control_ToggleOut();
			break;
		case 'e':   //EAR_R_EN控制
			EAR_R_EN_Control_ToggleOut();
			break;
		case 'f':   //键灯亮度控制
			kLedPWM_ToggleOut();
			break;
		case 'g':   //键灯全部点亮控制
			debug_printf_string("2023 key_light_allleds_control(1)!!\r\n");
			key_light_allleds_control(1);
			break;
		case 'h':   //键灯全部熄灭控制
			debug_printf_string("2023 key_light_allleds_control(0)!!\r\n");
			key_light_allleds_control(0);
			break;
		case 'v':
			printf("mcu version = %d\r\n", GetMcuVersion());
			break;
#if 0
		/*
	printf("j. key_leds some one on\r\n");
	printf("k. key_leds some one off\r\n");
	printf("z. key_leds 1-10 on\r\n");
	printf("x. key_leds 1-10 off\r\n");
	printf("n. key_leds 11-20 on\r\n");
	printf("m. key_leds 11-20 off\r\n");
	printf("q. key_leds 21-30 on\r\n");
	printf("w. key_leds 21-30 off\r\n");
	printf("r. key_leds 31-39 on\r\n");
	printf("t. key_leds 31-39 off\r\n");
	printf("o. key_leds all on -2\r\n");
	printf("p. key_leds all off -2\r\n");
		*/
		case 'j':   //点亮某个灯
			debug_printf_string("2023 key_light_allleds_control(0)!!\r\n");
			key_light_allleds_control(0);
			break;
		case 'k':   //熄灭某个灯
			debug_printf_string("2023 key_light_allleds_control(0)!!\r\n");
			key_light_allleds_control(0);
			break;
		case 'z':   //点亮1-10灯
			debug_printf_string("2023 key_light_allleds_control(0)!!\r\n");
			key_light_allleds_control(0);
			break;
		case 'x':   //熄灭1-10灯
			debug_printf_string("2023 key_light_allleds_control(0)!!\r\n");
			key_light_allleds_control(0);
			break;
		case 'n':   //点亮11-20灯
			debug_printf_string("2023 key_light_allleds_control(0)!!\r\n");
			key_light_allleds_control(0);
			break;
		case 'm':   //熄灭11-20灯
			debug_printf_string("2023 key_light_allleds_control(0)!!\r\n");
			key_light_allleds_control(0);
			break;
		case 'q':   //点亮21-30灯
			debug_printf_string("2023 key_light_allleds_control(0)!!\r\n");
			key_light_allleds_control(0);
			break;
		case 'w':   //熄灭21-30灯
			debug_printf_string("2023 key_light_allleds_control(0)!!\r\n");
			key_light_allleds_control(0);
			break;
		case 'r':   //点亮31-39灯
			debug_printf_string("2023 key_light_allleds_control(0)!!\r\n");
			key_light_allleds_control(0);
			break;
		case 't':   //熄灭31-39灯
			debug_printf_string("2023 key_light_allleds_control(0)!!\r\n");
			key_light_allleds_control(0);
			break;
		case 'o':   //键灯全部熄灭控制-2
			debug_printf_string("2023 key_light_allleds_control(0)!!\r\n");
			key_light_allleds_control(0);
			break;
		case 'p':   //键灯全部熄灭控制-2
			debug_printf_string("2023 key_light_allleds_control(0)!!\r\n");
			key_light_allleds_control(0);
			break;
#endif
	}
}



static void _uart_error(void)
{
    if (usart_interrupt_flag_get(EVAL_COM0, USART_INT_FLAG_PERR)) 
    {
        debug_printf_string("*** USART parity error! ***\n");
		usart_interrupt_flag_clear(USART0, USART_INT_FLAG_PERR);   //清中断标志
    }

    else if (usart_interrupt_flag_get(EVAL_COM0, USART_INT_FLAG_ERR_FERR)) 
    {
        debug_printf_string("*** USART frame error! ***\n");
		usart_interrupt_flag_clear(USART0, USART_INT_FLAG_ERR_FERR);   //清中断标志
    }

    else if (usart_interrupt_flag_get(EVAL_COM0, USART_INT_FLAG_ERR_ORERR)) 
    {
        debug_printf_string("*** USART overrun error! ***\n");
		usart_interrupt_flag_clear(USART0, USART_INT_FLAG_ERR_ORERR);   //清中断标志
    }

    else if (usart_interrupt_flag_get(EVAL_COM0, USART_INT_FLAG_ERR_NERR)) 
    {
        debug_printf_string("*** USART noise error! ***\n");
		usart_interrupt_flag_clear(USART0, USART_INT_FLAG_ERR_NERR);   //清中断标志
    }
}



static void debug_uart_send_isr(void)
{
	uint8_t c;
	BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
	size_t bytes = xStreamBufferBytesAvailable(_uart_tx_StreamBuffer_Handle);
	
	if(!bytes)   //没有数据
	{
		usart_interrupt_disable(EVAL_COM0, USART_INT_TBE);    //发送缓存空的中断
		return;
	}
	else
	{
		if(xStreamBufferReceiveFromISR(_uart_tx_StreamBuffer_Handle,&c,1,&pxHigherPriorityTaskWoken)) //不等待
		{  //如果缓存有数据
			Uart_Tx(DEBUG_COM_NUM, c);
		}
		else  //缓存没有数据
		{
			usart_interrupt_disable(EVAL_COM0, USART_INT_TBE);    //发送缓存空的中断
		}
	}
	
}



//调试串口中断处理函数
void USART0_IRQHandler(void)
{
//	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if(usart_interrupt_flag_get(EVAL_COM0, USART_INT_FLAG_RBNE))
    {
		usart_interrupt_flag_clear(EVAL_COM0, USART_INT_FLAG_RBNE);   //清中断标志
		__io_getchar();
//		xTaskNotifyFromISR(TaskHandle_Debug_Com, DEBUG_UART_RECV_BIT, eSetBits, &xHigherPriorityTaskWoken);  //唤醒休眠的任务
    }
	else if(usart_interrupt_flag_get(EVAL_COM0, USART_INT_FLAG_TBE))  //发送中断
	{
		usart_interrupt_flag_clear(EVAL_COM0, USART_INT_FLAG_TBE);   //清中断标志
		debug_uart_send_isr();
	}
    else //if(usart_interrupt_flag_get(EVAL_COM0, USART_INT_FLAG_RBNE) /*&& LL_USART_IsActiveFlag_NE(USARTx_INSTANCE)*/)
    {
        _uart_error();
    }
}






//调试串口的接收任务
void Com_Debug_Recv_Task(void * parameter)
{      
	uint8_t c;

	while(1)
	{		
		if(xStreamBufferReceive(_uart_rx_StreamBuffer_Handle,&c,1,portMAX_DELAY))
		{  //如果缓存有数据
			Com_Debug_Message_Handle1(c);
		}
		else  //缓存没有数据
		{
			
		}
	}         
}




//调试串口输出printf
/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    __io_putchar(ch);
	return ch;
}






//调试串口的打印任务
//void Com_Debug_Print_Task(void * parameter)
//{      
////	BaseType_t err = pdPASS;
//	uint8_t c;
//	BaseType_t xHigherPriorityTaskWoken = portMAX_DELAY;  //无限制等待
////    osMessageQueueGet(_uart_rx_queue_id, &c, NULL, osWaitForever);
////	size_t ret ;
//	
//	
//	while(1)
//	{
//		if(xStreamBufferReceive(_uart_tx_StreamBuffer_Handle,&c,1,xHigherPriorityTaskWoken))
//		{  //如果缓存有数据
//			Uart_Tx(DEBUG_COM_NUM, c);
//		}
//		else  //缓存没有数据
//		{
//			
//		}
//	}
//         
//}


//调试串口的打印任务，把打印和接收的任务放在一起
//void Com_Debug_Task(void * parameter)
//{      
////	BaseType_t err = pdPASS;
//	uint8_t c;
//	uint32_t ulNotificationValue;
//	//TickType_t xTicksToWait
////	BaseType_t xHigherPriorityTaskWoken = portMAX_DELAY;  //无限制等待
////    osMessageQueueGet(_uart_rx_queue_id, &c, NULL, osWaitForever);
//	size_t bytes,i ;
//	
//	
//	while(1)
//	{
//		
//		xTaskNotifyWait(ULONG_MAX,        //进入时，清零哪些位，0表示都不清零，ULONG_MAX的时候就会将任务通知值清零
//						0,   //退出时，清除对应的位，0表示都不清零
//						&ulNotificationValue,  //返回的通知值
//						portMAX_DELAY);    //无限等待

//		if(ulNotificationValue & DEBUG_UART_SEND_BIT)  //发送位被设置
//		{
//			bytes = xStreamBufferBytesAvailable(_uart_tx_StreamBuffer_Handle);
//			
//			for(i=0;i<bytes;i++)
//			{
//				if(xStreamBufferReceive(_uart_tx_StreamBuffer_Handle,&c,1,0)) //不等待
//				{  //如果缓存有数据
//					Uart_Tx(DEBUG_COM_NUM, c);
//				}
//				else  //缓存没有数据
//				{
//					break;
//				}
//			}
//		}
//		
//		if(ulNotificationValue & DEBUG_UART_RECV_BIT)
//		{
//			bytes = xStreamBufferBytesAvailable(_uart_rx_StreamBuffer_Handle);
//			for(i=0;i<bytes;i++)
//			{
//				if(xStreamBufferReceive(_uart_rx_StreamBuffer_Handle,&c,1,0)) //不等待
//				{  //如果缓存有数据
//					//Uart_Tx(DEBUG_COM_NUM, c);
//					Com_Debug_Message_Handle1(c);   //处理接收到的数据
//					//Com_Debug_Rne_Int_Handle();
//				}
//				else  //缓存没有数据
//				{
//					break;
//				}
//			}
//		}
//		
//		
//	}
//         
//}









