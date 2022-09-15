

#include "includes.h"

#include "fmc_flash.h"




//gd32f103vbt6  是md  128kflash  所以页大小是1k
#define RECORD_ADDR_BASE 0x801fc00   //第128k



//用于数据记录
uint8_t read_flash_config(flash_rcd_t* config)
{
	flash_rcd_t dat,dat1;
	uint32_t i = 0;
	do
	{
		//一直读到是0xffffffff		
		read_4Btye(RECORD_ADDR_BASE+(i*4), (uint32_t*)&dat, sizeof dat);
		if(dat.lcd_pwm != 0xffffffffU)
		{
			dat1 = dat;
		}
		else 
		{
			if(i == 0) //第一次就读到了0xffff,那就是没有保存过
			{
				return 255;
			}
			else
			{
				*config = dat1;   //0xffff之前的数据
				return 0;
			}
		}
		i++;
	}
	while(1);
	
//	return 0;
}


//把数据记录到flash中
uint8_t write_flash_config(flash_rcd_t * data)
{
	uint32_t k,offset;
	//1. 写入新的配置前，发现全部都被写过了，擦除
	if( 0 == (k=check_erase(RECORD_ADDR_BASE,sizeof(flash_rcd_t))))
	{
		MY_PRINTF("write_flash_config falsh write full，erase all\r\n");
		erase_flash(RECORD_ADDR_BASE, 1);    //擦除这一页
		offset = 0;   //偏移地址为0
	}
	else
	{
		MY_PRINTF("write_flash_config k = %d\r\n",k);
		//计算起始地址
		offset = FMC_PAGE_SIZE - (k)*4;   //找到一个空白区域写进去
	}
	MY_PRINTF("write_flash_config offset = %d\r\n",offset);
	//2. 写入数据
	write_flash(RECORD_ADDR_BASE+offset, (uint32_t *)data, sizeof(flash_rcd_t));
	
	
	return 0;
}
