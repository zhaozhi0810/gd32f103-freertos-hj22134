

#include "includes.h"
#include "fmc_flash.h"

/*
	对于主存储闪存容量不多于512KB的GD32F10x_CL和GD32F10x_HD，只使用了bank0；
	对 于 GD32F10x_MD ， 闪 存 页 大 小 为 1KB 。
	GD32F10x_CL 和 GD32F10x_HD ，GD32F10x_XD， bank0的闪存页大小为2KB， bank1的闪存页大小为4KB； 

	数据编程时要求4字节对齐或2字节对齐
	
	
	gd32f103vbt6  是md  128kflash  所以页大小是1k
*/
/*页大小*/
#define FMC_PAGE_SIZE           (((uint16_t)0x400U))   //0x400 表示1K

/*
	读取flash数据，函数内部做了4字节对齐操作
*/
void read_4Btye(uint32_t read_addr, uint32_t *data, uint32_t len)
{
    int dest_addr;
    dest_addr = read_addr & 0xfffffffc;//4字节对齐
    dest_addr += (read_addr & 0x03) > 0 ? 4 : 0;
    int real_len = len >> 2;
    uint32_t *addr = (uint32_t*)dest_addr;

    for(int i = 0; i < real_len; i++)
    {
        *(data + i) = *(addr + i);
    }
}


/*
	检查是否需要擦除页
	addr:待检查页起始地址

*/
int check_erase(int start_addr ,uint32_t datalen)
{
    //指着类型转换
    uint32_t *addr = (uint32_t*)start_addr;
	uint32_t k = 0;
	int i;
    for(i = 0; i<FMC_PAGE_SIZE >> 2; i++)
    {
        if(*(addr + i) == 0xffffffff)   //有没有写过的区域，记录一下空白的区域还有多少？
        {
            k++;
        }
    }

	if((datalen>>2) <= k)  //数据长度比空闲的区域小
	{
		return k;   //可以继续使用,返回值应该大于0
	}
	
    return 0;  //全部都被写过了，要重新擦除
}


/*
	擦除需要使用的扇区,并保存扇区前半部分没有使用的部分
*/
void erase_flash(uint32_t write_addr, int page_num)
{
//    int i = 0;
    //当前页的偏移地址
//    uint32_t page_offset = write_addr & (FMC_PAGE_SIZE - 1);
//    page_offset = page_offset >> 2; //换算为4字节
    //页起始地址
    uint32_t start_addr  = write_addr & (0xffffffff - FMC_PAGE_SIZE + 1);
    //指着类型转换
//    uint32_t *addr = (uint32_t*)start_addr;

//    uint32_t *buff = (uint32_t*)malloc(FMC_PAGE_SIZE);
//    //将不操作的空间保存起来

//    if(page_offset)
//    {
//        for( i = 0; i < page_offset; i++)
//        {
//            buff[i] = *(addr + i);
//        }
//    }

    uint32_t erase_counter;

    /* 解锁flash */
    fmc_unlock();


    //清除操作结束标志
    fmc_flag_clear(FMC_FLAG_BANK0_END);
    //清除擦除/错误标志
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    //清楚页编程错误标志
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

    /* 擦除使用到的页 */
    for(erase_counter = 0; erase_counter < page_num; erase_counter++)
    {
        //擦除指定的页，参数页地址
        if(check_erase(start_addr , FMC_PAGE_SIZE * erase_counter))
        {
            fmc_page_erase(start_addr + FMC_PAGE_SIZE * erase_counter);
            fmc_flag_clear(FMC_FLAG_BANK0_END);
            fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
            fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
        }
    }

		//写入扇区前半部分没有使用的空间
//    if(page_offset)
//    {
//        for( i = 0; i < page_offset; i++)
//        {
//            //对flash编程，也就是写数据，每次写入1个字=4个字节
//     //       fmc_word_program(start_addr + (i * 4), *(buff + i));
//            fmc_flag_clear(FMC_FLAG_BANK0_END);
//            fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
//            fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
//        }
//    }

    /* flash加锁，禁止编程 */
    fmc_lock();
//    free(buff);
}



/*
	写入flash任意长度的数据数据，函数内部做了4字节对齐操作
*/

void write_flash(uint32_t write_addr, uint32_t *data, int len)
{

    int dest_addr;
    dest_addr = write_addr & 0xfffffffc;//4字节对齐
    dest_addr += (write_addr & 0x03) > 0 ? 4 : 0;

    uint32_t page_num = len / FMC_PAGE_SIZE; //存放数据需要的扇区大小
    page_num += len % FMC_PAGE_SIZE ? 1 : 0;//
    //擦除扇区
//    erase_flash(dest_addr, page_num);
    int len_4Byte = len >> 2;
    /* 解锁 */
    fmc_unlock();

    for(int i = 0; i < len_4Byte; i++)
    {

        //对flash编程，也就是写数据，每次写入1个字=4个字节
        fmc_word_program(dest_addr + i * 4, *(data + i));
        fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
    }

    /* 加锁*/
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

