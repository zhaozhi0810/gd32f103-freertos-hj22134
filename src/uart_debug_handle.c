
/*
用于处理与cpu之间的串口通信
	串口为GD32的串口1，115200，8N1


调试串口接收的命令：
0.软件编译时间字符串
1.电压电流
2.cpu和主板温度，液晶屏温度，及lcd加热状态，风扇pwm值（io模式下只有0和100）
3.lcd的亮度值，屏的加电引脚状态，pd_n的状态
4.4路di值，4路光通路信息
5.硬件看门狗状态，（信号源，暂无）
6.cpu运行状态。（开机关机，重启，进入pmon，进入系统等）

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

#define DEBUG_UART_SEND_BIT (1<<0)
#define DEBUG_UART_RECV_BIT (1<<1)

//#define RECV_BUF_LEN 64
#if 0
static Queue_UART_STRUCT g_Queue_Debug_Recv;   //接收Debug数据队列，用于接收中断

frame_buf_t g_com_debug_buf={{0},FRAME_LENGHT};    //数据处理缓存
#endif

//static char* g_Cpu_Run_Status_str[] = {
//	"LS3A_POWER_DISABLE",   //确认是断电状态
//	"LS3A_POWEROFF",    //关机，断电
//	"LS3A_REBOOT",    //重启
//	"LS3A_RUNNING",    //进入pmon
//	"LS3A_RUN_OS",      //离开pmon，进入操作系统
//	"LS3A_POWER_ENABLE"     //已经通电，但是没有进入PMON的前一段;
//};


static StreamBufferHandle_t _uart_tx_StreamBuffer_Handle;
static StreamBufferHandle_t _uart_rx_StreamBuffer_Handle;
//static QueueHandle_t _uart_write_mutex_Handle;
//static SemaphoreHandle_t xSemaphore;

//TaskHandle_t  TaskHandle_Debug_Com;   //存放调试串口任务指针



void Com_Debug_init(uint32_t bandrate)
{
	gd_eval_com_init(DEBUG_COM_NUM,DEBUG_UART_BAUD_RATE);  //用于调试
	
		// Disable stream buffers so I/O occurs immediately
//	setvbuf(stdin,  NULL, _IONBF, 0); // should be a read-only stream
//	setvbuf(stdout, NULL, _IONBF, 0); // disables wait for \n before printing
//	setvbuf(stderr, NULL, _IONBF, 0); // should be already unbuffered
	
	
	// Initialise transmit and receive message queues
	_uart_tx_StreamBuffer_Handle = xStreamBufferCreate(64, 1);  //缓冲大小，触发字节数
	_uart_rx_StreamBuffer_Handle = xStreamBufferCreate(64,  1);

	// Initialise write mutex  QueueHandle_t xQueueCreateMutex
	//xSemaphore  = xSemaphoreCreateMutex() ;//xQueueCreateMutex(queueQUEUE_TYPE_MUTEX);

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
    // Queue char for transmission, block if queue full
//    osMessageQueuePut(_uart_tx_queue_id, &c, 0U, osWaitForever);
	const TickType_t x10ms = pdMS_TO_TICKS( 10 );
	
	xStreamBufferSend(_uart_tx_StreamBuffer_Handle,&c,1,x10ms);  //一直等待，直到成功，感觉可能有点问题
//	xTaskNotify(TaskHandle_Debug_Com, DEBUG_UART_SEND_BIT, eSetBits);  //唤醒休眠的任务
	
    // Enable TXE interrupt
    //LL_USART_EnableIT_TXE(USARTx_INSTANCE); 
	usart_interrupt_enable(EVAL_COM0, USART_INT_TBE);    //发送缓存空的中断
    return c;
}

int __io_getchar(void)
{
    // Dequeue received char, block if queue empty
    uint8_t dat;

	dat = (uint8_t)usart_data_receive(EVAL_COM0);//(USART3); 
		
	//static uint8_t c;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;  //要不要切换任务，属于返回参数
//    osMessageQueueGet(_uart_rx_queue_id, &c, NULL, osWaitForever);
	xStreamBufferSendFromISR(_uart_rx_StreamBuffer_Handle,&dat,1,&xHigherPriorityTaskWoken);  //读出串口的数据，写入buf
	//xStreamBufferReceiveFromISR
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
	debug_printf_string("\r\nDebug cmd:\r\n");
	debug_printf_string("0. print Program build time\r\n");
	debug_printf_string("1. 7 inch lcd PWM increace(5inch lcd has no effect)\r\n");
	debug_printf_string("2. 7 inch lcd PWM decreace(5inch lcd has no effect)\r\n");
	debug_printf_string("3. print my task_info\r\n");
	debug_printf_string("4. reset LCD & 9211\r\n");
	debug_printf_string("5. print Hard Watch Dog Status\r\n");
	debug_printf_string("6. print Mcu internal_temp\r\n");
	debug_printf_string("7. reset core board!!\r\n");
	debug_printf_string("other. print help\r\n");
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
			debug_printf_string((char*)g_build_time_str);  //打印编译的时间
			debug_printf_string("\r\n");
			debug_printf_string("Author:JC&DaZhi <vx:285408136>\r\n"); 
		break;
		case '1':
//			if(g_lcd_pwm < 100)
//			{
//				Lcd_pwm_out(g_lcd_pwm + 10);   //屏幕亮度加10
//				debug_printf_string("increase 7 inch lcd PWM\r\n");
//			}
//			else
				debug_printf_string("g_lcd_pwm = 100\r\n");
			break;
		case '2':
			//打印温度值
//			if(g_lcd_pwm >= 10)
//			{
//				Lcd_pwm_out(g_lcd_pwm - 10);   //屏幕亮度加10
//				debug_printf_string("decrease 7 inch lcd PWM\r\n");
//			}
//			else
				debug_printf_string("g_lcd_pwm = 0\r\n");
			break;
		case '3':
			query_task();
			
			break;
		case '4':
			debug_printf_string("reset LCD & 9211\r\n");  //lcd加电状态
			LT9211_Config();
			break;
		case '5':
			debug_printf_string("Watch Dog Status : off\r\n");   //暂时没有开启
			break;
		case '6':
//			debug_printf_string("Mcu internal_temp = %d\r\n",get_internal_temp());
			break;
		case '7':
			debug_printf_string("reset core board!!\r\n");  //lcd加电状态
			hard_wtd_reset_3399board();
			break;
	}
}


/*
	串口数据接收中断：
		前提：每一帧都是7个字节。
		队列中保存帧头，有后面的数据和校验和（共7个字节）
*/
//void Com_Debug_Rne_Int_Handle(void)
//{
//	uint8_t dat;

