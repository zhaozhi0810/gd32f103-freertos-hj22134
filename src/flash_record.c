

#include "includes.h"

#include "fmc_flash.h"




//gd32f103vbt6  ��md  128kflash  ����ҳ��С��1k
#define RECORD_ADDR_BASE 0x801fc00   //��128k



//�������ݼ�¼
uint8_t read_flash_config(flash_rcd_t* config)
{
	flash_rcd_t dat,dat1;
	uint32_t i = 0;
	do
	{
		//һֱ������0xffffffff		
		read_4Btye(RECORD_ADDR_BASE+(i*4), (uint32_t*)&dat, sizeof dat);
		if(dat.lcd_pwm != 0xffffffffU)
		{
			dat1 = dat;
		}
		else 
		{
			if(i == 0) //��һ�ξͶ�����0xffff,�Ǿ���û�б����
			{
				return 255;
			}
			else
			{
				*config = dat1;   //0xffff֮ǰ������
				return 0;
			}
		}
		i++;
	}
	while(1);
	
//	return 0;
}


//�����ݼ�¼��flash��
uint8_t write_flash_config(flash_rcd_t * data)
{
	uint32_t k,offset;
	//1. д���µ�����ǰ������ȫ������д���ˣ�����
	if( 0 == (k=check_erase(RECORD_ADDR_BASE,sizeof(flash_rcd_t))))
	{
		MY_PRINTF("write_flash_config falsh write full��erase all\r\n");
		erase_flash(RECORD_ADDR_BASE, 1);    //������һҳ
		offset = 0;   //ƫ�Ƶ�ַΪ0
	}
	else
	{
		MY_PRINTF("write_flash_config k = %d\r\n",k);
		//������ʼ��ַ
		offset = FMC_PAGE_SIZE - (k)*4;   //�ҵ�һ���հ�����д��ȥ
	}
	MY_PRINTF("write_flash_config offset = %d\r\n",offset);
	//2. д������
	write_flash(RECORD_ADDR_BASE+offset, (uint32_t *)data, sizeof(flash_rcd_t));
	
	
	return 0;
}
