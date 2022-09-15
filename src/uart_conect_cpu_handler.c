

/*
用于处理与cpu之间的串口通信
	串口为GD32的串口1，115200，8N1

*/

#include "includes.h"
#include "string.h"


#define FIXED_UART_CPU_LEN  //固定长度，没有定义则为非固定长度
#define CPU_UART_CMD_LEN 4 //cpu发出的都是4个字节
//#define RECV_BUF_LEN 64
//static Queue_UART_STRUCT g_Queue_Cpu_Recv;   //接收cpu数据队列，用于接收中断
//frame_buf_t g_com_cpu_buf={{0},CPU_UART_CMD_LEN};    //缓存

#define CPU_UART_HEAD1 0xa5    //注意与cpu端保持一致
//#define CPU_UART_HEAD2 0x5a


static StreamBufferHandle_t tocpu_tx_StreamBuffer_Handle;   //注意是静态的！！！！
static StreamBufferHandle_t tocpu_rx_StreamBuffer_Handle;

TaskHandle_t  TaskHandle_ToCpu_Com;   //存放调试串口任务指针



void Com_ToCpu_init(uint32_t bandrate)
{
	gd_eval_com_init(TOCPU_COM_NUM,bandrate);  //用于调试

	// Initialise transmit and receive message queues
	tocpu_tx_StreamBuffer_Handle = xStreamBufferCreate(16, 1);  //缓冲大小，触发字节数
	tocpu_rx_StreamBuffer_Handle = xStreamBufferCreate(16, 12);  //不需要这个触发了，所以触发字节大一点
}





//缓存初始化
//void Com_Cpu_Recive_Buff_Init(void)
//{
//	memset((void *)&g_Queue_Cpu_Recv, 0, sizeof(g_Queue_Cpu_Recv));
//}




/*
	串口数据接收中断：
		前提：每一帧都是7个字节。
		队列中保存帧头，有后面的数据和校验和（共7个字节）
*/
static void Com_Cpu_Rne_Int_Handle(void)
{
	uint8_t dat;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;  //要不要切换任务，属于返回参数
	
	dat = (uint8_t)usart_data_receive(EVAL_COM1);//(USART3);   //接收就存到队列中！！！！！2021-12-02
	
	xStreamBufferSendFromISR(tocpu_rx_StreamBuffer_Handle,&dat,1,&xHigherPriorityTaskWoken);  //读出串口的数据，写入buf
	
//	QueueUARTDataInsert(&g_Queue_Cpu_Recv,dat);   //接收的数据存入队列中。

}


/*
	串口空闲中断的处理

	1.判断接收到的字节数，>=7 表示正常
	2.正常就继续处理，读出7个字节，计算校验和，
	3.校验和正确，则处理命令

*/
static void Com_Cpu_Idle_Int_Handle(void)
{
	vTaskNotifyGiveFromISR(TaskHandle_ToCpu_Com,NULL);  //唤醒任务
//	Com_Frame_Handle(&g_com_cpu_buf, &g_Queue_Cpu_Recv,AnswerCpu_data);
}



static int _Com_Cpu_io_putchar(int c)
{
    // Queue char for transmission, block if queue full
//    osMessageQueuePut(_uart_tx_queue_id, &c, 0U, osWaitForever);
	const TickType_t x10ms = pdMS_TO_TICKS( 10 );
	
	xStreamBufferSend(tocpu_tx_StreamBuffer_Handle,&c,1,x10ms);  //一直等待，直到成功，感觉可能有点问题
//	xTaskNotify(TaskHandle_Debug_Com, DEBUG_UART_SEND_BIT, eSetBits);  //唤醒休眠的任务
	
    // Enable TXE interrupt
    //LL_USART_EnableIT_TXE(USARTx_INSTANCE); 
	usart_interrupt_enable(EVAL_COM1, USART_INT_TBE);    //发送缓存空的中断
    return c;
}



//发送一段字符
//com 表示从哪个端口发出
static void Com_Cpu_Uart_Tx_String(uint8_t *str, uint8_t length)
{
	uint8_t i;
	
	for(i = 0; i < length; i++)
		_Com_Cpu_io_putchar(str[i]);
	//	Uart_Tx(TOCPU_COM_NUM,str[i]);
}




