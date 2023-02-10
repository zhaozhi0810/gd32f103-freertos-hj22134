

/*
	���󰴼�ɨ��
	
	6*6  ͨ��iic��չ�����ġ�
	
*/


#include "includes.h"

#define KEYS_IIC_ADDR (0)  //ֻ��ʾA2A1A0 3�����ŵ�ֵ
#define NCA9555_IIC_CONTROLER  IIC3_INDEX   //��Ӧ�ⲿ�ж�13  2021-12-07

		
//ֻ��6�У���Щ�������ڷֱ�ɨ��ÿһ��
const static uint8_t key_scan_line[] = {0xfe,0xfd,0xfb,0xf7,0xef,0xdf};

// 6*6 �ļ��̾����ܹ���33������������������h�ļ��ж���
static BTN_INFO g_btns_info;

TaskHandle_t  TaskHandle_key_Matrix;   //��Ű�������ָ��



//�˿ڵ�����
void matrix_keys_init(void)
{
	uint8_t outcfg_dat[2]={0,0xff};   //IICоƬGPIO���ģʽ����Ӧ��λҪ����Ϊ0
	//1. iic�ĳ�ʼ��
	nca9555_init(NCA9555_IIC_CONTROLER);
		
	//���󰴼���P0�˿�����Ϊ�����P1�˿�����Ϊ���룬��ΪP1�˿���������������
	nca9555_write_2config(NCA9555_IIC_CONTROLER,KEYS_IIC_ADDR,outcfg_dat);

	nca9555_write_outport(NCA9555_IIC_CONTROLER,KEYS_IIC_ADDR,0, 0); //P0�˿����0

#ifdef 	BTNS_USE_INT   //����btns_leds.h�ж���
	//�ж����ų�ʼ��
	//2. �ж����ŵĳ�ʼ�� PB12���ⲿ�ж�12
	//2.1 ʱ��ʹ��
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_AF);		
	
	gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, GPIO_PIN_12);	
	
	//2.2 ����Ϊ�ⲿ�ж����ţ�
	gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOB, GPIO_PIN_SOURCE_12);
	
	//���ô�����ʽ���͵�ƽ����
	exti_init(EXTI_12, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
	exti_interrupt_enable(EXTI_12);
	exti_interrupt_flag_clear(EXTI_12);
	//2.3 nvic�����ж�
	//�жϿ�����ʹ�ܣ�ʹ�õ����ⲿ�ж�12
	nvic_irq_enable(EXTI10_15_IRQn,  7, 0);   //�����жϣ����������ȼ�

	//��ʼ��֮���ȡһ��
	matrix_keys_row_scan();	

#endif			
	memset(&g_btns_info,0,sizeof(g_btns_info));   //��������	
}








static uint8_t matrix_keys_row_scan(void)
{
	uint8_t key_row_dat;
	
	if(nca9555_read_inport(NCA9555_IIC_CONTROLER,KEYS_IIC_ADDR,1,&key_row_dat) == 0)  //��ʾ��ȡ�ɹ�
	{
		if((key_row_dat&0x3f) != 0x3f)   //ֻ�жϵ�6λ������ȱ�ʾ�а�������
		{
			return key_row_dat&0x3f;
		}
		else
		{
			return 0x3f;
		//	printf("ERROR: KEY_ROW_SCAN key_row_dat == 0x3f\r\n");
		}
	}
	else //iic��ȡʧ��
	{
		printf("ERROR: KEY_ROW_SCAN nca9555_read_inport\r\n");
		return 0xff;
	}
}



#if 1   //2023-02-09  ����
static uint8_t release_report = 0;  //�ɿ��ϱ���


//��⵽�����ͷź�ɨ��һ���С�
//P1(L�ź�)����ߣ�P0(H�ź�) ��Ϊ���룬����ĳЩλ�ߵ�ƽ��ʾ�а��������£������0���ʾ�������ɿ�
//col : 0-5 �ֱ��ʾL1 - L6
uint8_t matrix_keys_col_scan(uint8_t col)
{
	uint8_t outcfg_dat[2]={0xff,0};  //P0��Ϊ���룬P1��Ϊ��������������
	uint8_t key_row_dat;
	uint8_t outdat = 0;
	uint8_t ret;
	
	//P1 ���1
	outdat = 1<< col;
	nca9555_write_outport(NCA9555_IIC_CONTROLER,KEYS_IIC_ADDR,1, outdat); //P1��L�źţ��˿����0
	
	//P0������֮ǰ�������0���ͷ�һ�µ�ƽ
	nca9555_write_outport(NCA9555_IIC_CONTROLER,KEYS_IIC_ADDR,0, 0);
	//����
	nca9555_write_2config(NCA9555_IIC_CONTROLER,KEYS_IIC_ADDR,outcfg_dat);
	//��P0��H�źţ��˿�,û�а���ʱ��Ӧ����0���а���ʱ����0
	if(nca9555_read_inport(NCA9555_IIC_CONTROLER,KEYS_IIC_ADDR,0,&key_row_dat) == 0)  //��ʾ��ȡ�ɹ�
	{
		if((key_row_dat & 0x3f) != 0)   //ֻ�жϵ�6λ������ȱ�ʾ�а�������
		{
			ret = key_row_dat&0x3f;
		}
		else
		{
			ret = 0x7f;
		//	printf("ERROR: KEY_ROW_SCAN key_row_dat == 0x3f\r\n");
		}
	}
	else //iic��ȡʧ��
	{
		printf("ERROR: KEY_ROW_SCAN nca9555_read_inport\r\n");
		ret = 0xff;
	}
	
	//���ã�Ĭ����P0(H�ź�)�����P1(L�ź�) ��Ϊ����
	outcfg_dat[0]=0;   
	outcfg_dat[1]=0xff;
	nca9555_write_2config(NCA9555_IIC_CONTROLER,KEYS_IIC_ADDR,outcfg_dat);
	
	return ret;
}


//ʶ�����ĸ�����
//col : 0-5 �ֱ��ʾL1 - L6
void matrix_keys_scan_col(uint8_t col,uint8_t dat)
{	
	uint8_t index,j;
	if(!dat || (dat >= 0x3f))
		return;
	
	if(col > 5)
		return;
	
	for(j=0;j<ROW_NUM;j++)  //ÿһ��ɨ��һ��
	{
		index = 6*j+col;
		if(index > 35)
			continue;
		if(((dat>>j)&(1))) //�ߵ�ƽ��ʾ���£��ͱ�ʾ�ɿ�
		{
			if(!g_btns_info.value[index])
			{	
			//	g_btns_info.pressCnt[index]++;
			//	if(g_btns_info.pressCnt[index] == 2)//��⵽��ֹ1��
				{   //���������ϱ�һ��
					g_btns_info.value[index] = 1;
				//	g_btns_info.reportEn[index] = 1;  //�����ϱ�
					send_btn_change_to_cpu(index+1,1); //���Ͱ�������/�ɿ�
					if(more_debug_info)
						printf("@#@#btn:%d press\r\n",index+1);
					release_report = 1;   //��¼��Ҫ�ͷű�־
				}
			}
		}
		else if(g_btns_info.value[index]) //֮ǰ��״̬�ǰ���
		{								
			g_btns_info.value[index] = 0;
		//	g_btns_info.reportEn[index] = 2;   //�ɿ��ϱ�
			send_btn_change_to_cpu(index+1,0); //���Ͱ�������/�ɿ�
			if(more_debug_info)
				printf("@@@btn:%d release\r\n",index+1);
			g_btns_info.pressCnt[index] = 0;			
		}
	}	
}



#endif




							

/***
 *��������KEY_SCAN
 *��  �ܣ�6*6����ɨ��
 *����ֵ��1~36����Ӧ36������,0��ʾû�м�⵽
 */
