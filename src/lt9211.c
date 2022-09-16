/************************************************************
*	ProjectName:	   LT9211
*	FileName:	       LT9211.c
*	BuildData:	     2018-05-24
*	Version：        V1.0
* 	Author:          xhguo
* 	Company:	     Lontium
************************************************************/
#include	"includes.h"

static uint16_t hact, vact;
static uint16_t hs, vs;
static uint16_t hbp, vbp;
static uint16_t htotal, vtotal;
static uint16_t hfp, vfp;
static uint8_t VideoFormat = 0 ;

//uint8_t cmd_init_9211 = 0;  //1 复位9211


//enum VideoFormat Video_Format;
#define MIPI_LANE_CNT  MIPI_4_LANE // 0: 4lane
#define MIPI_SETTLE_VALUE 0x05//（0A or 05） 
#define PCR_M_VALUE 0x17 

//Video_pattern
#define VIDEO_PATTERN 0          

static uint8_t     I2CADR = 0x5A;   //iic地址（8位地址）
//#define LT9211_OUTPUT_PATTERN 1
   				 						//hfp, hs, hbp,hact,htotal,vfp, vs, vbp, vact,vtotal,pixclk Khz

//static struct video_timing video_1920x1080_60Hz   ={88, 44, 148,1920,  2200,  4,  5,  36, 1080, 1125, 148500};
//static struct video_timing video_1280x720_60Hz   ={ 52, 8, 52,1280,  1392, 8,  3,  5, 720, 736,  62200};
//static struct video_timing video_1024x600_60Hz   ={ 144, 48, 96,1024,  1312, 10,  2,  9, 600, 621,  48836};
//static struct video_timing video_1024x600_60Hz   ={ 160, 20, 120,1024,  1344, 12,  3,  20, 600, 635,  55000};
//static struct video_timing video_1024x600_60Hz   ={ 144, 48, 96,1024,  1312, 11,  3,  10, 600, 624,  49000};
//static struct video_timing video_1024x600_60Hz   ={ 120, 40, 120,1024,  1304, 14,  4,  14, 600, 632,  49500};
//static struct video_timing video_1024x600_60Hz   ={ 144, 40, 104,1024,  1312, 18,  1,  3, 600, 622,  48960};
//static struct video_timing video_1024x600_60Hz   ={ 100, 80, 108,1024,  1312, 7,  3,  11, 600, 621,  48885};
//static struct video_timing video_1024x600_60Hz   ={ 128, 32, 128,600,  888, 16,  4,  16, 1024, 1060,  56480};
static struct video_timing video_1024x600_60Hz   ={ 128, 32, 128,1024,  1312, 16,  4,  16, 600, 636,  56480};




static void LT9211_Reset(void)
{
	//复位引脚PE1 
//   P11 = 0;
	gpio_bit_reset(GPIOE, GPIO_PIN_1);
	vTaskDelay(100);
//   P11 = 1;
	
	gpio_bit_set(GPIOE, GPIO_PIN_1);    //高电平
	vTaskDelay(100);
}


//Mcu端的控制引脚初始化
void LT9211_Mcu_ControlPort_Init(void)
{
	//1. iic的初始化
	IicApp_Init(IIC1_INDEX);   //PB6，PB7 使用的是iic1（stm32称为是iic1）
	
	//2. 复位引脚的初始化
	//2.1 时钟使能
	rcu_periph_clock_enable(RCU_GPIOE);	
		
	gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_1);	
	//低电平
	gpio_bit_reset(GPIOE, GPIO_PIN_1);
		
}





//返回
uint8_t HDMI_ReadI2C_ByteN(uint8_t RegAddr,uint8_t *p_data,uint8_t N)
{
	uint8_t flag;
	
	//flag = i2c_read_byte(I2CADR, RegAddr, p_data,N);
	//返回非0为错误，0表示正确
	flag = ! IicApp_Read_Bytes(IIC1_INDEX,I2CADR, RegAddr, p_data,N);
	return flag;
}

uint8_t HDMI_WriteI2C_Byte(uint8_t RegAddr, uint8_t d)
{
	uint8_t flag;
	
	//flag=i2c_write_byte(I2CADR, RegAddr,&d,1);
	//返回非0为错误，0表示正确
	flag = ! IicApp_Write_Bytes(IIC1_INDEX,I2CADR, RegAddr, &d,1);
	return flag;
}

uint8_t HDMI_WriteI2C_ByteN(uint8_t RegAddr, uint8_t *d,uint8_t N)
{
	uint8_t flag;
	
	//flag=i2c_write_byte(I2CADR, RegAddr,d,N) ;
	//返回非0为错误，0表示正确
	flag = ! IicApp_Write_Bytes(IIC1_INDEX,I2CADR,RegAddr,d,N);
	return flag;
}