//发送中断
static void Com_Cpu_Send_Isr(void)
{
	uint8_t c;
	BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
	size_t bytes = xStreamBufferBytesAvailable(tocpu_tx_StreamBuffer_Handle);
	
	if(!bytes)   //没有数据
	{
		usart_interrupt_disable(EVAL_COM1, USART_INT_TBE);    //发送缓存空的中断
		return;
	}
	else
	{
		if(xStreamBufferReceiveFromISR(tocpu_tx_StreamBuffer_Handle,&c,1,&pxHigherPriorityTaskWoken)) //不等待
		{  //如果缓存有数据
			Uart_Tx(TOCPU_COM_NUM, c);
		}
		else  //缓存没有数据
		{
			usart_interrupt_disable(EVAL_COM1, USART_INT_TBE);    //发送缓存空的中断
		}
	}
	
}




//调试串口中断处理函数
void USART1_IRQHandler(void)
{
	static uint8_t ide_int_enable = 0;
//	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if(usart_interrupt_flag_get(EVAL_COM1, USART_INT_FLAG_RBNE))
    {
		usart_interrupt_flag_clear(EVAL_COM1, USART_INT_FLAG_RBNE);   //清中断标志
	
		Com_Cpu_Rne_Int_Handle();
//		usart_interrupt_flag_clear(USART1, USART_INT_FLAG_RBNE);   //清中断标志
		if(ide_int_enable == 0)
		{
			usart_interrupt_enable(EVAL_COM1, USART_INT_IDLE);    //允许空闲中断
			ide_int_enable = 1;
		}
		
		//	__io_getchar();
//		xTaskNotifyFromISR(TaskHandle_Debug_Com, DEBUG_UART_RECV_BIT, eSetBits, &xHigherPriorityTaskWoken);  //唤醒休眠的任务
    }
	else if(usart_interrupt_flag_get(EVAL_COM1, USART_INT_FLAG_TBE))  //发送中断
	{
		usart_interrupt_flag_clear(EVAL_COM1, USART_INT_FLAG_TBE);   //清中断标志
		Com_Cpu_Send_Isr();
	//	debug_uart_send_isr();
	}
	else if(usart_interrupt_flag_get(EVAL_COM1, USART_INT_FLAG_IDLE))  //空闲中断，表示一帧数据已结束
	{
		//解析命令，并处理。
		Com_Cpu_Idle_Int_Handle();
		usart_interrupt_flag_clear(EVAL_COM1, USART_INT_FLAG_IDLE);//清中断标志
		if(ide_int_enable)
		{
			usart_interrupt_disable(EVAL_COM1, USART_INT_IDLE);    //禁止空闲中断
			ide_int_enable = 0;
		}		
	}
    else //if(usart_interrupt_flag_get(EVAL_COM0, USART_INT_FLAG_RBNE) /*&& LL_USART_IsActiveFlag_NE(USARTx_INSTANCE)*/)
    {
    //    _uart_error();
    }	
}





