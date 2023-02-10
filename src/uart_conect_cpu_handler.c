

/*
用于处理与cpu之间的串口通信
	串口为GD32的串口1，115200，8N1

*/

#include "includes.h"
#include "string.h"


#define FIXED_UART_CPU_LEN  //固定长度，没有定义则为非固定长度
#define CPU_UART_CMD_LEN 4 //cpu发出的都是4个字节


#define CPU_UART_HEAD1 0xa5    //注意与cpu端保持一致
//#define CPU_UART_HEAD2 0x5a  //可能有用，不要删除


static StreamBufferHandle_t tocpu_tx_StreamBuffer_Handle;   //注意是静态的！！！！
static StreamBufferHandle_t tocpu_rx_StreamBuffer_Handle;

TaskHandle_t  TaskHandle_ToCpu_Com;   //存放调试串口任务指针



static void Com_ToCpu_init(uint32_t bandrate)
{
	gd_eval_com_init(TOCPU_COM_NUM,bandrate);  //用于调试

	// Initialise transmit and receive message queues
	tocpu_tx_StreamBuffer_Handle = xStreamBufferCreate(16, 1);  //缓冲大小，触发字节数
	tocpu_rx_StreamBuffer_Handle = xStreamBufferCreate(16, 12);  //不需要这个触发了，所以触发字节大一点
}



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
}



static int _Com_Cpu_io_putchar(int c)
{
	const TickType_t x10ms = pdMS_TO_TICKS( 10 );
	
	xStreamBufferSend(tocpu_tx_StreamBuffer_Handle,&c,1,x10ms);  //一直等待，直到成功，感觉可能有点问题

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
	uint8_t isreply = 0;   //0表示不应答，1表示应答
//	uint8_t dat;
	buf[1] = cmd[0];   //用于应答的指示
	buf[2] = 0;   //表示成功，255表示失败
	
	if(more_debug_info)  //选择增加打印信息
		MY_PRINTF("recive from Cpu_data cmd = %d data = %d\r\n",cmd[0],cmd[1]);
	
	switch(cmd[0])
	{
		case eMCU_LED_SETON_TYPE: //设置led ON
			key_light_leds_control(cmd[1],1);
			isreply = 0;
			break;
		case eMCU_LED_SETOFF_TYPE: //设置led OFF
			key_light_leds_control(cmd[1],0);
			isreply = 0;
		break;
		case eMCU_LCD_SETONOFF_TYPE:  //设置lcd 打开或者关闭,7寸屏控制
			if(cmd[1])
				Enable_LcdLight();   //打开屏幕
			else
				Disable_LcdLight();
			isreply = 0;
			break;
		case eMCU_LEDSETALL_TYPE:  //设置所有的led 打开或者关闭
			key_light_allleds_control(cmd[1]);
			isreply = 0;
//			if(cmd[1])
//				key_light_leds_control(40,1);   //打开所有的led
//			else
//				key_light_leds_control(40,0);   //关闭所有的led
		break;
		case eMCU_LED_STATUS_TYPE:		  //led 状态获取			
			buf[2] = get_led_status(cmd[1]); //获得的值保存在buf[2]中发回去		
			isreply = 1;
		//	Lcd_pwm_change(10);
			break;
		case eMCU_LEDSETPWM_TYPE:   //设置键灯led的pwm 亮度值
			set_Kleds_pwm_out(cmd[1]);
			isreply = 0;
		//	set_Led_Pwm(cmd[1]);
			break;
		case eMCU_GET_TEMP_TYPE:    //获得单片机的温度
			buf[2] = get_internal_temp()/100;   //只要整数部分。
			isreply = 1;
			break;
		case eMCU_HWTD_SETONOFF_TYPE:  //设置看门狗打开或者关闭
			if(cmd[1])
				hard_wtd_enable();   //打开看门狗
			else
				hard_wtd_disable();  //关闭
			isreply = 0;
			break;
		case eMCU_HWTD_FEED_TYPE:  //设置看门狗打开或者关闭
			hard_wtd_feed();  //喂狗
			isreply = 0;
			break;
		case eMCU_HWTD_SETTIMEOUT_TYPE:  //设置看门狗喂狗时间
			hard_wtd_set_timeout(cmd[1]);  
			isreply = 0;
			break;
		case eMCU_HWTD_GETTIMEOUT_TYPE:  //获取看门狗喂狗时间
			buf[2] = hard_wtd_get_timeout(); 
			isreply = 1;		
			break;
		case eMCU_RESET_COREBOARD_TYPE:  //复位核心板
			hard_wtd_reset_3399board();  //
			isreply = 0;
			break;
		case eMCU_RESET_LCD_TYPE:  //复位lcd 9211（复位引脚没有连通）
			if(xTaskGetHandle("lt9211") == NULL)  //如果没有这个任务
			{
			//	gpio_bit_reset(GPIOC, GPIO_PIN_2);  //OE3 输出低
			//	SHTDB_5IN_Disable();
				xTaskCreate(LT9211_Once_Task,"lt9211",configMINIMAL_STACK_SIZE+16,NULL,4,NULL);
			}	
			isreply = 0;
			break;
		case eMCU_RESET_LFBOARD_TYPE:  //复位底板
			//nothing to do  20220812
			//2022-12-19 改为单片机重启
			my_mcu_retart();
			isreply = 0;
			break;
		case eMCU_MICCTRL_SETONOFF_TYPE:  //设置mic_ctrl引脚的高低电平,已取消，改为3399控制，2022-12-12
			//nothing to do  20221213
		//	MicCtl_Control_OutHigh(cmd[1]); //高低电平 非0为高，0为低
			isreply = 0;
			break;
		case eMCU_LEDS_FLASH_TYPE:   //led闪烁控制,
			light_leds_add_flash(cmd[1]);
			isreply = 0;
			break;	
		case eMCU_LSPK_SETONOFF_TYPE:  //设置mic_ctrl引脚的高低电平			
			LSPK_Control_SetOutVal(cmd[1]);
			isreply = 0;
			break;
		case eMCU_GET_LCDTYPE_TYPE:
			buf[2] = get_LcdType_val();  //返回值0表示5寸屏，非0表示7寸屏,2022-12-12
			isreply = 1;
			break;
		case eMCU_V12_CTL_SETONOFF_TYPE:
			V12_CTL_Control_SetOutVal(cmd[1]); //高低电平 非0为高，0为低
			isreply = 0;
			break;
		case eMCU_5INLCD_SETONOFF_TYPE:  //5inch lcd 背光控制
			SHTDB_5IN_Control_SetOutVal(cmd[1]); //非0输出高电平点亮5inch，0输出低熄灭 5inch lcd
			isreply = 0;
			break;
		case eMCU_GET_MCUVERSION_TYPE:
			buf[2] = GetMcuVersion();  //返回值0表示5寸屏，非0表示7寸屏,2022-12-12
			isreply = 1;
			break;
		default:
			buf[2] = 255;   //表示失败
			//不可识别指令，返回错误码
			debug_printf_string("error: cpu uart send unkown cmd!!\r\n");
			isreply = 0;
			//DBG_PRINTF("error: cpu uart send unkown cmd!! cmd = %#x\r\n",cmd[0]); 
			break;
	}
	if(isreply)   //是否每一条指令都需要应答。2023-01-15-722
	{  //2023-01-14 不是每一条指令都需要应答
		buf[3] = CheckSum_For_Uart(buf,3);    //计算并存储校验和，
	//	printf(".x");
		Com_Cpu_Uart_Tx_String(buf, 4);   //从串口1 发送数据
	}
}