uint8_t HDMI_ReadI2C_Byte(uint8_t RegAddr)
{
	uint8_t  p_data=0;
	
	if (HDMI_ReadI2C_ByteN(RegAddr,&p_data,1))  //非零表示ok
	{
		return p_data;
	}
	return 0;
}




void LT9211_ChipID(void)
{
//	HDMI_WriteI2C_Byte(0xff,0x81);//register bank
//	printf("\r\nLT9211 Chip ID:%x,",HDMI_ReadI2C_Byte(0x00));
//	printf("%x, ",HDMI_ReadI2C_Byte(0x01));
//	printf("%x, ",HDMI_ReadI2C_Byte(0x02));
//	printf("\r\n");
}

void LT9211_SystemInt(void)
{
	  /* system clock init */		   
	HDMI_WriteI2C_Byte(0xff,0x82);
	HDMI_WriteI2C_Byte(0x01,0x18);
	
	HDMI_WriteI2C_Byte(0xff,0x86);
	HDMI_WriteI2C_Byte(0x06,0x61); 	
	HDMI_WriteI2C_Byte(0x07,0xa8); //fm for sys_clk
	  
	HDMI_WriteI2C_Byte(0xff,0x87); //初始化 txpll 寄存器列表默认值给错了
	HDMI_WriteI2C_Byte(0x14,0x08); //default value
	HDMI_WriteI2C_Byte(0x15,0x00); //default value
	HDMI_WriteI2C_Byte(0x18,0x0f);
	HDMI_WriteI2C_Byte(0x22,0x08); //default value
	HDMI_WriteI2C_Byte(0x23,0x00); //default value
	HDMI_WriteI2C_Byte(0x26,0x0f); 
}

void LT9211_MipiRxPhy(void)
{
	HDMI_WriteI2C_Byte(0xff,0xd0);
	HDMI_WriteI2C_Byte(0x00,MIPI_LANE_CNT);	// 0: 4 Lane / 1: 1 Lane / 2 : 2 Lane / 3: 3 Lane
	/* Mipi rx phy */
	HDMI_WriteI2C_Byte(0xff,0x82);
	HDMI_WriteI2C_Byte(0x02,0x44); //port A mipi rx enable
	/*port A/B input 8205/0a bit6_4:EQ current setting*/
	HDMI_WriteI2C_Byte(0x05,0x36); //port A CK lane swap  0x32--0x36 for WYZN Glassbit2- Port A mipi/lvds rx s2p input clk select: 1 = From outer path.
    HDMI_WriteI2C_Byte(0x0d,0x26); //bit6_4:Port B Mipi/lvds rx abs refer current  0x26 0x76
	HDMI_WriteI2C_Byte(0x17,0x0c);
	HDMI_WriteI2C_Byte(0x1d,0x0c);
	
	HDMI_WriteI2C_Byte(0x0a,0x81);//eq control for LIEXIN  horizon line display issue 0xf7->0x80
	HDMI_WriteI2C_Byte(0x0b,0x00);//eq control  0x77->0x00
#ifdef _Mipi_PortA_ 
	  /*port a*/
	HDMI_WriteI2C_Byte(0x07,0x9f); //port clk enable  （只开Portb时,porta的lane0 clk要打开）
	HDMI_WriteI2C_Byte(0x08,0xfc); //port lprx enable
#endif  
#ifdef _Mipi_PortB_	
	  /*port a*/
	  HDMI_WriteI2C_Byte(0x07,0x9f); //port clk enable  （只开Portb时,porta的lane0 clk要打开）
	  HDMI_WriteI2C_Byte(0x08,0xfc); //port lprx enable	
	  /*port b*/
	  HDMI_WriteI2C_Byte(0x0f,0x9F); //port clk enable
	  HDMI_WriteI2C_Byte(0x10,0xfc); //port lprx enable
		HDMI_WriteI2C_Byte(0x04,0xa1);
#endif
	/*port diff swap*/
	HDMI_WriteI2C_Byte(0x09,0x01); //port a diff swap
	HDMI_WriteI2C_Byte(0x11,0x01); //port b diff swap	

	/*port lane swap*/
	HDMI_WriteI2C_Byte(0xff,0x86);		
	HDMI_WriteI2C_Byte(0x33,0x1b); //port a lane swap	1b:no swap	
	HDMI_WriteI2C_Byte(0x34,0x1b); //port b lane swap 1b:no swap

}


