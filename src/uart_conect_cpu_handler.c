

/*
用于处理与cpu之间的串口通信
	串口为GD32的串口1，115200，8N1

*/

#include "includes.h"
#include "string.h"


#define CPU_UART_CMD_LEN 4 //cpu发出的都是4个字节
//#define RECV_BUF_LEN 64
static Queue_UART_STRUCT g_Queue_Cpu_Recv;   //接收cpu数据队列，用于接收中断
frame_buf_t g_com_cpu_buf={{0},CPU_UART_CMD_LEN};    //缓存

#define CPU_UART_HEAD1 0xa5    //注意与cpu端保持一致
//#define CPU_UART_HEAD2 0x5a






//缓存初始化
void Com_Cpu_Recive_Buff_Init(void)
{
	memset((void *)&g_Queue_Cpu_Recv, 0, sizeof(g_Queue_Cpu_Recv));
}




/*
	串口数据接收中断：
		前提：每一帧都是7个字节。
		队列中保存帧头，有后面的数据和校验和（共7个字节）
*/
void Com_Cpu_Rne_Int_Handle(void)
{
	uint8_t dat;

	dat = (uint8_t)usart_data_receive(EVAL_COM1);//(USART3);   //接收就存到队列中！！！！！2021-12-02

	QueueUARTDataInsert(&g_Queue_Cpu_Recv,dat);   //接收的数据存入队列中。

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
			set_Led_Pwm(cmd[1]);
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
			g_task_id |= 1<<4 ;  //在main中去复位 2022-09-06
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
	Uart_Tx_String(TOCPU_COM_NUM, buf, 4);   //从串口1 发送数据
}


void Com_Frame_Handle(frame_buf_t* buf, Queue_UART_STRUCT* Queue_buf,message_handle handle)
{
	uint8_t length,i,j;
	uint8_t offset = 0;  //这两个变量用于帧错误的情况
	
	while(1)  //考虑到可能接收到多帧要处理的情况
	{		
		length = QueueUARTDataLenGet(Queue_buf);	
		if(length < buf->datalen )   //（包括帧头）小于7个字节，不是完整的一帧
		{	
			return ;   //继续等待
		}	
		length = buf->datalen;   //计算需要读出的字节数
		offset = CPU_UART_CMD_LEN - buf->datalen ;    //缓存中偏移字节
		for(i=0;i<length;i++)
		{
			//这里不判断错误了，前面已经检测确实有这么多字节。
			QueueUARTDataDele(Queue_buf,buf->com_handle_buf+i+offset) ;  //com_data 空出1个字节，为了兼容之前的校验和算法，及数据解析算法
		}

		if((buf->com_handle_buf[0] == CPU_UART_HEAD1) /*&& (buf->com_handle_buf[1] == CPU_UART_HEAD2) */ &&(0 == Uart_Verify_Data_CheckSum(buf->com_handle_buf,CPU_UART_CMD_LEN)))   //第二参数是数据总长度，包括校验和共7个字节
		{
			//校验和正确。
			handle(&buf->com_handle_buf[1]);   //只要传命令过去就可以了		
		}	
		else  //校验和不正确，可能是帧有错误。
		{
			for(i=1;i<CPU_UART_CMD_LEN;i++)   //前面的判断出问题，考虑是帧错误，寻找下一个帧头！！！
			{
				if(buf->com_handle_buf[i] == CPU_UART_HEAD1)   //中间找到一个帧头
				{
					break;
				}
			}		
			if(i != CPU_UART_CMD_LEN) //在数据中间找到帧头！！！
			{
				buf->datalen = i;   //下一次需要读的字节数
			//	offset = FRAME_LENGHT-i;  //存储的偏移位置的计算

				for(j=0;i<CPU_UART_CMD_LEN;i++,j++)   //有可能帧头不对，所以第一个字节还是要拷贝一下
				{
					buf->com_handle_buf[j] = buf->com_handle_buf[i];   //把剩下的拷贝过去
				}
			}
			else  //在数据中间没有找到帧头
			{
				buf->datalen = CPU_UART_CMD_LEN;  //	下一次需要读的字节数
			//	offset = 0;
			}
		}	
	}//end while 1
}





/*
	串口空闲中断的处理

	1.判断接收到的字节数，>=7 表示正常
	2.正常就继续处理，读出7个字节，计算校验和，
	3.校验和正确，则处理命令

*/
void Com_Cpu_Idle_Int_Handle(void)
{
	Com_Frame_Handle(&g_com_cpu_buf, &g_Queue_Cpu_Recv,AnswerCpu_data);
}




/*************************单片机发送给cpu***************************************/
/**************************************************************/
//应答cpu的获取信息的请求
//void AnswerCpu_GetInfo(uint16_t ask)
//{
//	unsigned char buf[32];      //变长缓存  		
//	uint8_t index = 0;

//	buf[0] = FRAME_HEAD;  		//帧头，第一个字节
//	buf[1] = 0;  				//第二个字节，长度
//	buf[2] = ask>>8; buf[3] = ask;   //第3，4个字节表示获取的状态，用于cpu解析