//与cpu通信串口的接收任务
void Com_ToCPU_Recv_Task(void * parameter)
{      
	size_t bytes,i,j;	
	uint8_t datalen= CPU_UART_CMD_LEN,offset=0;  //datalen表示需要读的字节数
	uint8_t buf[CPU_UART_CMD_LEN];   //接收缓存
	uint8_t read_ret = 0;
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
			read_ret = xStreamBufferReceive(tocpu_rx_StreamBuffer_Handle,buf+offset,datalen,0);//无限等待改为不等待了
			if(read_ret)  
			{  //如果缓存有数据
				bytes -= read_ret;  //读出这次之后，还有多少字节
				if((read_ret == datalen) && (buf[0] == CPU_UART_HEAD1) && (0 == Uart_Verify_Data_CheckSum(buf,CPU_UART_CMD_LEN)))  //找到数据帧的头部
				{												
					AnswerCpu_data(buf+1);
				}
				else  //校验和不正确,或者读出的字节数也不对，可能是帧有错误。
				{
					for(i=1;i<read_ret;i++)   //前面的判断出问题，考虑是帧错误，寻找下一个帧头！！！
					{
						if(buf[i] == CPU_UART_HEAD1)   //中间找到一个帧头
						{
							break;
						}
					}		
					if(i != read_ret) //在数据中间找到帧头！！！
					{
						datalen = i;   //下一次需要读的字节数
					//	offset = FRAME_LENGHT-i;  //存储的偏移位置的计算

						for(j=0;i<read_ret;i++,j++)   //有可能帧头不对，所以第一个字节还是要拷贝一下
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
		}while((bytes) >= CPU_UART_CMD_LEN);  //剩余字节多余一帧，2022-12-20，修正bug		
	}         
}



