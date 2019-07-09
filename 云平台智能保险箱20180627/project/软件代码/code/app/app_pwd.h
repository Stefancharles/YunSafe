
/******************************************************************************
* @file    app/app_pwd.h 
* @author  zhao
* @version V1.0.0
* @date    2018.06.18
* @brief   密码eeprom管理 头文件
******************************************************************************/
#ifndef __APP_PWD_H__
#define __APP_PWD_H__
#include "bsp_flash.h"
#include <stdio.h>
#include <stdlib.h>

//MEMSDataTypedef 参数
#define ADC_CHANNEL_NUM		5
typedef struct
{
	unsigned short theshold;
	unsigned short static_ADCValue[ADC_CHANNEL_NUM];
}MEMSDataTypedef;
extern enum 
{
	MENU_IDLE,
	MENU_ADD_DIGI_PWD,
	MENU_ID_CARD_PWD,
	MENU_MEMS,
	MENU_ALERT_CLEAR
}MENU_STATUS;

extern enum
{
	PWD_ERROR,
	PWD_SUCCESS,
}PWD_STATUS;

extern enum
{
	EEP_SAVE_FLAG = 0,
	EEP_DIGIT_PWD_OFFSET = 7,
	EEP_DIGIT_PWD_START = 8,
	EEP_ID_CARD_PWD_OFFSET = 71,
	EEP_ID_CARD_PWD_START = 72,
	EEP_MEMS_START = 112,
}EEP_MAP;

/**
 * EEPROM start address in flash
 * As for STM32F103VE (512KB flash), 0x0807F000 is the begining of the last 2 pages.
 * Check the *.map file (in listing folder) to find out the used ROM size.
 */
#define EEP_SAVE_FLAG_ADDRESS    ((uint32_t)0x0807D800)
#define EEP_DIGIT_PWD_OFFSET_ADDRESS   ((uint32_t)0x0807E000)
#define EEP_DIGIT_PWD_START_ADDRESS   ((uint32_t)0x0807E800)
#define EEP_ID_CARD_PWD_OFFSET_ADDRESS    ((uint32_t)0x0807F000)
#define EEP_ID_CARD_PWD_START_ADDRESS    ((uint32_t)0x0807F800)
#define EEP_MEMS_START_ADDRESS    ((uint32_t)0x0807D000)



//digit password
#define MAX_DIGIT_PWD_NUM 5//可存储的密码个数
#define DIGIT_PWD_LEN 6//数字密码长度
extern unsigned char digitPwd[MAX_DIGIT_PWD_NUM][DIGIT_PWD_LEN];//数字密码缓存
extern unsigned char digitPwd_offset;//密码保存在缓存中的位置
extern unsigned char curDigitPwd[DIGIT_PWD_LEN];//当前输入的密码
extern unsigned char curDigitPwd_offset;//当前输入密码到第几位

//LF id card password
#define MAX_ID_CARD_PWD_NUM 5//可存储的IDcard密码个数
#define ID_CARD_PWD_LEN 8//id card密码长度
extern unsigned char idCardPwd[MAX_ID_CARD_PWD_NUM][ID_CARD_PWD_LEN];//id card密码缓存
extern unsigned char idCardPwd_offset;//密码保存在缓存中的位置
extern unsigned char curIdCardPwd[ID_CARD_PWD_LEN];//当前输入的密码


extern MEMSDataTypedef memsData;

void param_Init(void);
int save_DigitPwd(unsigned char pwd[],int len);
unsigned char **read_DigitPwd(void);
int save_IdCardPwd(unsigned char pwd[],int len);
int delete_IdCardPwd(unsigned char pwd[],int len);
unsigned char **read_IdCardPwd(void);
void save_MEMSData(void);
void read_MEMSData(void);





#endif //__APP_PWD_H__

