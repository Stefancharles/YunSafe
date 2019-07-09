/******************************************************************************
* @ File name --> led.c
* @ Author    --> By@ MALIWEI
* @ Version   --> V1.0
* @ Date      --> 
* @ Brief     --> 蜂鸣器控制相关函数
*
* @ Copyright (C) 20**
* @ All rights reserved
******************************************************************************/

#include "hal_buzzer.h"

//全局变量
BuzzerState_TypeDef Buzzer_State;

/******************************************************************************
* Function Name --> Buzzer接口初始化
* Description   --> none
* Input         --> none
* Output        --> none
* Reaturn       --> none 
******************************************************************************/
void Buzzer_Init(void)
{
	STM32_GPIOx_Init(Buzzer0_Init);
	BUZZER0 = 0;
}

/***************************************************************************************************
 * @fn      HalBuzzerBlink
 *
 * @brief   Blink the leds
 *
 * @param   leds       - bit mask value of leds to be blinked
 *          numBlinks  - number of blinks
 *          percent    - the percentage in each period where the led
 *                       will be on
 *          period     - length of each cycle in milliseconds
 *
 * @return  None
 ***************************************************************************************************/
#define BUZZER_MULTIPLE 10
void HalBuzzerBlink (uint8_t leds, uint8_t numBlinks, uint8_t percent, uint16_t period)
{
  uint8_t led;
	uint8_t pos;
	HalBuzzerOnOff (leds, HAL_BUZZER_MODE_ON);
  if (leds && percent && period)
  {
    if (percent < 100)
    {
      led = HAL_BUZZER_0;
      leds &= HAL_BUZZER_ALL;
			pos=0;
      while (leds)
      {
        if (leds & led)
        {
					Buzzer_State.numBlinksGlobal[pos]=numBlinks;//闪烁次数;
					Buzzer_State.percentGlobal[pos]=(period*BUZZER_MULTIPLE*percent)/100;//一个周期中亮的时长
					Buzzer_State.periodGlobal[pos]=period*BUZZER_MULTIPLE;//一个周期时长
					Buzzer_State.BuzzerTimeCount[pos]=0;
					leds ^= led;
        }
				pos++;
        led <<= 1;
      }
    }
    else
    {
      HalBuzzerOnOff (leds, HAL_BUZZER_MODE_ON);                    /* >= 100%, turn on */
    }
  }
  else
  {
    HalBuzzerOnOff (leds, HAL_BUZZER_MODE_OFF);                     /* No on time, turn off */
  }
}

/***************************************************************************************************
 * @fn      HalBuzzerOnOff
 *
 * @brief   Turns specified Buzzer ON or OFF
 *
 * @param   leds - Buzzer bit mask
 *          mode - BUZZER_ON,BUZZER_OFF,
 *
 * @return  none
 ***************************************************************************************************/
void HalBuzzerOnOff (uint8_t leds, uint8_t mode)
{
  if (leds & HAL_BUZZER_0)
  {
    if (mode == HAL_BUZZER_MODE_ON)
    {
      HAL_TURN_ON_BUZZER0;
    }
    else
    {
      HAL_TURN_OFF_BUZZER0;
    }
  }
}

void HalBuzzerBlinkProcess ( void )
{
	uint8_t i;
	for(i=0;i<HAL_BUZZER_DEFAULT_MAX_BUZZERS;i++)
	{
		if(Buzzer_State.numBlinksGlobal[i]>0)
		{
			if(Buzzer_State.BuzzerTimeCount[i]<Buzzer_State.periodGlobal[i])
			{
				if(Buzzer_State.BuzzerTimeCount[i]++<Buzzer_State.percentGlobal[i])
				{
					HAL_TOGGLE_BUZZER0;
				}
				else
				{
					HAL_TURN_OFF_BUZZER0;
				}
			}
			else
			{
				Buzzer_State.BuzzerTimeCount[i]=0;
				Buzzer_State.numBlinksGlobal[i]--;
			}
		}
	}
}

