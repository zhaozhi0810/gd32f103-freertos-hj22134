
/*
���ڴ�����cpu֮��Ĵ���ͨ��
	����ΪGD32�Ĵ���1��115200��8N1


���Դ��ڽ��յ����
0.�������ʱ���ַ���
1.��ѹ����
2.cpu�������¶ȣ�Һ�����¶ȣ���lcd����״̬������pwmֵ��ioģʽ��ֻ��0��100��
3.lcd������ֵ�����ļӵ�����״̬��pd_n��״̬
4.4·diֵ��4·��ͨ·��Ϣ
5.Ӳ�����Ź�״̬�����ź�Դ�����ޣ�
6.cpu����״̬���������ػ�������������pmon������ϵͳ�ȣ�

����ʶ�������Ҳ�Ǵ�ӡ��ʾ�ͱ���ʱ���ַ���

2022-09-09 ���ڴ��ڵ�freertos�ĵ��Թ���
1.��ӡ���֣�printf����fputc��fputcд�����ͻ����У�
2.������������ÿ�λ��������ݣ���ִ�д�ӡ����
3.���ղ��֣������жϣ����ö�����ָ�����������д�루ʹ��fromisr�ķ����������ջ��档
4.������������ÿ�λ��������ݾͽ������������������

ע�⣺
�������һֱ���ڽ����ٽ����Ĵ����С�port.c  configASSERT( ( portNVIC_INT_CTRL_REG & portVECTACTIVE_MASK ) == 0 );

�����
�������֣������ڽ����ж��У�Ȼ���ֵ����˽����ٽ����Ĵ��루printf�е��õģ����ڰٶ��п���˵�������ж��н����ٽ�����
�������¸����˷����������ж�ֻ����������ݣ�д�뻺�棬֮�����ݽ�����ʹ�ñ��������ɡ�

���ڽ���˿����ڽ����ٽ����Ĵ����е����⡣��������

*/



#include "includes.h"



#define DEBUG_UART_BAUD_RATE 115200

#define DEBUG_UART_SEND_BIT (1<<0)
#define DEBUG_UART_RECV_BIT (1<<1)

//#define RECV_BUF_LEN 64
#if 0
static Queue_UART_STRUCT g_Queue_Debug_Recv;   //����Debug���ݶ��У����ڽ����ж�

frame_buf_t g_com_debug_buf={{0},FRAME_LENGHT};    //���ݴ�����
#endif

//static char* g_Cpu_Run_Status_str[] = {
//	"LS3A_POWER_DISABLE",   //ȷ���Ƕϵ�״̬
//	"LS3A_POWEROFF",    //�ػ����ϵ�
//	"LS3A_REBOOT",    //����
//	"LS3A_RUNNING",    //����pmon
//	"LS3A_RUN_OS",      //�뿪pmon���������ϵͳ
//	"LS3A_POWER_ENABLE"     //�Ѿ�ͨ�磬����û�н���PMON��ǰһ��;
//};


static StreamBufferHandle_t _uart_tx_StreamBuffer_Handle;
static StreamBufferHandle_t _uart_rx_StreamBuffer_Handle;
//static QueueHandle_t _uart_write_mutex_Handle;
//static SemaphoreHandle_t xSemaphore;

//TaskHandle_t  TaskHandle_Debug_Com;   //��ŵ��Դ�������ָ��



void Com_Debug_init(uint32_t bandrate)
{
	gd_eval_com_init(DEBUG_COM_NUM,DEBUG_UART_BAUD_RATE);  //���ڵ���
	
		// Disable stream buffers so I/O occurs immediately
//	setvbuf(stdin,  NULL, _IONBF, 0); // should be a read-only stream
//	setvbuf(stdout, NULL, _IONBF, 0); // disables wait for \n before printing
//	setvbuf(stderr, NULL, _IONBF, 0); // should be already unbuffered
	
	
	// Initialise transmit and receive message queues
	_uart_tx_StreamBuffer_Handle = xStreamBufferCreate(64, 1);  //�����С�������ֽ���
	_uart_rx_StreamBuffer_Handle = xStreamBufferCreate(64,  1);

	// Initialise write mutex  QueueHandle_t xQueueCreateMutex
	//xSemaphore  = xSemaphoreCreateMutex() ;//xQueueCreateMutex(queueQUEUE_TYPE_MUTEX);

}



