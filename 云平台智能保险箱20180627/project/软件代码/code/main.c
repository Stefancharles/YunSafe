/* ==================================================================
#     FileName: main.c
#      History:
================================================================== */
/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/* Library includes. */
#include "stm32f10x.h"
#include "stm32f10x_it.h"
/* user files */
#include "My_InitTask.h"
#include "systick.h"
#include "LCD12864.h"
#include "keyboard_drv.h"
#include "pwm_output.h"
#include "adc.h"
#include "lf125k.h"
#include "eeprom.h"
#include "rtc.h"
#include "ultrasonic.h"
#include "segled_16bit.h"
#include "buzzer.h"
#include "ir_alert.h"
#include "door.h"
#include "hal_uart4.h"
#include "bsp_flash.h"
#include "app_pwd.h"
#include "WiFiToCloud.h"

#define __DEBUG__  
#ifdef __DEBUG__  
#define DEBUG(format,...) printf("File: "__FILE__", Line: %05d: "format"\r\n", __LINE__, ##__VA_ARGS__)  
#else  
#define DEBUG(format,...)  
#endif  

#define	LCD_PRINT(row, colum, buf, fmt, args...) do{\
	memset(buf, 0, sizeof(buf));\
	sprintf(buf, fmt,##args);\
	lcd_clr_row(row);\
	lcd_write(row, colum, buf, strlen(buf));}while(0)



#if 0
	//MEMSDataTypedef 鍙傛暟
#define ADC_CHANNEL_NUM		5
typedef struct
{
	unsigned short theshold;
	unsigned short static_ADCValue[ADC_CHANNEL_NUM];
}MEMSDataTypedef;
enum 
{
	MENU_IDLE,
	MENU_ADD_DIGI_PWD,
	MENU_ID_CARD_PWD,
	MENU_MEMS,
	MENU_ALERT_CLEAR
}MENU_STATUS;

enum
{
	PWD_ERROR,
	PWD_SUCCESS,
}PWD_STATUS;

enum
{
	EEP_SAVE_FLAG = 0,
	EEP_DIGIT_PWD_OFFSET = 7,
	EEP_DIGIT_PWD_START = 8,
	EEP_ID_CARD_PWD_OFFSET = 71,
	EEP_ID_CARD_PWD_START = 72,
	EEP_MEMS_START = 112,
}EEP_MAP;
#endif
extern uint8_t MEMS_ALERT;
extern uint16_t mems_theshold_val;
extern  unsigned char time_show[10];
uint32_t rtc_second = 0, is_rtc_update = 0;
uint32_t ultrasonic_dist = 0, is_ultrasonic_update = 0;
uint8_t adc_nofify_rank = 0;

uint32_t TimeCount=0;//系统时基

char menuStatus = MENU_IDLE;
//digit password
//#define MAX_DIGIT_PWD_NUM 5//鍙瓨鍌ㄧ殑瀵嗙爜涓暟
//#define DIGIT_PWD_LEN 6//鏁板瓧瀵嗙爜闀垮害
//unsigned char digitPwd[MAX_DIGIT_PWD_NUM][DIGIT_PWD_LEN];//鏁板瓧瀵嗙爜缂撳瓨
//unsigned char digitPwd_offset = 0;//瀵嗙爜淇濆瓨鍦ㄧ紦瀛樹腑鐨勪綅缃�
//unsigned char curDigitPwd[DIGIT_PWD_LEN];//褰撳墠杈撳叆鐨勫瘑鐮�
//unsigned char curDigitPwd_offset = 0;//褰撳墠杈撳叆瀵嗙爜鍒扮鍑犱綅
//LF id card password
//#define MAX_ID_CARD_PWD_NUM 5//鍙瓨鍌ㄧ殑IDcard瀵嗙爜涓暟
//#define ID_CARD_PWD_LEN 8//id card瀵嗙爜闀垮害
//unsigned char idCardPwd[MAX_ID_CARD_PWD_NUM][ID_CARD_PWD_LEN];//id card瀵嗙爜缂撳瓨
//unsigned char idCardPwd_offset = 0;//瀵嗙爜淇濆瓨鍦ㄧ紦瀛樹腑鐨勪綅缃�
//unsigned char curIdCardPwd[ID_CARD_PWD_LEN];//褰撳墠杈撳叆鐨勫瘑鐮�

//MEMSDataTypedef memsData = 
//{
//	.static_ADCValue[0] = 1000,
//	.static_ADCValue[1] = 2000,
//	.static_ADCValue[2] = 1000,
//};
unsigned char lcdRefresh = 1;
unsigned char isAlert = 0;//报警标志

#if 0
void clearPwdCache(void)
{
	curDigitPwd_offset = 0;
	memset(curDigitPwd, 0, DIGIT_PWD_LEN);
	memset(curIdCardPwd, 0, ID_CARD_PWD_LEN);
}
int isRightPwd_Digit(unsigned char pwd[],int len)
{
	int i;
	if(len != DIGIT_PWD_LEN)
	{
		return PWD_ERROR;
	}
	for(i=0; i < MAX_DIGIT_PWD_NUM; i++)
	{
		if(memcmp(pwd, digitPwd[i], DIGIT_PWD_LEN) == 0)
		{
			break;
		}
	}
	if(i < MAX_DIGIT_PWD_NUM)
	{
		return PWD_SUCCESS;
	}
	else
	{
		return PWD_ERROR;
	}
}


int isRightPwd_IdCard(unsigned char pwd[],int len)
{
	int i;
	if(len != ID_CARD_PWD_LEN)
		return PWD_ERROR;
	for(i=0; i < MAX_ID_CARD_PWD_NUM; i++)
	{
		if(memcmp(pwd, idCardPwd[i], ID_CARD_PWD_LEN) == 0)
			break;
	}
	if(i < MAX_ID_CARD_PWD_NUM)
	{	
		return PWD_SUCCESS;
	}
	else
	{
		return PWD_ERROR;
	}
}

