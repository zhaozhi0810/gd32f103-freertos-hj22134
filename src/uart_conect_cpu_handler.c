

/*
���ڴ�����cpu֮��Ĵ���ͨ��
	����ΪGD32�Ĵ���1��115200��8N1

*/

#include "includes.h"
#include "string.h"


#define CPU_UART_CMD_LEN 4 //cpu�����Ķ���4���ֽ�
//#define RECV_BUF_LEN 64
static Queue_UART_STRUCT g_Queue_Cpu_Recv;   //����cpu���ݶ��У����ڽ����ж�
frame_buf_t g_com_cpu_buf={{0},CPU_UART_CMD_LEN};    //����

#define CPU_UART_HEAD1 0xa5    //ע����cpu�˱���һ��
//#define CPU_UART_HEAD2 0x5a






//�����ʼ��
void Com_Cpu_Recive_Buff_Init(void)
{
	memset((void *)&g_Queue_Cpu_Recv, 0, sizeof(g_Queue_Cpu_Recv));
}




/*
	�������ݽ����жϣ�
		ǰ�᣺ÿһ֡����7���ֽڡ�
		�����б���֡ͷ���к�������ݺ�У��ͣ���7���ֽڣ�
*/
void Com_Cpu_Rne_Int_Handle(void)
{
	uint8_t dat;

	dat = (uint8_t)usart_data_receive(EVAL_COM1);//(USART3);   //���վʹ浽�����У���������2021-12-02

	QueueUARTDataInsert(&g_Queue_Cpu_Recv,dat);   //���յ����ݴ�������С�

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
			set_Led_Pwm(cmd[1]);
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
			g_task_id |= 1<<4 ;  //��main��ȥ��λ 2022-09-06
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
	Uart_Tx_String(TOCPU_COM_NUM, buf, 4);   //�Ӵ���1 ��������
}


void Com_Frame_Handle(frame_buf_t* buf, Queue_UART_STRUCT* Queue_buf,message_handle handle)
{
	uint8_t length,i,j;
	uint8_t offset = 0;  //��������������֡��������
	
	while(1)  //���ǵ����ܽ��յ���֡Ҫ��������
	{		
		length = QueueUARTDataLenGet(Queue_buf);	
		if(length < buf->datalen )   //������֡ͷ��С��7���ֽڣ�����������һ֡
		{	
			return ;   //�����ȴ�
		}	
		length = buf->datalen;   //������Ҫ�������ֽ���
		offset = CPU_UART_CMD_LEN - buf->datalen ;    //������ƫ���ֽ�
		for(i=0;i<length;i++)
		{
			//���ﲻ�жϴ����ˣ�ǰ���Ѿ����ȷʵ����ô���ֽڡ�
			QueueUARTDataDele(Queue_buf,buf->com_handle_buf+i+offset) ;  //com_data �ճ�1���ֽڣ�Ϊ�˼���֮ǰ��У����㷨�������ݽ����㷨
		}

		if((buf->com_handle_buf[0] == CPU_UART_HEAD1) /*&& (buf->com_handle_buf[1] == CPU_UART_HEAD2) */ &&(0 == Uart_Verify_Data_CheckSum(buf->com_handle_buf,CPU_UART_CMD_LEN)))   //�ڶ������������ܳ��ȣ�����У��͹�7���ֽ�
		{
			//У�����ȷ��
			handle(&buf->com_handle_buf[1]);   //ֻҪ�������ȥ�Ϳ�����		
		}	
		else  //У��Ͳ���ȷ��������֡�д���
		{
			for(i=1;i<CPU_UART_CMD_LEN;i++)   //ǰ����жϳ����⣬������֡����Ѱ����һ��֡ͷ������
			{
				if(buf->com_handle_buf[i] == CPU_UART_HEAD1)   //�м��ҵ�һ��֡ͷ
				{
					break;
				}
			}		
			if(i != CPU_UART_CMD_LEN) //�������м��ҵ�֡ͷ������
			{
				buf->datalen = i;   //��һ����Ҫ�����ֽ���
			//	offset = FRAME_LENGHT-i;  //�洢��ƫ��λ�õļ���

				for(j=0;i<CPU_UART_CMD_LEN;i++,j++)   //�п���֡ͷ���ԣ����Ե�һ���ֽڻ���Ҫ����һ��
				{
					buf->com_handle_buf[j] = buf->com_handle_buf[i];   //��ʣ�µĿ�����ȥ
				}
			}
			else  //�������м�û���ҵ�֡ͷ
			{
				buf->datalen = CPU_UART_CMD_LEN;  //	��һ����Ҫ�����ֽ���
			//	offset = 0;
			}
		}	
	}//end while 1
}