// Make printf thread safe via linker wrapping and protecting the internals with a mutex
//int __wrap_printf(char *fmt, ...)
//{
//    int result = 0;
//	/* ͨ����ֵ�ź���ʵ����Դ������ʣ����õȴ�ֱ����Դ���� */
//    if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdPASS)
//    {
//        va_list args;
//        va_start(args, fmt);
//        result = vprintf(fmt, args);
//        va_end(args);
//        xSemaphoreGive(xSemaphore);/* vTaskDelayUntil �Ǿ����ӳ٣�vTaskDelay ������ӳ١�*/
//    }
//    return result;
//}

static int __io_putchar(int c)
{
    // Queue char for transmission, block if queue full
//    osMessageQueuePut(_uart_tx_queue_id, &c, 0U, osWaitForever);
	const TickType_t x10ms = pdMS_TO_TICKS( 10 );
	
	xStreamBufferSend(_uart_tx_StreamBuffer_Handle,&c,1,x10ms);  //һֱ�ȴ���ֱ���ɹ����о������е�����
//	xTaskNotify(TaskHandle_Debug_Com, DEBUG_UART_SEND_BIT, eSetBits);  //�������ߵ�����
	
    // Enable TXE interrupt
    //LL_USART_EnableIT_TXE(USARTx_INSTANCE); 
	usart_interrupt_enable(EVAL_COM0, USART_INT_TBE);    //���ͻ���յ��ж�
    return c;
}