//	dat = (uint8_t)usart_data_receive(EVAL_COM0);//(USART3);  
//	Com_Debug_Message_Handle1(dat);   //直接处理
////	QueueUARTDataInsert(&g_Queue_Debug_Recv,dat);   //接收的数据存入队列中。
//}







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
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
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
//	BaseType_t xHigherPriorityTaskWoken = portMAX_DELAY;  //无限制等待
//	vTaskDelay(20000);	
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








//调试串口输出printf
/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
	//Uart_Tx(DEBUG_COM_NUM, ch);   //从串口0输出
    __io_putchar(ch);
	return ch;
}




/*
	串口收到命令后的处理。串口的空闲中断处理函数调用
		前提： 收到完整的数据包，校验和正确。

	单片机能够收到的命令：
	// 1.设置视频源,没有该功能
	4.设置lcd的pwm（亮度）
	5.关机或重启命令。

*/

#if 0

static void Com_Debug_Message_Handle(uint8_t* buf)
{		
	com_frame_t* pdata = (com_frame_t*)(buf+1);    //+1是跳过帧头，使用结构体初始化
	int8_t t;
	
	switch(pdata->data_type)
    {
        case eMCU_CMD_TYPE:    //cpu发送给单片机的都是cmd！！
            t = pdata->data.cmd.cmd;
            switch(t)
            {
				case eMCU_CPUGETINFO_CMD:   //获取设备信息的命令
				//	AnswerCpu_GetInfo(pdata->data.cmd.param1<<8 | pdata->data.cmd.param2); //使用函数解析，并返回数据
					break;
				case eMCU_CPUSET_CMD:    //设置屏幕亮度
					if(pdata->data.cmd.param1 == eMCU_LCD_SETPWM_CMD)
					{
						t = pdata->data.cmd.param2;   //这个值可正可负，根据它的正负来调亮或者灭
						t = g_lcd_pwm + t;   //计算得出新的结果
						Lcd_pwm_out(t);     //重新设置pwm的值
				//		AnswerCpu_Status(eUART_SUCCESS);   //应答成功
					}
					else if(pdata->data.cmd.param1 == eMCU_SWITCH_DVI_SRC_CMD) //切换视频源
					{
						t = pdata->data.cmd.param2;  //0 为本地，1为外部
//						if(t)
//							dvi_switch_set(DVI_OTHER);   //设置后会上报给cpu
//						else
//							dvi_switch_set(DVI_LOONGSON);   //本地视频
				//		AnswerCpu_Status(eUART_SUCCESS);   //应答成功
					}
					else	
				//		AnswerCpu_Status(eUART_ERR_PARAM);  //应答参数错误				
				break;
                default:
					DBG_PRINTF("ERROR: %s\n","eUART_ERR_PARAM");
				//	AnswerCpu_Status(eUART_ERR_PARAM);  //应答参数错误
                break;
            }

        break;
        default:
			DBG_PRINTF("ERROR: %s\n","eUART_ERR_CMD_UNKNOW");
		//	AnswerCpu_Status(eUART_ERR_CMD_UNKNOW);  //应答命令未知 
        break;
    }	
}
#endif



/*
	串口空闲中断的处理,调试串口不再开启空闲中断

	1.判断接收到的字节数，>=7 表示正常
	2.正常就继续处理，读出7个字节，计算校验和，
	3.校验和正确，则处理命令
*/
void Com_Debug_Idle_Int_Handle(void)
{
//	Com_Frame_Handle(&g_com_debug_buf, &g_Queue_Debug_Recv,Com_Debug_Message_Handle);
}