void LT9211_MipiRxDigital(void)
{	   
    HDMI_WriteI2C_Byte(0xff,0x86);
#ifdef _Mipi_PortA_ 	
    HDMI_WriteI2C_Byte(0x30,0x85); //mipirx HL swap	 	
#endif

#ifdef _Mipi_PortB_	
   HDMI_WriteI2C_Byte(0x30,0x8f); //mipirx HL swap
#endif
	 HDMI_WriteI2C_Byte(0xff,0xD8);
#ifdef _Mipi_PortA_ 	
	HDMI_WriteI2C_Byte(0x16,0x00); //mipirx HL swap	  bit7- 0:portAinput	
#endif

#ifdef _Mipi_PortB_	
   HDMI_WriteI2C_Byte(0x16,0x80); //mipirx HL swap bit7- portBinput
#endif	
	
	HDMI_WriteI2C_Byte(0xff,0xd0);	
    HDMI_WriteI2C_Byte(0x43,0x12); //rpta mode enable,ensure da_mlrx_lptx_en=0
	
	HDMI_WriteI2C_Byte(0x02,MIPI_SETTLE_VALUE); //mipi rx controller	//settle值
		  
}

void LT9211_SetVideoTiming(struct video_timing *video_format)
{
	//Timer0_vTaskDelay(100);
	vTaskDelay(100);
	HDMI_WriteI2C_Byte(0xff,0xd0);
	HDMI_WriteI2C_Byte(0x0d,(uint8_t)(video_format->vtotal>>8)); //vtotal[15:8]
	HDMI_WriteI2C_Byte(0x0e,(uint8_t)(video_format->vtotal)); //vtotal[7:0]
	HDMI_WriteI2C_Byte(0x0f,(uint8_t)(video_format->vact>>8)); //vactive[15:8]
	HDMI_WriteI2C_Byte(0x10,(uint8_t)(video_format->vact)); //vactive[7:0]
	HDMI_WriteI2C_Byte(0x15,(uint8_t)(video_format->vs)); //vs[7:0]
	HDMI_WriteI2C_Byte(0x17,(uint8_t)(video_format->vfp>>8)); //vfp[15:8]
	HDMI_WriteI2C_Byte(0x18,(uint8_t)(video_format->vfp)); //vfp[7:0]	

	HDMI_WriteI2C_Byte(0x11,(uint8_t)(video_format->htotal>>8)); //htotal[15:8]
	HDMI_WriteI2C_Byte(0x12,(uint8_t)(video_format->htotal)); //htotal[7:0]
	HDMI_WriteI2C_Byte(0x13,(uint8_t)(video_format->hact>>8)); //hactive[15:8]
	HDMI_WriteI2C_Byte(0x14,(uint8_t)(video_format->hact)); //hactive[7:0]
	HDMI_WriteI2C_Byte(0x16,(uint8_t)(video_format->hs)); //hs[7:0]
	HDMI_WriteI2C_Byte(0x19,(uint8_t)(video_format->hfp>>8)); //hfp[15:8]
	HDMI_WriteI2C_Byte(0x1a,(uint8_t)(video_format->hfp)); //hfp[7:0]	

}

void LT9211_TimingSet(void)
{
  uint16_t hact ;	
	uint16_t vact ;	
	uint8_t fmt ;		
	uint8_t pa_lpn = 0;
	
	//Timer0_vTaskDelay(500);//500-->100
	vTaskDelay(500);
	HDMI_WriteI2C_Byte(0xff,0xd0);
	hact = (HDMI_ReadI2C_Byte(0x82)<<8) + HDMI_ReadI2C_Byte(0x83) ;
	hact = hact/3;
	fmt = (HDMI_ReadI2C_Byte(0x84) &0x0f);
	vact = (HDMI_ReadI2C_Byte(0x85)<<8) +HDMI_ReadI2C_Byte(0x86);
	pa_lpn = HDMI_ReadI2C_Byte(0x9c);
//	printf("\r\nhact = %u\n\r",hact);
//	printdec_u32(hact);
//	printf("\r\nvact = %u\n\r",vact);
//	printdec_u32(vact);		
//	printf("\r\nfmt = %x\n\r", fmt);
//	printf("\r\npa_lpn = %x\r\n", pa_lpn);	

	//Timer0_vTaskDelay(100);
	vTaskDelay(100);
//	if (1)//(hact == video_1920x1080_60Hz.hact ) &&( vact == video_1920x1080_60Hz.vact ))
//	{
//		 VideoFormat = video_1920x1080_60Hz_vic;
//		 LT9211_SetVideoTiming(&video_1920x1080_60Hz);
//		printf("1920x1080_60Hz\r\n");
//	}
//	else 
//	if (1)//(hact == video_1280x720_60Hz.hact ) &&( vact == video_1280x720_60Hz.vact ))
//	{
//		VideoFormat = video_1280x720_60Hz_vic;
//		LT9211_SetVideoTiming(&video_1280x720_60Hz);
//		printf("1280x720_60Hz\r\n");
//		
//		
//	}
	if (1)//(hact == video_1280x720_60Hz.hact ) &&( vact == video_1280x720_60Hz.vact ))
	{
		VideoFormat = video_1024x600_60Hz_vic;
		LT9211_SetVideoTiming(&video_1024x600_60Hz);
		debug_printf_string("1024x600_60Hz\r\n");		
	}	
	else 
	{
	   VideoFormat = video_none;
	    debug_printf_string("video_none\r\n");		 
	}
}


