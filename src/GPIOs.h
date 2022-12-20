

#ifndef __GPIOS_H__
#define __GPIOS_H__

#include <gd32f10x.h>

//extern TaskHandle_t  TaskHandle_Morseptt;   //���morseptt��������ָ��


//gpio ���ų�ʼ��
void Gpios_init(void);


//void Wxen_Control_Init(void);
void Wxen_Control_Enable(void);
//��ֹ�����ţ��ϵ�1.1v
void Wxen_Control_Disable(void);


//void OePins_Control_Init(void);
//which 1-4 �ֱ��ʾoe1-oe4
void OePins_Output_Hight(uint8_t which);
//which 1-4 �ֱ��ʾoe1-oe4
void OePins_Output_Low(uint8_t which);


//PE15����ʼ���������
//void LcdCtrl_Control_Init(void);
//ʹ�ܸ����ţ�ͨ��Lcd��Դ
void LcdCtrl_Enable(void);
//��ֹ�����ţ��ϵ�Lcd��Դ
void LcdCtrl_Disable(void);



//PD8  lcd-reset ,��֪���Ǹߵ�ƽ��λ���ǵ͵�ƽ��λ
//�ȼ���͵�ƽ��λ�ɡ�
//void lcd_reset_control_init(void);
//����һ��lcd�ĸ�λ�ź�
void lcd_reset_control(void);



//PD6  MicCtl �������(����status ��0����ߣ�0�����)
void MicCtl_Control_SetOutVal(uint8_t status);

//PD6  MicCtl
//void MicCtl_Control_Init(void);
//���lcd��Ļ�����ͣ�2022-09-21֮�������ĵװ壬ͨ���밴�����������������
//��ȡ���ŵĵ�ƽ���ж�lcd�����ͣ�֮ǰ����3399�������жϵġ�
//����ֵ0��ʾ5��������0��ʾ7����
//2022-09-21 Ŀǰ��û���µĵװ������жϣ�Ĭ�Ϸ���0����ʾ5������
uint8_t Get_Lcd_Type(void);
//��ȡ��������
/*����ֵ
1�������ն˼�������
2���ڹҢ����ն˼������ͣ������ģ�����
4��Ƕ��ʽ��/��/���͡��������͡��ڹҢ����ն˼�������
6���๦�����ն˼�������(ֻ�������7��������������5��)
*/
uint8_t get_LcdType_val(void);

//PA7  LSPK_CRL
void LSPK_Control_Init(void);

void LSPK_Enable(void);
void LSPK_Disable(void);

////PA7  LSPK �������(����status ��0����ߣ�0�����)
void LSPK_Control_SetOutVal(uint8_t status);
//��ת
void LSPK_Control_ToggleOut(void);


//PC6  V12_CTL
void V12_CTL_Control_Init(void);

void V12_CTL_Enable(void);
void V12_CTL_Disable(void);

////PC6  V12_CTL �������(����status ��0����ߣ�0�����)
void V12_CTL_Control_SetOutVal(uint8_t status);
//��ת
void V12_CTL_Control_ToggleOut(void);



//PC8,9,10,��������
void LcdType_Control_Init(void);


//PC5  5�米��ʹ�ܣ�
//2022-12-13 ����
void SHTDB_5IN_Control_Init(void);
////PC5  SHTDB_5IN �������(����status ��0����ߵ�ƽ����5inch��0�����Ϩ�� 5inch lcd)
void SHTDB_5IN_Control_SetOutVal(uint8_t status);
//��ת
void SHTDB_5IN_Control_ToggleOut(void);
//�ߵ�ƽ���� 5inch lcd
void SHTDB_5IN_Enable(void);
//�͵�ƽϨ�� 5inch lcd
void SHTDB_5IN_Disable(void);


//PC11  ������ʹ��
//2022-12-13 ����
void SPKEN_Control_Init(void);
////PC11  ������ʹ��
void SPKEN_Control_SetOutVal(uint8_t status);
//��ת
void SPKEN_Control_ToggleOut(void);

//2022-12-13 ����
void EAR_L_EN_Control_Init(void);
////PC12  ������ʹ��
void EAR_L_EN_Control_SetOutVal(uint8_t status);
//��ת
void EAR_L_EN_Control_ToggleOut(void);

//2022-12-13 ����
void EAR_R_EN_Control_Init(void);
////PC13  ������ʹ��
void EAR_R_EN_Control_SetOutVal(uint8_t status);
//��ת
void EAR_R_EN_Control_ToggleOut(void);

#endif

