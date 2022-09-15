
#ifndef __UART_H__
#define __UART_H__

#include <gd32f10x.h>
//#include "gd32f103r_eval.h"


#define FRAME_HEAD 0xA5


#define COMn                             2U

#define EVAL_COM0                        USART0     //GD32 是从0开始命名，stm32是从1开始
#define EVAL_COM0_CLK                    RCU_USART0
#define EVAL_COM0_TX_PIN                 GPIO_PIN_9
#define EVAL_COM0_RX_PIN                 GPIO_PIN_10
#define EVAL_COM0_GPIO_PORT              GPIOA
#define EVAL_COM0_GPIO_CLK               RCU_GPIOA
#define EVAL_COM0_NVIC 					USART0_IRQn    //中断号


#define EVAL_COM1                        USART1
#define EVAL_COM1_CLK                    RCU_USART1
#define EVAL_COM1_TX_PIN                 GPIO_PIN_2
#define EVAL_COM1_RX_PIN                 GPIO_PIN_3
#define EVAL_COM1_GPIO_PORT              GPIOA
#define EVAL_COM1_GPIO_CLK               RCU_GPIOA
#define EVAL_COM1_NVIC 					USART1_IRQn   //中断号



//void send_btn_change_to_cpu(uint8_t whichkey,uint8_t status);


#define UART_BUFF_FULL_COVER    //队列使用回绕覆盖模式
#define MAX_QUEUE_UART_LEN (64)  //每个队列缓存空间

typedef struct{
	uint8_t sendIndex;
	uint8_t recvIndex;
	uint8_t length;      //接收到的数据长度，类型跟开辟的空间有关
	uint8_t dataBuf[MAX_QUEUE_UART_LEN];
}Queue_UART_STRUCT;








#define  DEBUG_PRINTF    //用于输出一些出错信息
//#define  MY_DEBUG        //用于输出一些调试信息，正常使用时不需要

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
  

  
//串口初始化函数，参数要求是宏定义 EVAL_COM0 or EVAL_COM1
void gd_eval_com_init(uint32_t com,uint32_t bandrate); 
  
//队列插入数据，返回0表示成功，其他表示失败
int32_t QueueUARTDataInsert(Queue_UART_STRUCT *Queue,uint8_t data);
//从队列取出数据,通过参数data返回，返回0表示成功，其他表示失败
int32_t QueueUARTDataDele(Queue_UART_STRUCT *Queue,uint8_t *data);  
//获取队列的数据长度，返回长度值
uint32_t QueueUARTDataLenGet(Queue_UART_STRUCT *Queue); 

//校验和计算,返回校验和
uint8_t CheckSum_For_Uart(uint8_t *buf, uint8_t len);

//校验数据,返回0表示校验成功，其他表示验证失败
int32_t Uart_Verify_Data_CheckSum(uint8_t *data,uint8_t len);  


void Uart_Tx(uint8_t com_no, uint8_t ch);
//串口发送字符串
void Uart_Tx_String(uint8_t com_no, uint8_t *str, uint8_t length);


//串口去使能，针对与cpu连接的串口，调试串口不需要实现
void gd_eval_com_deinit(void);


//发送按键的数据到cpu
//whichkey 1 - 36
//status 0松开 or 1 按下
void send_btn_change_to_cpu(uint8_t whichkey,uint8_t status);

#endif
