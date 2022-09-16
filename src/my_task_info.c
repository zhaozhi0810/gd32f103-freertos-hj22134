#include "includes.h"

static char InfoBuffer[256];


//注释的代码中包含了一些任务管理的方法，建议先不删除！！！！！


void query_task(void)
{  
//	uint8_t i =0 ;
//	uint32_t TotalRunTime;
//	UBaseType_t Priority;
//	TaskStatus_t *StatusArray;
//	UBaseType_t  ArraySize;
//	TaskHandle_t  TestHandler;  //查询任务的句柄
//	
//	
//	UBaseType_t   MinStackSize;
//	eTaskState    TaskState;  //任务状态

//	//查询优秀级
////	debug_printf_string("----------------------任务优秀级--------------------\r\n");
////	Priority = uxTaskPriorityGet(QUERYTask_Handler);  
////	printf("Task Priority = %d\n",(int)Priority);
//	

//	debug_printf_string("------------------------任务信息--------------------\r\n");
//	//获取当前系统中存在的任务数量
//	ArraySize = uxTaskGetNumberOfTasks();
//	//为数组申请空间
//	StatusArray = pvPortMalloc(ArraySize * sizeof(TaskStatus_t));         //分配内存
//	if(NULL != StatusArray)   //内存申请成功后来获取信息
//	{
//		//获取系统中任务状态
//		ArraySize  = 	uxTaskGetSystemState( (TaskStatus_t *) StatusArray,
//                                      ( UBaseType_t )ArraySize,
//                                      (uint32_t * ) &TotalRunTime );
//		//统计完成后输出信息
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
	
//	vTaskGetRunTimeStats((char *)&InfoBuffer);   //可以得到运行计数和cpu占用率
	
	
//	printf("----------------------任务查找--------------------\r\n");
//	//根据任务名字查找某个任务的句柄 
//	TestHandler = xTaskGetHandle("query_task");
//	printf("TestHandler = %#x\r\n",(unsigned int)TestHandler);
//	printf("TestHandler = %#x\r\n",(unsigned int )QUERYTask_Handler);
//	
//	
//	printf("----------------------任务状态--------------------\r\n");
//	//获取某个任务的壮态，这个壮态是 eTaskState 类型。 
//	TaskState = eTaskGetState(QUERYTask_Handler);
//	printf("TaskState = %d\r\n",TaskState);
//	
//	
//	//以一种表格的形式输出当前系统中所有任务的详细信息
//	printf("----------------------任务信息--------------------\r\n");
//	vTaskList(InfoBuffer);
//	printf("%s\r\n",InfoBuffer);
//	while(1)
//    {
//       
//		  	vTaskDelay(500);
//			 //获取任务的堆栈的历史剩余最小值，FreeRTOS 中叫做“高水位线”
//			printf("------------------任务高水位线----------------\r\n");
//			  MinStackSize = uxTaskGetStackHighWaterMark(QUERYTask_Handler);
//			  printf("MinStackSize = %d\r\n",(int)MinStackSize);
//			  vTaskDelay(1000);
//				//printf(1000);("led2 is running\r\n");
//    }
}







