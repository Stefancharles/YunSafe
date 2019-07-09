/**
  ******************************************************************************
  * @file    ADC/ADC1_DMA/main.c 
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
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "stm32f10x.h"
#include "adc.h"
#include "timer.h"

/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/** @addtogroup ADC_ADC1_DMA
  * @{
  */ 

#define ADC_CHANNEL_NUM		5
#define ADC_RANK_MAX		5
#define ADC_SAMPLE_TIMES	10


//#define EN_ADC1_IT

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define ADC1_DR_Address    ((uint32_t)0x4001244C)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
ADC_InitTypeDef ADC_InitStructure;
DMA_InitTypeDef DMA_InitStructure;
__IO uint16_t ADCConvertedValueBuf[ADC_CHANNEL_NUM];
uint16_t ADCMltConvertedValue[ADC_SAMPLE_TIMES][ADC_CHANNEL_NUM];
uint16_t ADCConvertedValue[ADC_CHANNEL_NUM];
uint16_t ADCPriorConvertedValue[ADC_CHANNEL_NUM];
uint16_t MEMS_static_ADCValue[ADC_CHANNEL_NUM];

volatile uint8_t MEMS_ALERT = 0;

uint16_t mems_theshold_val = MEMS_THRESHOLD_DEFAULT;

const int32_t mems_alert_timeout = (1000000/TIME2_PERIOD_US);  //1000ms
const int32_t adc_sample_rate = (6000/TIME2_PERIOD_US);  //6ms

ADC_CALLBACK_FUNC adc_nofify_callbk = NULL;


#define ADC_DEBOUNCE_MV		2  //2.4mv for debounce



/* Private function prototypes -----------------------------------------------*/
void ADC1_RCC_Configuration(void);
void ADC1_GPIO_Configuration(void);
void ADC1_NVIC_Configuration(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief   ADC1 init program
  * @param  None
  * @retval None
  */
void ADC1_init(ADC_CALLBACK_FUNC nofify_callbk)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f10x_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f10x.c file
     */     
       
  /* System clocks configuration ---------------------------------------------*/
  ADC1_RCC_Configuration();

  /* NVIC configuration ------------------------------------------------------*/
  ADC1_NVIC_Configuration();

  /* GPIO configuration ------------------------------------------------------*/
  ADC1_GPIO_Configuration();

  /* DMA1 channel1 configuration ----------------------------------------------*/
  DMA_DeInit(DMA1_Channel1);
  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADCConvertedValueBuf[0];
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = ADC_CHANNEL_NUM;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; // Circular mode
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);

  DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);

  /* Enable DMA1 channel1 */
  DMA_Cmd(DMA1_Channel1, ENABLE);
  
  /* ADC1 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = ADC_CHANNEL_NUM;
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC1 regular channel10 configuration */ 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, ADC1_CH10_RANK1, ADC_SampleTime_7Cycles5);
  /* ADC1 regular channel11 configuration */ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_11, ADC1_CH11_RANK2, ADC_SampleTime_7Cycles5);
  /* ADC1 regular channel12 configuration */ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_12, ADC1_CH12_RANK3, ADC_SampleTime_7Cycles5);
  /* ADC1 regular channel13 configuration */ 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, ADC1_CH13_RANK4, ADC_SampleTime_7Cycles5);
  /* ADC1 regular channel14 configuration */ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_14, ADC1_CH14_RANK5, ADC_SampleTime_7Cycles5);

  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);

  #ifdef EN_ADC1_IT
  /* Enable ADC1 EOC interrupt */
  ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
  #endif
  
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);

  /* Enable ADC1 reset calibration register */   
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));

  /* Start ADC1 calibration */
  ADC_StartCalibration(ADC1);
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));
     
  /* Start ADC1 Software Conversion */ 
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);

  if(nofify_callbk){
  	adc_nofify_callbk = nofify_callbk;
  }

}

/**
  * @brief  Configures the different system clocks.
  * @param  None
  * @retval None
  */
void ADC1_RCC_Configuration(void)
{
#if defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || defined (STM32F10X_HD_VL)
  /* ADCCLK = PCLK2/2 */
  RCC_ADCCLKConfig(RCC_PCLK2_Div2); 
#else
  /* ADCCLK = PCLK2/4 */
  RCC_ADCCLKConfig(RCC_PCLK2_Div4); 
#endif
  /* Enable peripheral clocks ------------------------------------------------*/
  /* Enable DMA1 clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

  /* Enable ADC1 and GPIOC clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC, ENABLE);
}

/**
  * @brief  Configures the different GPIO ports.
  * @param  None
  * @retval None
  */
void ADC1_GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Configure 
  PC.0(ADC Channel10) PC.1(ADC Channel11) PC.2(ADC Channel12) PC.3(ADC Channel13) PC.04 (ADC Channel14) 
  as analog input -------------------------*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
}

