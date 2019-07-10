
/******************************************************************************
* @file    app/app_pwd.c 
* @author  zhao
* @version V1.0.0
* @date    2018.06.18
* @brief   密码eeprom管理
******************************************************************************/
#include "app_pwd.h"
#define __DEBUG__  
#ifdef __DEBUG__  
#define DEBUG(format,...) printf("File: "__FILE__", Line: %05d: "format"\r\n", __LINE__, ##__VA_ARGS__)  
#else  
#define DEBUG(format,...)  
#endif  


unsigned char digitPwd[MAX_DIGIT_PWD_NUM][DIGIT_PWD_LEN];//数字密码缓存
unsigned char digitPwd_offset = 0;//密码保存在缓存中的位置
unsigned char curDigitPwd[DIGIT_PWD_LEN];//当前输入的密码
unsigned char curDigitPwd_offset = 0;//当前输入密码到第几位

//LF id card password

unsigned char idCardPwd[MAX_ID_CARD_PWD_NUM][ID_CARD_PWD_LEN];//id card密码缓存
unsigned char idCardPwd_offset = 0;//密码保存在缓存中的位置
unsigned char curIdCardPwd[ID_CARD_PWD_LEN];//当前输入的密码


extern MEMSDataTypedef memsData = 
{
	.static_ADCValue[0] = 1000,
	.static_ADCValue[1] = 2000,
	.static_ADCValue[2] = 1000,
};

/*
  * @brief  clearPwdCache 清空输入密码缓存
  * @param  none
  * @note  1.参数初始化
           2.是否初次上电  第一次上电 重置密码 
  * @Date:2018.6.20
  * @author:zhao
  * @return:none
*/
void clear_PwdCache(void)
{
	curDigitPwd_offset = 0;
	memset(curDigitPwd, 0, DIGIT_PWD_LEN);
	memset(curIdCardPwd, 0, ID_CARD_PWD_LEN);
}

/*
  * @brief  isRightPwd_Digit 判断数字密码是否正确
  * @param  none
  * @note  1.参数初始化
           2.是否初次上电  第一次上电 重置密码 
  * @Date:2018.6.20
  * @author:zhao
  * @return:none
*/
int isRight_Pwd_Digit(unsigned char pwd[],int len)
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

