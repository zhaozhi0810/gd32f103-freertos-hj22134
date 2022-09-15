

#ifndef __FMC_FLASH_H__
#define __FMC_FLASH_H__


#include <gd32f10x.h>

#define FMC_PAGE_SIZE           (((uint16_t)0x400U)) 
/*
	读取flash数据，函数内部做了4字节对齐操作
*/
void read_4Btye(uint32_t read_addr, uint32_t *data, uint32_t len);

/*
	检查是否需要擦除页
	addr:待检查页起始地址

*/
int check_erase(int start_addr ,uint32_t datalen);


/*
	擦除需要使用的扇区,并保存扇区前半部分没有使用的部分
*/
void erase_flash(uint32_t write_addr, int page_num);



/*
	写入flash任意长度的数据数据，函数内部做了4字节对齐操作
*/

void write_flash(uint32_t write_addr, uint32_t *data, int len);

#endif