/*
	���ڿ����жϵĴ���

	1.�жϽ��յ����ֽ�����>=7 ��ʾ����
	2.�����ͼ�����������7���ֽڣ�����У��ͣ�
	3.У�����ȷ����������

*/
void Com_Cpu_Idle_Int_Handle(void)
{
	Com_Frame_Handle(&g_com_cpu_buf, &g_Queue_Cpu_Recv,AnswerCpu_data);
}




/*************************��Ƭ�����͸�cpu***************************************/
/**************************************************************/
//Ӧ��cpu�Ļ�ȡ��Ϣ������
//void AnswerCpu_GetInfo(uint16_t ask)
//{
//	unsigned char buf[32];      //�䳤����  		
//	uint8_t index = 0;

//	buf[0] = FRAME_HEAD;  		//֡ͷ����һ���ֽ�
//	buf[1] = 0;  				//�ڶ����ֽڣ�����
//	buf[2] = ask>>8; buf[3] = ask;   //��3��4���ֽڱ�ʾ��ȡ��״̬������cpu����

//	index = 4;
//	if(ask & (1<<eBITS_VOL))   //��Ҫ��ȡ4·��ѹ,ÿһ������ռ�����ֽ�
//	{
//		buf[index++]  = g_p0v95_vol>>8;     //���4����ѹֵ
//		buf[index++]  = g_p0v95_vol;
//		buf[index++]  = g_p1v0_vol>>8;
//		buf[index++]  = g_p1v0_vol;
//		buf[index++]  = g_p1v2_vol>>8;     //���4����ѹֵ
//		buf[index++]  = g_p1v2_vol;
//		buf[index++]  = g_p12v_vol>>8;
//		buf[index++]  = g_p12v_vol;
//		buf[index++]  = g_power_vol>>8;     //�����ѹ���Ե�Դ���ġ�g_power_vol,g_power_cur
//		buf[index++]  = g_power_vol;
//	}	
//	
//	if(ask & (1<<eBITS_CUR))   //��Ҫ��ȡ1·����,�ӵ�Դģ�������
//	{
//		buf[index++]  = g_power_cur>>8;     //���1������ֵ
//		buf[index++]  = g_power_cur;
//	}

//	if(ask & (1<<eBITS_CB_TMP))   //��Ҫcpu�������¶�,ȥ����С������
//	{
//		buf[index++]  = g_cpu_temp>>4;     //���cpu�������¶�(cpu��Ӧ�ÿ������з�����)
//		buf[index++]  = g_board_temp>>4;
//	}
//	
//	if(ask & (1<<eBITS_LCD_TMP))   //��Ҫ��·lcd�¶�
//	{
//		buf[index++]  = g_lcd_temp[0]>>4;     //�����·lcd�¶�(cpu��Ӧ�ÿ������з�����)
//		buf[index++]  = g_lcd_temp[1]>>4;
//	}
//	
//	if(ask & (1<<eBITS_LCD_PWM))   //��Ҫ��·lcd��������Ϣ
//	{
//		buf[index++]  = g_lcd_pwm;     //���1��pwmֵ
//	}
//	
//	if(ask & (1<<eBITS_BITSTATUS))   //��Ҫ���ֽ�λ״̬��Ϣ
//	{
//		bitstatus_t b_status;
//		b_status.di_4ttl = Get_Di_4Ttl_Status();
//		b_status.dvi_src = 0;  //0��ʾ���أ�1��ʾ�ⲿ������������ܣ�����Ϊ0
//		b_status.lcd_beat = !!Get_Lcd_Heat_Status();  //0Ϊδ���ȣ�1Ϊ���ڼ���
//		b_status.watch_dog_status = 0;   //�������ָ������Ź�����cpu�Ƿ����ϵͳ���������޸ù���
//		uint16_t* p = (void*)&b_status;
//		buf[index++]  = *p>>8;     //���λ״̬
//		buf[index++]  = *p;     //���λ״̬
//	}
//	
//	if(ask & (1<<eBITS_FAN_PWM))   //��Ҫ���ȵ�pwmֵ
//	{
//		buf[index++]  = g_fan_pwm;     //���1��pwmֵ
//	}
//	
//	buf[1] = index + 1; //�����ǰ���֡ͷ�����ļ���͡�
//	if(index > 4)  //��ʾ��������Ҫ
//	{		
//		buf[2] |= 0x80;    //���λ��1����ʾ״̬�ɹ�
////		buf[index] = CheckSum_For_Uart(buf,index);    //���㲢�洢У��ͣ�
////		Uart_Tx_String(1, buf, index + 1);   //��������
//	}
//	else   //û��������ݵ�ʱ��Ӧ��һ��error cmd
//	{	
//		buf[2] &= 0x7f;   //���λ��0����ʾӦ���������Ч 
//	}
//	
//	buf[index] = CheckSum_For_Uart(buf,index);    //���㲢�洢У��ͣ�
//	Uart_Tx_String(1, buf, index + 1);   //��������
//}




