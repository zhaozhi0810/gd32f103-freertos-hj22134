

/*
���ڴ�����cpu֮��Ĵ���ͨ��
	����ΪGD32�Ĵ���1��115200��8N1

*/

#include "includes.h"
#include "string.h"


#define FIXED_UART_CPU_LEN  //�̶����ȣ�û�ж�����Ϊ�ǹ̶�����
#define CPU_UART_CMD_LEN 4 //cpu�����Ķ���4���ֽ�
//#define RECV_BUF_LEN 64
//static Queue_UART_STRUCT g_Queue_Cpu_Recv;   //����cpu���ݶ��У����ڽ����ж�
//frame_buf_t g_com_cpu_buf={{0},CPU_UART_CMD_LEN};    //����

#define CPU_UART_HEAD1 0xa5    //ע����cpu�˱���һ��
//#define CPU_UART_HEAD2 0x5a


static StreamBufferHandle_t tocpu_tx_StreamBuffer_Handle;   //ע���Ǿ�̬�ģ�������
static StreamBufferHandle_t tocpu_rx_StreamBuffer_Handle;

TaskHandle_t  TaskHandle_ToCpu_Com;   //��ŵ��Դ�������ָ��



void Com_ToCpu_init(uint32_t bandrate)
{
	gd_eval_com_init(TOCPU_COM_NUM,bandrate);  //���ڵ���

	// Initialise transmit and receive message queues
	tocpu_tx_StreamBuffer_Handle = xStreamBufferCreate(16, 1);  //�����С�������ֽ���
	tocpu_rx_StreamBuffer_Handle = xStreamBufferCreate(16, 12);  //����Ҫ��������ˣ����Դ����ֽڴ�һ��
}





//�����ʼ��
//void Com_Cpu_Recive_Buff_Init(void)
//{
//	memset((void *)&g_Queue_Cpu_Recv, 0, sizeof(g_Queue_Cpu_Recv));
//}




/*
	�������ݽ����жϣ�
		ǰ�᣺ÿһ֡����7���ֽڡ�
		�����б���֡ͷ���к�������ݺ�У��ͣ���7���ֽڣ�
*/
static void Com_Cpu_Rne_Int_Handle(void)
{
	uint8_t dat;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;  //Ҫ��Ҫ�л��������ڷ��ز���
	
	dat = (uint8_t)usart_data_receive(EVAL_COM1);//(USART3);   //���վʹ浽�����У���������2021-12-02
	
	xStreamBufferSendFromISR(tocpu_rx_StreamBuffer_Handle,&dat,1,&xHigherPriorityTaskWoken);  //�������ڵ����ݣ�д��buf
	
//	QueueUARTDataInsert(&g_Queue_Cpu_Recv,dat);   //���յ����ݴ�������С�

}


/*
	���ڿ����жϵĴ���

	1.�жϽ��յ����ֽ�����>=7 ��ʾ����
	2.�����ͼ�����������7���ֽڣ�����У��ͣ�
	3.У�����ȷ����������

*/
static void Com_Cpu_Idle_Int_Handle(void)
{
	vTaskNotifyGiveFromISR(TaskHandle_ToCpu_Com,NULL);  //��������
//	Com_Frame_Handle(&g_com_cpu_buf, &g_Queue_Cpu_Recv,AnswerCpu_data);
}



static int _Com_Cpu_io_putchar(int c)
{
    // Queue char for transmission, block if queue full
//    osMessageQueuePut(_uart_tx_queue_id, &c, 0U, osWaitForever);
	const TickType_t x10ms = pdMS_TO_TICKS( 10 );
	
	xStreamBufferSend(tocpu_tx_StreamBuffer_Handle,&c,1,x10ms);  //һֱ�ȴ���ֱ���ɹ����о������е�����
//	xTaskNotify(TaskHandle_Debug_Com, DEBUG_UART_SEND_BIT, eSetBits);  //�������ߵ�����
	
    // Enable TXE interrupt
    //LL_USART_EnableIT_TXE(USARTx_INSTANCE); 
	usart_interrupt_enable(EVAL_COM1, USART_INT_TBE);    //���ͻ���յ��ж�
    return c;
}



//����һ���ַ�
//com ��ʾ���ĸ��˿ڷ���
static void Com_Cpu_Uart_Tx_String(uint8_t *str, uint8_t length)
{
	uint8_t i;
	
	for(i = 0; i < length; i++)
		_Com_Cpu_io_putchar(str[i]);
	//	Uart_Tx(TOCPU_COM_NUM,str[i]);
}