//应答cpu的设置信息的请求 errcode为0表示成功，其他值为错误码 应小于0x7f
void AnswerCpu_data(uint8_t *cmd)
{
	uint8_t buf[8] = {CPU_UART_HEAD1};    //头部信息
//	uint8_t dat;
	buf[1] = cmd[0];   //用于应答的指示
	buf[2] = 0;   //表示成功，255表示失败

	switch(cmd[0])
	{
		case eMCU_LED_SETON_TYPE: //设置led ON
			key_light_leds_control(cmd[1],1);
			break;
		case eMCU_LED_SETOFF_TYPE: //设置led OFF
			key_light_leds_control(cmd[1],0);
		break;
		case eMCU_LCD_SETONOFF_TYPE:  //设置lcd 打开或者关闭
			if(cmd[1])
				Enable_LcdLight();   //打开屏幕
			else
				Disable_LcdLight();
			break;
		case eMCU_LEDSETALL_TYPE:  //设置所有的led 打开或者关闭
			if(cmd[1])
				key_light_leds_control(32,1);   //打开所有的led
			else
				key_light_leds_control(32,0);   //关闭所有的led
		break;
		case eMCU_LED_STATUS_TYPE:		  //led 状态获取			
			buf[2] = get_led_status(cmd[1]); //获得的值保存在buf[2]中发回去		
		//	Lcd_pwm_change(10);
			break;
		case eMCU_LEDSETPWM_TYPE:   //设置led的pwm 亮度值
//			set_Led_Pwm(cmd[1]);
			break;
		case eMCU_GET_TEMP_TYPE:    //获得单片机的温度
			buf[2] = get_internal_temp()/100;   //只要整数部分。
			break;
		case eMCU_HWTD_SETONOFF_TYPE:  //设置看门狗打开或者关闭
			if(cmd[1])
				hard_wtd_enable();   //打开看门狗
			else
				hard_wtd_disable();  //关闭
			break;
		case eMCU_HWTD_FEED_TYPE:  //设置看门狗打开或者关闭
			hard_wtd_feed();  //喂狗
			break;
		case eMCU_HWTD_SETTIMEOUT_TYPE:  //设置看门狗喂狗时间
			hard_wtd_set_timeout(cmd[1]);  
			break;
		case eMCU_HWTD_GETTIMEOUT_TYPE:  //获取看门狗喂狗时间
			buf[2] = hard_wtd_get_timeout();  
			break;
		case eMCU_RESET_COREBOARD_TYPE:  //复位核心板
			hard_wtd_reset_3399board();  //
			break;
		case eMCU_RESET_LCD_TYPE:  //复位lcd 9211（复位引脚没有连通）
			//LT9211_Config();
		//	cmd_init_9211 = 1;   //在main中去复位
//			g_task_id |= 1<<4 ;  //在main中去复位 2022-09-06
			break;
		case eMCU_RESET_LFBOARD_TYPE:  //复位底板，好像没有这个功能！！！
			//nothing to do  20220812
			break;
		case eMCU_MICCTRL_SETONOFF_TYPE:  //设置mic_ctrl引脚的高低电平
			MicCtl_Control_OutHigh(cmd[1]); //高低电平 非0为高，0为低
			break;
		default:
			buf[2] = 255;   //表示失败
			//不可识别指令，返回错误码
			DBG_PRINTF("error: cpu uart send unkown cmd!! cmd = %#x\r\n",cmd[0]); 
			break;
	}
	buf[3] = CheckSum_For_Uart(buf,3);    //计算并存储校验和，
//	printf(".x");
	Com_Cpu_Uart_Tx_String(buf, 4);   //从串口1 发送数据
}


//void Com_Frame_Handle(frame_buf_t* buf, Queue_UART_STRUCT* Queue_buf,message_handle handle)
//{
//	uint8_t length,i,j;
//	uint8_t offset = 0;  //这两个变量用于帧错误的情况
//	
//	while(1)  //考虑到可能接收到多帧要处理的情况
//	{		
//		length = QueueUARTDataLenGet(Queue_buf);	
//		if(length < buf->datalen )   //（包括帧头）小于7个字节，不是完整的一帧
//		{	
//			return ;   //继续等待
//		}	
//		length = buf->datalen;   //计算需要读出的字节数
//		offset = CPU_UART_CMD_LEN - buf->datalen ;    //缓存中偏移字节
//		for(i=0;i<length;i++)
//		{
//			//这里不判断错误了，前面已经检测确实有这么多字节。
//			QueueUARTDataDele(Queue_buf,buf->com_handle_buf+i+offset) ;  //com_data 空出1个字节，为了兼容之前的校验和算法，及数据解析算法
//		}

//		if((buf->com_handle_buf[0] == CPU_UART_HEAD1) /*&& (buf->com_handle_buf[1] == CPU_UART_HEAD2) */ &&(0 == Uart_Verify_Data_CheckSum(buf->com_handle_buf,CPU_UART_CMD_LEN)))   //第二参数是数据总长度，包括校验和共7个字节
//		{
//			//校验和正确。
//			handle(&buf->com_handle_buf[1]);   //只要传命令过去就可以了		
//		}	
//		else  //校验和不正确，可能是帧有错误。
//		{
//			for(i=1;i<CPU_UART_CMD_LEN;i++)   //前面的判断出问题，考虑是帧错误，寻找下一个帧头！！！
//			{
//				if(buf->com_handle_buf[i] == CPU_UART_HEAD1)   //中间找到一个帧头
//				{
//					break;
//				}
//			}		
//			if(i != CPU_UART_CMD_LEN) //在数据中间找到帧头！！！
//			{
//				buf->datalen = i;   //下一次需要读的字节数
//			//	offset = FRAME_LENGHT-i;  //存储的偏移位置的计算

