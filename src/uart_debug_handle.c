
/*
���ڵ��ԵĴ���ͨ��
	����ΪGD32�Ĵ���1��115200��8N1

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


static StreamBufferHandle_t _uart_tx_StreamBuffer_Handle;
static StreamBufferHandle_t _uart_rx_StreamBuffer_Handle;

uint8_t more_debug_info = 0;   //��ӡ����ĵ�����Ϣ��0�򲻴�ӡ




void Com_Debug_init(uint32_t bandrate)
{
	gd_eval_com_init(DEBUG_COM_NUM,DEBUG_UART_BAUD_RATE);  //���ڵ���
	
	_uart_tx_StreamBuffer_Handle = xStreamBufferCreate(64, 1);  //�����С�������ֽ���
	_uart_rx_StreamBuffer_Handle = xStreamBufferCreate(64,  1);

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
	const TickType_t x10ms = pdMS_TO_TICKS( 10 );
	
	xStreamBufferSend(_uart_tx_StreamBuffer_Handle,&c,1,x10ms);  //һֱ�ȴ���ֱ���ɹ����о������е�����

	usart_interrupt_enable(EVAL_COM0, USART_INT_TBE);    //���ͻ���յ��ж�
    return c;
}

int __io_getchar(void)
{
    // Dequeue received char, block if queue empty
    uint8_t dat;

	dat = (uint8_t)usart_data_receive(EVAL_COM0);//(USART3); 

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;  //Ҫ��Ҫ�л��������ڷ��ز���

	xStreamBufferSendFromISR(_uart_rx_StreamBuffer_Handle,&dat,1,&xHigherPriorityTaskWoken);  //�������ڵ����ݣ�д��buf

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




extern void* __Vectors;
int main(void);


static void  Com_Debug_Print_Help(void)
{
	printf("vector = %p\r\n",main);
	printf("\r\nDebug cmd:\r\n");
	printf("0. print Program build time\r\n");
	if(Get_Lcd_Type()){  //����1��ʾ7������0��ʾ5����
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
	printf("p. print more debug info\r\n");
	printf("v. print Program version\r\n");	
//	printf("y. goto ota program \r\n");  //void goto_ota_program(uint32_t ota_addr)
	printf("other. print help\r\n");
}


extern void goto_ota_program(uint32_t ota_addr);

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
			debug_printf_string("*FreeRTOS* ");
			debug_printf_string((char*)g_build_time_str);  //��ӡ�����ʱ��
			debug_printf_string("\r\n");
			debug_printf_string("Author:JC&DaZhi <vx:285408136>\r\n"); 
		break;
		
		case '1':
			if(Get_Lcd_Type()){
				if(g_lcd_pwm < 100)
				{
					Lcd_pwm_out(g_lcd_pwm + 10);   //��Ļ���ȼ�10
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
					Lcd_pwm_out(g_lcd_pwm - 10);   //��Ļ���ȼ�10
					debug_printf_string("decrease 7 inch lcd PWM\r\n");
				}
				else
					debug_printf_string("g_lcd_pwm = 0\r\n");
			}
			break;
		case '3':
			query_task();			
			break;
		case '4':  //9211����
			debug_printf_string("reset LCD & 9211\r\n");  //lcd�ӵ�״̬
			LT9211_Config();
			break;
		case '5':  //���Ź�״̬
			debug_printf_string("Watch Dog Status : ");
			if(get_hard_wtd_status())
				debug_printf_string("on\r\n");   //����
			else
				debug_printf_string("off\r\n");   //�ر�
			break;
		case '6':  //mcu�ڲ��¶�
			debug_printf_string("Mcu internal_temp = ");
			debug_printf_u32(get_internal_temp(),10);
			debug_printf_string("\r\n");
//			debug_printf_string("Mcu internal_temp = %d\r\n",get_internal_temp());
			break;
		case '7':   
			debug_printf_string("reset core board!!\r\n");  //lcd�ӵ�״̬
			hard_wtd_reset_3399board();
			break;
		case '8':  //LSPK �̵������� ��ΪPA7
			LSPK_Control_ToggleOut();
			break;
		case '9':   //mores 12 ��ѹ�������
			V12_CTL_Control_ToggleOut();
			break;
		case 'a':   //�������ȵ���
			kLedPWM_ToggleOut();
			break;
		case 'b':   //5��lcd�������
			SHTDB_5IN_Control_ToggleOut();
			break;
		case 'c':   //SPEN����
			SPKEN_Control_ToggleOut();
			break;
		case 'd':   //EAR_L_EN����
			EAR_L_EN_Control_ToggleOut();
			break;
		case 'e':   //EAR_R_EN����
			EAR_R_EN_Control_ToggleOut();
			break;
		case 'f':   //�������ȿ���
			kLedPWM_ToggleOut();
			break;
		case 'g':   //����ȫ����������
			debug_printf_string("2023 key_light_allleds_control(1)!!\r\n");
			key_light_allleds_control(1);
			break;
		case 'h':   //����ȫ��Ϩ�����
			debug_printf_string("2023 key_light_allleds_control(0)!!\r\n");
			key_light_allleds_control(0);
			break;
		case 'v':
			printf("mcu version = %d\r\n", GetMcuVersion());
			break;
		case 'p':
			if(more_debug_info){
				printf("close more debug info\r\n");
				more_debug_info = 0;
			}
			else{
				printf("print more debug info\r\n");
				more_debug_info = 1;
			}
				
			break;
//		case 'y':
		//	goto_ota_program(0x801d000);   //�������12K��0x802,0000 - 0x3000
//			break;
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
		case 'j':   //����ĳ����
			debug_printf_string("2023 key_light_allleds_control(0)!!\r\n");
			key_light_allleds_control(0);
			break;
		case 'k':   //Ϩ��ĳ����
			debug_printf_string("2023 key_light_allleds_control(0)!!\r\n");
			key_light_allleds_control(0);
			break;
		case 'z':   //����1-10��
			debug_printf_string("2023 key_light_allleds_control(0)!!\r\n");
			key_light_allleds_control(0);
			break;
		case 'x':   //Ϩ��1-10��
			debug_printf_string("2023 key_light_allleds_control(0)!!\r\n");
			key_light_allleds_control(0);
			break;
		case 'n':   //����11-20��
			debug_printf_string("2023 key_light_allleds_control(0)!!\r\n");
			key_light_allleds_control(0);
			break;
		case 'm':   //Ϩ��11-20��
			debug_printf_string("2023 key_light_allleds_control(0)!!\r\n");
			key_light_allleds_control(0);
			break;
		case 'q':   //����21-30��
			debug_printf_string("2023 key_light_allleds_control(0)!!\r\n");
			key_light_allleds_control(0);
			break;
		case 'w':   //Ϩ��21-30��
			debug_printf_string("2023 key_light_allleds_control(0)!!\r\n");
			key_light_allleds_control(0);
			break;
		case 'r':   //����31-39��
			debug_printf_string("2023 key_light_allleds_control(0)!!\r\n");
			key_light_allleds_control(0);
			break;
		case 't':   //Ϩ��31-39��
			debug_printf_string("2023 key_light_allleds_control(0)!!\r\n");
			key_light_allleds_control(0);
			break;
		case 'o':   //����ȫ��Ϩ�����-2
			debug_printf_string("2023 key_light_allleds_control(0)!!\r\n");
			key_light_allleds_control(0);
			break;
		case 'p':   //����ȫ��Ϩ�����-2
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
//	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
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




//���Դ������printf
/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    __io_putchar(ch);
	return ch;
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