//�����ж�
static void Com_Cpu_Send_Isr(void)
{
	uint8_t c;
	BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
	size_t bytes = xStreamBufferBytesAvailable(tocpu_tx_StreamBuffer_Handle);
	
	if(!bytes)   //û������
	{
		usart_interrupt_disable(EVAL_COM1, USART_INT_TBE);    //���ͻ���յ��ж�
		return;
	}
	else
	{
		if(xStreamBufferReceiveFromISR(tocpu_tx_StreamBuffer_Handle,&c,1,&pxHigherPriorityTaskWoken)) //���ȴ�
		{  //�������������
			Uart_Tx(TOCPU_COM_NUM, c);
		}
		else  //����û������
		{
			usart_interrupt_disable(EVAL_COM1, USART_INT_TBE);    //���ͻ���յ��ж�
		}
	}
	
}




//���Դ����жϴ�����
void USART1_IRQHandler(void)
{
	static uint8_t ide_int_enable = 0;
//	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if(usart_interrupt_flag_get(EVAL_COM1, USART_INT_FLAG_RBNE))
    {
		usart_interrupt_flag_clear(EVAL_COM1, USART_INT_FLAG_RBNE);   //���жϱ�־
	
		Com_Cpu_Rne_Int_Handle();
//		usart_interrupt_flag_clear(USART1, USART_INT_FLAG_RBNE);   //���жϱ�־
		if(ide_int_enable == 0)
		{
			usart_interrupt_enable(EVAL_COM1, USART_INT_IDLE);    //��������ж�
			ide_int_enable = 1;
		}
		
		//	__io_getchar();
//		xTaskNotifyFromISR(TaskHandle_Debug_Com, DEBUG_UART_RECV_BIT, eSetBits, &xHigherPriorityTaskWoken);  //�������ߵ�����
    }
	else if(usart_interrupt_flag_get(EVAL_COM1, USART_INT_FLAG_TBE))  //�����ж�
	{
		usart_interrupt_flag_clear(EVAL_COM1, USART_INT_FLAG_TBE);   //���жϱ�־
		Com_Cpu_Send_Isr();
	//	debug_uart_send_isr();
	}
	else if(usart_interrupt_flag_get(EVAL_COM1, USART_INT_FLAG_IDLE))  //�����жϣ���ʾһ֡�����ѽ���
	{
		//�������������
		Com_Cpu_Idle_Int_Handle();
		usart_interrupt_flag_clear(EVAL_COM1, USART_INT_FLAG_IDLE);//���жϱ�־
		if(ide_int_enable)
		{
			usart_interrupt_disable(EVAL_COM1, USART_INT_IDLE);    //��ֹ�����ж�
			ide_int_enable = 0;
		}		
	}
    else //if(usart_interrupt_flag_get(EVAL_COM0, USART_INT_FLAG_RBNE) /*&& LL_USART_IsActiveFlag_NE(USARTx_INSTANCE)*/)
    {
    //    _uart_error();
    }	
}