//				for(j=0;i<CPU_UART_CMD_LEN;i++,j++)   //有可能帧头不对，所以第一个字节还是要拷贝一下
//				{
//					buf->com_handle_buf[j] = buf->com_handle_buf[i];   //把剩下的拷贝过去
//				}
//			}
//			else  //在数据中间没有找到帧头
//			{
//				buf->datalen = CPU_UART_CMD_LEN;  //	下一次需要读的字节数
//			//	offset = 0;
//			}
//		}	
//	}//end while 1
//}









//与cpu通信串口的接收任务
void Com_ToCPU_Recv_Task(void * parameter)
{      
//	uint8_t flag = 0;
//	uint32_t ulNotificationValue;
//	BaseType_t xHigherPriorityTaskWoken = portMAX_DELAY;  //无限制等待
	size_t bytes,i,j;	
	uint8_t datalen= CPU_UART_CMD_LEN,offset=0;  //datalen表示需要读的字节数
	uint8_t buf[CPU_UART_CMD_LEN];   //接收缓存
	
	Com_ToCpu_init(115200);
	
	
	while(1)
	{
//		xTaskNotifyWait(ULONG_MAX,        //进入时，清零哪些位，0表示都不清零，ULONG_MAX的时候就会将任务通知值清零
//						0,   //退出时，清除对应的位，0表示都不清零
//						&ulNotificationValue,  //返回的通知值
//						portMAX_DELAY);    //无限等待
		ulTaskNotifyTake(ULONG_MAX,  //退出时，清除对应的位，0表示都不清零
						portMAX_DELAY); //无限等待
		
		do{
		
			bytes = xStreamBufferBytesAvailable(tocpu_rx_StreamBuffer_Handle);  //获取缓存的字节数
			
			if(bytes < datalen)  //至少等于帧长，datalen不为CPU_UART_CMD_LEN表示buf可能存在数据
				break;

			offset = CPU_UART_CMD_LEN - datalen;  //缓存的偏移

			if(xStreamBufferReceive(tocpu_rx_StreamBuffer_Handle,buf+offset,datalen,0))  //无限等待改为不等待了
			{  //如果缓存有数据
				if((buf[0] == CPU_UART_HEAD1) && (0 == Uart_Verify_Data_CheckSum(buf,CPU_UART_CMD_LEN)))  //找到数据帧的头部
				{												
					AnswerCpu_data(buf+1);
				}
				else  //校验和不正确，可能是帧有错误。
				{
					for(i=1;i<CPU_UART_CMD_LEN;i++)   //前面的判断出问题，考虑是帧错误，寻找下一个帧头！！！
					{
						if(buf[i] == CPU_UART_HEAD1)   //中间找到一个帧头
						{
							break;
						}
					}		
					if(i != CPU_UART_CMD_LEN) //在数据中间找到帧头！！！
					{
						datalen = i;   //下一次需要读的字节数
					//	offset = FRAME_LENGHT-i;  //存储的偏移位置的计算

						for(j=0;i<CPU_UART_CMD_LEN;i++,j++)   //有可能帧头不对，所以第一个字节还是要拷贝一下
						{
							buf[j] = buf[i];   //把剩下的拷贝过去
						}
					}
					else  //在数据中间没有找到帧头
					{
						datalen = CPU_UART_CMD_LEN;  //	下一次需要读的字节数
					//	offset = 0;
					}
				}										
			}
			else //读缓存数据出现问题
				break;
			//bytes -= CPU_UART_HEAD1+i-1;  //计算剩余字节				
		}while((offset-datalen) >= CPU_UART_CMD_LEN);  //剩余字节多余一帧		
	}         
}



