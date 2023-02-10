

/*
���ڴ�����cpu֮��Ĵ���ͨ��
	����ΪGD32�Ĵ���1��115200��8N1

*/

#include "includes.h"
#include "string.h"


#define FIXED_UART_CPU_LEN  //�̶����ȣ�û�ж�����Ϊ�ǹ̶�����
#define CPU_UART_CMD_LEN 4 //cpu�����Ķ���4���ֽ�


#define CPU_UART_HEAD1 0xa5    //ע����cpu�˱���һ��
//#define CPU_UART_HEAD2 0x5a  //�������ã���Ҫɾ��


static StreamBufferHandle_t tocpu_tx_StreamBuffer_Handle;   //ע���Ǿ�̬�ģ�������
static StreamBufferHandle_t tocpu_rx_StreamBuffer_Handle;

TaskHandle_t  TaskHandle_ToCpu_Com;   //��ŵ��Դ�������ָ��



static void Com_ToCpu_init(uint32_t bandrate)
{
	gd_eval_com_init(TOCPU_COM_NUM,bandrate);  //���ڵ���

	// Initialise transmit and receive message queues
	tocpu_tx_StreamBuffer_Handle = xStreamBufferCreate(16, 1);  //�����С�������ֽ���
	tocpu_rx_StreamBuffer_Handle = xStreamBufferCreate(16, 12);  //����Ҫ��������ˣ����Դ����ֽڴ�һ��
}



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
}



static int _Com_Cpu_io_putchar(int c)
{
	const TickType_t x10ms = pdMS_TO_TICKS( 10 );
	
	xStreamBufferSend(tocpu_tx_StreamBuffer_Handle,&c,1,x10ms);  //һֱ�ȴ���ֱ���ɹ����о������е�����

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
	uint8_t isreply = 0;   //0��ʾ��Ӧ��1��ʾӦ��
//	uint8_t dat;
	buf[1] = cmd[0];   //����Ӧ���ָʾ
	buf[2] = 0;   //��ʾ�ɹ���255��ʾʧ��
	
	if(more_debug_info)  //ѡ�����Ӵ�ӡ��Ϣ
		MY_PRINTF("recive from Cpu_data cmd = %d data = %d\r\n",cmd[0],cmd[1]);
	
	switch(cmd[0])
	{
		case eMCU_LED_SETON_TYPE: //����led ON
			key_light_leds_control(cmd[1],1);
			isreply = 0;
			break;
		case eMCU_LED_SETOFF_TYPE: //����led OFF
			key_light_leds_control(cmd[1],0);
			isreply = 0;
		break;
		case eMCU_LCD_SETONOFF_TYPE:  //����lcd �򿪻��߹ر�,7��������
			if(cmd[1])
				Enable_LcdLight();   //����Ļ
			else
				Disable_LcdLight();
			isreply = 0;
			break;
		case eMCU_LEDSETALL_TYPE:  //�������е�led �򿪻��߹ر�
			key_light_allleds_control(cmd[1]);
			isreply = 0;
//			if(cmd[1])
//				key_light_leds_control(40,1);   //�����е�led
//			else
//				key_light_leds_control(40,0);   //�ر����е�led
		break;
		case eMCU_LED_STATUS_TYPE:		  //led ״̬��ȡ			
			buf[2] = get_led_status(cmd[1]); //��õ�ֵ������buf[2]�з���ȥ		
			isreply = 1;
		//	Lcd_pwm_change(10);
			break;
		case eMCU_LEDSETPWM_TYPE:   //���ü���led��pwm ����ֵ
			set_Kleds_pwm_out(cmd[1]);
			isreply = 0;
		//	set_Led_Pwm(cmd[1]);
			break;
		case eMCU_GET_TEMP_TYPE:    //��õ�Ƭ�����¶�
			buf[2] = get_internal_temp()/100;   //ֻҪ�������֡�
			isreply = 1;
			break;
		case eMCU_HWTD_SETONOFF_TYPE:  //���ÿ��Ź��򿪻��߹ر�
			if(cmd[1])
				hard_wtd_enable();   //�򿪿��Ź�
			else
				hard_wtd_disable();  //�ر�
			isreply = 0;
			break;
		case eMCU_HWTD_FEED_TYPE:  //���ÿ��Ź��򿪻��߹ر�
			hard_wtd_feed();  //ι��
			isreply = 0;
			break;
		case eMCU_HWTD_SETTIMEOUT_TYPE:  //���ÿ��Ź�ι��ʱ��
			hard_wtd_set_timeout(cmd[1]);  
			isreply = 0;
			break;
		case eMCU_HWTD_GETTIMEOUT_TYPE:  //��ȡ���Ź�ι��ʱ��
			buf[2] = hard_wtd_get_timeout(); 
			isreply = 1;		
			break;
		case eMCU_RESET_COREBOARD_TYPE:  //��λ���İ�
			hard_wtd_reset_3399board();  //
			isreply = 0;
			break;
		case eMCU_RESET_LCD_TYPE:  //��λlcd 9211����λ����û����ͨ��
			if(xTaskGetHandle("lt9211") == NULL)  //���û���������
			{
			//	gpio_bit_reset(GPIOC, GPIO_PIN_2);  //OE3 �����
			//	SHTDB_5IN_Disable();
				xTaskCreate(LT9211_Once_Task,"lt9211",configMINIMAL_STACK_SIZE+16,NULL,4,NULL);
			}	
			isreply = 0;
			break;
		case eMCU_RESET_LFBOARD_TYPE:  //��λ�װ�
			//nothing to do  20220812
			//2022-12-19 ��Ϊ��Ƭ������
			my_mcu_retart();
			isreply = 0;
			break;
		case eMCU_MICCTRL_SETONOFF_TYPE:  //����mic_ctrl���ŵĸߵ͵�ƽ,��ȡ������Ϊ3399���ƣ�2022-12-12
			//nothing to do  20221213
		//	MicCtl_Control_OutHigh(cmd[1]); //�ߵ͵�ƽ ��0Ϊ�ߣ�0Ϊ��
			isreply = 0;
			break;
		case eMCU_LEDS_FLASH_TYPE:   //led��˸����,
			light_leds_add_flash(cmd[1]);
			isreply = 0;
			break;	
		case eMCU_LSPK_SETONOFF_TYPE:  //����mic_ctrl���ŵĸߵ͵�ƽ			
			LSPK_Control_SetOutVal(cmd[1]);
			isreply = 0;
			break;
		case eMCU_GET_LCDTYPE_TYPE:
			buf[2] = get_LcdType_val();  //����ֵ0��ʾ5��������0��ʾ7����,2022-12-12
			isreply = 1;
			break;
		case eMCU_V12_CTL_SETONOFF_TYPE:
			V12_CTL_Control_SetOutVal(cmd[1]); //�ߵ͵�ƽ ��0Ϊ�ߣ�0Ϊ��
			isreply = 0;
			break;
		case eMCU_5INLCD_SETONOFF_TYPE:  //5inch lcd �������
			SHTDB_5IN_Control_SetOutVal(cmd[1]); //��0����ߵ�ƽ����5inch��0�����Ϩ�� 5inch lcd
			isreply = 0;
			break;
		case eMCU_GET_MCUVERSION_TYPE:
			buf[2] = GetMcuVersion();  //����ֵ0��ʾ5��������0��ʾ7����,2022-12-12
			isreply = 1;
			break;
		default:
			buf[2] = 255;   //��ʾʧ��
			//����ʶ��ָ����ش�����
			debug_printf_string("error: cpu uart send unkown cmd!!\r\n");
			isreply = 0;
			//DBG_PRINTF("error: cpu uart send unkown cmd!! cmd = %#x\r\n",cmd[0]); 
			break;
	}
	if(isreply)   //�Ƿ�ÿһ��ָ���ҪӦ��2023-01-15-722
	{  //2023-01-14 ����ÿһ��ָ���ҪӦ��
		buf[3] = CheckSum_For_Uart(buf,3);    //���㲢�洢У��ͣ�
	//	printf(".x");
		Com_Cpu_Uart_Tx_String(buf, 4);   //�Ӵ���1 ��������
	}
}