////Ӧ��cpu��������Ϣ������ errcodeΪ0��ʾ�ɹ�������ֵΪ������ ӦС��0x7f
//void AnswerCpu_Status(uart_err_t errcode)
//{
//	unsigned char buf[8] = {0};  		
//	buf[0] = FRAME_HEAD;  //֡ͷ
//	
//	buf[1] = 5; //�����ǰ���֡ͷ�����ļ���͡�
//	
//	if(!errcode)
//		buf[2] |= 0x80;    //���λ��1����ʾ״̬�ɹ�
//	else  //�������ʧ���ˣ�����
//		buf[2] = errcode & 0x7f;   //����һ��������
//		
//	buf[4] = CheckSum_For_Uart(buf,4);    //���㲢�洢У��ͣ�
//	Uart_Tx_String(1, buf, 5);   //��������
//}



#if 0


/*
	�����յ������Ĵ������ڵĿ����жϴ���������
		ǰ�᣺ �յ����������ݰ���У�����ȷ��

	��Ƭ���ܹ��յ������
	// 1.������ƵԴ,û�иù���
	4.����lcd��pwm�����ȣ�
	5.�ػ����������

*/

static void Com_Cpu_Message_Handle(uint8_t* buf)
{	
	
	
	
	
	
	
//	com_frame_t* pdata = (com_frame_t*)(buf+1);    //+1������֡ͷ��ʹ�ýṹ���ʼ��
//	int8_t t;
//	switch(pdata->data_type)
//    {	
//        case eMCU_CMD_TYPE:    //cpu���͸���Ƭ���Ķ���cmd����
//            t = pdata->data.cmd.cmd;
//            switch(t)
//            {
//				case eMCU_CPUGETINFO_CMD:   //��ȡ�豸��Ϣ������
//					AnswerCpu_GetInfo(pdata->data.cmd.param1<<8 | pdata->data.cmd.param2); //ʹ�ú�������������������
//					break;
//				case eMCU_CPUSET_CMD:    //������Ļ����
//					if(pdata->data.cmd.param1 == eMCU_LCD_SETPWM_CMD)
//					{
//						t = pdata->data.cmd.param2;   //���ֵ�����ɸ���������������������������
//						t = g_lcd_pwm + t;   //����ó��µĽ��
//						Lcd_pwm_out(t);     //��������pwm��ֵ
//						AnswerCpu_Status(eUART_SUCCESS);   //Ӧ��ɹ�
//					}
//					else if(pdata->data.cmd.param1 == eMCU_SWITCH_DVI_SRC_CMD) //�л���ƵԴ
//					{
//						t = pdata->data.cmd.param2;  //0 Ϊ���أ�1Ϊ�ⲿ
////						if(t)
////							dvi_switch_set(DVI_OTHER);   //���ú���ϱ���cpu
////						else
////							dvi_switch_set(DVI_LOONGSON);   //������Ƶ
//						AnswerCpu_Status(eUART_SUCCESS);   //Ӧ��ɹ�
//					}
//					else	
//						AnswerCpu_Status(eUART_ERR_PARAM);  //Ӧ���������				
//				break;
//                default:
//					AnswerCpu_Status(eUART_ERR_PARAM);  //Ӧ���������
//                break;
//            }

//        break;
//        default:
//			AnswerCpu_Status(eUART_ERR_CMD_UNKNOW);  //Ӧ������δ֪ 
//        break;
//    }

}