char matrix_keys_scan(void)
{    
    uint8_t key_row_num=0;        //��ɨ������¼
    uint8_t i,j;
	uint8_t index,col_dat;   //
	uint8_t unpress_count = 0;   //���������жϣ�������û���а������������в���ʶ��  
	
	
	key_row_num = matrix_keys_row_scan();
	if(key_row_num < 0x3f)   //��ȡ����һ����Ч�İ�������
	{
		for(i=0;i<COL_NUM;i++)  //ÿһ��ɨ��һ��
		{
			if(nca9555_write_outport(NCA9555_IIC_CONTROLER,KEYS_IIC_ADDR,0, key_scan_line[i])) //P0�˿����0
			{
				printf("ERROR: KEY_ROW_SCAN nca9555_write_outport i=%d\r\n",i);
				continue;  //д��ʧ�ܣ�ֱ����������
				//	return -1;
			}
			//�ٴζ�ȡ����
			key_row_num = matrix_keys_row_scan();
			if(key_row_num >= 0x3f)   //ɨ�赽��һ��û�а���
				unpress_count++;

			for(j=0;j<ROW_NUM;j++)  //ÿһ��ɨ��һ��
			{
				index = 6*i+j;
				if(!((key_row_num>>j)&(1))) //����
				{
					if(g_btns_info.pressCnt[index] < 2)
					{	
						g_btns_info.pressCnt[index]++;
						if(g_btns_info.pressCnt[index] == 2)//��⵽��ֹ1��
						{   //���������ϱ�һ��
							g_btns_info.value[index] = 1;
						//	g_btns_info.reportEn[index] = 1;  //�����ϱ�
							send_btn_change_to_cpu(index+1,1); //���Ͱ�������/�ɿ�
							if(more_debug_info)
								printf("----btn:%d press\r\n",index+1);
							release_report = 1;   //��¼��Ҫ�ͷű�־
						}
					}
				}
				else //�ɿ�
				{
					if(g_btns_info.value[index]) //֮ǰ��״̬�ǰ���
					{
						#if 1
						col_dat = matrix_keys_col_scan(j);
				//		printf("j = %d dat = %#x,index = %d\r\n",j,col_dat,index);
						if(col_dat >= 0x3f)
						{						
							g_btns_info.value[index] = 0;
						//	g_btns_info.reportEn[index] = 2;   //�ɿ��ϱ�
							send_btn_change_to_cpu(index+1,0); //���Ͱ�������/�ɿ�
							if(more_debug_info)
								printf("++++btn:%d release\r\n",index+1);
							g_btns_info.pressCnt[index] = 0;
						}
						else{
							matrix_keys_scan_col(j,col_dat);
						}
						#else
						g_btns_info.value[index] = 0;
					//	g_btns_info.reportEn[index] = 2;   //�ɿ��ϱ�
						send_btn_change_to_cpu(index+1,0); //���Ͱ�������/�ɿ�
						printf("++++btn:%d release\r\n",index+1);
						g_btns_info.pressCnt[index] = 0;
						#endif
					}		
				}
			}
		}
		
		if(unpress_count >= 6) //���������жϣ�������û���а���
		{
			for(index=0;index<COL_NUM;index++)   //�ٽ���һ����ɨ��
			{
				col_dat = matrix_keys_col_scan(index);
			//	printf("33- dat = %#x,index = %d\r\n",col_dat,index);
				if(col_dat < 0x3f)
					matrix_keys_scan_col(index,col_dat);
			}
		}		
	}
	else
	{
		if(release_report)  //��Ҫ�ϱ��ͷ���Ϣ��
		{		
			for(index=0;index<COL_NUM*ROW_NUM;index++)
			{
				if(g_btns_info.value[index]) //֮ǰ��״̬�ǰ���
				{
					#if 1
					j = index%6;
					col_dat = matrix_keys_col_scan(j);
				//	printf("22- j = %d dat = %#x,index = %d\r\n",j,col_dat,index);
					if(col_dat >= 0x3f)
					{						
						g_btns_info.value[index] = 0;
					//	g_btns_info.reportEn[index] = 2;   //�ɿ��ϱ�
						send_btn_change_to_cpu(index+1,0); //���Ͱ�������/�ɿ�
						if(more_debug_info)
							printf("####btn:%d release\r\n",index+1);
						g_btns_info.pressCnt[index] = 0;
					}
					#else
				
					g_btns_info.value[index] = 0;
				//	g_btns_info.reportEn[index] = 2;   //�ɿ��ϱ�
					send_btn_change_to_cpu(index+1,0); //���Ͱ�������/�ɿ�
					printf("#####btn:%d release\r\n",index+1);
					g_btns_info.pressCnt[index] = 0;
					#endif
				}
				else{
					matrix_keys_scan_col(j,col_dat);
				}
				
				//��⵽�İ�������ȫ������
				g_btns_info.pressCnt[index] = 0;
				
			}
		//	btn_start_scan = 0;   //��������ɨ��
			release_report = 0;
		}
	}
//	unpress_count = 0;   //����û�а�����������
	nca9555_write_outport(NCA9555_IIC_CONTROLER,KEYS_IIC_ADDR,0, 0);	
	
	return 0;
}



#if 0
/***
 *��������KEY_SCAN
 *��  �ܣ�6*6����ɨ��
 *����ֵ��1~36����Ӧ36������,0��ʾû�м�⵽
 */
