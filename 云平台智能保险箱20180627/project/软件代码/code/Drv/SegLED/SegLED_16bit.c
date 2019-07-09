#include <stdio.h>
#include <string.h>
#include "stm32f10x.h"
#include "segled_16bit.h"
#include "timer.h"

const int32_t ledseg_refresh_tm = (2500/TIME2_PERIOD_US);  //2.5ms , 50Hz


#define LED_0   0
#define LED_1   1
#define LED_2   2
#define LED_3   3
#define LED_4   4
#define LED_5   5
#define LED_6   6
#define LED_7   7
#define LED_8   8
#define LED_9   9
#define LED_a   0x0a
#define LED_b   0x0b
#define LED_c   0x0c
#define LED_d   0x0d
#define LED_E   0x0e
#define LED_F   0x0f
#define LED_H   0x10
#define LED_h   0x11
#define LED_L   0x12
#define LED_n   0x13
#define LED_N   0x14
#define LED_o   0x15
#define LED_p   0x16
#define LED_q   0x17
#define LED_r   0x18
#define LED_t   0x19
#define LED_U   0x1A
#define LED_y   0x1B
#define LED__   0x1C
#define LED_SPACE    0x1D

#define SEG_H    ~0x80
#define B_dot    0x80
#define MAX_LEDCH 0X1D

#define SEGLED_LED_SUM		8		


#define SEGLED_BIT_MAX		7
#define SEGLED_BIT_MIN		0
#define SEGLED_DATA_MAX		0x09
#define SEGLED_DATA_MIN		0x00
#define SEGLED_DATA_WITH_DOT_MAX		0xf9
#define SEGLED_DATA_WITH_DOT_MIN		0xf0


uint8_t segled_show_buf[SEGLED_LED_SUM] = {0};

/*************LED*************************************/
const uint8_t ledseg_tab[]  =
{
        0xC0,/*0*/
        0xF9,/*1*/
        0xA4,/*2*/
        0xB0,/*3*/
        0x99,/*4*/
        0x92,/*5*/
        0x82,/*6*/
        0xF8,/*7*/
        0x80,/*8*/
        0x90,/*9*/
		0x7f,/*dot*/
		#if 0
        0x88,/*A*/
        0x83,/*b*/
        0xA7,/*c*/
        0xA1,/*d*/
        0x86,/*E*/
        0x8E,/*F*/
        0x89,/*H 10*/
        0x8B,/*h 11*/
        0xC7,/*L 12*/
        0xAB,/*n 13*/
        0xC8,/*N 14*/
        0xA3,/*o 15*/
        0x8C,/*P 16*/
        0x98,/*q 17*/
        0xAF,/*r 18*/
        0x87,/*t 19 */
        0xC1,/*U 1a*/
        0x91,/*y 1b*/
        0xBF,/*- 1c*/
        0xFF, /*  1D*/
        0xFF,
        0x00, /*  1F*/
        #endif
};
const uint8_t com_tab[SEGLED_LED_SUM] = {1,2,4,8,0x10,0x20,0x40,0x80};

void serial_to_parallel_74HC595_gpio_config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = SEGLED_PINS;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(SEGLED_GPIO, &GPIO_InitStructure);
}


void s_to_p_74hc595_out_16bits(uint8_t data_hight, uint8_t data_low)
{
   uint16_t dout;
   uint8_t i;
   
   dout = (data_hight<<8) | data_low;
   
   //RCK_595 =0 ;
   GPIO_ResetBits(SEGLED_GPIO, SEGLED_RCK_PIN);
   
   //SCK_595=0;
   GPIO_ResetBits(SEGLED_GPIO, SEGLED_SCK_PIN);
   // 串行移位输出
   for(i=0; i<16; i++)

   {
        //SCK_595 =0 ;
		GPIO_ResetBits(SEGLED_GPIO, SEGLED_SCK_PIN);
        if(dout &0x8000) 
           //SI_595=1;
		   GPIO_SetBits(SEGLED_GPIO, SEGLED_DATA_PIN);
        else
           //SI_595=0;
		   GPIO_ResetBits(SEGLED_GPIO, SEGLED_DATA_PIN);
		__ASM("NOP");
        GPIO_SetBits(SEGLED_GPIO, SEGLED_SCK_PIN);
       __ASM("NOP");
	    dout = dout<<1;
   }
   //SCK_595=0;
   GPIO_ResetBits(SEGLED_GPIO, SEGLED_SCK_PIN);
   // 输出锁存
   //RCK_595 =1 ;
   GPIO_SetBits(SEGLED_GPIO, SEGLED_RCK_PIN);
  __ASM("NOP");
  __ASM("NOP");
   GPIO_ResetBits(SEGLED_GPIO, SEGLED_RCK_PIN);
}

/***************************************************
函 数 名：
函数功能：led动态显示
			中断函数里调用，小心编写！！！！
参    数：无
返 回 值：无
****************************************************/
/****************************************************************************************/
void refresh_segleds(void)  
{
	uint8_t segi, show_data;
	static uint8_t led_bit = 0;
	
	led_bit++;
	led_bit &= 7;
	show_data = segled_show_buf[led_bit];
	segi = ledseg_tab[show_data & 0x0f];
	if(show_data & SEGLED_DATA_WITH_DOT_MIN)
		segi &= SEG_H;
 	s_to_p_74hc595_out_16bits(com_tab[led_bit], segi);
}

/* led_bit 0 ~ 7
   data 0x00 ~ 0x09(show data are : 0~9)
   data 0xf0 ~ 0xf9(show data are : 0~9 with dot)
   return 0, ok
   return -1, error*/
int set_segleds_bit(uint8_t led_bit, uint8_t data)
{
	if(led_bit > SEGLED_BIT_MAX)
		return -1;
	if((data > SEGLED_DATA_MAX && data < SEGLED_DATA_WITH_DOT_MIN) || (data > SEGLED_DATA_WITH_DOT_MAX))
		return -1;
	segled_show_buf[led_bit] = data;
	
	return 0;
}

void init_74HC595_for_segleds(void)
{
	serial_to_parallel_74HC595_gpio_config();
	
	s_to_p_74hc595_out_16bits(0xff, ledseg_tab[0]);
}


