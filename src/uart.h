
#ifndef __UART_H__
#define __UART_H__

#include <gd32f10x.h>
//#include "gd32f103r_eval.h"


#define FRAME_HEAD 0xA5


#define COMn                             2U

#define EVAL_COM0                        USART0     //GD32 �Ǵ�0��ʼ������stm32�Ǵ�1��ʼ
#define EVAL_COM0_CLK                    RCU_USART0
#define EVAL_COM0_TX_PIN                 GPIO_PIN_9
#define EVAL_COM0_RX_PIN                 GPIO_PIN_10
#define EVAL_COM0_GPIO_PORT              GPIOA
#define EVAL_COM0_GPIO_CLK               RCU_GPIOA
#define EVAL_COM0_NVIC 					USART0_IRQn    //�жϺ�


#define EVAL_COM1                        USART1
#define EVAL_COM1_CLK                    RCU_USART1
#define EVAL_COM1_TX_PIN                 GPIO_PIN_2
#define EVAL_COM1_RX_PIN                 GPIO_PIN_3
#define EVAL_COM1_GPIO_PORT              GPIOA
#define EVAL_COM1_GPIO_CLK               RCU_GPIOA
#define EVAL_COM1_NVIC 					USART1_IRQn   //�жϺ�



//void send_btn_change_to_cpu(uint8_t whichkey,uint8_t status);


#define UART_BUFF_FULL_COVER    //����ʹ�û��Ƹ���ģʽ
#define MAX_QUEUE_UART_LEN (64)  //ÿ�����л���ռ�

typedef struct{
	uint8_t sendIndex;
	uint8_t recvIndex;
	uint8_t length;      //���յ������ݳ��ȣ����͸����ٵĿռ��й�
	uint8_t dataBuf[MAX_QUEUE_UART_LEN];
}Queue_UART_STRUCT;








//#define  DEBUG_PRINTF    //�������һЩ������Ϣ
#define  MY_DEBUG        //�������һЩ������Ϣ������ʹ��ʱ����Ҫ

#ifdef DEBUG_PRINTF
  #define DBG_PRINTF(fmt, args...)  \
  do{\
    printf("<<File:%s  Line:%d  Function:%s>> ", __FILE__, __LINE__, __FUNCTION__);\
    printf(fmt, ##args);\
  }while(0)
#else
  #define DBG_PRINTF(fmt, args...)   
#endif


  
#ifdef MY_DEBUG
#define MY_PRINTF(fmt, args...)  \
  do{\
	printf("<<MY_DEBUG>> ");\
    printf(fmt, ##args);\
  }while(0)
#else
  #define MY_PRINTF(fmt, args...)   
#endif
  

  
//���ڳ�ʼ������������Ҫ���Ǻ궨�� EVAL_COM0 or EVAL_COM1
void gd_eval_com_init(uint32_t com,uint32_t bandrate); 
  
//���в������ݣ�����0��ʾ�ɹ���������ʾʧ��
int32_t QueueUARTDataInsert(Queue_UART_STRUCT *Queue,uint8_t data);
//�Ӷ���ȡ������,ͨ������data���أ�����0��ʾ�ɹ���������ʾʧ��
int32_t QueueUARTDataDele(Queue_UART_STRUCT *Queue,uint8_t *data);  
//��ȡ���е����ݳ��ȣ����س���ֵ
uint32_t QueueUARTDataLenGet(Queue_UART_STRUCT *Queue); 

//У��ͼ���,����У���
uint8_t CheckSum_For_Uart(uint8_t *buf, uint8_t len);

//У������,����0��ʾУ��ɹ���������ʾ��֤ʧ��
int32_t Uart_Verify_Data_CheckSum(uint8_t *data,uint8_t len);  


void Uart_Tx(uint8_t com_no, uint8_t ch);
//���ڷ����ַ���
void Uart_Tx_String(uint8_t com_no, uint8_t *str, uint8_t length);


//����ȥʹ�ܣ������cpu���ӵĴ��ڣ����Դ��ڲ���Ҫʵ��
void gd_eval_com_deinit(void);


//���Ͱ��������ݵ�cpu
//whichkey 1 - 36
//status 0�ɿ� or 1 ����
void send_btn_change_to_cpu(uint8_t whichkey,uint8_t status);

#endif
