/**
  ******************************************************************************
  * @file    TIM/PWM_Output/main.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "..\..\..\..\Libraries\stm32f10x.h"
#include "buzzer.h"

/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/** @addtogroup TIM_PWM_Output
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint16_t CCR1_Val = 10000;
static uint16_t PrescalerValue = 0;
unsigned char buzzer_on = 0;

/* Private function prototypes -----------------------------------------------*/
void TIM1_RCC_Configuration(void);
void TIM1_GPIO_Configuration(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  init program
  * @param  None
  * @retval None
  */
static void TIM1_pwm_init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f10x_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f10x.c file
     */     
       
  /* System Clocks Configuration */
  TIM1_RCC_Configuration();

  /* GPIO Configuration */
  TIM1_GPIO_Configuration();

  /* -----------------------------------------------------------------------
    TIM1 Configuration: generate 4 PWM signals with 4 different duty cycles:
    The TIM1CLK frequency is set to SystemCoreClock (Hz), to get TIM1 counter
    clock at 3 MHz the Prescaler is computed as following:
     - Prescaler = (TIM1CLK / TIM1 counter clock) - 1
    SystemCoreClock is set to 72 MHz for Low-density, Medium-density, High-density
    and Connectivity line devices and to 24 MHz for Low-Density Value line and
    Medium-Density Value line devices

    The TIM1 is running at 50 Hz: TIM1 Frequency = TIM1 counter clock/(ARR + 1)
                                                  = 3 MHz / 60000 = 50 Hz
    TIM1 Channel1 duty cycle = (TIM1_CCR1/ TIM1_ARR)* 100 = 50%
    TIM1 Channel2 duty cycle = (TIM1_CCR2/ TIM1_ARR)* 100 = 37.5%
    TIM1 Channel3 duty cycle = (TIM1_CCR3/ TIM1_ARR)* 100 = 25%
    TIM1 Channel4 duty cycle = (TIM1_CCR4/ TIM1_ARR)* 100 = 12.5%
  ----------------------------------------------------------------------- */
  /* Compute the prescaler value */
  PrescalerValue = (uint16_t) (SystemCoreClock / 1000000) - 1;
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 20000;
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

  /* PWM1 Mode configuration: Channel1 */
/*  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = CCR1_Val;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC1Init(TIM1, &TIM_OCInitStructure);

  TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
*/
  TIM_ARRPreloadConfig(TIM1, ENABLE);

//  TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);
  /* TIM1 enable counter */
  TIM_Cmd(TIM1, ENABLE);

}

/**
  * @brief  Configures the different system clocks.
  * @param  None
  * @retval None
  */
void TIM1_RCC_Configuration(void)
{
  /* TIM1 clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

  /* GPIOA and GPIOA clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
}

/**
  * @brief  Configure the TIM1 Ouput Channels.
  * @param  None
  * @retval None
  */
void TIM1_GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /*GPIOA Configuration: TIM1 channel1, 2, 3 and 4 */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  GPIO_Init(GPIOA, &GPIO_InitStructure);
}


/**
  * @brief  set TIM1_CH1 PWM duty to control servo degree
  * @param  degree: 0 ~ 180
  */
void TIM1_CH1_Set_Compare(unsigned short val)
{
	//printf("%d\r\n", cmp_val);
	TIM_SetCompare1(TIM1, val);
}

static void TIME6_NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}
#define	TIME6_INPUT_FREQ	1000000   //1M HZ
#define TIME6_TICK_US		(1000000/TIME6_INPUT_FREQ)
#define	TIME6_PERIOD_US		250   //unit us


static void TIME6_cfg(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	//允许TIM2的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);

	//重新将Timer设置为缺省值
	TIM_DeInit(TIM6);

	//采用内部时钟给TIM2提供时钟源
	//TIM_InternalClockConfig(TIM6);

	//预分频系数为36000-1，这样计数器时钟为72MHz/3600 = 20kHz
	TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / TIME6_INPUT_FREQ) -1;

	//设置时钟分割
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;

	//设置计数器模式为向上计数模式
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	//设置计数溢出大小，每计1000个数就产生一个更新事件
	TIM_TimeBaseStructure.TIM_Period = (TIME6_PERIOD_US / TIME6_TICK_US);
	//将配置应用到TIM2中
	TIM_TimeBaseInit(TIM6,&TIM_TimeBaseStructure);
	
	//清除溢出中断标志
	TIM_ClearFlag(TIM6, TIM_FLAG_Update);

	//禁止ARR预装载缓冲器
	TIM_ARRPreloadConfig(TIM6, ENABLE);

	//开启TIM2的中断
	TIM_ITConfig(TIM6,TIM_IT_Update,DISABLE);

}

static int TIME6_init(void)
{
	#if 0
	TIM2_test_GPIO_cfg();
	#endif

	//interrupt init
	TIME6_NVIC_Configuration();

	//tim2 timer init
	TIME6_cfg();
	
	//开启定时器2
	TIM_Cmd(TIM6,ENABLE);
	
	return 0;
}

//enter ISR per TIME2_PERIOD_US
void TIM6_IRQHandler(void)
{
	static uint8_t data = 0;
	
   //检测是否发生溢出更新事件
   if(TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
   {
		 //清除TIM2的中断待处理位
		 TIM_ClearITPendingBit(TIM6 , TIM_FLAG_Update);

			if(buzzer_on){
				GPIO_WriteBit(GPIOA, GPIO_Pin_8, data & 0x01);
				data++;
			}
   }
}

void buzzerInit(void)
{
//	TIM1_pwm_init();
	TIME6_init();
	TIM1_GPIO_Configuration();
}

void buzzerOption(unsigned short cycle_us, unsigned short val)
{
	TIM_SetAutoreload(TIM1, cycle_us);
	TIM_SetCompare1(TIM1, val);
}

void buzzerOpen(void)
{
//	buzzerOption(20*1000, 10*1000);
	/*buzzer_on = 1;
	TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE);
	TIM_Cmd(TIM6,ENABLE);*/
}

void buzzerClose(void)
{
//	buzzerOption(20*1000, 0);
	/*buzzer_on = 0;
	TIM_ITConfig(TIM6,TIM_IT_Update,DISABLE);
	TIM_Cmd(TIM6,DISABLE);*/
}
/**
  * @}
  */ 

/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