//Ӧ��cpu��������Ϣ������ errcodeΪ0��ʾ�ɹ�������ֵΪ������ ӦС��0x7f
void AnswerCpu_data(uint8_t *cmd)
{
	uint8_t buf[8] = {CPU_UART_HEAD1};    //ͷ����Ϣ
//	uint8_t dat;
	buf[1] = cmd[0];   //����Ӧ���ָʾ
	buf[2] = 0;   //��ʾ�ɹ���255��ʾʧ��

	switch(cmd[0])
	{
		case eMCU_LED_SETON_TYPE: //����led ON
			key_light_leds_control(cmd[1],1);
			break;
		case eMCU_LED_SETOFF_TYPE: //����led OFF
			key_light_leds_control(cmd[1],0);
		break;
		case eMCU_LCD_SETONOFF_TYPE:  //����lcd �򿪻��߹ر�
			if(cmd[1])
				Enable_LcdLight();   //����Ļ
			else
				Disable_LcdLight();
			break;
		case eMCU_LEDSETALL_TYPE:  //�������е�led �򿪻��߹ر�
			if(cmd[1])
				key_light_leds_control(32,1);   //�����е�led
			else
				key_light_leds_control(32,0);   //�ر����е�led
		break;
		case eMCU_LED_STATUS_TYPE:		  //led ״̬��ȡ			
			buf[2] = get_led_status(cmd[1]); //��õ�ֵ������buf[2]�з���ȥ		
		//	Lcd_pwm_change(10);
			break;
		case eMCU_LEDSETPWM_TYPE:   //����led��pwm ����ֵ
//			set_Led_Pwm(cmd[1]);
			break;
		case eMCU_GET_TEMP_TYPE:    //��õ�Ƭ�����¶�
			buf[2] = get_internal_temp()/100;   //ֻҪ�������֡�
			break;
		case eMCU_HWTD_SETONOFF_TYPE:  //���ÿ��Ź��򿪻��߹ر�
			if(cmd[1])
				hard_wtd_enable();   //�򿪿��Ź�
			else
				hard_wtd_disable();  //�ر�
			break;
		case eMCU_HWTD_FEED_TYPE:  //���ÿ��Ź��򿪻��߹ر�
			hard_wtd_feed();  //ι��
			break;
		case eMCU_HWTD_SETTIMEOUT_TYPE:  //���ÿ��Ź�ι��ʱ��
			hard_wtd_set_timeout(cmd[1]);  
			break;
		case eMCU_HWTD_GETTIMEOUT_TYPE:  //��ȡ���Ź�ι��ʱ��
			buf[2] = hard_wtd_get_timeout();  
			break;
		case eMCU_RESET_COREBOARD_TYPE:  //��λ���İ�
			hard_wtd_reset_3399board();  //
			break;
		case eMCU_RESET_LCD_TYPE:  //��λlcd 9211����λ����û����ͨ��
			//LT9211_Config();
		//	cmd_init_9211 = 1;   //��main��ȥ��λ
//			g_task_id |= 1<<4 ;  //��main��ȥ��λ 2022-09-06
			break;
		case eMCU_RESET_LFBOARD_TYPE:  //��λ�װ壬����û��������ܣ�����
			//nothing to do  20220812
			break;
		case eMCU_MICCTRL_SETONOFF_TYPE:  //����mic_ctrl���ŵĸߵ͵�ƽ
			MicCtl_Control_OutHigh(cmd[1]); //�ߵ͵�ƽ ��0Ϊ�ߣ�0Ϊ��
			break;
		default:
			buf[2] = 255;   //��ʾʧ��
			//����ʶ��ָ����ش�����
			DBG_PRINTF("error: cpu uart send unkown cmd!! cmd = %#x\r\n",cmd[0]); 
			break;
	}
	buf[3] = CheckSum_For_Uart(buf,3);    //���㲢�洢У��ͣ�
//	printf(".x");
	Com_Cpu_Uart_Tx_String(buf, 4);   //�Ӵ���1 ��������
}


//void Com_Frame_Handle(frame_buf_t* buf, Queue_UART_STRUCT* Queue_buf,message_handle handle)
//{
//	uint8_t length,i,j;
//	uint8_t offset = 0;  //��������������֡��������
//	
//	while(1)  //���ǵ����ܽ��յ���֡Ҫ��������
//	{		
//		length = QueueUARTDataLenGet(Queue_buf);	
//		if(length < buf->datalen )   //������֡ͷ��С��7���ֽڣ�����������һ֡
//		{	
//			return ;   //�����ȴ�
//		}	
//		length = buf->datalen;   //������Ҫ�������ֽ���
//		offset = CPU_UART_CMD_LEN - buf->datalen ;    //������ƫ���ֽ�
//		for(i=0;i<length;i++)
//		{
//			//���ﲻ�жϴ����ˣ�ǰ���Ѿ����ȷʵ����ô���ֽڡ�
//			QueueUARTDataDele(Queue_buf,buf->com_handle_buf+i+offset) ;  //com_data �ճ�1���ֽڣ�Ϊ�˼���֮ǰ��У����㷨�������ݽ����㷨
//		}

//		if((buf->com_handle_buf[0] == CPU_UART_HEAD1) /*&& (buf->com_handle_buf[1] == CPU_UART_HEAD2) */ &&(0 == Uart_Verify_Data_CheckSum(buf->com_handle_buf,CPU_UART_CMD_LEN)))   //�ڶ������������ܳ��ȣ�����У��͹�7���ֽ�
//		{
//			//У�����ȷ��
//			handle(&buf->com_handle_buf[1]);   //ֻҪ�������ȥ�Ϳ�����		
//		}	
//		else  //У��Ͳ���ȷ��������֡�д���
//		{
//			for(i=1;i<CPU_UART_CMD_LEN;i++)   //ǰ����жϳ����⣬������֡����Ѱ����һ��֡ͷ������
//			{
//				if(buf->com_handle_buf[i] == CPU_UART_HEAD1)   //�м��ҵ�һ��֡ͷ
//				{
//					break;
//				}
//			}		
//			if(i != CPU_UART_CMD_LEN) //�������м��ҵ�֡ͷ������
//			{
//				buf->datalen = i;   //��һ����Ҫ�����ֽ���
//			//	offset = FRAME_LENGHT-i;  //�洢��ƫ��λ�õļ���

