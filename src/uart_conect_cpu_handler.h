
#ifndef UART_CONECT_CPU_HANDLER_H
#define UART_CONECT_CPU_HANDLER_H


#include <gd32f10x.h>
#include "uart.h"

/*
2022-04-21 调整（不主动发送呢，就可以节省cpu和单片机的精力，不用老是处理）


*/


//注意单片机与cpu保持一致  2022-07-28
//#pragma pack(1) 这个会设置全局的，注释掉
//数据总共4个字节，这里不含帧头
typedef struct
{
	unsigned char data_type;   //led的控制，状态的获取，lcd的熄灭
	unsigned char data;
//	mcu_data_t data;
	unsigned char crc;     //校验和
}__attribute__((packed))com_frame_t;    //注意对齐方式




typedef enum
{	
	eMCU_LED_STATUS_TYPE=50,  //获得led的状态
	eMCU_KEY_STATUS_TYPE,    //获得按键的状态
	eMCU_LED_SETON_TYPE,    //设置对应的led亮
	eMCU_LED_SETOFF_TYPE,    //设置对应的led灭
	eMCU_LCD_SETONOFF_TYPE,  //lcd打开关闭
	eMCU_KEY_CHANGE_TYPE,    //按键被修改上报
    eMCU_LEDSETALL_TYPE,     //对所有led进行控制，点亮或者熄灭
	eMCU_LEDSETPWM_TYPE,     //设置所有led的亮度 
	eMCU_GET_TEMP_TYPE,      //获得单片机内部温度	
	eMCU_HWTD_SETONOFF_TYPE,   //开门狗设置开关
	eMCU_HWTD_FEED_TYPE,       //看门狗喂狗
	eMCU_HWTD_SETTIMEOUT_TYPE,    //设置看门狗喂狗时间
	eMCU_HWTD_GETTIMEOUT_TYPE,    //获取看门狗喂狗时间
	eMCU_RESET_COREBOARD_TYPE,  //复位核心板
	eMCU_RESET_LCD_TYPE,        //复位lcd 9211（复位引脚没有连通）
	eMCU_RESET_LFBOARD_TYPE,    //复位底板，好像没有这个功能！！！
	eMCU_MICCTRL_SETONOFF_TYPE//,  //MICCTRL 引脚的控制
}mcu_data_type;



#define FRAME_LENGHT (8)    //数据帧的字节数


typedef struct frame_buf
{
	uint8_t com_handle_buf[FRAME_LENGHT];   //接收缓存
	uint8_t datalen;            //帧长-缓存中的数据长度，即下一次要读的字节数
}frame_buf_t;


//消息处理的函数指针
typedef void (*message_handle)(uint8_t* );

//中断处理函数
void Com_Cpu_Rne_Int_Handle(void);
//帧数据处理函数
void Com_Frame_Handle(frame_buf_t* buf, Queue_UART_STRUCT* Queue_buf,message_handle handle);
//中断处理函数
void Com_Cpu_Idle_Int_Handle(void);




//void Send_Fan_Div_Status_ToCpu(bitstatus_t b_status,uint8_t fan_pwm,uint8_t lcd_pwm);
#if 0
//发送2个电压
void Send_Vol_ToCpu(data_type type,short vol1,short vol2);
//发送温度
void Send_Temp_ToCpu(data_type type,short cpu_temp,short board_temp);

#endif
//发送dvi视频被切换的数据到cpu
//source 1（本地）或者2（外部）
//void Send_Dvi_Change_ToCpu(int source);

//发送命令数据到cpu
//cmd请参考uart.h中宏定义
//param 参数。
//void Send_Cmd_ToCpu(int cmd,int param);

//应答cpu的获取信息的请求
void AnswerCpu_GetInfo(uint16_t ask);
//应答cpu的设置信息的请求 errcode为0表示成功，其他值为错误码 应小于0x7f
//void AnswerCpu_Status(uart_err_t errcode);


//缓存初始化
void Com_Cpu_Recive_Buff_Init(void);

#endif