/*
* @brief  paramInit
* @param  none
* @note  1.鍙傛暟鍒濆鍖�
       2.鏄惁鍒濇涓婄數  绗竴娆′笂鐢� 閲嶇疆瀵嗙爜 
* @Date:2018.6.19
* @author:zhao
* @return:none
*/
void paramInit(void)
{
	unsigned char save_flag;
	unsigned short NumByteToRead = 1;
	unsigned short NumByteToWrite;
	int i;
/*
  * @note  1.鍙傛暟鍒濆鍖�
           2.sEE_ReadBuffer  鑾峰彇   鍒濆鍖栧彧 濡傛灉鏄涓€娆′笂鐢� 0 byte 鍐欏叆 0xA3
           3.
  * @Date:2018.6.19
  * @author:zhao
*/
	sEE_ReadBuffer(&save_flag, EEP_SAVE_FLAG, &NumByteToRead);
	DEBUG("%x", save_flag);
	if(save_flag != 0xA3)
	{
		save_flag = 0xA3;
		NumByteToWrite = 1;
		sEE_WriteBuffer(&save_flag, EEP_SAVE_FLAG, NumByteToWrite);
		//digit pwd
		for(i=0; i < MAX_DIGIT_PWD_NUM; i++)
		{
			if(i == 0)
			{
				memset(digitPwd[i], 0, DIGIT_PWD_LEN);//default pwd 000000
			}
			else
			{
				memset(digitPwd[i], 0xEE, DIGIT_PWD_LEN);
			}
		}
		NumByteToWrite = 1;
		sEE_WriteBuffer(&digitPwd_offset, EEP_DIGIT_PWD_OFFSET, NumByteToWrite);
		NumByteToWrite = MAX_DIGIT_PWD_NUM * DIGIT_PWD_LEN;
		sEE_WriteBuffer((unsigned char *)digitPwd[0], EEP_DIGIT_PWD_START, NumByteToWrite);
		//id card pwd
		for(i=0; i < MAX_ID_CARD_PWD_NUM; i++)
		{
		  memset(idCardPwd[i], 0xEE, ID_CARD_PWD_LEN);
		}
		NumByteToWrite = 1;
		sEE_WriteBuffer(&idCardPwd_offset, EEP_ID_CARD_PWD_OFFSET, NumByteToWrite);
		NumByteToWrite = MAX_ID_CARD_PWD_NUM * ID_CARD_PWD_LEN;
		sEE_WriteBuffer((unsigned char *)idCardPwd[0], EEP_ID_CARD_PWD_START, NumByteToWrite);
		//mems
		memsData.theshold = 50;
		//memset(&memsData.static_ADCValue[0], 0, sizeof(memsData.static_ADCValue));
		NumByteToWrite = sizeof(MEMSDataTypedef);
		
		sEE_WriteBuffer((unsigned char *)&memsData, EEP_MEMS_START, NumByteToWrite);
	}
}

int saveDigitPwd(unsigned char pwd[],int len)
{	
	unsigned short data_len = 0;
	//static unsigned char pwd_offset = 0;
	if(digitPwd_offset>= MAX_DIGIT_PWD_NUM)
		digitPwd_offset = 0;	
	memset(digitPwd[digitPwd_offset], 0xff, DIGIT_PWD_LEN);
	memcpy(digitPwd[digitPwd_offset], pwd, len);
	digitPwd_offset++;
	data_len = 1;
	sEE_WriteBuffer(&digitPwd_offset, EEP_DIGIT_PWD_OFFSET, data_len);
	data_len = MAX_DIGIT_PWD_NUM * DIGIT_PWD_LEN;
	sEE_WriteBuffer((unsigned char *)&digitPwd[0][0], EEP_DIGIT_PWD_START, data_len);
	return 0;
}


/*
  * @brief  readDigitPwd 璇诲彇鏁板瓧瀵嗙爜
  * @param  none
  * @note  1.鍙傛暟鍒濆鍖�
           2.鍒嗛厤鍐呭瓨 calloc(128,sizeof(char))
           3.鑾峰彇褰撳墠缂撳瓨浣嶇疆
           4.digitPwd缂撳瓨鍐橣F
           5.灏哾igitPwd缂撳瓨鍐欏叆 debug_buff  鎵撳嵃 瀵嗙爜  strcat 涓茶仈瀛楃涓�
           6.free 閲婃斁鍐呭瓨
  * @Date:2018.6.19
  * @author:zhao
  * @return:(unsigned char **)digitPwd  鎸囬拡鍦板潃
*/
unsigned char **readDigitPwd(void)
{	
	int i,j;
	unsigned short data_len = 0;
	char *debug_buf = calloc(128, sizeof(char));
	char temp_buf[8];	
	data_len = 1;
	sEE_ReadBuffer(&digitPwd_offset, EEP_DIGIT_PWD_OFFSET, &data_len);
	
	memset(digitPwd[0], 0xff, MAX_DIGIT_PWD_NUM * DIGIT_PWD_LEN);
	data_len = MAX_DIGIT_PWD_NUM * DIGIT_PWD_LEN;
	sEE_ReadBuffer(digitPwd[0], EEP_DIGIT_PWD_START, &data_len);


	for(i=0; i < MAX_DIGIT_PWD_NUM; i++)
	{
		for(j=0; j < DIGIT_PWD_LEN; j++)
		{
			sprintf(temp_buf, "%x ", digitPwd[i][j]);
			strcat(debug_buf, temp_buf);
		}
		strcat(debug_buf, "\r\n");
	}
	DEBUG("digitpwd: %s", debug_buf);
	delay_ms(5);
	free(debug_buf);
	
	return (unsigned char **)digitPwd;

}


int saveIdCardPwd(unsigned char pwd[],int len)
{	
	unsigned short data_len = 0;
	//static unsigned char pwd_offset = 0;
	if(idCardPwd_offset>= MAX_ID_CARD_PWD_NUM)
		idCardPwd_offset = 0;	
	memset(idCardPwd[idCardPwd_offset], 0xff, ID_CARD_PWD_LEN);
	memcpy(idCardPwd[idCardPwd_offset], pwd, len);
	idCardPwd_offset++;
	data_len = 1;
	sEE_WriteBuffer(&idCardPwd_offset, EEP_ID_CARD_PWD_OFFSET, data_len);
	data_len = MAX_ID_CARD_PWD_NUM * ID_CARD_PWD_LEN;
	sEE_WriteBuffer((unsigned char *)&idCardPwd[0][0], EEP_ID_CARD_PWD_START, data_len);
	return 0;
}


int deleteIdCardPwd(unsigned char pwd[],int len)
{	
	
	int i;
	unsigned short data_len;
	if(len != ID_CARD_PWD_LEN)
		return PWD_ERROR;
	for(i=0; i < MAX_ID_CARD_PWD_NUM; i++)
	{
		if(memcmp(pwd, idCardPwd[i], ID_CARD_PWD_LEN) == 0)
		{
			memset(idCardPwd[i], 0xff, ID_CARD_PWD_LEN);
			break;
		}
	}
	if(i < MAX_ID_CARD_PWD_NUM)
	{
		data_len = MAX_ID_CARD_PWD_NUM * ID_CARD_PWD_LEN;
		sEE_WriteBuffer((unsigned char *)&idCardPwd[0][0], EEP_ID_CARD_PWD_START, data_len);
	}
	return 0;
}

unsigned char **readIdCardPwd(void)
{	
	unsigned short data_len = 0;
	data_len = 1;
	sEE_ReadBuffer(&idCardPwd_offset, EEP_ID_CARD_PWD_OFFSET, &data_len);
	
	memset(idCardPwd[0], 0xff, MAX_ID_CARD_PWD_NUM * ID_CARD_PWD_LEN);
	data_len = MAX_ID_CARD_PWD_NUM * ID_CARD_PWD_LEN;
	sEE_ReadBuffer(idCardPwd[0], EEP_ID_CARD_PWD_START, &data_len);	
	return (unsigned char **)idCardPwd;

}

