#include "includes.h"

static char InfoBuffer[256];


//ע�͵Ĵ����а�����һЩ�������ķ����������Ȳ�ɾ������������


void query_task(void)
{  
//	uint8_t i =0 ;
//	uint32_t TotalRunTime;
//	UBaseType_t Priority;
//	TaskStatus_t *StatusArray;
//	UBaseType_t  ArraySize;
//	TaskHandle_t  TestHandler;  //��ѯ����ľ��
//	
//	
//	UBaseType_t   MinStackSize;
//	eTaskState    TaskState;  //����״̬

//	//��ѯ���㼶
////	debug_printf_string("----------------------�������㼶--------------------\r\n");
////	Priority = uxTaskPriorityGet(QUERYTask_Handler);  
////	printf("Task Priority = %d\n",(int)Priority);
//	

//	debug_printf_string("------------------------������Ϣ--------------------\r\n");
//	//��ȡ��ǰϵͳ�д��ڵ���������
//	ArraySize = uxTaskGetNumberOfTasks();
//	//Ϊ��������ռ�
//	StatusArray = pvPortMalloc(ArraySize * sizeof(TaskStatus_t));         //�����ڴ�
//	if(NULL != StatusArray)   //�ڴ�����ɹ�������ȡ��Ϣ
//	{
//		//��ȡϵͳ������״̬
//		ArraySize  = 	uxTaskGetSystemState( (TaskStatus_t *) StatusArray,
//                                      ( UBaseType_t )ArraySize,
//                                      (uint32_t * ) &TotalRunTime );
//		//ͳ����ɺ������Ϣ
//		debug_printf_string("TaskName\tPriority\tTaskNumber\tTaskState\tMinStackSize\r\n");
//		for(i = 0;i< ArraySize;i++)
//		{
//			debug_printf_string((char*)(StatusArray[i].pcTaskName));
//			debug_printf_string("\t");
//			debug_printf_u32((int)StatusArray[i].uxCurrentPriority,10);
//			debug_printf_string("\t");
//			debug_printf_u32((int)StatusArray[i].xTaskNumber,10);
//			
//			debug_printf_string("\t");
//			TestHandler = xTaskGetHandle(StatusArray[i].pcTaskName);
//			TaskState = eTaskGetState(TestHandler);
//			debug_printf_u32((int)TaskState,10);
//			MinStackSize = uxTaskGetStackHighWaterMark(TestHandler);
//			debug_printf_string("\t");
//			debug_printf_u32((int)MinStackSize,10);
//			debug_printf_string("\t\r\n");
//			//printf("%s\t\t%d\t\t%d\t\t\t\r\r\n",StatusArray[i].pcTaskName,
//			//(int)StatusArray[i].uxCurrentPriority,
//			//(int)StatusArray[i].xTaskNumber);
//		
//		}
//		
//	}
	debug_printf_string("----------------------task info--------------------\r\n");
	debug_printf_string("TaskName  TaskState Priority  LeftStack  TaskNum \r\n");
	vTaskList(InfoBuffer);
	debug_printf_string(InfoBuffer);
	debug_printf_string("\r\n");
	debug_printf_string("  B: Block  R: Ready  D: Delete  S: Stop  X: Run \r\n");
	
//	vTaskGetRunTimeStats((char *)&InfoBuffer);   //���Եõ����м�����cpuռ����
	
	
//	printf("----------------------�������--------------------\r\n");
//	//�����������ֲ���ĳ������ľ�� 
//	TestHandler = xTaskGetHandle("query_task");
//	printf("TestHandler = %#x\r\n",(unsigned int)TestHandler);
//	printf("TestHandler = %#x\r\n",(unsigned int )QUERYTask_Handler);
//	
//	
//	printf("----------------------����״̬--------------------\r\n");
//	//��ȡĳ�������׳̬�����׳̬�� eTaskState ���͡� 
//	TaskState = eTaskGetState(QUERYTask_Handler);
//	printf("TaskState = %d\r\n",TaskState);
//	
//	
//	//��һ�ֱ�����ʽ�����ǰϵͳ�������������ϸ��Ϣ
//	printf("----------------------������Ϣ--------------------\r\n");
//	vTaskList(InfoBuffer);
//	printf("%s\r\n",InfoBuffer);
//	while(1)
//    {
//       
//		  	vTaskDelay(500);
//			 //��ȡ����Ķ�ջ����ʷʣ����Сֵ��FreeRTOS �н�������ˮλ�ߡ�
//			printf("------------------�����ˮλ��----------------\r\n");
//			  MinStackSize = uxTaskGetStackHighWaterMark(QUERYTask_Handler);
//			  printf("MinStackSize = %d\r\n",(int)MinStackSize);
//			  vTaskDelay(1000);
//				//printf(1000);("led2 is running\r\n");
//    }
}