/**
  * @brief  Configures Vector Table base location.
  * @param  None
  * @retval None
  */
void ADC1_NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  #ifdef EN_ADC1_IT
  /* Enable DMA1 channel1 IRQ Channel */
  NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  #endif

  /* Enable DMA1 channel1 IRQ Channel */
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}



uint16_t get_ADC1_result(unsigned char rank)
{
	if(rank > ADC_RANK_MAX)
		return 0xffff;
	return ADCConvertedValue[rank - 1];
}


uint16_t get_ADC1_voltege_mv(unsigned char rank)
{
	if(rank > ADC_RANK_MAX)
		return 0xffff;

	//ad*(3.3v/0xfff)
	return (ADCConvertedValue[rank - 1] * 3300)/0xfff;
}


void start_ADC1(void)
{
	memcpy((void*)ADCPriorConvertedValue, (void*)ADCConvertedValue, sizeof(ADCConvertedValue));
	/* Start ADC1 Software Conversion */ 
  	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

#define ABS(_x, _y) (((_x) > (_y)) ? ((_x) -(_y)) : ((_y) -(_x)))

unsigned char is_MEMS_threshold_exceed()
{
	
	if(ABS(ADCConvertedValue[ADC1_CH10_RANK1 - 1], MEMS_static_ADCValue[ADC1_CH10_RANK1 - 1]) > mems_theshold_val)
		return 1;
	else if(ABS(ADCConvertedValue[ADC1_CH11_RANK2 - 1], MEMS_static_ADCValue[ADC1_CH11_RANK2 - 1]) > mems_theshold_val)
		return 1;
	else if(ABS(ADCConvertedValue[ADC1_CH12_RANK3 - 1], MEMS_static_ADCValue[ADC1_CH12_RANK3 - 1]) > mems_theshold_val)
		return 1;
	else
		return 0;
}

void set_ADC1_update_flag(void)
{
	static uint8_t sample_count = 0;
	uint8_t nofify_rank = 0;
	
	memcpy((void*)&ADCMltConvertedValue[sample_count][0], (void*)ADCConvertedValueBuf, sizeof(ADCConvertedValueBuf));
	sample_count++;
	if(sample_count >= ADC_SAMPLE_TIMES){
		uint8_t i, j;
		uint32_t tmp;
		
		sample_count = 0;
		for(i = 0; i < ADC_CHANNEL_NUM; i++){
			tmp = 0;
			for(j = 0; j < ADC_SAMPLE_TIMES; j++){
				tmp += ADCMltConvertedValue[j][i];
			}
			tmp = tmp / ADC_SAMPLE_TIMES;
			if(ABS(tmp, ADCPriorConvertedValue[i]) > ADC_DEBOUNCE_MV){
				ADCConvertedValue[i] = tmp;
				nofify_rank = 1 << i;
			}
		}

		if(nofify_rank)
		{
			(*adc_nofify_callbk)(nofify_rank);
		}
		if(is_MEMS_threshold_exceed() && MEMS_ALERT < MEMS_ALERT_TRIGER)
			MEMS_ALERT++;
	}

}

void set_MEMS_static_ADC_value(int16_t rk1, int16_t rk2, int16_t rk3)
{
	MEMS_static_ADCValue[ADC1_CH10_RANK1 - 1] = rk1;
	MEMS_static_ADCValue[ADC1_CH11_RANK2 - 1] = rk2;
	MEMS_static_ADCValue[ADC1_CH12_RANK3 - 1] = rk3;
}


/**
  * @brief	This function handles ADC1_2 interrupt request.
  * @param	None
  * @retval None
  */
void ADC1_2_IRQHandler(void)
{
	/* Clear ADC1 AWD pending interrupt bit */
	ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);

	printf("ADC1:%d\r\n", ADC_GetConversionValue(ADC1));
}

/**
  * @brief  This function handles DMA1 Channel 1 interrupt request.
  * @param  None
  * @retval None
  */
void DMA1_Channel1_IRQHandler(void)
{
	//printf("%d %d\r\n", DMA_GetCurrDataCounter(DMA1_Channel1), DMA_GetITStatus(DMA1_IT_TC1));
	
    /* Clear DMA1 Channel1 Half Transfer, Transfer Complete and Global interrupt pending bits */
	set_ADC1_update_flag();
    DMA_ClearITPendingBit(DMA1_IT_GL1);
}



#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}

#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
