

#ifndef __GPIOS_H__
#define __GPIOS_H__

#include <gd32f10x.h>

//extern TaskHandle_t  TaskHandle_Morseptt;   //存放morseptt按键任务指针


//gpio 引脚初始化
void Gpios_init(void);


//void Wxen_Control_Init(void);
void Wxen_Control_Enable(void);
//禁止该引脚，断电1.1v
void Wxen_Control_Disable(void);


//void OePins_Control_Init(void);
//which 1-4 分别表示oe1-oe4
void OePins_Output_Hight(uint8_t which);
//which 1-4 分别表示oe1-oe4
void OePins_Output_Low(uint8_t which);


//PE15，初始化后输出低
//void LcdCtrl_Control_Init(void);
//使能该引脚，通电Lcd电源
void LcdCtrl_Enable(void);
//禁止该引脚，断电Lcd电源
void LcdCtrl_Disable(void);



//PD8  lcd-reset ,不知道是高电平复位还是低电平复位
//先假设低电平复位吧。
//void lcd_reset_control_init(void);
//触发一次lcd的复位信号
void lcd_reset_control(void);



//PD6  MicCtl 输出控制(参数status 非0输出高，0输出低)
void MicCtl_Control_SetOutVal(uint8_t status);

//PD6  MicCtl
//void MicCtl_Control_Init(void);
//获得lcd屏幕的类型，2022-09-21之后新做的底板，通过与按键板的三个引脚相连
//读取引脚的电平，判断lcd的类型，之前是由3399的引脚判断的。
//返回值0表示5寸屏，非0表示7寸屏
//2022-09-21 目前还没有新的底板用于判断，默认返回0（表示5寸屏）
uint8_t Get_Lcd_Type(void);
//获取键盘类型
/*返回值
1：防爆终端键盘类型
2：壁挂Ⅲ型终端键盘类型（不关心！！）
4：嵌入式Ⅰ/Ⅱ/Ⅲ型、防风雨型、壁挂Ⅱ型终端键盘类型
6：多功能型终端键盘类型(只有这个是7寸屏，其他都是5寸)
*/
uint8_t get_LcdType_val(void);

//PA7  LSPK_CRL
void LSPK_Control_Init(void);

void LSPK_Enable(void);
void LSPK_Disable(void);

////PA7  LSPK 输出控制(参数status 非0输出高，0输出低)
void LSPK_Control_SetOutVal(uint8_t status);
//翻转
void LSPK_Control_ToggleOut(void);


//PC6  V12_CTL
void V12_CTL_Control_Init(void);

void V12_CTL_Enable(void);
void V12_CTL_Disable(void);

////PC6  V12_CTL 输出控制(参数status 非0输出高，0输出低)
void V12_CTL_Control_SetOutVal(uint8_t status);
//翻转
void V12_CTL_Control_ToggleOut(void);



//PC8,9,10,输入引脚
void LcdType_Control_Init(void);


//PC5  5寸背光使能？
//2022-12-13 增加
void SHTDB_5IN_Control_Init(void);
////PC5  SHTDB_5IN 输出控制(参数status 非0输出高电平点亮5inch，0输出低熄灭 5inch lcd)
void SHTDB_5IN_Control_SetOutVal(uint8_t status);
//翻转
void SHTDB_5IN_Control_ToggleOut(void);
//高电平点亮 5inch lcd
void SHTDB_5IN_Enable(void);
//低电平熄灭 5inch lcd
void SHTDB_5IN_Disable(void);


//PC11  扬声器使能
//2022-12-13 增加
void SPKEN_Control_Init(void);
////PC11  扬声器使能
void SPKEN_Control_SetOutVal(uint8_t status);
//翻转
void SPKEN_Control_ToggleOut(void);

//2022-12-13 增加
void EAR_L_EN_Control_Init(void);
////PC12  扬声器使能
void EAR_L_EN_Control_SetOutVal(uint8_t status);
//翻转
void EAR_L_EN_Control_ToggleOut(void);

//2022-12-13 增加
void EAR_R_EN_Control_Init(void);
////PC13  扬声器使能
void EAR_R_EN_Control_SetOutVal(uint8_t status);
//翻转
void EAR_R_EN_Control_ToggleOut(void);

#endif

