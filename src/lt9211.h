
#ifndef		_LT9211_H
#define		_LT9211_H

#include <gd32f10x.h>

//////////////////////LT9211 Config////////////////////////////////
#define _Mipi_PortA_
//#define _Mipi_PortB_   


//extern uint8_t cmd_init_9211;  //1 复位9211


typedef enum LT9211_OUTPUTMODE_ENUM
{
    OUTPUT_RGB888         =0,
    OUTPUT_BT656_8BIT     =1,
    OUTPUT_BT1120_16BIT   =2,
    OUTPUT_LVDS_2_PORT    =3,
	OUTPUT_LVDS_1_PORT    =4,
    OUTPUT_YCbCr444       =5,
    OUTPUT_YCbCr422_16BIT
}e_lt9211_outmode_t;

#define LT9211_OutPutModde  OUTPUT_LVDS_1_PORT

typedef enum VIDEO_INPUTMODE_ENUM
{
    INPUT_RGB888          =1,
    INPUT_YCbCr444        =2 ,
    INPUT_YCbCr422_16BIT  =3
}Video_Input_Mode_TypeDef;

#define Video_Input_Mode  INPUT_YCbCr444


//#define lvds_format_JEIDA

//#define lvds_sync_de_only


//////////option for debug///////////


typedef struct video_timing{
	uint16_t hfp;
	uint16_t hs;
	uint16_t hbp;
	uint16_t hact;
	uint16_t htotal;
	uint16_t vfp;
	uint16_t vs;
	uint16_t vbp;
	uint16_t vact;
	uint16_t vtotal;
	uint32_t pclk_khz;
}video_timming_t;

typedef enum VideoFormat
{
	video_1024x600_60Hz_vic=1,
	video_1366x768_60Hz_vic,
	video_1280x1024_60Hz_vic,
	video_1920x1080_60Hz_vic,
	video_1920x1200_60Hz_vic,
	video_640x480_60Hz_vic, 
	video_1280x720_60Hz_vic,   //zhao added 2022-07-04
    video_none
}e_videofomat_t;

typedef struct Lane_No{
	uint8_t	swing_high_byte;
	uint8_t	swing_low_byte;
	uint8_t	emph_high_byte;
    uint8_t	emph_low_byte;
}lane_no_t;

extern void	LT9211_Config(void);

typedef enum  _MIPI_LANE_NUMBER
{	
	MIPI_1_LANE = 1,
	MIPI_2_LANE = 2,
	MIPI_3_LANE = 3,	
	MIPI_4_LANE = 0   //default 4Lane
} MIPI_LANE_NUMBER__TypeDef;

typedef enum  _REG8235_PIXCK_DIVSEL   ////dessc pll to generate pixel clk
{	
//[1:0]PIXCK_DIVSEL 
//00 176M~352M
//01 88M~176M
//10 44M~88M
//11 22M~44M	
	PIXCLK_LARGER_THAN_176M  = 0x80,
	PIXCLK_88M_176M          = 0x81,//zyqa 81->83
	PIXCLK_44M_88M           = 0x82,	
	PIXCLK_22M_44M           = 0x83   //default 4Lane
} REG8235_PIXCK_DIVSEL_TypeDef;

#define  PCLK_KHZ_44000    44000 
#define  PCLK_KHZ_88000    88000
#define  PCLK_KHZ_176000   176000
#define  PCLK_KHZ_352000   352000 



//Mcu端的控制引脚初始化
void LT9211_Mcu_ControlPort_Init(void);

//void LT9211_Reset(void);  //控制屏幕复位

//通过9211控制lcd复位一次
//void Lt9211_lcd_reset(void); //控制屏幕复位
//通过9211控制lcd复位一次
void LT9211_Config(void);



//创建后，删除自己
void LT9211_Once_Task(void* arg);
#endif
/**********************END OF FILE******************/