void LT9211_MipiRxPll(void)
{
  	/* dessc pll */
	HDMI_WriteI2C_Byte(0xff,0x82);
	HDMI_WriteI2C_Byte(0x2d,0x48);
    HDMI_WriteI2C_Byte(0x35,PIXCLK_44M_88M); /*0x82*/  // PIXCLK_44M_88M		
}


void LT9211_MipiPcr(void)
{	
	uint8_t loopx;
	uint8_t pcr_m;
	
    HDMI_WriteI2C_Byte(0xff,0xd0); 	
    HDMI_WriteI2C_Byte(0x0c,0x60);  //fifo position
	HDMI_WriteI2C_Byte(0x1c,0x60);  //fifo position
	HDMI_WriteI2C_Byte(0x24,0x70);  //pcr mode( de hs vs)
			
	HDMI_WriteI2C_Byte(0x2d,0x30); //M up limit
	HDMI_WriteI2C_Byte(0x31,0x0a); //M down limit

	/*stage1 hs mode*/
	HDMI_WriteI2C_Byte(0x25,0xf0);  //line limit
	HDMI_WriteI2C_Byte(0x2a,0x30);  //step in limit
	HDMI_WriteI2C_Byte(0x21,0x4f);  //hs_step
	HDMI_WriteI2C_Byte(0x22,0x00); 

	/*stage2 hs mode*/
	HDMI_WriteI2C_Byte(0x1e,0x01);  //RGD_DIFF_SND[7:4],RGD_DIFF_FST[3:0]
	HDMI_WriteI2C_Byte(0x23,0x80);  //hs_step
    /*stage2 de mode*/
	HDMI_WriteI2C_Byte(0x0a,0x02); //de adjust pre line
	HDMI_WriteI2C_Byte(0x38,0x02); //de_threshold 1
	HDMI_WriteI2C_Byte(0x39,0x04); //de_threshold 2
	HDMI_WriteI2C_Byte(0x3a,0x08); //de_threshold 3
	HDMI_WriteI2C_Byte(0x3b,0x10); //de_threshold 4
		
	HDMI_WriteI2C_Byte(0x3f,0x04); //de_step 1
	HDMI_WriteI2C_Byte(0x40,0x08); //de_step 2
	HDMI_WriteI2C_Byte(0x41,0x10); //de_step 3
	HDMI_WriteI2C_Byte(0x42,0x20); //de_step 4

	HDMI_WriteI2C_Byte(0x2b,0xa0); //stable out
	//Timer0_vTaskDelay(100);
    HDMI_WriteI2C_Byte(0xff,0xd0);   //enable HW pcr_m
	pcr_m = HDMI_ReadI2C_Byte(0x26);
	pcr_m &= 0x7f;
	HDMI_WriteI2C_Byte(0x26,pcr_m);
	HDMI_WriteI2C_Byte(0x27,0x0f);

	HDMI_WriteI2C_Byte(0xff,0x81);  //pcr reset
	HDMI_WriteI2C_Byte(0x20,0xbf); // mipi portB div issue
	HDMI_WriteI2C_Byte(0x20,0xff);
	//Timer0_vTaskDelay(5);
	vTaskDelay(5);
	HDMI_WriteI2C_Byte(0x0B,0x6F);
	HDMI_WriteI2C_Byte(0x0B,0xFF);
	

	// 	Timer0_vTaskDelay(800);//800->120
	vTaskDelay(800);
	
	{
		for(loopx = 0; loopx < 10; loopx++) //Check pcr_stable 10
		{
			//Timer0_vTaskDelay(200);
			vTaskDelay(200);   
			HDMI_WriteI2C_Byte(0xff,0xd0);
			if(HDMI_ReadI2C_Byte(0x87)&0x08)
			{
				debug_printf_string("\r\nLT9211 pcr stable");
				break;
			}
			else
			{
				debug_printf_string("\r\nLT9211 pcr unstable!!!!");
			}
	  	}
  	}
	
	HDMI_WriteI2C_Byte(0xff,0xd0);
	HDMI_ReadI2C_Byte(0x94);
	//printf("LT9211 pcr_stable_M=%x\n",(HDMI_ReadI2C_Byte(0x94)&0x7F));//打印M值
}