void saveMEMSData(void)
{
	unsigned short NumByteToWrite;
	NumByteToWrite = sizeof(MEMSDataTypedef);
	sEE_WriteBuffer((unsigned char *)&memsData, EEP_MEMS_START, NumByteToWrite);
	
	DEBUG("MEMS theshold %d, ADC Val:%d %d %d",memsData.theshold, memsData.static_ADCValue[0], memsData.static_ADCValue[1], memsData.static_ADCValue[2]);
}
void readMEMSData(void)
{
	unsigned short data_len;
	data_len = sizeof(MEMSDataTypedef);
	sEE_ReadBuffer((unsigned char *)&memsData, EEP_MEMS_START, &data_len);
	DEBUG("MEMS theshold %d, ADC Val:%d %d %d",memsData.theshold, memsData.static_ADCValue[0], memsData.static_ADCValue[1], memsData.static_ADCValue[2]);
}

#endif
#define LIGHT_PORT GPIOD
#define LIGHT_PIN GPIO_Pin_12

void lightInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);

	GPIO_InitStructure.GPIO_Pin = LIGHT_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(LIGHT_PORT, &GPIO_InitStructure);

}

void lightOpen(void)
{
	GPIO_WriteBit(LIGHT_PORT, LIGHT_PIN, 1);
}
void lightClose(void)
{
	GPIO_WriteBit(LIGHT_PORT, LIGHT_PIN, 0);
}