//				for(j=0;i<CPU_UART_CMD_LEN;i++,j++)   //�п���֡ͷ���ԣ����Ե�һ���ֽڻ���Ҫ����һ��
//				{
//					buf->com_handle_buf[j] = buf->com_handle_buf[i];   //��ʣ�µĿ�����ȥ
//				}
//			}
//			else  //�������м�û���ҵ�֡ͷ
//			{
//				buf->datalen = CPU_UART_CMD_LEN;  //	��һ����Ҫ�����ֽ���
//			//	offset = 0;
//			}
//		}	
//	}//end while 1
//}









//��cpuͨ�Ŵ��ڵĽ�������
void Com_ToCPU_Recv_Task(void * parameter)
{      
//	uint8_t flag = 0;
//	uint32_t ulNotificationValue;
//	BaseType_t xHigherPriorityTaskWoken = portMAX_DELAY;  //�����Ƶȴ�
	size_t bytes,i,j;	
	uint8_t datalen= CPU_UART_CMD_LEN,offset=0;  //datalen��ʾ��Ҫ�����ֽ���
	uint8_t buf[CPU_UART_CMD_LEN];   //���ջ���
	
	Com_ToCpu_init(115200);
	
	
	while(1)
	{
//		xTaskNotifyWait(ULONG_MAX,        //����ʱ��������Щλ��0��ʾ�������㣬ULONG_MAX��ʱ��ͻὫ����ֵ֪ͨ����
//						0,   //�˳�ʱ�������Ӧ��λ��0��ʾ��������
//						&ulNotificationValue,  //���ص�ֵ֪ͨ
//						portMAX_DELAY);    //���޵ȴ�
		ulTaskNotifyTake(ULONG_MAX,  //�˳�ʱ�������Ӧ��λ��0��ʾ��������
						portMAX_DELAY); //���޵ȴ�
		
		do{
		
			bytes = xStreamBufferBytesAvailable(tocpu_rx_StreamBuffer_Handle);  //��ȡ������ֽ���
			
			if(bytes < datalen)  //���ٵ���֡����datalen��ΪCPU_UART_CMD_LEN��ʾbuf���ܴ�������
				break;

			offset = CPU_UART_CMD_LEN - datalen;  //�����ƫ��

			if(xStreamBufferReceive(tocpu_rx_StreamBuffer_Handle,buf+offset,datalen,0))  //���޵ȴ���Ϊ���ȴ���
			{  //�������������
				if((buf[0] == CPU_UART_HEAD1) && (0 == Uart_Verify_Data_CheckSum(buf,CPU_UART_CMD_LEN)))  //�ҵ�����֡��ͷ��
				{												
					AnswerCpu_data(buf+1);
				}
				else  //У��Ͳ���ȷ��������֡�д���
				{
					for(i=1;i<CPU_UART_CMD_LEN;i++)   //ǰ����жϳ����⣬������֡����Ѱ����һ��֡ͷ������
					{
						if(buf[i] == CPU_UART_HEAD1)   //�м��ҵ�һ��֡ͷ
						{
							break;
						}
					}		
					if(i != CPU_UART_CMD_LEN) //�������м��ҵ�֡ͷ������
					{
						datalen = i;   //��һ����Ҫ�����ֽ���
					//	offset = FRAME_LENGHT-i;  //�洢��ƫ��λ�õļ���

						for(j=0;i<CPU_UART_CMD_LEN;i++,j++)   //�п���֡ͷ���ԣ����Ե�һ���ֽڻ���Ҫ����һ��
						{
							buf[j] = buf[i];   //��ʣ�µĿ�����ȥ
						}
					}
					else  //�������м�û���ҵ�֡ͷ
					{
						datalen = CPU_UART_CMD_LEN;  //	��һ����Ҫ�����ֽ���
					//	offset = 0;
					}
				}										
			}
			else //���������ݳ�������
				break;
			//bytes -= CPU_UART_HEAD1+i-1;  //����ʣ���ֽ�				
		}while((offset-datalen) >= CPU_UART_CMD_LEN);  //ʣ���ֽڶ���һ֡		
	}         
}