//��cpuͨ�Ŵ��ڵĽ�������
void Com_ToCPU_Recv_Task(void * parameter)
{      
	size_t bytes,i,j;	
	uint8_t datalen= CPU_UART_CMD_LEN,offset=0;  //datalen��ʾ��Ҫ�����ֽ���
	uint8_t buf[CPU_UART_CMD_LEN];   //���ջ���
	uint8_t read_ret = 0;
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
			read_ret = xStreamBufferReceive(tocpu_rx_StreamBuffer_Handle,buf+offset,datalen,0);//���޵ȴ���Ϊ���ȴ���
			if(read_ret)  
			{  //�������������
				bytes -= read_ret;  //�������֮�󣬻��ж����ֽ�
				if((read_ret == datalen) && (buf[0] == CPU_UART_HEAD1) && (0 == Uart_Verify_Data_CheckSum(buf,CPU_UART_CMD_LEN)))  //�ҵ�����֡��ͷ��
				{												
					AnswerCpu_data(buf+1);
				}
				else  //У��Ͳ���ȷ,���߶������ֽ���Ҳ���ԣ�������֡�д���
				{
					for(i=1;i<read_ret;i++)   //ǰ����жϳ����⣬������֡����Ѱ����һ��֡ͷ������
					{
						if(buf[i] == CPU_UART_HEAD1)   //�м��ҵ�һ��֡ͷ
						{
							break;
						}
					}		
					if(i != read_ret) //�������м��ҵ�֡ͷ������
					{
						datalen = i;   //��һ����Ҫ�����ֽ���
					//	offset = FRAME_LENGHT-i;  //�洢��ƫ��λ�õļ���

						for(j=0;i<read_ret;i++,j++)   //�п���֡ͷ���ԣ����Ե�һ���ֽڻ���Ҫ����һ��
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
		}while((bytes) >= CPU_UART_CMD_LEN);  //ʣ���ֽڶ���һ֡��2022-12-20������bug		
	}         
}