int __io_getchar(void)
{
    // Dequeue received char, block if queue empty
    uint8_t dat;

	dat = (uint8_t)usart_data_receive(EVAL_COM0);//(USART3); 
		
	//static uint8_t c;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;  //Ҫ��Ҫ�л��������ڷ��ز���
//    osMessageQueueGet(_uart_rx_queue_id, &c, NULL, osWaitForever);
	xStreamBufferSendFromISR(_uart_rx_StreamBuffer_Handle,&dat,1,&xHigherPriorityTaskWoken);  //�������ڵ����ݣ�д��buf
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

//base ��ʾ���ƣ�10Ϊ10���ƣ�������Ϊ16����
void debug_printf_u32(uint32_t dat,uint8_t base)
{
	char arr[16] = {0};
	uint8_t i;
	
	arr[15] = 0;
	if(base == 10)  //10�������
	{
		for(i=14;(i<16) && dat;i--)
		{
			arr[i] = dat%10 + '0';
			dat /=10;
		}
	}
	else  //16�������
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
	debug_printf_string(" ");  //��һ���ո�
}


//base ��ʾ���ƣ�10Ϊ10���ƣ�������Ϊ16����
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




//�����������������Դ��ڽ��յ��ļ򵥵ĵ�������
static void Com_Debug_Message_Handle1(uint8_t buf)
{
//	uint8_t t;
	switch(buf)
	{
		default:   //cmd��ӡ��ʱ�򣬿��ܳ����˿���ʾ�ַ�������
			debug_printf_string("ERROR: Command Unknow \r\n");   //����ʶ�������
			Com_Debug_Print_Help();
		case '0':
			debug_printf_string((char*)g_build_time_str);  //��ӡ�����ʱ��
			debug_printf_string("\r\n");
			debug_printf_string("Author:JC&DaZhi <vx:285408136>\r\n"); 
		break;
		case '1':
//			if(g_lcd_pwm < 100)
//			{
//				Lcd_pwm_out(g_lcd_pwm + 10);   //��Ļ���ȼ�10
//				debug_printf_string("increase 7 inch lcd PWM\r\n");
//			}
//			else
				debug_printf_string("g_lcd_pwm = 100\r\n");
			break;
		case '2':
			//��ӡ�¶�ֵ
//			if(g_lcd_pwm >= 10)
//			{
//				Lcd_pwm_out(g_lcd_pwm - 10);   //��Ļ���ȼ�10
//				debug_printf_string("decrease 7 inch lcd PWM\r\n");
//			}
//			else
				debug_printf_string("g_lcd_pwm = 0\r\n");
			break;
		case '3':
			query_task();
			
			break;
		case '4':
			debug_printf_string("reset LCD & 9211\r\n");  //lcd�ӵ�״̬
			LT9211_Config();
			break;
		case '5':
			debug_printf_string("Watch Dog Status : off\r\n");   //��ʱû�п���
			break;
		case '6':
//			debug_printf_string("Mcu internal_temp = %d\r\n",get_internal_temp());
			break;
		case '7':
			debug_printf_string("reset core board!!\r\n");  //lcd�ӵ�״̬
			hard_wtd_reset_3399board();
			break;
	}
}


/*
	�������ݽ����жϣ�
		ǰ�᣺ÿһ֡����7���ֽڡ�
		�����б���֡ͷ���к�������ݺ�У��ͣ���7���ֽڣ�
*/
//void Com_Debug_Rne_Int_Handle(void)
//{
//	uint8_t dat;

//	dat = (uint8_t)usart_data_receive(EVAL_COM0);//(USART3);  
//	Com_Debug_Message_Handle1(dat);   //ֱ�Ӵ���
////	QueueUARTDataInsert(&g_Queue_Debug_Recv,dat);   //���յ����ݴ�������С�
//}







static void _uart_error(void)
{
    if (usart_interrupt_flag_get(EVAL_COM0, USART_INT_FLAG_PERR)) 
    {
        debug_printf_string("*** USART parity error! ***\n");
		usart_interrupt_flag_clear(USART0, USART_INT_FLAG_PERR);   //���жϱ�־
    }

    else if (usart_interrupt_flag_get(EVAL_COM0, USART_INT_FLAG_ERR_FERR)) 
    {
        debug_printf_string("*** USART frame error! ***\n");
		usart_interrupt_flag_clear(USART0, USART_INT_FLAG_ERR_FERR);   //���жϱ�־
    }

    else if (usart_interrupt_flag_get(EVAL_COM0, USART_INT_FLAG_ERR_ORERR)) 
    {
        debug_printf_string("*** USART overrun error! ***\n");
		usart_interrupt_flag_clear(USART0, USART_INT_FLAG_ERR_ORERR);   //���жϱ�־
    }

    else if (usart_interrupt_flag_get(EVAL_COM0, USART_INT_FLAG_ERR_NERR)) 
    {
        debug_printf_string("*** USART noise error! ***\n");
		usart_interrupt_flag_clear(USART0, USART_INT_FLAG_ERR_NERR);   //���жϱ�־
    }
}



static void debug_uart_send_isr(void)
{
	uint8_t c;
	BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
	size_t bytes = xStreamBufferBytesAvailable(_uart_tx_StreamBuffer_Handle);
	
	if(!bytes)   //û������
	{
		usart_interrupt_disable(EVAL_COM0, USART_INT_TBE);    //���ͻ���յ��ж�
		return;
	}
	else
	{
		if(xStreamBufferReceiveFromISR(_uart_tx_StreamBuffer_Handle,&c,1,&pxHigherPriorityTaskWoken)) //���ȴ�
		{  //�������������
			Uart_Tx(DEBUG_COM_NUM, c);
		}
		else  //����û������
		{
			usart_interrupt_disable(EVAL_COM0, USART_INT_TBE);    //���ͻ���յ��ж�
		}
	}
	
}



//���Դ����жϴ�����
void USART0_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if(usart_interrupt_flag_get(EVAL_COM0, USART_INT_FLAG_RBNE))
    {
		usart_interrupt_flag_clear(EVAL_COM0, USART_INT_FLAG_RBNE);   //���жϱ�־
		__io_getchar();
//		xTaskNotifyFromISR(TaskHandle_Debug_Com, DEBUG_UART_RECV_BIT, eSetBits, &xHigherPriorityTaskWoken);  //�������ߵ�����
    }
	else if(usart_interrupt_flag_get(EVAL_COM0, USART_INT_FLAG_TBE))  //�����ж�
	{
		usart_interrupt_flag_clear(EVAL_COM0, USART_INT_FLAG_TBE);   //���жϱ�־
		debug_uart_send_isr();
	}
    else //if(usart_interrupt_flag_get(EVAL_COM0, USART_INT_FLAG_RBNE) /*&& LL_USART_IsActiveFlag_NE(USARTx_INSTANCE)*/)
    {
        _uart_error();
    }
}






