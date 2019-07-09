#include "hal_timer2.h"
#include "hal_uart4.h"
#include "hal_usart1.h"
#include "public.h"

uint8_t F_UseUsartN=0; //系统目前使用的串口标记，0-使用USART1; 1-使用USART2;

//================================================================
void set_mem(uint8_t *buf, uint8_t len,uint8_t val)
{
    while(len--)
    {
        *buf=val;
        buf++;   
    }  
}

uint8_t lrc_chk(uint8_t *buf, uint8_t len)
{
    uint8_t   chk=0;
    while(len--)
    {
        chk -= *buf;
        buf++;   
    }  
	return chk;  
}

/*************************/
uint8_t rev_datapage(uint8_t *buf, uint8_t *len)
{
	uint8_t i;
	
	if(F_UART4_RX_FINISH != 0)
	{
		F_UART4_RX_FINISH=0;
		F_UseUsartN=1;
		
		for(i=0; i<UART4_RX_COUNT; i++)
		{
			buf[i]=UART4_RX_BUF[i];
		}
		
		UART4_RX_COUNT=0;
		return 1;
	}
	else if(F_USART1_RX_FINISH != 0)
	{
		F_USART1_RX_FINISH=0;
		F_UseUsartN=0;

		for(i=0; i<USART1_RX_COUNT; i++)
		{
			buf[i]=USART1_RX_BUF[i];
		}
		
		USART1_RX_COUNT=0;
		return 1;
	}
	else
	{//串口未接收完成
		return	0;
	}
}

void send_datapag(uint8_t *buf)
{
	uartsend(buf,strlen((const char *)buf));	
}

void uartsend(uint8_t *buf, uint8_t len)
{
	while(len--)
	{
		if(F_UseUsartN != 0)
		{
			UART4_SendData((uint16_t)(*buf));
		}
		else
		{
			USART1_SendData((uint16_t)(*buf));
		}
		buf++;
	}
}