//typedef void (*message_handle)(uint8_t );


//��Ƭ���������ݸ�cpu�����ɵ������̴߳����ˡ�dataֻ��Ҫ�����������ͺ����ݡ�ͷ����crc�ɸú�����ɡ�
/*
 * data ���ڷ��͵����ݣ�����Ҫ����֡ͷ��У��ͣ�ֻҪ�����������ͺ����ݣ���5���ֽڣ�
 * ����ֵ
 * 	0��ʾ�ɹ���������ʾʧ��
 * */
static int Send_Mcu_Data_ToCpu(const void* data)
{	
	unsigned char buf[8];  	
	
	buf[0] = FRAME_HEAD;  //֡ͷ	
	memcpy(buf+1,data,sizeof(com_frame_t)-1);    //����

	//crc���¼���
	buf[sizeof(com_frame_t)] = CheckSum_For_Uart(buf,sizeof(com_frame_t));  //У��ͣ��洢�ڵ�7���ֽ��ϣ������±�6.
	
	//UART3_TX_STRING(buf, sizeof(com_frame_t)+1);   //com_frame_t��û�а�������ͷ�����Լ�1���ֽ�	
	Uart_Tx_String(1, buf, sizeof(com_frame_t)+1);
	//���ͳɹ����ȴ�Ӧ��
	return 0;   //��ʱû�еȴ�Ӧ��2021-11-23

}



//�����������ݵ�cpu
//cmd��ο�uart.h�к궨��
//param ������
void Send_Cmd_ToCpu(int cmd,int param)
{
	com_frame_t data;
	data.data_type = eMCU_CMD_TYPE;
	data.data.cmd.cmd = cmd;
	data.data.cmd.param_len = 1;     //��һ������
	data.data.cmd.param1 = param;   //��һ������
	Send_Mcu_Data_ToCpu(&data);
}

//����dvi��Ƶ���л������ݵ�cpu
//source 1�����أ�����2���ⲿ��
void Send_Dvi_Change_ToCpu(int source)
{
	com_frame_t data;
	data.data_type = eMCU_DIV_CHANGE_TYPE;
    data.data.fdd.bstatus.dvi_src = source-1;
	Send_Mcu_Data_ToCpu(&data);
}



//���������cpu�¶�
void Send_Temp_ToCpu(data_type type,short cpu_temp,short board_temp)
{
	com_frame_t data;
	data.data_type = type;//eMCU_CB_TEMP_TYPE;
	data.data.cb_temp.temp1 = cpu_temp;
	data.data.cb_temp.temp2 = board_temp;
	Send_Mcu_Data_ToCpu(&data);
}

//����2����ѹ
void Send_Vol_ToCpu(data_type type,short vol1,short vol2)
{
	com_frame_t data;
	
	data.data_type = type;//eMCU_VOL12_TYPE;
	data.data.vol_12.vol1 = vol1;
	data.data.vol_12.vol2 = vol2;

	Send_Mcu_Data_ToCpu(&data);
}



//2021-12-15����
//��ʱ���ͷ���״̬��dvi�͹��ϵ�״̬
void Send_Fan_Div_Status_ToCpu(bitstatus_t b_status,uint8_t fan_pwm,uint8_t lcd_pwm)
{
	com_frame_t data;
	
	data.data_type = eMCU_FAN_DIV_DI_TYPE;
	data.data.fdd.bstatus = b_status;  //λ״̬
	data.data.fdd.fan_pwm = fan_pwm;    //����pwmֵ
	data.data.fdd.lcd_pwm = lcd_pwm;    //lcdpwmֵ
//	data.data.fdd.bstatus.di_4ttl = di;
	Send_Mcu_Data_ToCpu(&data);
}

#endif