//���Դ��ڵĽ�������
void Com_Debug_Recv_Task(void * parameter)
{      
	uint8_t c;
//	BaseType_t xHigherPriorityTaskWoken = portMAX_DELAY;  //�����Ƶȴ�
//	vTaskDelay(20000);	
	while(1)
	{		
		if(xStreamBufferReceive(_uart_rx_StreamBuffer_Handle,&c,1,portMAX_DELAY))
		{  //�������������
			Com_Debug_Message_Handle1(c);
		}
		else  //����û������
		{
			
		}
	}         
}



//���Դ��ڵĴ�ӡ����
//void Com_Debug_Print_Task(void * parameter)
//{      
////	BaseType_t err = pdPASS;
//	uint8_t c;
//	BaseType_t xHigherPriorityTaskWoken = portMAX_DELAY;  //�����Ƶȴ�
////    osMessageQueueGet(_uart_rx_queue_id, &c, NULL, osWaitForever);
////	size_t ret ;
//	
//	
//	while(1)
//	{
//		if(xStreamBufferReceive(_uart_tx_StreamBuffer_Handle,&c,1,xHigherPriorityTaskWoken))
//		{  //�������������
//			Uart_Tx(DEBUG_COM_NUM, c);
//		}
//		else  //����û������
//		{
//			
//		}
//	}
//         
//}


//���Դ��ڵĴ�ӡ���񣬰Ѵ�ӡ�ͽ��յ��������һ��
//void Com_Debug_Task(void * parameter)
//{      
////	BaseType_t err = pdPASS;
//	uint8_t c;
//	uint32_t ulNotificationValue;
//	//TickType_t xTicksToWait
////	BaseType_t xHigherPriorityTaskWoken = portMAX_DELAY;  //�����Ƶȴ�
////    osMessageQueueGet(_uart_rx_queue_id, &c, NULL, osWaitForever);
//	size_t bytes,i ;
//	
//	
//	while(1)
//	{
//		
//		xTaskNotifyWait(ULONG_MAX,        //����ʱ��������Щλ��0��ʾ�������㣬ULONG_MAX��ʱ��ͻὫ����ֵ֪ͨ����
//						0,   //�˳�ʱ�������Ӧ��λ��0��ʾ��������
//						&ulNotificationValue,  //���ص�ֵ֪ͨ
//						portMAX_DELAY);    //���޵ȴ�

