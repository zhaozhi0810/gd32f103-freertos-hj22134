

#include "includes.h"
#include "fmc_flash.h"

/*
	�������洢��������������512KB��GD32F10x_CL��GD32F10x_HD��ֻʹ����bank0��
	�� �� GD32F10x_MD �� �� �� ҳ �� С Ϊ 1KB ��
	GD32F10x_CL �� GD32F10x_HD ��GD32F10x_XD�� bank0������ҳ��СΪ2KB�� bank1������ҳ��СΪ4KB�� 

	���ݱ��ʱҪ��4�ֽڶ����2�ֽڶ���
	
	
	gd32f103vbt6  ��md  128kflash  ����ҳ��С��1k
*/
/*ҳ��С*/
#define FMC_PAGE_SIZE           (((uint16_t)0x400U))   //0x400 ��ʾ1K

/*
	��ȡflash���ݣ������ڲ�����4�ֽڶ������
*/
void read_4Btye(uint32_t read_addr, uint32_t *data, uint32_t len)
{
    int dest_addr;
    dest_addr = read_addr & 0xfffffffc;//4�ֽڶ���
    dest_addr += (read_addr & 0x03) > 0 ? 4 : 0;
    int real_len = len >> 2;
    uint32_t *addr = (uint32_t*)dest_addr;

    for(int i = 0; i < real_len; i++)
    {
        *(data + i) = *(addr + i);
    }
}


/*
	����Ƿ���Ҫ����ҳ
	addr:�����ҳ��ʼ��ַ

*/
int check_erase(int start_addr ,uint32_t datalen)
{
    //ָ������ת��
    uint32_t *addr = (uint32_t*)start_addr;
	uint32_t k = 0;
	int i;
    for(i = 0; i<FMC_PAGE_SIZE >> 2; i++)
    {
        if(*(addr + i) == 0xffffffff)   //��û��д�������򣬼�¼һ�¿հ׵������ж��٣�
        {
            k++;
        }
    }

	if((datalen>>2) <= k)  //���ݳ��ȱȿ��е�����С
	{
		return k;   //���Լ���ʹ��,����ֵӦ�ô���0
	}
	
    return 0;  //ȫ������д���ˣ�Ҫ���²���
}


/*
	������Ҫʹ�õ�����,����������ǰ�벿��û��ʹ�õĲ���
*/
void erase_flash(uint32_t write_addr, int page_num)
{
//    int i = 0;
    //��ǰҳ��ƫ�Ƶ�ַ
//    uint32_t page_offset = write_addr & (FMC_PAGE_SIZE - 1);
//    page_offset = page_offset >> 2; //����Ϊ4�ֽ�
    //ҳ��ʼ��ַ
    uint32_t start_addr  = write_addr & (0xffffffff - FMC_PAGE_SIZE + 1);
    //ָ������ת��
//    uint32_t *addr = (uint32_t*)start_addr;

//    uint32_t *buff = (uint32_t*)malloc(FMC_PAGE_SIZE);
//    //���������Ŀռ䱣������

//    if(page_offset)
//    {
//        for( i = 0; i < page_offset; i++)
//        {
//            buff[i] = *(addr + i);
//        }
//    }

    uint32_t erase_counter;

    /* ����flash */
    fmc_unlock();


    //�������������־
    fmc_flag_clear(FMC_FLAG_BANK0_END);
    //�������/�����־
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    //���ҳ��̴����־
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

    /* ����ʹ�õ���ҳ */
    for(erase_counter = 0; erase_counter < page_num; erase_counter++)
    {
        //����ָ����ҳ������ҳ��ַ
        if(check_erase(start_addr , FMC_PAGE_SIZE * erase_counter))
        {
            fmc_page_erase(start_addr + FMC_PAGE_SIZE * erase_counter);
            fmc_flag_clear(FMC_FLAG_BANK0_END);
            fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
            fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
        }
    }

		//д������ǰ�벿��û��ʹ�õĿռ�
//    if(page_offset)
//    {
//        for( i = 0; i < page_offset; i++)
//        {
//            //��flash��̣�Ҳ����д���ݣ�ÿ��д��1����=4���ֽ�
//     //       fmc_word_program(start_addr + (i * 4), *(buff + i));
//            fmc_flag_clear(FMC_FLAG_BANK0_END);
//            fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
//            fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
//        }
//    }

    /* flash��������ֹ��� */
    fmc_lock();
//    free(buff);
}



/*
	д��flash���ⳤ�ȵ��������ݣ������ڲ�����4�ֽڶ������
*/

void write_flash(uint32_t write_addr, uint32_t *data, int len)
{

    int dest_addr;
    dest_addr = write_addr & 0xfffffffc;//4�ֽڶ���
    dest_addr += (write_addr & 0x03) > 0 ? 4 : 0;

    uint32_t page_num = len / FMC_PAGE_SIZE; //���������Ҫ��������С
    page_num += len % FMC_PAGE_SIZE ? 1 : 0;//
    //��������
//    erase_flash(dest_addr, page_num);
    int len_4Byte = len >> 2;
    /* ���� */
    fmc_unlock();

    for(int i = 0; i < len_4Byte; i++)
    {

        //��flash��̣�Ҳ����д���ݣ�ÿ��д��1����=4���ֽ�
        fmc_word_program(dest_addr + i * 4, *(data + i));
        fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
    }

    /* ����*/
    fmc_lock();


}


//uint8_t write[2300];
//uint8_t read[2300];
//int flash_test(void)
//{

//    for(int i = 0; i < 2300; i++)
//    {
//        write[i] = i % 256;
//    }

//    write_flash(0x800400c, (uint32_t*)write, 2300);
//    read_4Btye(0x800400c, (uint32_t*)read, 2300);

//    for(int i = 0; i < 2300; i++)
//    {
//        if(write[i] != read[i])
//        {
//            printf("write error %d\r\n", i);
//            return 0;
//        }
//    }

//    printf("write ok \r\n");
//    return 1;
//}

