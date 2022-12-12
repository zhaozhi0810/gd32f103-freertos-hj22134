

#include "includes.h"
//#include "gd32f103r_eval.h"
//#include "stdio.h"
/*
	两个串口，串口1（调试），和串口2（发送数据到cpu）
	
	串口函数没有设计接收函数
	两个波特率都是115200，8N1
	
*/




static const rcu_periph_enum COM_CLK[COMn] = {EVAL_COM0_CLK, EVAL_COM1_CLK};
static const uint32_t COM_TX_PIN[COMn] = {EVAL_COM0_TX_PIN, EVAL_COM1_TX_PIN};
static const uint32_t COM_RX_PIN[COMn] = {EVAL_COM0_RX_PIN, EVAL_COM1_RX_PIN};
static const uint32_t COM_GPIO_PORT[COMn] = {EVAL_COM0_GPIO_PORT, EVAL_COM1_GPIO_PORT};
static const rcu_periph_enum COM_GPIO_CLK[COMn] = {EVAL_COM0_GPIO_CLK, EVAL_COM1_GPIO_CLK};
static const uint32_t COM_NVIC[COMn] = {EVAL_COM0_NVIC,EVAL_COM1_NVIC};
static const uint8_t COM_PRIO[COMn] = {8, 6};     //优先级配置，在这可以调整优先级啊！！！！
	


static uint8_t uart_inited = 0 ; //bit0,bit1有效，为1表示初始化了，为0表示未初始化

/*!
    \brief      configure COM port
    \param[in]  com: COM on the board
      \arg        EVAL_COM0: COM0 on the board
      \arg        EVAL_COM1: COM1 on the board
    \param[out] none
    \retval     none
*/
void gd_eval_com_init(uint32_t com_id,uint32_t bandrate)
{
    uint32_t com = EVAL_COM0;
		
    if(TOCPU_COM_NUM == com_id){
        com = EVAL_COM1;
    }
		
	if(uart_inited & (1<<com_id))   //已经初始化了，就不用再来初始化了。
		return;
		
    /*1. 使能GPIO时钟 enable GPIO clock */
    rcu_periph_clock_enable(COM_GPIO_CLK[com_id]);

    /*2. 使能uart时钟 enable USART clock */
    rcu_periph_clock_enable(COM_CLK[com_id]);

    /*3. 引脚初始化为复用功能模式 connect port to USARTx_Tx */
    gpio_init(COM_GPIO_PORT[com_id], GPIO_MODE_AF_PP, GPIO_OSPEED_2MHZ, COM_TX_PIN[com_id]);

    /*4. 引脚初始化为复用功能模式 connect port to USARTx_Rx */
    gpio_init(COM_GPIO_PORT[com_id], GPIO_MODE_IN_FLOATING, GPIO_OSPEED_2MHZ, COM_RX_PIN[com_id]);

    /*4. uart控制器初始化 USART configure */
    usart_deinit(com);
    usart_baudrate_set(com, bandrate);
    usart_word_length_set(com, USART_WL_8BIT);   //数据位
    usart_stop_bit_set(com, USART_STB_1BIT);    //停止位
    usart_parity_config(com, USART_PM_NONE);    //校验位
    usart_hardware_flow_rts_config(com, USART_RTS_DISABLE);   //流控
    usart_hardware_flow_cts_config(com, USART_CTS_DISABLE);
    usart_receive_config(com, USART_RECEIVE_ENABLE);
    usart_transmit_config(com, USART_TRANSMIT_ENABLE);
    usart_enable(com);
	
	//5. 接收中断的初始化。
	usart_interrupt_enable(com, USART_INT_RBNE);    //接收中断
//	usart_interrupt_enable(com, USART_INT_ERR);
	

	//6. nvic的配置
	//nvic_irq_enable(COM_NVIC[com_id],  COM_PRIO[com_id]>>2, COM_PRIO[com_id]&0x3);   //允许中断，并设置优先级
	nvic_irq_enable(COM_NVIC[com_id],  COM_PRIO[com_id], 0);	//全部是抢占优先级
	uart_inited |= 1<<com_id;
}


//串口去使能，针对与cpu连接的串口，调试串口不需要实现
void gd_eval_com_deinit(void)
{
	//关闭中断
	nvic_irq_disable(EVAL_COM1_NVIC); 
	usart_interrupt_enable(EVAL_COM1, USART_INT_RBNE);    //接收中断
	usart_interrupt_enable(EVAL_COM1, USART_INT_IDLE);    //空闲中断
	
	usart_receive_config(EVAL_COM1, USART_RECEIVE_DISABLE);     //去除使能
    usart_transmit_config(EVAL_COM1, USART_TRANSMIT_DISABLE);
    usart_disable(EVAL_COM1);  //去除使能

	//引脚也设置为普通io口
	gpio_init(EVAL_COM1_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, EVAL_COM1_RX_PIN);
	gpio_init(EVAL_COM1_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, EVAL_COM1_TX_PIN);
}	