//		if(ulNotificationValue & DEBUG_UART_SEND_BIT)  //����λ������
//		{
//			bytes = xStreamBufferBytesAvailable(_uart_tx_StreamBuffer_Handle);
//			
//			for(i=0;i<bytes;i++)
//			{
//				if(xStreamBufferReceive(_uart_tx_StreamBuffer_Handle,&c,1,0)) //���ȴ�
//				{  //�������������
//					Uart_Tx(DEBUG_COM_NUM, c);
//				}
//				else  //����û������
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
//				if(xStreamBufferReceive(_uart_rx_StreamBuffer_Handle,&c,1,0)) //���ȴ�
//				{  //�������������
//					//Uart_Tx(DEBUG_COM_NUM, c);
//					Com_Debug_Message_Handle1(c);   //������յ�������
//					//Com_Debug_Rne_Int_Handle();
//				}
//				else  //����û������
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








//���Դ������printf
/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
	//Uart_Tx(DEBUG_COM_NUM, ch);   //�Ӵ���0���
    __io_putchar(ch);
	return ch;
}




/*
	�����յ������Ĵ������ڵĿ����жϴ���������
		ǰ�᣺ �յ����������ݰ���У�����ȷ��

	��Ƭ���ܹ��յ������
	// 1.������ƵԴ,û�иù���
	4.����lcd��pwm�����ȣ�
	5.�ػ����������

*/

#if 0

static void Com_Debug_Message_Handle(uint8_t* buf)
{		
	com_frame_t* pdata = (com_frame_t*)(buf+1);    //+1������֡ͷ��ʹ�ýṹ���ʼ��
	int8_t t;
	
	switch(pdata->data_type)
    {
        case eMCU_CMD_TYPE:    //cpu���͸���Ƭ���Ķ���cmd����
            t = pdata->data.cmd.cmd;
            switch(t)
            {
				case eMCU_CPUGETINFO_CMD:   //��ȡ�豸��Ϣ������
				//	AnswerCpu_GetInfo(pdata->data.cmd.param1<<8 | pdata->data.cmd.param2); //ʹ�ú�������������������
					break;
				case eMCU_CPUSET_CMD:    //������Ļ����
					if(pdata->data.cmd.param1 == eMCU_LCD_SETPWM_CMD)
					{
						t = pdata->data.cmd.param2;   //���ֵ�����ɸ���������������������������
						t = g_lcd_pwm + t;   //����ó��µĽ��
						Lcd_pwm_out(t);     //��������pwm��ֵ
				//		AnswerCpu_Status(eUART_SUCCESS);   //Ӧ��ɹ�
					}
					else if(pdata->data.cmd.param1 == eMCU_SWITCH_DVI_SRC_CMD) //�л���ƵԴ
					{
						t = pdata->data.cmd.param2;  //0 Ϊ���أ�1Ϊ�ⲿ
//						if(t)
//							dvi_switch_set(DVI_OTHER);   //���ú���ϱ���cpu
//						else
//							dvi_switch_set(DVI_LOONGSON);   //������Ƶ
				//		AnswerCpu_Status(eUART_SUCCESS);   //Ӧ��ɹ�
					}
					else	
				//		AnswerCpu_Status(eUART_ERR_PARAM);  //Ӧ���������				
				break;
                default:
					DBG_PRINTF("ERROR: %s\n","eUART_ERR_PARAM");
				//	AnswerCpu_Status(eUART_ERR_PARAM);  //Ӧ���������
                break;
            }

        break;
        default:
			DBG_PRINTF("ERROR: %s\n","eUART_ERR_CMD_UNKNOW");
		//	AnswerCpu_Status(eUART_ERR_CMD_UNKNOW);  //Ӧ������δ֪ 
        break;
    }	
}
#endif



/*
	���ڿ����жϵĴ���,���Դ��ڲ��ٿ��������ж�

	1.�жϽ��յ����ֽ�����>=7 ��ʾ����
	2.�����ͼ�����������7���ֽڣ�����У��ͣ�
	3.У�����ȷ����������
*/
void Com_Debug_Idle_Int_Handle(void)
{
//	Com_Frame_Handle(&g_com_debug_buf, &g_Queue_Debug_Recv,Com_Debug_Message_Handle);
}