/*
  * @brief  isRightPwd_IdCard 判断ID卡密码是否正确
  * @param  none
  * @note  1.参数初始化
           2.是否初次上电  第一次上电 重置密码 
  * @Date:2018.6.20
  * @author:zhao
  * @return:OK:PWD_SUCCESS  NO:PWD_ERROR
*/
int isRight_Pwd_IdCard(unsigned char pwd[],int len)
{
	int i;
	read_IdCardPwd();
	if(len != ID_CARD_PWD_LEN)
	{
		printf("len != ID_CARD_PWD_LEN\r\n");
		return PWD_ERROR;
	}
	for(i=0; i < MAX_ID_CARD_PWD_NUM; i++)
	{
		if(memcmp(pwd, &idCardPwd[i][0], ID_CARD_PWD_LEN) == 0)
		{
			break;
		}
	}
	printf("ID PWD's i=%d\r\n",i);
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
  * @note  1.参数初始化
           2.是否初次上电  第一次上电 重置密码 
  * @Date:2018.6.19
  * @author:zhao
  * @return:none
*/
void param_Init(void)
{
	unsigned char save_flag;
	unsigned short NumByteToRead = 1;
	unsigned short NumByteToWrite;
	int i;
/*
  * @note  1.参数初始化
           2.sEE_ReadBuffer  获取   初始化只 如果是第一次上电 0 byte 写入 0xA3
           3.
  * @Date:2018.6.19
  * @author:zhao
*/
//	sEE_ReadBuffer(&save_flag, EEP_SAVE_FLAG, &NumByteToRead);
    bsp_flash_read(EEP_SAVE_FLAG_ADDRESS, &save_flag, NumByteToRead);

	DEBUG("%x", save_flag);
	if(save_flag != 0xA3)
	{
		save_flag = 0xA3;
		NumByteToWrite = 1;
//		sEE_WriteBuffer(&save_flag, EEP_SAVE_FLAG, NumByteToWrite);
		bsp_flash_erase(EEP_SAVE_FLAG_ADDRESS, NumByteToWrite);
		bsp_flash_write(EEP_SAVE_FLAG_ADDRESS, &save_flag, NumByteToWrite);
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
//		sEE_WriteBuffer(&digitPwd_offset, EEP_DIGIT_PWD_OFFSET, NumByteToWrite);
		bsp_flash_erase(EEP_DIGIT_PWD_OFFSET_ADDRESS, NumByteToWrite);
		bsp_flash_write(EEP_DIGIT_PWD_OFFSET_ADDRESS, &digitPwd_offset, NumByteToWrite);
		NumByteToWrite = MAX_DIGIT_PWD_NUM * DIGIT_PWD_LEN;
//		sEE_WriteBuffer((unsigned char *)digitPwd[0], EEP_DIGIT_PWD_START, NumByteToWrite);
		bsp_flash_erase(EEP_DIGIT_PWD_START_ADDRESS, NumByteToWrite);
		bsp_flash_write(EEP_DIGIT_PWD_START_ADDRESS, (unsigned char *)digitPwd[0], NumByteToWrite);
		//id card pwd
		for(i=0; i < MAX_ID_CARD_PWD_NUM; i++)
		{
		  memset(digitPwd[i], 0xEE, ID_CARD_PWD_LEN);
		}
		NumByteToWrite = 1;
//		sEE_WriteBuffer(&idCardPwd_offset, EEP_ID_CARD_PWD_OFFSET, NumByteToWrite);
		bsp_flash_erase(EEP_ID_CARD_PWD_OFFSET_ADDRESS, NumByteToWrite);
		bsp_flash_write(EEP_ID_CARD_PWD_OFFSET_ADDRESS, &idCardPwd_offset, NumByteToWrite);
		NumByteToWrite = MAX_ID_CARD_PWD_NUM * ID_CARD_PWD_LEN;
//		sEE_WriteBuffer((unsigned char *)idCardPwd[0], EEP_ID_CARD_PWD_START, NumByteToWrite);
		bsp_flash_erase(EEP_ID_CARD_PWD_START_ADDRESS, NumByteToWrite);
		bsp_flash_write(EEP_ID_CARD_PWD_START_ADDRESS, (unsigned char *)idCardPwd[0], NumByteToWrite);
		//mems
		memsData.theshold = 50;
		//memset(&memsData.static_ADCValue[0], 0, sizeof(memsData.static_ADCValue));
		NumByteToWrite = sizeof(MEMSDataTypedef);
		
//		sEE_WriteBuffer((unsigned char *)&memsData, EEP_MEMS_START, NumByteToWrite);
		bsp_flash_erase(EEP_MEMS_START_ADDRESS, NumByteToWrite);
		bsp_flash_write(EEP_MEMS_START_ADDRESS, (unsigned char *)&memsData, NumByteToWrite);
	}
}

int save_DigitPwd(unsigned char pwd[],int len)
{	
	unsigned short data_len = 0;
	//static unsigned char pwd_offset = 0;
	if(digitPwd_offset>= MAX_DIGIT_PWD_NUM)
		digitPwd_offset = 0;	
	memset(digitPwd[digitPwd_offset], 0xff, DIGIT_PWD_LEN);
	memcpy(digitPwd[digitPwd_offset], pwd, len);
	digitPwd_offset++;
	data_len = 1;
//	sEE_WriteBuffer(&digitPwd_offset, EEP_DIGIT_PWD_OFFSET, data_len);
	bsp_flash_erase(EEP_DIGIT_PWD_OFFSET_ADDRESS, data_len);
	bsp_flash_write(EEP_DIGIT_PWD_OFFSET_ADDRESS, &digitPwd_offset, data_len);
	data_len = MAX_DIGIT_PWD_NUM * DIGIT_PWD_LEN;
//	sEE_WriteBuffer((unsigned char *)&digitPwd[0][0], EEP_DIGIT_PWD_START, data_len);
	bsp_flash_erase(EEP_DIGIT_PWD_START_ADDRESS, data_len);
	bsp_flash_write(EEP_DIGIT_PWD_START_ADDRESS,(unsigned char *)&digitPwd[0][0], data_len);
	return 0;
}


/*
  * @brief  read_DigitPwd 读取数字密码
  * @param  none
  * @note  1.参数初始化
           2.分配内存 calloc(128,sizeof(char))
           3.获取当前缓存位置
           4.digitPwd缓存写FF
           5.将digitPwd缓存写入 debug_buff  打印 密码  strcat 串联字符串
           6.free 释放内存
  * @Date:2018.6.20
  * @author:zhao
  * @return:(unsigned char **)digitPwd  指针地址
*/
unsigned char **read_DigitPwd(void)
{	
	int i,j;
	unsigned short data_len = 0;
	char *debug_buf = calloc(128, sizeof(char));
	char temp_buf[8];	
	data_len = 1;
//	sEE_ReadBuffer(&digitPwd_offset, EEP_DIGIT_PWD_OFFSET, &data_len);
	bsp_flash_read(EEP_DIGIT_PWD_OFFSET_ADDRESS, &digitPwd_offset, data_len);
	memset(digitPwd[0], 0xff, MAX_DIGIT_PWD_NUM * DIGIT_PWD_LEN);
	data_len = MAX_DIGIT_PWD_NUM * DIGIT_PWD_LEN;
//	sEE_ReadBuffer(digitPwd[0], EEP_DIGIT_PWD_START, &data_len);
	bsp_flash_read(EEP_DIGIT_PWD_START_ADDRESS, digitPwd[0], data_len);


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


int save_IdCardPwd(unsigned char pwd[],int len)
{	
	unsigned short data_len = 0;
	//static unsigned char pwd_offset = 0;
	if(idCardPwd_offset>= MAX_ID_CARD_PWD_NUM)
		idCardPwd_offset = 0;	
	memset(idCardPwd[idCardPwd_offset], 0xff, ID_CARD_PWD_LEN);
	memcpy(idCardPwd[idCardPwd_offset], pwd, len);
	idCardPwd_offset++;
	data_len = 1;
//	sEE_WriteBuffer(&idCardPwd_offset, EEP_ID_CARD_PWD_OFFSET, data_len);
	bsp_flash_erase(EEP_ID_CARD_PWD_OFFSET_ADDRESS, data_len);
	bsp_flash_write(EEP_ID_CARD_PWD_OFFSET_ADDRESS,&idCardPwd_offset, data_len);
	data_len = MAX_ID_CARD_PWD_NUM * ID_CARD_PWD_LEN;
//	sEE_WriteBuffer((unsigned char *)&idCardPwd[0][0], EEP_ID_CARD_PWD_START, data_len);
	bsp_flash_erase(EEP_ID_CARD_PWD_START_ADDRESS, data_len);
	bsp_flash_write(EEP_ID_CARD_PWD_START_ADDRESS,(unsigned char *)&idCardPwd[0][0], data_len);
	return 0;
}


int delete_IdCardPwd(unsigned char pwd[],int len)
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
//		sEE_WriteBuffer((unsigned char *)&idCardPwd[0][0], EEP_ID_CARD_PWD_START, data_len);
		bsp_flash_erase(EEP_ID_CARD_PWD_START_ADDRESS, data_len);
	    bsp_flash_write(EEP_ID_CARD_PWD_START_ADDRESS,(unsigned char *)&idCardPwd[0][0], data_len);
	}
	return 0;
}