char matrix_keys_scan(void)
{    
    uint8_t key_row_num=0;        //��ɨ������¼
    uint8_t i,j;
	uint8_t index;   //
	static uint8_t release_report = 0;  //�ɿ��ϱ���
	
	key_row_num = matrix_keys_row_scan();
	if(key_row_num < 0x3f)   //��ȡ����һ����Ч�İ�������
	{
		for(i=0;i<COL_NUM;i++)  //ÿһ��ɨ��һ��
		{
			if(nca9555_write_outport(NCA9555_IIC_CONTROLER,KEYS_IIC_ADDR,0, key_scan_line[i])) //P0�˿����0
			{
				printf("ERROR: KEY_ROW_SCAN nca9555_write_outport i=%d\r\n",i);
				continue;  //д��ʧ�ܣ�ֱ����������
				//	return -1;
			}
			//�ٴζ�ȡ����
			key_row_num = matrix_keys_row_scan();

			for(j=0;j<ROW_NUM;j++)  //ÿһ��ɨ��һ��
			{
				index = 6*i+j;
				if(!((key_row_num>>j)&(1))) //����
				{
					if(g_btns_info.pressCnt[index] < 2)
					{	
						g_btns_info.pressCnt[index]++;
						if(g_btns_info.pressCnt[index] == 2)//��⵽��ֹ1��
						{   //���������ϱ�һ��
							g_btns_info.value[index] = 1;
						//	g_btns_info.reportEn[index] = 1;  //�����ϱ�
							send_btn_change_to_cpu(index+1,1); //���Ͱ�������/�ɿ�
							printf("----btn:%d press\r\n",index+1);
							release_report = 1;   //��¼��Ҫ�ͷű�־
						}
					}
				}
				else //�ɿ�
				{
					if(g_btns_info.value[index]) //֮ǰ��״̬�ǰ���
					{
						g_btns_info.value[index] = 0;
					//	g_btns_info.reportEn[index] = 2;   //�ɿ��ϱ�
						send_btn_change_to_cpu(index+1,0); //���Ͱ�������/�ɿ�
						printf("++++btn:%d release\r\n",index+1);
						g_btns_info.pressCnt[index] = 0;
					}		
				}
			}
		}
	}
	else
	{
		if(release_report)  //��Ҫ�ϱ��ͷ���Ϣ��
		{		
			for(index=0;index<COL_NUM*ROW_NUM;index++)
			{
				if(g_btns_info.value[index]) //֮ǰ��״̬�ǰ���
				{
					g_btns_info.value[index] = 0;
				//	g_btns_info.reportEn[index] = 2;   //�ɿ��ϱ�
					send_btn_change_to_cpu(index+1,0); //���Ͱ�������/�ɿ�
					printf("#####btn:%d release\r\n",index+1);
					g_btns_info.pressCnt[index] = 0;
				}
				
			}
		//	btn_start_scan = 0;   //��������ɨ��
			release_report = 0;
		}
	}

	nca9555_write_outport(NCA9555_IIC_CONTROLER,KEYS_IIC_ADDR,0, 0);	
	
	return 0;
}

#endif






#ifdef 	BTNS_USE_INT
//�ⲿ�ж�12�Ĵ�����,�������º��ɿ����ᴥ���жϣ�������
void exint12_handle(void)
{

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	
	xTaskNotifyFromISR(TaskHandle_key_Matrix, 0, eIncrement, &xHigherPriorityTaskWoken);  //�������ߵ�����
	//���ҽ�ֹ�ж�
	exti_interrupt_disable(EXTI_12);   //ɨ�����֮����ʹ��
}

#endif

#if 0
void EXTI10_15_IRQHandler(void)
{
	if(exti_interrupt_flag_get(EXTI_12))
	{
		
		exint12_handle();
		//	exint456_handle();
	}
	exti_interrupt_flag_clear(EXTI_12);  //���ϱ�־
}


#endif


/*
	main��������ɨ������30msһ��,��ѯ��ʽ

	btn_start_scan ���жϺ������ã����º��ɿ���������Ϊ��0
	����������
*/
void task_matrix_keys_scan(void* arg)
{	
	//1. ���󰴼�ɨ���ʼ��
	matrix_keys_init();

	while(1)
	{	
		//�ȴ����񱻻���
		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);   //��1��Ȼ�����޵ȴ�
		{	
			matrix_keys_scan();

			vTaskDelay(30);    //��ʱ30ms
#ifdef 	BTNS_USE_INT	
			exti_interrupt_enable(EXTI_12);   //ɨ�����֮����ʹ��		
#endif

		}		
	}
}