/**
* @brief 
* @param 
* @return 
*/
void menuIdle_Handle(void)
{
	KBD_EVENT key;
	int ret, i;
	char lcd_show[LCD_COLUMN_NUM+1];
//	uint8_t tryCount=0;
	uint8_t buf[8];

	//lcd
	if(lcdRefresh)
	{
		lcdRefresh = 0;
		/*0*/
		LCD_PRINT(0,0,lcd_show,"1 Input pwd:");
		/*1*/
		memset(lcd_show, 0, sizeof(lcd_show));				
		for(i=0; i < DIGIT_PWD_LEN; i++)
		{
			if(i < curDigitPwd_offset)
				strcat(lcd_show, "*");
			else
				strcat(lcd_show, "_");
		}
		lcd_clr_row(1);
		lcd_write(1, 0, lcd_show, strlen(lcd_show));
	}
	//LF
	//read data
	memset(buf, 0, sizeof(buf));
	if(isAlert == 0)
	{
//tryLF125K:
		ret = LF125K_read_1_page(buf);
		if(ret == 0)
		{
			memset(buf, 0, sizeof(buf));
			ret = LF125K_read_1_page(buf);
		}
		if(ret == 0)
		{
			DEBUG("id card:%X %X %X %X %X %X %X %X", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
			//processe data
			if(isRight_Pwd_IdCard(buf, ID_CARD_PWD_LEN))
			{
				//TODO open door
				doorOpen();
				//TODO lcd show "pwd right"
				LCD_PRINT(3,0,lcd_show,"Door open");
				delay_ms(1000);
				lcd_clr_row(3);
			}
			else
			{
				//TODO buzzer on
				//buzzerOpen();
				//lightOpen();
				//TODO lcd show "pwd error" 					
				LCD_PRINT(3,0,lcd_show,"Id card error");
				delay_ms(500);
				lcd_clr_row(3);
//				if(tryCount++<3)
//				{
//					goto tryLF125K;
//				}
			}	
		}
	}
	if(FlagDefense)
	{
		//IR红外
		if(irAlert() && (!isDoorOpen()))
		{
			isAlert = 1;
			LCD_PRINT(2,0,lcd_show,"IR ALERT");
			//delay_ms(500);
			buzzerOpen();
			lightOpen();
		}
		//mems
		if(MEMS_ALERT >= MEMS_ALERT_TRIGER && !isDoorOpen())
		{
			MEMS_ALERT = 0;
			isAlert = 1;
			LCD_PRINT(2,0,lcd_show,"MEMS ALERT");
			//delay_ms(500);
			buzzerOpen();
			lightOpen();
		}
	}
	
	//keybord
	ret = get_kbd_fifo(&key);
	if(ret == 0)
	{
		lcdRefresh = 1;
		switch(key.key_no)
		{
			case KBD_KEY0:
			case KBD_KEY1:
			case KBD_KEY2:
			case KBD_KEY3:
			case KBD_KEY4:
			case KBD_KEY5:
			case KBD_KEY6:
			case KBD_KEY7:
			case KBD_KEY8:
			case KBD_KEY9:
				if(key.key_event == KBD_KEY_DONW)
				{
					if(curDigitPwd_offset < DIGIT_PWD_LEN)
						curDigitPwd[curDigitPwd_offset++] = key.key_no;
				}
				break;
			case KBD_UP:
				if(key.key_event == KBD_KEY_DONW || key.key_event == KBD_KEY_HOLD)
				{
					DEBUG("MEMS theshold %d, ADC Val:%d %d %d, ALERT:%d",
						memsData.theshold, 
						get_ADC1_result(ADC1_CH10_RANK1), 
						get_ADC1_result(ADC1_CH11_RANK2), 
						get_ADC1_result(ADC1_CH12_RANK3),
						MEMS_ALERT);
				}
				break;
			case KBD_DOWN:
				break;
			case KBD_LEFT:
				break;
			case KBD_RIGHT:
				break;
			case KBD_PROJ1:
				if(key.key_event == KBD_KEY_DONW)
				{
					//menuStatus = MENU_IDLE;
					//clearPwdCache();
					//lcd_clr();
				}
				break;
			case KBD_PROJ2:
				if(key.key_event == KBD_KEY_DONW)
				{
					menuStatus = MENU_ADD_DIGI_PWD;
					clear_PwdCache();
					lcd_clr();
				}
				break;
			case KBD_PROJ3:
				if(key.key_event == KBD_KEY_DONW)
				{
					menuStatus = MENU_ID_CARD_PWD;
					clear_PwdCache();
					lcd_clr();
				}
				break;
			case KBD_PROJ4:
				if(key.key_event == KBD_KEY_DONW)
				{
					menuStatus = MENU_MEMS;
					clear_PwdCache();
					lcd_clr();
				}
				break;
			case KBD_PROJ5:
				if(key.key_event == KBD_KEY_DONW)
				{
					menuStatus = MENU_ALERT_CLEAR;
					clear_PwdCache();
					lcd_clr();
				}

				break;
			case KBD_FUNC:
				break;
			case KEY_HASH_KEY:
				break;
			case KEY_KPASTERISK:
				break;
			case KEY_BACKSPACE:
				if(key.key_event == KBD_KEY_DONW || key.key_event == KBD_KEY_HOLD)
				{
					if(curDigitPwd_offset > 0)
						curDigitPwd_offset--;
				}
				break;
			case KEY_ENTER:
				if(key.key_event == KBD_KEY_DONW)
				{
					if(isRight_Pwd_Digit(curDigitPwd, curDigitPwd_offset))
					{
						doorOpen();
						LCD_PRINT(3,0,lcd_show,"Door open");
						delay_ms(1000);
						lcd_clr_row(3);
					}
					else
					{
						LCD_PRINT(3,0,lcd_show,"Password error");
						delay_ms(1000);
						lcd_clr_row(3);
					}
					clear_PwdCache();
				}
				break;
			case KEY_ESC:
				curDigitPwd_offset = 0;
				lcd_clr();
				break;
			case KEY_INVALID:
				break;
	
			default:break;
		}
	}

}
/**
* @brief 
* @param 
* @return 
*/
void menuAddDigitPwd_Handle(void)
{
	KBD_EVENT key;
	int ret, i;
	char lcd_show[LCD_COLUMN_NUM+1];
	uint8_t buf[8];
	static uint8_t isAlreadyOldPwd = 0;
	if(lcdRefresh)
	{
		lcdRefresh = 0;
		//lcd
		/*0*/
		if(isAlreadyOldPwd)
		{
			LCD_PRINT(0,0,lcd_show,"Input new pwd:");
		}
		else
		{
			LCD_PRINT(0,0,lcd_show,"2 Input old pwd:");
		}
		/*1*/
		memset(lcd_show, 0, sizeof(lcd_show));
		for(i=0; i < DIGIT_PWD_LEN; i++)
		{
			if(i < curDigitPwd_offset)
				strcat(lcd_show, "*");
			else
				strcat(lcd_show, "_");
		}
		lcd_clr_row(1);
		lcd_write(1, 0, lcd_show, strlen(lcd_show));
	}
	if(FlagDefense)
	{
		//IR红外
		if(irAlert() && !isDoorOpen() && isAlert == 0)
		{
			isAlert = 1;
			LCD_PRINT(2,0,lcd_show,"IR ALERT");
			//delay_ms(500);
			buzzerOpen();
			lightOpen();
		}
		//mems
		if(MEMS_ALERT >= MEMS_ALERT_TRIGER && !isDoorOpen() && isAlert == 0)
		{
			MEMS_ALERT = 0;
			isAlert = 1;
			LCD_PRINT(2,0,lcd_show,"MEMS ALERT");
			//delay_ms(500);
			buzzerOpen();
			lightOpen();
		}
	}
	//keybord
	ret = get_kbd_fifo(&key);
	if(ret == 0)
	{
		lcdRefresh = 1;
		switch(key.key_no)
		{
			case KBD_KEY0:
			case KBD_KEY1:
			case KBD_KEY2:
			case KBD_KEY3:
			case KBD_KEY4:
			case KBD_KEY5:
			case KBD_KEY6:
			case KBD_KEY7:
			case KBD_KEY8:
			case KBD_KEY9:
				if(key.key_event == KBD_KEY_DONW)
				{
					if(curDigitPwd_offset < DIGIT_PWD_LEN)
						curDigitPwd[curDigitPwd_offset++] = key.key_no;
				}
				break;
			case KBD_UP:
				break;
			case KBD_DOWN:
				break;
			case KBD_LEFT:
				break;
			case KBD_RIGHT:
				break;
			case KBD_PROJ1:
				if(key.key_event == KBD_KEY_DONW)
				{
					menuStatus = MENU_IDLE;
					isAlreadyOldPwd = 0;
					clear_PwdCache();
					lcd_clr();
				}
				break;
			case KBD_PROJ2:
				if(key.key_event == KBD_KEY_DONW)
				{
					//menuStatus = MENU_ADD_DIGI_PWD;
					//isAlreadyOldPwd = 0;
					//clearPwdCache();
				}
				break;
			case KBD_PROJ3:
				if(key.key_event == KBD_KEY_DONW)
				{
					menuStatus = MENU_ID_CARD_PWD;
					isAlreadyOldPwd = 0;
					clear_PwdCache();
					lcd_clr();
				}
				break;
			case KBD_PROJ4:
				if(key.key_event == KBD_KEY_DONW)
				{
					menuStatus = MENU_MEMS;
					isAlreadyOldPwd = 0;
					clear_PwdCache();
					lcd_clr();
				}
				break;
			case KBD_PROJ5:
				if(key.key_event == KBD_KEY_DONW)
				{
					menuStatus = MENU_ALERT_CLEAR;
					isAlreadyOldPwd = 0;
					clear_PwdCache();
					lcd_clr();
				}

				break;
			case KBD_FUNC:	
				break;
			case KEY_HASH_KEY:
				break;
			case KEY_KPASTERISK:
				break;
			case KEY_BACKSPACE:
				if(key.key_event == KBD_KEY_DONW || key.key_event == KBD_KEY_HOLD)
				{
					if(curDigitPwd_offset > 0)
						curDigitPwd_offset--;
				}
				break;
			case KEY_ENTER:
				if(key.key_event == KBD_KEY_DONW)
				{
					if(isAlreadyOldPwd){//已经输入旧密码
						if(curDigitPwd_offset == DIGIT_PWD_LEN)
						{
							save_DigitPwd(curDigitPwd, DIGIT_PWD_LEN);
							LCD_PRINT(3,0,lcd_show,"Save pwd");
							delay_ms(1000);
							lcd_clr_row(3);
						}
					}
					else
					{
						if(isRight_Pwd_Digit(curDigitPwd, curDigitPwd_offset))
						{
							isAlreadyOldPwd = 1;
						}
						else
						{
							isAlreadyOldPwd = 0;
							LCD_PRINT(3,0,lcd_show,"Password error");
							delay_ms(1000);
							lcd_clr_row(3);
						}
					}
					clear_PwdCache();
				}
				break;
			case KEY_ESC:
				if(key.key_event == KBD_KEY_DONW)
				{
					menuStatus = MENU_IDLE;
					isAlreadyOldPwd = 0;
					clear_PwdCache();
					lcd_clr();
				}
				break;
			case KEY_INVALID:
				break;
	
			default:break;
		}
	}

}
/**
* @brief 
* @param 
* @return 
*/
void menuIdCardPwd_Handle(void)
{
	KBD_EVENT key;
	int ret, i;
	char lcd_show[LCD_COLUMN_NUM+1];
	uint8_t buf[8];
	static uint8_t isAlreadyOldPwd = 0;
	if(lcdRefresh)
	{
		lcdRefresh = 0;
		//lcd
		if(isAlreadyOldPwd)
		{
			LCD_PRINT(0,0,lcd_show,"Id card");
			LCD_PRINT(1,0,lcd_show,"Add or delete");
		}
		else
		{
			LCD_PRINT(0,0,lcd_show,"3 Input pwd:");
			memset(lcd_show, 0, sizeof(lcd_show));
			for(i=0; i < DIGIT_PWD_LEN; i++)
			{
				if(i < curDigitPwd_offset)
					strcat(lcd_show, "*");
				else
					strcat(lcd_show, "_");
			}
			lcd_clr_row(1);
			lcd_write(1, 0, lcd_show, strlen(lcd_show));
		}
	}
	if(FlagDefense)
	{	
		//IR红外
		if(irAlert() && !isDoorOpen() && isAlert == 0)
		{
			isAlert = 1;
			LCD_PRINT(2,0,lcd_show,"IR ALERT");
			//delay_ms(500);
			buzzerOpen();
			lightOpen();
		}
		//mems
		if(MEMS_ALERT >= MEMS_ALERT_TRIGER && !isDoorOpen() && isAlert == 0)
		{
			MEMS_ALERT = 0;
			isAlert = 1;
			LCD_PRINT(2,0,lcd_show,"MEMS ALERT");
			//delay_ms(500);
			buzzerOpen();
			lightOpen();
		}
	}
	//keybord
	ret = get_kbd_fifo(&key);
	if(ret == 0)
	{
		lcdRefresh = 1;
		switch(key.key_no)
		{
			case KBD_KEY0:
			case KBD_KEY1:
			case KBD_KEY2:
			case KBD_KEY3:
			case KBD_KEY4:
			case KBD_KEY5:
			case KBD_KEY6:
			case KBD_KEY7:
			case KBD_KEY8:
			case KBD_KEY9:
				if(key.key_event == KBD_KEY_DONW)
				{
					if(curDigitPwd_offset < DIGIT_PWD_LEN)
						curDigitPwd[curDigitPwd_offset++] = key.key_no;
				}
				break;
			case KBD_UP:
				break;
			case KBD_DOWN:
				if(key.key_event == KBD_KEY_DONW)
				{
					if(isAlreadyOldPwd)
					{
						//read data
						memset(buf, 0, sizeof(buf));
						ret = LF125K_read_1_page(buf);
						if(ret == 0)
						{
							DEBUG("id card:%X %X %X %X %X %X %X %X", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
							delete_IdCardPwd(buf,ID_CARD_PWD_LEN);
							LCD_PRINT(3,0,lcd_show,"Delete ok");
							delay_ms(1000);
							lcd_clr_row(3); 
						}
						else
						{
							LCD_PRINT(3,0,lcd_show,"Delete false");
							delay_ms(1000);
							lcd_clr_row(3); 
						}
					}
					else
					{
						if(isRight_Pwd_Digit(curDigitPwd, curDigitPwd_offset))
						{
							isAlreadyOldPwd = 1;
						}
						else
						{
							isAlreadyOldPwd = 0;
							LCD_PRINT(3,0,lcd_show,"Password error");
							delay_ms(1000);
							lcd_clr_row(3);
						}
					}
					clear_PwdCache();
				}	

				break;
			case KBD_LEFT:
				break;
			case KBD_RIGHT:
				break;
			case KBD_PROJ1:
				if(key.key_event == KBD_KEY_DONW)
				{
					menuStatus = MENU_IDLE;
					isAlreadyOldPwd = 0;
					clear_PwdCache();
					lcd_clr();
				}
				break;
			case KBD_PROJ2:
				if(key.key_event == KBD_KEY_DONW)
				{
					menuStatus = MENU_ADD_DIGI_PWD;
					isAlreadyOldPwd = 0;
					clear_PwdCache();
					lcd_clr();
				}
				break;
			case KBD_PROJ3:
				if(key.key_event == KBD_KEY_DONW)
				{
					//menuStatus = MENU_ID_CARD_PWD;
					//isAlreadyOldPwd = 0;
					//clear_PwdCache();
					//lcd_clr();
				}
				break;
			case KBD_PROJ4:
				if(key.key_event == KBD_KEY_DONW)
				{
					menuStatus = MENU_MEMS;
					isAlreadyOldPwd = 0;
					clear_PwdCache();
					lcd_clr();
				}

				break;
			case KBD_PROJ5:
				if(key.key_event == KBD_KEY_DONW)
				{
					menuStatus = MENU_ALERT_CLEAR;
					isAlreadyOldPwd = 0;
					clear_PwdCache();
					lcd_clr();
				}		
				break;
			case KBD_FUNC:
				break;
			case KEY_HASH_KEY:
				break;
			case KEY_KPASTERISK:
				break;
			case KEY_BACKSPACE:
				if(key.key_event == KBD_KEY_DONW || key.key_event == KBD_KEY_HOLD)
				{
					if(curDigitPwd_offset > 0)
						curDigitPwd_offset--;
				}
				break;
			case KEY_ENTER:
				if(key.key_event == KBD_KEY_DONW)
				{
					if(isAlreadyOldPwd)
					{
						//read data
						memset(buf, 0, sizeof(buf));
						ret = LF125K_read_1_page(buf);
						if(ret == 0)
						{
							DEBUG("id card:%X %X %X %X %X %X %X %X", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
							save_IdCardPwd(buf,ID_CARD_PWD_LEN);
							LCD_PRINT(3,0,lcd_show,"Registered ok");
							delay_ms(1000);
							lcd_clr_row(3); 
						}
						else
						{
							LCD_PRINT(3,0,lcd_show,"Registered false");
							delay_ms(1000);
							lcd_clr_row(3); 
						}
					}
					else
					{
						if(isRight_Pwd_Digit(curDigitPwd, curDigitPwd_offset))
						{
							isAlreadyOldPwd = 1;
						}
						else
						{
							isAlreadyOldPwd = 0;
							LCD_PRINT(3,0,lcd_show,"Password error");
							delay_ms(1000);
							lcd_clr_row(3);
						}
					}
					clear_PwdCache();
				}
				break;
			case KEY_ESC:
				if(key.key_event == KBD_KEY_DONW)
				{
					menuStatus = MENU_IDLE;
					isAlreadyOldPwd = 0;
					clear_PwdCache();
					lcd_clr();
				}
				break;
			case KEY_INVALID:
				break;
	
			default:break;
		}
	}

}
/**
* @brief 
* @param 
* @return 
*/
void menuMEMS_Handle(void)
{
	KBD_EVENT key;
	int ret, i;
	char lcd_show[LCD_COLUMN_NUM+1];
	uint8_t buf[8];
	uint16_t val=0;
	static uint8_t isAlreadyOldPwd = 0;
	if(lcdRefresh)
	{
		lcdRefresh = 0;
		//lcd
		if(isAlreadyOldPwd)
		{
			LCD_PRINT(0,0,lcd_show,"MEMS hold val");
			for(i=0; i < curDigitPwd_offset; i++)
				lcd_show[i] = curDigitPwd[i] + '0'; 
			lcd_show[i] = '\0';
			lcd_clr_row(1);
			lcd_write(1, 0, lcd_show, strlen(lcd_show));
		}
		else
		{
			LCD_PRINT(0,0,lcd_show,"4 Input pwd:");
			memset(lcd_show, 0, sizeof(lcd_show));
			for(i=0; i < DIGIT_PWD_LEN; i++)
			{
				if(i < curDigitPwd_offset)
					strcat(lcd_show, "*");
				else
					strcat(lcd_show, "_");
			}
			lcd_clr_row(1);
			lcd_write(1, 0, lcd_show, strlen(lcd_show));
		}
	}
	if(FlagDefense)
	{
		//IR红外
		if(irAlert() && !isDoorOpen() && isAlert == 0)
		{
			isAlert = 1;
			LCD_PRINT(2,0,lcd_show,"IR ALERT");
			//delay_ms(500);
			buzzerOpen();
			lightOpen();
		}
		//mems
		if(MEMS_ALERT >= MEMS_ALERT_TRIGER && !isDoorOpen() && isAlert == 0)
		{
			MEMS_ALERT = 0;
			isAlert = 1;
			LCD_PRINT(2,0,lcd_show,"MEMS ALERT");
			//delay_ms(500);
			buzzerOpen();
			lightOpen();
		}
	}
	//keybord
	ret = get_kbd_fifo(&key);
	if(ret == 0)
	{
		lcdRefresh = 1;
		switch(key.key_no)
		{
			case KBD_KEY0:
			case KBD_KEY1:
			case KBD_KEY2:
			case KBD_KEY3:
			case KBD_KEY4:
			case KBD_KEY5:
			case KBD_KEY6:
			case KBD_KEY7:
			case KBD_KEY8:
			case KBD_KEY9:
				if(key.key_event == KBD_KEY_DONW)
				{
					if(curDigitPwd_offset < DIGIT_PWD_LEN)
						curDigitPwd[curDigitPwd_offset++] = key.key_no;
				}
				break;
			case KBD_UP:
				break;
			case KBD_DOWN:
				//set the MEMS ADC base value
				if(key.key_event == KBD_KEY_DONW && isAlreadyOldPwd)
				{
					int16_t rk1, rk2, rk3;

					rk1 = get_ADC1_result(ADC1_CH10_RANK1);
					rk2 = get_ADC1_result(ADC1_CH11_RANK2);
					rk3 = get_ADC1_result(ADC1_CH12_RANK3);
					set_MEMS_static_ADC_value(rk1, rk2, rk3);
					memsData.static_ADCValue[0] = rk1;
					memsData.static_ADCValue[1] = rk2;
					memsData.static_ADCValue[2] = rk3;
					save_MEMSData();
					MEMS_ALERT = 0;
					LCD_PRINT(3,0,lcd_show,"MEMS ADJ OK");
					delay_ms(1000);
					lcd_clr_row(3);
				}	
				break;
			case KBD_LEFT:
				break;
			case KBD_RIGHT:
				break;
			case KBD_PROJ1:
				if(key.key_event == KBD_KEY_DONW)
				{
					menuStatus = MENU_IDLE;
					isAlreadyOldPwd = 0;
					clear_PwdCache();
					lcd_clr();
				}
				break;
			case KBD_PROJ2:
				if(key.key_event == KBD_KEY_DONW)
				{
					menuStatus = MENU_ADD_DIGI_PWD;
					isAlreadyOldPwd = 0;
					clear_PwdCache();
					lcd_clr();
				}
				break;
			case KBD_PROJ3:
				if(key.key_event == KBD_KEY_DONW)
				{
					menuStatus = MENU_ID_CARD_PWD;
					isAlreadyOldPwd = 0;
					clear_PwdCache();
					lcd_clr();
				}
				break;
			case KBD_PROJ4:
				if(key.key_event == KBD_KEY_DONW)
				{
					//menuStatus = MENU_MEMS;
					//isAlreadyOldPwd = 0;
					//clear_PwdCache();
					//lcd_clr();
				}

				break;
			case KBD_PROJ5:
				if(key.key_event == KBD_KEY_DONW)
				{
					menuStatus = MENU_ALERT_CLEAR;
					isAlreadyOldPwd = 0;
					clear_PwdCache();
					lcd_clr();
				}				
				break;
			case KBD_FUNC:
				break;
			case KEY_HASH_KEY:
				break;
			case KEY_KPASTERISK:
				break;
			case KEY_BACKSPACE:
				if(key.key_event == KBD_KEY_DONW || key.key_event == KBD_KEY_HOLD)
				{
					if(curDigitPwd_offset > 0)
						curDigitPwd_offset--;
				}	
				break;
			case KEY_ENTER:
				if(key.key_event == KBD_KEY_DONW)
				{
					if(isAlreadyOldPwd)
					{
						//set MEMS theshold value
						for(i=0; i < curDigitPwd_offset; i++)
							val = val* 10 + curDigitPwd[i];
						mems_theshold_val = val;
						memsData.theshold = val;
						save_MEMSData();
						MEMS_ALERT = 0;
						LCD_PRINT(3,0,lcd_show,"New hlod:%d", val);
						delay_ms(1000);
						lcd_clr_row(3);
					}
					else
					{
						if(isRight_Pwd_Digit(curDigitPwd, curDigitPwd_offset))
						{
							isAlreadyOldPwd = 1;
						}
						else
						{
							isAlreadyOldPwd = 0;
							LCD_PRINT(3,0,lcd_show,"Password error");
							delay_ms(1000);
							lcd_clr_row(3);
						}
					}
					clear_PwdCache();
				}
				break;
			case KEY_ESC:
				if(key.key_event == KBD_KEY_DONW)
				{
					menuStatus = MENU_IDLE;
					isAlreadyOldPwd = 0;
					clear_PwdCache();
					lcd_clr();
				}
				break;
			case KEY_INVALID:
				break;
	
			default:break;
		}
	}

}
/**
* @brief 
* @param 
* @return 
*/
void menuAlertClear_Handle(void)
{
	KBD_EVENT key;
	int ret, i;
	char lcd_show[LCD_COLUMN_NUM+1];
	uint8_t buf[8];
	uint16_t val=0;
	static uint8_t isAlreadyOldPwd = 0;
	if(lcdRefresh)
	{
		lcdRefresh = 0;

		LCD_PRINT(0,0,lcd_show,"5 Input pwd:");
		memset(lcd_show, 0, sizeof(lcd_show));
		for(i=0; i < DIGIT_PWD_LEN; i++)
		{
			if(i < curDigitPwd_offset)
				strcat(lcd_show, "*");
			else
				strcat(lcd_show, "_");
		}
		lcd_clr_row(1);
		lcd_write(1, 0, lcd_show, strlen(lcd_show));

	}
	if(FlagDefense)
	{
		//IR红外
		if(irAlert() && !isDoorOpen() && isAlert == 0)
		{
			isAlert = 1;
			LCD_PRINT(2,0,lcd_show,"IR ALERT");
			//delay_ms(500);
			buzzerOpen();
			lightOpen();
		}
		//mems
		if(MEMS_ALERT >= MEMS_ALERT_TRIGER && !isDoorOpen() && isAlert == 0)
		{
			MEMS_ALERT = 0;
			isAlert = 1;
			LCD_PRINT(2,0,lcd_show,"MEMS ALERT");
			//delay_ms(500);
			buzzerOpen();
			lightOpen();
		}
	}
	//keybord
	ret = get_kbd_fifo(&key);
	if(ret == 0)
	{
		lcdRefresh = 1;
		switch(key.key_no)
		{
			case KBD_KEY0:
			case KBD_KEY1:
			case KBD_KEY2:
			case KBD_KEY3:
			case KBD_KEY4:
			case KBD_KEY5:
			case KBD_KEY6:
			case KBD_KEY7:
			case KBD_KEY8:
			case KBD_KEY9:
				if(key.key_event == KBD_KEY_DONW)
				{
					if(curDigitPwd_offset < DIGIT_PWD_LEN)
						curDigitPwd[curDigitPwd_offset++] = key.key_no;
				}
				break;
			case KBD_UP:
				break;
			case KBD_DOWN:
				break;
			case KBD_LEFT:
				break;
			case KBD_RIGHT:
				break;
			case KBD_PROJ1:
				if(key.key_event == KBD_KEY_DONW)
				{
					menuStatus = MENU_IDLE;
					isAlreadyOldPwd = 0;
					clear_PwdCache();
					lcd_clr();
				}
				break;
			case KBD_PROJ2:
				if(key.key_event == KBD_KEY_DONW)
				{
					menuStatus = MENU_ADD_DIGI_PWD;
					isAlreadyOldPwd = 0;
					clear_PwdCache();
					lcd_clr();
				}
				break;
			case KBD_PROJ3:
				if(key.key_event == KBD_KEY_DONW)
				{
					menuStatus = MENU_ID_CARD_PWD;
					isAlreadyOldPwd = 0;
					clear_PwdCache();
					lcd_clr();
				}
				break;
			case KBD_PROJ4:
				if(key.key_event == KBD_KEY_DONW)
				{
					menuStatus = MENU_MEMS;
					isAlreadyOldPwd = 0;
					clear_PwdCache();
					lcd_clr();
				}

				break;
			case KBD_PROJ5:
				if(key.key_event == KBD_KEY_DONW)
				{
					menuStatus = MENU_ALERT_CLEAR;
					isAlreadyOldPwd = 0;
					clear_PwdCache();
					lcd_clr();
				}				
				break;
			case KBD_FUNC:
				break;
			case KEY_HASH_KEY:
				break;
			case KEY_KPASTERISK:
				break;
			case KEY_BACKSPACE:
				if(key.key_event == KBD_KEY_DONW || key.key_event == KBD_KEY_HOLD)
				{
					if(curDigitPwd_offset > 0)
						curDigitPwd_offset--;
				}	
				break;
			case KEY_ENTER:
				if(key.key_event == KBD_KEY_DONW)
				{
					if(isRight_Pwd_Digit(curDigitPwd, curDigitPwd_offset))
					{
						isAlert = 0;
						MEMS_ALERT = 0;
						buzzerClose();
						lightClose();
						lcd_clr_row(2);
						LCD_PRINT(3,0,lcd_show,"Clear alert", val);
						delay_ms(1000);
						lcd_clr_row(3);
					}
					else
					{
						LCD_PRINT(3,0,lcd_show,"Password error");
						delay_ms(1000);
						lcd_clr_row(3);
					}
					clear_PwdCache();
				}
				break;
			case KEY_ESC:
				if(key.key_event == KBD_KEY_DONW)
				{
					menuStatus = MENU_IDLE;
					isAlreadyOldPwd = 0;
					clear_PwdCache();
					lcd_clr();
				}
				break;
			case KEY_INVALID:
				break;
	
			default:break;
		}
	}

}

/*******************************************************************
*函数：char *USER_GetJsonValue(char *cJson, char *Tag)
*功能：json为字符串序列，将json格式中的目标对象Tag对应的值字符串转换为数值
*输入：
		char *cJson json字符串
		char *Tag 要操作的对象标签
*输出：返回数值的字符串形式的启始地址
*特殊说明：用户可以在此基础上改造和扩展该函数，这里只是个简单的DEMO
*******************************************************************/
char *USER_GetJsonValue(char *cJson, char *Tag)
{
	char *target = NULL;
	static char temp[10];
	int8_t i=0;
	
	memset(temp, 0x00, 128);
	sprintf(temp,"\"%s\":",Tag);
	target=strstr((const char *)cJson, (const char *)temp);
	if(target == NULL)
	{
		//printf("空字符！\r\n");
		return NULL;
	}
	i=strlen((const char *)temp);
	target=target+i;
	memset(temp, 0x00, 128);
	for(i=0; i<10; i++, target++)//数值超过10个位为非法，由于2^32=4294967296
	{
		if((*target<='9')&&(*target>='0'))
		{
			temp[i]=*target;
		}
		else
		{
			break;
		}
	}
	temp[i+1] = '\0';
	//printf("数值=%s\r\n",temp);
	return (char *)temp;
}

/*******************************************************************
*函数：void USER_DataAnalysisProcess(char *RxBuf)
*功能：解析服务器数据
*输入：char *RxBuf 服务器下发数据
*输出：
*特殊说明：用户可以在此基础上改造和扩展该函数，这里只是个简单的DEMO
*******************************************************************/
void USER_DataAnalysisProcess(char *RxBuf)
{
	char *cmdid = NULL;
	uint8_t TxetBuf[128];
	if(strstr((const char *)RxBuf, (const char *)PING_REQ) != NULL)//心跳请求？
	{
		if(ESP8266_IpSend((char *)PING_RSP, strlen((const char *)PING_RSP)) < 0)//响应心跳
		{//发送失败
			printf("发送心跳包失败！\r\n");
		}
		else
		{
			printf("心跳包！\r\n");
		}
	}
	else if(strstr((const char *)RxBuf, (const char *)"\"t\":5") != NULL)//命令请求？
	{
		if(strstr((const char *)RxBuf, (const char *)"\"apitag\":\"defense\"") != NULL)//布防/撤防请求
		{
			memset(TxetBuf,0x00,128);//清空缓存
			if((strstr((const char *)RxBuf, (const char *)"\"data\":1") != NULL))//布防
			{
				FlagDefense=1;
				printf("布防！\r\n");
				;//...
				;//...
				;//...
				cmdid = USER_GetJsonValue((char *)RxBuf, (char *)"cmdid");
				sprintf((char *)TxetBuf,"{\"t\":6,\"cmdid\":%s,\"status\":0,\"data\":1}",cmdid);
				//printf("%s\r\n",TxetBuf);////////////////////////////////////////////////////////////
				if(ESP8266_IpSend((char *)TxetBuf, strlen((char *)TxetBuf)) < 0)
				{//发送失败
					printf("发送响应失败！\r\n");
				}
			}
			else if((strstr((const char *)RxBuf, (const char *)"\"data\":0") != NULL))//撤防
			{
				FlagDefense=0;
				{
					isAlert = 0;//清除警告
					MEMS_ALERT = 0;
					buzzerClose();
					lightClose();
					lcd_clr_row(2);
					lcd_clr_row(3);
				}
				printf("撤防！\r\n");
				;//...
				;//...
				;//...
				cmdid = USER_GetJsonValue((char *)RxBuf, (char *)"cmdid");
				sprintf((char *)TxetBuf,"{\"t\":6,\"cmdid\":%s,\"status\":0,\"data\":0}",cmdid);
				//printf("%s\r\n",TxetBuf);////////////////////////////////////////////////////////////
				if(ESP8266_IpSend((char *)TxetBuf, strlen((char *)TxetBuf)) < 0)
				{//发送失败
					printf("发送响应失败！\r\n");
				}
			}
		}
		else if(strstr((const char *)RxBuf, (const char *)"\"apitag\":\"ctrl\"") != NULL)//开锁/关锁请求
		{
			memset(TxetBuf,0x00,128);//清空缓存
			if((strstr((const char *)RxBuf, (const char *)"\"data\":1") != NULL))//开锁
			{
				isAlert=0;//清除警告
				doorOpen();
				printf("开锁！\r\n");
				;//...
				;//...
				;//...
				cmdid = USER_GetJsonValue((char *)RxBuf, (char *)"cmdid");
				sprintf((char *)TxetBuf,"{\"t\":6,\"cmdid\":%s,\"status\":0,\"data\":1}",cmdid);
				//printf("%s\r\n",TxetBuf);////////////////////////////////////////////////////////////
				if(ESP8266_IpSend((char *)TxetBuf, strlen((char *)TxetBuf)) < 0)
				{//发送失败
					printf("发送响应失败！\r\n");
				}
			}
			else if((strstr((const char *)RxBuf, (const char *)"\"data\":0") != NULL))//关锁
			{
				printf("关锁！\r\n");
				;//...
				;//...
				;//...
				cmdid = USER_GetJsonValue((char *)RxBuf, (char *)"cmdid");
				sprintf((char *)TxetBuf,"{\"t\":6,\"cmdid\":%s,\"status\":0,\"data\":0}",cmdid);
				//printf("%s\r\n",TxetBuf);////////////////////////////////////////////////////////////
				if(ESP8266_IpSend((char *)TxetBuf, strlen((char *)TxetBuf)) < 0)
				{//发送失败
					printf("发送响应失败！\r\n");
				}
				isAlert=0;
			}
		}
	}
}

int main( void )				 
{
	uint8_t TryCount=0;
	uint8_t IpData[128];
	int8_t temp;
	
	char lcd_show[LCD_COLUMN_NUM+1];

	My_InitTask();
	irInit();
	doorInit();
	lightInit();
	
	printf("STM32 start...");
	SYSTICK_Delay10ms(1);
	show_logo();

	param_Init();

	read_DigitPwd();
	read_IdCardPwd();
	read_MEMSData();
	mems_theshold_val = memsData.theshold;
	memsData.static_ADCValue[0] = get_ADC1_result(ADC1_CH10_RANK1);
	memsData.static_ADCValue[1] = get_ADC1_result(ADC1_CH11_RANK2);
	memsData.static_ADCValue[2] = get_ADC1_result(ADC1_CH12_RANK3);
	set_MEMS_static_ADC_value(memsData.static_ADCValue[0], memsData.static_ADCValue[1], memsData.static_ADCValue[2]);
	MEMS_ALERT = 0;
	DEBUG("MEMS theshold %d, ADC Val:%d %d %d",memsData.theshold, memsData.static_ADCValue[0], memsData.static_ADCValue[1], memsData.static_ADCValue[2]);

	//连接服务器
	for(TryCount=0; TryCount<3; TryCount++)
	{
		LCD_PRINT(0,0,lcd_show,"Connect to sever");
		LCD_PRINT(1,0,lcd_show,"Try %d times",TryCount);
		LCD_PRINT(2,0,lcd_show,"... ...");
		temp=ConnectToServer((char *)MY_DEVICE_ID, (char *)MA_SECRET_KEY);
		if(temp != 0)
		{
			printf("Connect To Server ERROR=%d\r\n",temp);
		}
		else
		{
			break;
		}
	}
	ClrAtRxBuf();//清空AT缓存
	lcd_clr();//清屏

	while(1)
	{
		switch(menuStatus)
		{
			case MENU_IDLE:
				menuIdle_Handle();
				break;
			case MENU_ADD_DIGI_PWD:
				menuAddDigitPwd_Handle();
				break;
			case MENU_ID_CARD_PWD:
				menuIdCardPwd_Handle();
				break;
			case MENU_MEMS:
				menuMEMS_Handle();
				break;
			case MENU_ALERT_CLEAR:
				menuAlertClear_Handle();
				break;

			default:
				menuStatus = MENU_IDLE;
				break;
		}

		if(F_AT_RX_FINISH)
		{	 // 接收到数据包

			ESP8266_GetIpData((uint8_t *)AT_RX_BUF, (char *)IpData);
			USER_DataAnalysisProcess((char *)IpData);
			memset(IpData, 0x00, 128);
			ClrAtRxBuf();
		}
		if(TimeCount >= 1000)//10S发送一次数据
		{
			TimeCount=0;
			if(isAlert)
			{
				ESP8266_SendSensor((char *)"alarm", (char *)"2018-06-20 14:10:26", isAlert+1);//ESP8266_SendSensor(isAlert, (char *)"2018-06-20 14:10:26");
			}
			else
			{
				ESP8266_SendSensor((char *)"alarm", (char *)"2018-06-20 14:10:26", FlagLockSta);//ESP8266_SendSensor(FlagLockSta, (char *)"2018-06-20 14:10:26");
			}
			ClrAtRxBuf();
			printf("发送传感数据alarm=%d！\r\n",isAlert);
			ESP8266_SendSensor((char *)"DefenseState", (char *)"2018-06-20 14:10:26", FlagDefense);
			ClrAtRxBuf();
			printf("发送防御状态defense=%d！\r\n",FlagDefense);
		}
		if(FlagDefense == 0)
		{
			isAlert = 0;//清除警告
			MEMS_ALERT = 0;
		}
	}
}