unsigned char **read_IdCardPwd(void)
{	
	unsigned short data_len = 0;
	data_len = 1;
//	sEE_ReadBuffer(&idCardPwd_offset, EEP_ID_CARD_PWD_OFFSET, &data_len);
	bsp_flash_read(EEP_ID_CARD_PWD_OFFSET_ADDRESS, &idCardPwd_offset, data_len);
	memset(idCardPwd[0], 0xff, MAX_ID_CARD_PWD_NUM * ID_CARD_PWD_LEN);
	data_len = MAX_ID_CARD_PWD_NUM * ID_CARD_PWD_LEN;
//	sEE_ReadBuffer(idCardPwd[0], EEP_ID_CARD_PWD_START, &data_len);	
	bsp_flash_read(EEP_ID_CARD_PWD_START_ADDRESS, idCardPwd[0], data_len);
	return (unsigned char **)idCardPwd;

}

void save_MEMSData(void)
{
	unsigned short NumByteToWrite;
	NumByteToWrite = sizeof(MEMSDataTypedef);
//	sEE_WriteBuffer((unsigned char *)&memsData, EEP_MEMS_START, NumByteToWrite);
	bsp_flash_erase(EEP_MEMS_START_ADDRESS, NumByteToWrite);
    bsp_flash_write(EEP_MEMS_START_ADDRESS,(unsigned char *)&memsData, NumByteToWrite);
	DEBUG("MEMS theshold %d, ADC Val:%d %d %d",memsData.theshold, memsData.static_ADCValue[0], memsData.static_ADCValue[1], memsData.static_ADCValue[2]);
}
void read_MEMSData(void)
{
	unsigned short data_len;
	data_len = sizeof(MEMSDataTypedef);
//	sEE_ReadBuffer((unsigned char *)&memsData, EEP_MEMS_START, &data_len);
	bsp_flash_read(EEP_MEMS_START_ADDRESS, (unsigned char *)&memsData, data_len);
	DEBUG("MEMS theshold %d, ADC Val:%d %d %d",memsData.theshold, memsData.static_ADCValue[0], memsData.static_ADCValue[1], memsData.static_ADCValue[2]);
}