//	index = 4;
//	if(ask & (1<<eBITS_VOL))   //需要获取4路电压,每一个数据占两个字节
//	{
//		buf[index++]  = g_p0v95_vol>>8;     //填充4个电压值
//		buf[index++]  = g_p0v95_vol;
//		buf[index++]  = g_p1v0_vol>>8;
//		buf[index++]  = g_p1v0_vol;
//		buf[index++]  = g_p1v2_vol>>8;     //填充4个电压值
//		buf[index++]  = g_p1v2_vol;
//		buf[index++]  = g_p12v_vol>>8;
//		buf[index++]  = g_p12v_vol;
//		buf[index++]  = g_power_vol>>8;     //这个电压来自电源给的。g_power_vol,g_power_cur
//		buf[index++]  = g_power_vol;
//	}	
//	
//	if(ask & (1<<eBITS_CUR))   //需要获取1路电流,从电源模块给出。
//	{
//		buf[index++]  = g_power_cur>>8;     //填充1个电流值
//		buf[index++]  = g_power_cur;
//	}

//	if(ask & (1<<eBITS_CB_TMP))   //需要cpu和主板温度,去除了小数部分
//	{
//		buf[index++]  = g_cpu_temp>>4;     //填充cpu和主板温度(cpu端应该看成是有符号数)
//		buf[index++]  = g_board_temp>>4;
//	}
//	
//	if(ask & (1<<eBITS_LCD_TMP))   //需要两路lcd温度
//	{
//		buf[index++]  = g_lcd_temp[0]>>4;     //填充两路lcd温度(cpu端应该看成是有符号数)
//		buf[index++]  = g_lcd_temp[1]>>4;
//	}
//	
//	if(ask & (1<<eBITS_LCD_PWM))   //需要两路lcd的亮度信息
//	{
//		buf[index++]  = g_lcd_pwm;     //填充1个pwm值
//	}
//	
//	if(ask & (1<<eBITS_BITSTATUS))   //需要两字节位状态信息
//	{
//		bitstatus_t b_status;
//		b_status.di_4ttl = Get_Di_4Ttl_Status();
//		b_status.dvi_src = 0;  //0表示本地，1表示外部，暂无这个功能，所以为0
//		b_status.lcd_beat = !!Get_Lcd_Heat_Status();  //0为未加热，1为正在加热
//		b_status.watch_dog_status = 0;   //这里可能指软件看门狗，看cpu是否进入系统？？？暂无该功能
//		uint16_t* p = (void*)&b_status;
//		buf[index++]  = *p>>8;     //填充位状态
//		buf[index++]  = *p;     //填充位状态
//	}
//	
//	if(ask & (1<<eBITS_FAN_PWM))   //需要风扇的pwm值
//	{
//		buf[index++]  = g_fan_pwm;     //填充1个pwm值
//	}
//	
//	buf[1] = index + 1; //长度是包括帧头到最后的检验和。
//	if(index > 4)  //表示有数据需要
//	{		
//		buf[2] |= 0x80;    //最高位置1，表示状态成功
////		buf[index] = CheckSum_For_Uart(buf,index);    //计算并存储校验和，
////		Uart_Tx_String(1, buf, index + 1);   //发送数据
//	}
//	else   //没有填充数据的时候。应答一个error cmd
//	{	
//		buf[2] &= 0x7f;   //最高位是0，表示应答的内容无效 
//	}
//	
//	buf[index] = CheckSum_For_Uart(buf,index);    //计算并存储校验和，
//	Uart_Tx_String(1, buf, index + 1);   //发送数据
//}




////应答cpu的设置信息的请求 errcode为0表示成功，其他值为错误码 应小于0x7f
//void AnswerCpu_Status(uart_err_t errcode)
//{
//	unsigned char buf[8] = {0};  		
//	buf[0] = FRAME_HEAD;  //帧头
//	
//	buf[1] = 5; //长度是包括帧头到最后的检验和。
//	
//	if(!errcode)
//		buf[2] |= 0x80;    //最高位置1，表示状态成功
//	else  //否则就是失败了！！！
//		buf[2] = errcode & 0x7f;   //设置一个错误码
//		
//	buf[4] = CheckSum_For_Uart(buf,4);    //计算并存储校验和，
//	Uart_Tx_String(1, buf, 5);   //发送数据
//}



#if 0


/*
	串口收到命令后的处理。串口的空闲中断处理函数调用
		前提： 收到完整的数据包，校验和正确。

	单片机能够收到的命令：
	// 1.设置视频源,没有该功能
	4.设置lcd的pwm（亮度）
	5.关机或重启命令。

*/