void LT9211_Txpll(void)
{  
	uint8_t loopx;
	if( (LT9211_OutPutModde == OUTPUT_LVDS_2_PORT) || (LT9211_OutPutModde == OUTPUT_LVDS_1_PORT) )
	{
		HDMI_WriteI2C_Byte(0xff,0x82);
		HDMI_WriteI2C_Byte(0x36,0x01); //b7:txpll_pd
		if( LT9211_OutPutModde == OUTPUT_LVDS_1_PORT )
		{
			HDMI_WriteI2C_Byte(0x37,0x29);
		}
		else
		{
			HDMI_WriteI2C_Byte(0x37,0x2a);
		}
		HDMI_WriteI2C_Byte(0x38,0x06);
		HDMI_WriteI2C_Byte(0x39,0x30);
		HDMI_WriteI2C_Byte(0x3a,0x8e);

		HDMI_WriteI2C_Byte(0xFF,0x81);
		HDMI_WriteI2C_Byte(0x20,0xF7);// LVDS Txpll soft reset
		HDMI_WriteI2C_Byte(0x20,0xFF);


		HDMI_WriteI2C_Byte(0xff,0x87);
		HDMI_WriteI2C_Byte(0x37,0x14);
		HDMI_WriteI2C_Byte(0x13,0x00);
		HDMI_WriteI2C_Byte(0x13,0x80);
		//Timer0_vTaskDelay(100);
		vTaskDelay(100);
		for(loopx = 0; loopx < 10; loopx++) //Check Tx PLL cal
		{

			HDMI_WriteI2C_Byte(0xff,0x87);			
			if(HDMI_ReadI2C_Byte(0x1f)& 0x80)
			{
				if(HDMI_ReadI2C_Byte(0x20)& 0x80)
				{
					debug_printf_string("LT9211 tx pll lock\r\n");
				}
				else
				{
					debug_printf_string("LT9211 tx pll unlocked\r\n");
				}					
				debug_printf_string("LT9211 tx pll cal done\r\n");
				break;
			}
			else
			{
			debug_printf_string("LT9211 tx pll unlocked\r\n");
			}
		}
	} 
	debug_printf_string(" system success\r\n");	 	
}

void LT9211_TxPhy(void)
{		
	HDMI_WriteI2C_Byte(0xff,0x82);
	if( (LT9211_OutPutModde == OUTPUT_LVDS_2_PORT) || (LT9211_OutPutModde ==OUTPUT_LVDS_1_PORT) )
	{
		 /* dual-port lvds tx phy */	
		HDMI_WriteI2C_Byte(0x62,0x00); //ttl output disable
		if(LT9211_OutPutModde == OUTPUT_LVDS_2_PORT)
		{
		  HDMI_WriteI2C_Byte(0x3b,0x88);//disable lvds output
		}
		else
		{
			HDMI_WriteI2C_Byte(0x3b,0x08);//disable lvds output
		}
	}
	 // HDMI_WriteI2C_Byte(0x3b,0xb8); //dual port lvds enable	
	HDMI_WriteI2C_Byte(0x3e,0x92); 
	HDMI_WriteI2C_Byte(0x3f,0x48); 	
	HDMI_WriteI2C_Byte(0x40,0x31); 		
	HDMI_WriteI2C_Byte(0x43,0x80); 		
	HDMI_WriteI2C_Byte(0x44,0x00);
	HDMI_WriteI2C_Byte(0x45,0x00); 		
	HDMI_WriteI2C_Byte(0x49,0x00);
	HDMI_WriteI2C_Byte(0x4a,0x01);
	HDMI_WriteI2C_Byte(0x4e,0x00);		
	HDMI_WriteI2C_Byte(0x4f,0x00);
	HDMI_WriteI2C_Byte(0x50,0x00);
	HDMI_WriteI2C_Byte(0x53,0x00);
	HDMI_WriteI2C_Byte(0x54,0x01);
	
	HDMI_WriteI2C_Byte(0xff,0x81);
	HDMI_WriteI2C_Byte(0x20,0x79); //TX PHY RESET & mlrx mltx calib reset
	HDMI_WriteI2C_Byte(0x20,0xff); 
	
}

//#define lvds_format_JEIDA


