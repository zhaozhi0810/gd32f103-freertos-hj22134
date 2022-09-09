
#ifndef UART_CONECT_CPU_HANDLER_H
#define UART_CONECT_CPU_HANDLER_H


#include <gd32f10x.h>
#include "uart.h"

/*
2022-04-21 �����������������أ��Ϳ��Խ�ʡcpu�͵�Ƭ���ľ������������Ǵ���


*/


//ע�ⵥƬ����cpu����һ��  2022-07-28
//#pragma pack(1) ���������ȫ�ֵģ�ע�͵�
//�����ܹ�4���ֽڣ����ﲻ��֡ͷ
typedef struct
{
	unsigned char data_type;   //led�Ŀ��ƣ�״̬�Ļ�ȡ��lcd��Ϩ��
	unsigned char data;
//	mcu_data_t data;
	unsigned char crc;     //У���
}__attribute__((packed))com_frame_t;    //ע����뷽ʽ




typedef enum
{	
	eMCU_LED_STATUS_TYPE=50,  //���led��״̬
	eMCU_KEY_STATUS_TYPE,    //��ð�����״̬
	eMCU_LED_SETON_TYPE,    //���ö�Ӧ��led��
	eMCU_LED_SETOFF_TYPE,    //���ö�Ӧ��led��
	eMCU_LCD_SETONOFF_TYPE,  //lcd�򿪹ر�
	eMCU_KEY_CHANGE_TYPE,    //�������޸��ϱ�
    eMCU_LEDSETALL_TYPE,     //������led���п��ƣ���������Ϩ��
	eMCU_LEDSETPWM_TYPE,     //��������led������ 
	eMCU_GET_TEMP_TYPE,      //��õ�Ƭ���ڲ��¶�	
	eMCU_HWTD_SETONOFF_TYPE,   //���Ź����ÿ���
	eMCU_HWTD_FEED_TYPE,       //���Ź�ι��
	eMCU_HWTD_SETTIMEOUT_TYPE,    //���ÿ��Ź�ι��ʱ��
	eMCU_HWTD_GETTIMEOUT_TYPE,    //��ȡ���Ź�ι��ʱ��
	eMCU_RESET_COREBOARD_TYPE,  //��λ���İ�
	eMCU_RESET_LCD_TYPE,        //��λlcd 9211����λ����û����ͨ��
	eMCU_RESET_LFBOARD_TYPE,    //��λ�װ壬����û��������ܣ�����
	eMCU_MICCTRL_SETONOFF_TYPE//,  //MICCTRL ���ŵĿ���
}mcu_data_type;



#define FRAME_LENGHT (8)    //����֡���ֽ���


typedef struct frame_buf
{
	uint8_t com_handle_buf[FRAME_LENGHT];   //���ջ���
	uint8_t datalen;            //֡��-�����е����ݳ��ȣ�����һ��Ҫ�����ֽ���
}frame_buf_t;


//��Ϣ����ĺ���ָ��
typedef void (*message_handle)(uint8_t* );

//�жϴ�����
void Com_Cpu_Rne_Int_Handle(void);
//֡���ݴ�����
void Com_Frame_Handle(frame_buf_t* buf, Queue_UART_STRUCT* Queue_buf,message_handle handle);
//�жϴ�����
void Com_Cpu_Idle_Int_Handle(void);




//void Send_Fan_Div_Status_ToCpu(bitstatus_t b_status,uint8_t fan_pwm,uint8_t lcd_pwm);
#if 0
//����2����ѹ
void Send_Vol_ToCpu(data_type type,short vol1,short vol2);
//�����¶�
void Send_Temp_ToCpu(data_type type,short cpu_temp,short board_temp);

#endif
//����dvi��Ƶ���л������ݵ�cpu
//source 1�����أ�����2���ⲿ��
//void Send_Dvi_Change_ToCpu(int source);

//�����������ݵ�cpu
//cmd��ο�uart.h�к궨��
//param ������
//void Send_Cmd_ToCpu(int cmd,int param);

//Ӧ��cpu�Ļ�ȡ��Ϣ������
void AnswerCpu_GetInfo(uint16_t ask);
//Ӧ��cpu��������Ϣ������ errcodeΪ0��ʾ�ɹ�������ֵΪ������ ӦС��0x7f
//void AnswerCpu_Status(uart_err_t errcode);


//�����ʼ��
void Com_Cpu_Recive_Buff_Init(void);

#endif