static void Com_Cpu_Message_Handle(uint8_t* buf)
{	
	
	
	
	
	
	
//	com_frame_t* pdata = (com_frame_t*)(buf+1);    //+1是跳过帧头，使用结构体初始化
//	int8_t t;
//	switch(pdata->data_type)
//    {	
//        case eMCU_CMD_TYPE:    //cpu发送给单片机的都是cmd！！
//            t = pdata->data.cmd.cmd;
//            switch(t)
//            {
//				case eMCU_CPUGETINFO_CMD:   //获取设备信息的命令
//					AnswerCpu_GetInfo(pdata->data.cmd.param1<<8 | pdata->data.cmd.param2); //使用函数解析，并返回数据
//					break;
//				case eMCU_CPUSET_CMD:    //设置屏幕亮度
//					if(pdata->data.cmd.param1 == eMCU_LCD_SETPWM_CMD)
//					{
//						t = pdata->data.cmd.param2;   //这个值可正可负，根据它的正负来调亮或者灭
//						t = g_lcd_pwm + t;   //计算得出新的结果
//						Lcd_pwm_out(t);     //重新设置pwm的值
//						AnswerCpu_Status(eUART_SUCCESS);   //应答成功
//					}
//					else if(pdata->data.cmd.param1 == eMCU_SWITCH_DVI_SRC_CMD) //切换视频源
//					{
//						t = pdata->data.cmd.param2;  //0 为本地，1为外部
////						if(t)
////							dvi_switch_set(DVI_OTHER);   //设置后会上报给cpu
////						else
////							dvi_switch_set(DVI_LOONGSON);   //本地视频
//						AnswerCpu_Status(eUART_SUCCESS);   //应答成功
//					}
//					else	
//						AnswerCpu_Status(eUART_ERR_PARAM);  //应答参数错误				
//				break;
//                default:
//					AnswerCpu_Status(eUART_ERR_PARAM);  //应答参数错误
//                break;
//            }

//        break;
//        default:
//			AnswerCpu_Status(eUART_ERR_CMD_UNKNOW);  //应答命令未知 
//        break;
//    }

}


//typedef void (*message_handle)(uint8_t );


//单片机发送数据给cpu，不由单独的线程处理了。data只需要包含数据类型和数据。头部和crc由该函数完成。
/*
 * data 用于发送的数据，不需要包括帧头和校验和，只要包括数据类型和数据（共5个字节）
 * 返回值
 * 	0表示成功，其他表示失败
 * */
static int Send_Mcu_Data_ToCpu(const void* data)
{	
	unsigned char buf[8];  	
	
	buf[0] = FRAME_HEAD;  //帧头	
	memcpy(buf+1,data,sizeof(com_frame_t)-1);    //拷贝

	//crc重新计算
	buf[sizeof(com_frame_t)] = CheckSum_For_Uart(buf,sizeof(com_frame_t));  //校验和，存储在第7个字节上，数组下标6.
	
	//UART3_TX_STRING(buf, sizeof(com_frame_t)+1);   //com_frame_t并没有包含数据头，所以加1个字节	
	Uart_Tx_String(1, buf, sizeof(com_frame_t)+1);
	//发送成功，等待应答
	return 0;   //暂时没有等待应答2021-11-23

}



//发送命令数据到cpu
//cmd请参考uart.h中宏定义
//param 参数。
void Send_Cmd_ToCpu(int cmd,int param)
{
	com_frame_t data;
	data.data_type = eMCU_CMD_TYPE;
	data.data.cmd.cmd = cmd;
	data.data.cmd.param_len = 1;     //带一个参数
	data.data.cmd.param1 = param;   //带一个参数
	Send_Mcu_Data_ToCpu(&data);
}

//发送dvi视频被切换的数据到cpu
//source 1（本地）或者2（外部）
void Send_Dvi_Change_ToCpu(int source)
{
	com_frame_t data;
	data.data_type = eMCU_DIV_CHANGE_TYPE;
    data.data.fdd.bstatus.dvi_src = source-1;
	Send_Mcu_Data_ToCpu(&data);
}



//发送主板和cpu温度
void Send_Temp_ToCpu(data_type type,short cpu_temp,short board_temp)
{
	com_frame_t data;
	data.data_type = type;//eMCU_CB_TEMP_TYPE;
	data.data.cb_temp.temp1 = cpu_temp;
	data.data.cb_temp.temp2 = board_temp;
	Send_Mcu_Data_ToCpu(&data);
}

//发送2个电压
void Send_Vol_ToCpu(data_type type,short vol1,short vol2)
{
	com_frame_t data;
	
	data.data_type = type;//eMCU_VOL12_TYPE;
	data.data.vol_12.vol1 = vol1;
	data.data.vol_12.vol2 = vol2;

	Send_Mcu_Data_ToCpu(&data);
}



//2021-12-15调整
//定时发送风扇状态，dvi和故障灯状态
void Send_Fan_Div_Status_ToCpu(bitstatus_t b_status,uint8_t fan_pwm,uint8_t lcd_pwm)
{
	com_frame_t data;
	
	data.data_type = eMCU_FAN_DIV_DI_TYPE;
	data.data.fdd.bstatus = b_status;  //位状态
	data.data.fdd.fan_pwm = fan_pwm;    //风扇pwm值
	data.data.fdd.lcd_pwm = lcd_pwm;    //lcdpwm值
//	data.data.fdd.bstatus.di_4ttl = di;
	Send_Mcu_Data_ToCpu(&data);
}

#endif