void LT9211_TxDigital(void)
{ 
	if( (LT9211_OutPutModde == OUTPUT_LVDS_2_PORT) || (LT9211_OutPutModde == OUTPUT_LVDS_1_PORT) ) 
	{
		debug_printf_string("\rLT9211 set to OUTPUT_LVDS");
		HDMI_WriteI2C_Byte(0xff,0x85); /* lvds tx controller */
		HDMI_WriteI2C_Byte(0x59,0x40); 	//bit4-LVDSTX Display color depth set 1-8bit, 0-6bit; 
		HDMI_WriteI2C_Byte(0x5a,0xaa); 
		HDMI_WriteI2C_Byte(0x5b,0xaa);
		if( LT9211_OutPutModde == OUTPUT_LVDS_2_PORT )
		{
			HDMI_WriteI2C_Byte(0x5c,0x03);	//lvdstx port sel 01:dual;00:single
		}
		else
		{
			HDMI_WriteI2C_Byte(0x5c,0x00);
		}
		HDMI_WriteI2C_Byte(0x88,0x50);	
		HDMI_WriteI2C_Byte(0xa1,0x77); 	
		HDMI_WriteI2C_Byte(0xff,0x86);	
		HDMI_WriteI2C_Byte(0x40,0x40); //tx_src_sel
		/*port src sel*/
		HDMI_WriteI2C_Byte(0x41,0x34);	
		HDMI_WriteI2C_Byte(0x42,0x10);
		HDMI_WriteI2C_Byte(0x43,0x23); //pt0_tx_src_sel
		HDMI_WriteI2C_Byte(0x44,0x41);
		HDMI_WriteI2C_Byte(0x45,0x02); //pt1_tx_src_scl

#ifdef lvds_format_JEIDA
    HDMI_WriteI2C_Byte(0xff,0x85);
		HDMI_WriteI2C_Byte(0x59,0xd0); 	
		HDMI_WriteI2C_Byte(0xff,0xd8);
		HDMI_WriteI2C_Byte(0x11,0x40);
#endif	
	}  		
}

void LT9211_ClockCheckDebug(void)
{

	uint32_t fm_value;
	HDMI_WriteI2C_Byte(0xff,0x86);
	HDMI_WriteI2C_Byte(0x00,0x01);
	//Timer0_vTaskDelay(300);
	vTaskDelay(300);
    fm_value = 0;
	fm_value = (HDMI_ReadI2C_Byte(0x08) &(0x0f));
	fm_value = (fm_value<<8) ;
	fm_value = fm_value + HDMI_ReadI2C_Byte(0x09);
	fm_value = (fm_value<<8) ;
	fm_value = fm_value + HDMI_ReadI2C_Byte(0x0a);
	//printf("\r\nmipi input byte clock: %u",fm_value);
//	printdec_u32(fm_value);
	
	HDMI_WriteI2C_Byte(0xff,0x86);
	HDMI_WriteI2C_Byte(0x00,0x0a);
	//Timer0_vTaskDelay(300);
	vTaskDelay(300);
    fm_value = 0;
	fm_value = (HDMI_ReadI2C_Byte(0x08) &(0x0f));
	fm_value = (fm_value<<8) ;
	fm_value = fm_value + HDMI_ReadI2C_Byte(0x09);
	fm_value = (fm_value<<8) ;
	fm_value = fm_value + HDMI_ReadI2C_Byte(0x0a);
	//printf("\r\ndessc pixel clock: %u\n",fm_value);
//	printdec_u32(fm_value);

}