void Uart_Tx(uint8_t com_no, uint8_t ch)
{
	const uint32_t com[] = {EVAL_COM0,EVAL_COM1};
    
	if(com_no > TOCPU_COM_NUM){
        return;
    }
	
	if(uart_inited & (1<<com_no))   //uart没有初始化
	{
		while(RESET == usart_flag_get(com[com_no], USART_FLAG_TBE));
		usart_data_transmit(com[com_no], (uint8_t)ch); 
	}	
}







//发送一段字符
//com 表示从哪个端口发出
void Uart_Tx_String(uint8_t com_no, uint8_t *str, uint8_t length)
{
	uint8_t i;
	
	for(i = 0; i < length; i++)
		Uart_Tx(com_no,str[i]);
}





//队列插入数据
int32_t QueueUARTDataInsert(Queue_UART_STRUCT *Queue,uint8_t data)
{
	if(MAX_QUEUE_UART_LEN == Queue->length)
	{
#ifdef UART_BUFF_FULL_COVER   //覆盖回绕模式
		Queue->sendIndex = (Queue->sendIndex + 1) % MAX_QUEUE_UART_LEN;
		Queue->length--;
#else
		return -1;
#endif
	}
	
	Queue->dataBuf[Queue->recvIndex] = data;
	
	Queue->recvIndex = (Queue->recvIndex + 1) % MAX_QUEUE_UART_LEN;
	
	Queue->length++;
	
	return 0;
}

int32_t QueueUARTDataDele(Queue_UART_STRUCT *Queue,uint8_t *data)
{
	if(0 == Queue->length)
		return -1;
	
	*data = Queue->dataBuf[Queue->sendIndex];
	
	Queue->sendIndex = (Queue->sendIndex + 1) % MAX_QUEUE_UART_LEN;
	
	Queue->length--;
	
	return 0;
}

//void QueueUARTDataIndexRecover(Queue_UART_STRUCT *Queue)
//{
//	Queue->sendIndex = (Queue->sendIndex - 1) % MAX_QUEUE_UART_LEN;
//	
//	Queue->length++;
//}

uint32_t QueueUARTDataLenGet(Queue_UART_STRUCT *Queue)
{
	return Queue->length;
}




//校验和计算
uint8_t CheckSum_For_Uart(uint8_t *buf, uint8_t len)
{
	uint8_t sum;
	uint8_t i;

	for(i=0,sum=0; i<len; i++)
		sum += buf[i];

	return sum;
}



//校验数据,返回0表示校验成功，其他表示验证失败
int32_t Uart_Verify_Data_CheckSum(uint8_t *data,uint8_t len)
{
	uint8_t check;
	int32_t ret = -1;
	
	if(data == NULL)
		return -1;
	
	//读取原数据中的校验值
	check = data[len - 1];
	
	//重新计算校验值
	if(check==CheckSum_For_Uart(data,len - 1))
		ret = 0;
	
	return ret;
}




uint8_t checksum(uint8_t *buf, uint8_t len)
{
	uint8_t sum;
	uint8_t i;

	for(i=0,sum=0; i<len; i++)
		sum += buf[i];

	return sum;
}



//校验数据
int32_t verify_data(uint8_t *data,uint8_t len)
{
	uint8_t check;
	int32_t ret = -1;
	
	if(data == NULL)
		return -1;
	
	//读取原数据中的校验值
	check = data[len - 1];
	
	//重新计算校验值
	if(check==checksum(data,len - 1))
		ret = 0;
	
	return ret;
}





//发送按键的数据到cpu
//whichkey 1 - 36
//status 0松开 or 1 按下
void send_btn_change_to_cpu(uint8_t whichkey,uint8_t status)
{
	unsigned char buf[5];  	
	
	if(!(uart_inited & 2))   //串口没初始化
	{
		debug_printf_string("error: TOCPU_COM not init\n");
		return ;
	}
	buf[0] = FRAME_HEAD;  //帧头	
	buf[1] = whichkey;    //
	buf[2] = status;    //
	//crc重新计算
	buf[3] = buf[0] + buf[1] + buf[2];  //校验和，0,1,2相加.
	
	Uart_Tx_String(TOCPU_COM_NUM,buf, 4);   //com_frame_t并没有包含数据头，所以加1个字节	
}