void LT9211_VideoCheckDebug(void)
{

	uint8_t sync_polarity;

	HDMI_WriteI2C_Byte(0xff,0x86);
	sync_polarity = HDMI_ReadI2C_Byte(0x70);
	vs = HDMI_ReadI2C_Byte(0x71);

	hs = HDMI_ReadI2C_Byte(0x72);
	hs = (hs<<8) + HDMI_ReadI2C_Byte(0x73);
	
	vbp = HDMI_ReadI2C_Byte(0x74);
	vfp = HDMI_ReadI2C_Byte(0x75);

	hbp = HDMI_ReadI2C_Byte(0x76);
	hbp = (hbp<<8) + HDMI_ReadI2C_Byte(0x77);

	hfp = HDMI_ReadI2C_Byte(0x78);
	hfp = (hfp<<8) + HDMI_ReadI2C_Byte(0x79);

	vtotal = HDMI_ReadI2C_Byte(0x7A);
	vtotal = (vtotal<<8) + HDMI_ReadI2C_Byte(0x7B);

	htotal = HDMI_ReadI2C_Byte(0x7C);
	htotal = (htotal<<8) + HDMI_ReadI2C_Byte(0x7D);

	vact = HDMI_ReadI2C_Byte(0x7E);
	vact = (vact<<8)+ HDMI_ReadI2C_Byte(0x7F);

	hact = HDMI_ReadI2C_Byte(0x80);
	hact = (hact<<8) + HDMI_ReadI2C_Byte(0x81);

	//printf("\r\nsync_polarity = %x", sync_polarity);
	debug_printf_string_u32("sync_polarity = ",sync_polarity,16);
	//printf("\r\nhfp, hs, hbp, hact, htotal = %u %u %u %u %u\r\n",hfp,hs,hbp,hact,htotal);
//	printdec_u32(hfp);
	debug_printf_string_u32("hfp = ",hfp,10);
//	printdec_u32(hs);
	debug_printf_string_u32("hs = ",hs,10);
//	printdec_u32(hbp);
	debug_printf_string_u32("hbp = ",hbp,10);
//	printdec_u32(hact);
	debug_printf_string_u32("hact = ",hact,10);
//	printdec_u32(htotal);
	debug_printf_string_u32("htotal = ",htotal,10);
	//printf("\r\nvfp, vs, vbp, vact, vtotal = %u %u %u %u %u\r\n",vfp,vs,vbp,vact,vtotal);
//	printdec_u32(vfp);
	debug_printf_string_u32("vfp = ",vfp,10);
//	printdec_u32(vs);
	debug_printf_string_u32("vs = ",vs,10);
//	printdec_u32(vbp);
	debug_printf_string_u32("vbp = ",vbp,10);
//	printdec_u32(vact);
	debug_printf_string_u32("vact = ",vact,10);
//	printdec_u32(vtotal);
	debug_printf_string_u32("vtotal = ",vtotal,10);

}

void LT9211_Pattern(struct video_timing *video_format)
{
	uint32_t pclk_khz;
	uint8_t dessc_pll_post_div;
	uint32_t pcr_m, pcr_k;

	pclk_khz = video_format->pclk_khz;     

	HDMI_WriteI2C_Byte(0xff,0xf9);
	HDMI_WriteI2C_Byte(0x3e,0x80);  
 
	HDMI_WriteI2C_Byte(0xff,0x85);
	HDMI_WriteI2C_Byte(0x88,0xc0);   //0x90:TTL RX-->Mipi TX  ; 0xd0:lvds RX->MipiTX  0xc0:Chip Video pattern gen->Lvd TX

	HDMI_WriteI2C_Byte(0xa1,0x64); 
	HDMI_WriteI2C_Byte(0xa2,0xff); 

	HDMI_WriteI2C_Byte(0xa3,(uint8_t)((video_format->hs+video_format->hbp)/256));
	HDMI_WriteI2C_Byte(0xa4,(uint8_t)((video_format->hs+video_format->hbp)%256));//h_start

	HDMI_WriteI2C_Byte(0xa5,(uint8_t)((video_format->vs+video_format->vbp)%256));//v_start

   	HDMI_WriteI2C_Byte(0xa6,(uint8_t)(video_format->hact/256));
	HDMI_WriteI2C_Byte(0xa7,(uint8_t)(video_format->hact%256)); //hactive

	HDMI_WriteI2C_Byte(0xa8,(uint8_t)(video_format->vact/256));
	HDMI_WriteI2C_Byte(0xa9,(uint8_t)(video_format->vact%256));  //vactive

   	HDMI_WriteI2C_Byte(0xaa,(uint8_t)(video_format->htotal/256));
	HDMI_WriteI2C_Byte(0xab,(uint8_t)(video_format->htotal%256));//htotal

   	HDMI_WriteI2C_Byte(0xac,(uint8_t)(video_format->vtotal/256));
	HDMI_WriteI2C_Byte(0xad,(uint8_t)(video_format->vtotal%256));//vtotal

   	HDMI_WriteI2C_Byte(0xae,(uint8_t)(video_format->hs/256)); 
	HDMI_WriteI2C_Byte(0xaf,(uint8_t)(video_format->hs%256));   //hsa

	HDMI_WriteI2C_Byte(0xb0,(uint8_t)(video_format->vs%256));    //vsa

       //dessc pll to generate pixel clk
	HDMI_WriteI2C_Byte(0xff,0x82); //dessc pll
	HDMI_WriteI2C_Byte(0x2d,0x48); //pll ref select xtal 

	if(pclk_khz < 44000)
	{
	  	HDMI_WriteI2C_Byte(0x35,0x83);
		dessc_pll_post_div = 16;
	}

	else if(pclk_khz < 88000)
	{
	  	HDMI_WriteI2C_Byte(0x35,0x82);
		dessc_pll_post_div = 8;
	}

	else if(pclk_khz < 176000)
	{
	  	HDMI_WriteI2C_Byte(0x35,0x81);
		dessc_pll_post_div = 4;
	}

	else if(pclk_khz < 352000)
	{
	  	HDMI_WriteI2C_Byte(0x35,0x80);
		dessc_pll_post_div = 0;
	}

	pcr_m = (pclk_khz * dessc_pll_post_div) /25;
	pcr_k = pcr_m%1000;
	pcr_m = pcr_m/1000;

	pcr_k <<= 14; 

	//pixel clk
 	HDMI_WriteI2C_Byte(0xff,0xd0); //pcr
	HDMI_WriteI2C_Byte(0x2d,0x7f);
	HDMI_WriteI2C_Byte(0x31,0x00);

	HDMI_WriteI2C_Byte(0x26,0x80|((uint8_t)pcr_m));
	HDMI_WriteI2C_Byte(0x27,(uint8_t)((pcr_k>>16)&0xff)); //K
	HDMI_WriteI2C_Byte(0x28,(uint8_t)((pcr_k>>8)&0xff)); //K
	HDMI_WriteI2C_Byte(0x29,(uint8_t)(pcr_k&0xff)); //K
}

void lt9211_lvds_tx_logic_rst(void)       
{
    HDMI_WriteI2C_Byte(0xff,0x81);	  
    HDMI_WriteI2C_Byte(0x0d,0xfb); //LVDS TX LOGIC RESET 
    //Timer0_vTaskDelay(10);
	vTaskDelay(10);
    HDMI_WriteI2C_Byte(0x0d,0xff); //LVDS TX LOGIC RESET  RELEASE
}



void lt9211_lvds_tx_en(void)       
{
    HDMI_WriteI2C_Byte(0xff,0x82);

    if(LT9211_OutPutModde == OUTPUT_LVDS_2_PORT)
    {
        HDMI_WriteI2C_Byte(0x3b,0xb8);//dual-port lvds output Enable
    }
    else
    {
        HDMI_WriteI2C_Byte(0x3b,0x38);//signal-port lvds output Enable
    }
}

void LT9211_Patten_debug_M2LVDS(void)
{ 
 #if VIDEO_PATTERN
	debug_printf_string("\r\n LT9211_Patten_debug");
	LT9211_ChipID();
	LT9211_SystemInt();
  
 	vTaskDelay(100);

	vTaskDelay(10);
	LT9211_Txpll();
	LT9211_TxPhy();	
	LT9211_TxDigital();
	LT9211_Pattern(&video_1024x600_60Hz);   //video_1920x1080_60Hz   video_1024x600_60Hz
	
	lt9211_lvds_tx_logic_rst();    //LVDS TX LOGIC RESET 
	lt9211_lvds_tx_en();           //LVDS TX output enable 

	LT9211_ClockCheckDebug();
	LT9211_VideoCheckDebug();

	while(1);
 #endif
}


static void LT9211_Init(void)
{ 
	debug_printf_string("\r\n LT9211_mipi to TTL\r\n");	
	LT9211_ChipID();
	LT9211_SystemInt();
	vTaskDelay(10);
	LT9211_MipiRxPhy();
	LT9211_MipiRxDigital(); 
	LT9211_TimingSet();
	LT9211_MipiRxPll();
	vTaskDelay(10);
	LT9211_MipiPcr();
	//Timer0_vTaskDelay(10);
	vTaskDelay(10);
	LT9211_Txpll();
	LT9211_TxPhy();	
	LT9211_TxDigital();
	vTaskDelay(10);
	lt9211_lvds_tx_logic_rst();    //LVDS TX LOGIC RESET 
	lt9211_lvds_tx_en();           //LVDS TX output enable 
	vTaskDelay(10);
	LT9211_ClockCheckDebug();
	LT9211_VideoCheckDebug();

}


//通过9211控制lcd复位一次
void LT9211_Config(void)
{ 
//	if(cmd_init_9211)
	{
		key_light_allleds_control(SET);
		
		lcd_reset_control();   //lcd的复位引脚控制PD8 gpios.c
	
		LT9211_Reset();
#if VIDEO_PATTERN
  
 	LT9211_Patten_debug_M2LVDS();
#else
	
		LT9211_Init();
#endif
//		cmd_init_9211 = 0;
		
		key_light_allleds_control(RESET);  //关闭所有的键灯

		//这个用于点亮屏幕，尽量晚一点时间2022-09-16
		OePins_Output_Hight(3);   //屏幕点亮 通过OE3控制的cpu发出的pwm
	}
}


//创建后，删除自己
void LT9211_Once_Task(void* arg)
{	
	LT9211_Mcu_ControlPort_Init();
	vTaskDelay(10);
	LT9211_Config();
	//while(1)
	{
		vTaskDelay(1000);
	}
	vTaskDelete(NULL);  //删除自己
}


/**********************END OF FILE******************/

