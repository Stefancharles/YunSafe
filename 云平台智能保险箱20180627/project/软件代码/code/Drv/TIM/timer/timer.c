#include <stdio.h>
#include "stm32f10x.h"
#include "systick.h"
#include "keyboard_drv.h"
#include "adc.h"
#include "ultrasonic.h"
#include "segled_16bit.h"
#include "timer.h"
//#include "buzzer.h"

extern const int32_t mems_alert_timeout;  //300ms
extern const int32_t adc_sample_rate;  //50ms
extern uint8_t MEMS_ALERT;
extern const int32_t ledseg_refresh_tm; //2.5ms , 50Hz
extern const int32_t kbd_scan_tm;  //10ms


void TIM2_NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}

void TIM2_cfg(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	//允许TIM2的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);

	//重新将Timer设置为缺省值
	TIM_DeInit(TIM2);

	//采用内部时钟给TIM2提供时钟源
	TIM_InternalClockConfig(TIM2);

	//预分频系数为36000-1，这样计数器时钟为72MHz/3600 = 20kHz
	TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / TIME2_INPUT_FREQ) -1;

	//设置时钟分割
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;

	//设置计数器模式为向上计数模式
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	//设置计数溢出大小，每计1000个数就产生一个更新事件
	TIM_TimeBaseStructure.TIM_Period = (TIME2_PERIOD_US / TIME2_TICK_US);
	//将配置应用到TIM2中
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);
	
	//清除溢出中断标志
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);

	//禁止ARR预装载缓冲器
	TIM_ARRPreloadConfig(TIM2, DISABLE);

	//开启TIM2的中断
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);

}

void TIM2_test_GPIO_cfg(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;                 //选择引脚5
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //输出频率最大50MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //带上拉电阻输出
	GPIO_Init(GPIOB,&GPIO_InitStructure);
}

int TIM2_init(void)
{
	#if 0
	TIM2_test_GPIO_cfg();
	#endif

	//interrupt init
	TIM2_NVIC_Configuration();

	//tim2 timer init
	TIM2_cfg();
	
	//开启定时器2
	TIM_Cmd(TIM2,ENABLE);
	
	return 0;
}




//enter ISR per TIME2_PERIOD_US
void TIM2_IRQHandler(void)
{
	static int32_t count = 0;
//	static uint8_t data = 0;
//	static uint16_t buzzerPrescaler = 0;
	
   //检测是否发生溢出更新事件
   if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
   {
      count++;
	  if(count >= adc_sample_rate * mems_alert_timeout * kbd_scan_tm * ledseg_refresh_tm)
	  	count = 0;
	  
      //清除TIM2的中断待处理位
      TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update);

	  //do keboard scaning
	  if(count % kbd_scan_tm == 0)
	  	kbd_scan();

	  if(count % adc_sample_rate == 0){
		//do ADC per 30ms 
		start_ADC1();
	  }
	  if(count % mems_alert_timeout == 0){
		if(MEMS_ALERT < MEMS_ALERT_TRIGER)
			MEMS_ALERT = 0;
	  }

	  if(count % ledseg_refresh_tm == 0){
	  	refresh_segleds();
	  }

#if 0

      //将PB.5管脚输出数值写入ReadValue
      ReadValue = GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_5);

      if(ReadValue == 0)
      {
         GPIO_SetBits(GPIOB,GPIO_Pin_5);
      }    
      else
      {
         GPIO_ResetBits(GPIOB,GPIO_Pin_5);      
      }
#endif
	  //printf("timeout");
   }
}





/*
void RCC_cfg()
{

      

       //定义错误状态变量

       ErrorStatus HSEStartUpStatus;

      

       //将RCC寄存器重新设置为默认值

       RCC_DeInit();

 

       //打开外部高速时钟晶振

       RCC_HSEConfig(RCC_HSE_ON);

 

       //等待外部高速时钟晶振工作

       HSEStartUpStatus = RCC_WaitForHSEStartUp();

       if(HSEStartUpStatus == SUCCESS)

       {

              //设置AHB时钟(HCLK)为系统时钟

              RCC_HCLKConfig(RCC_SYSCLK_Div1);

 

              //设置高速AHB时钟(APB2)为HCLK时钟

              RCC_PCLK2Config(RCC_HCLK_Div1);

 

              //设置低速AHB时钟(APB1)为HCLK的2分频

              RCC_PCLK1Config(RCC_HCLK_Div2);

             

              //设置FLASH代码延时

              FLASH_SetLatency(FLASH_Latency_2);

 

              //使能预取指缓存

              FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

 

              //设置PLL时钟，为HSE的9倍频 8MHz * 9 = 72MHz

              RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

 

              //使能PLL

              RCC_PLLCmd(ENABLE);

 

              //等待PLL准备就绪

              while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

 

              //设置PLL为系统时钟源

              RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

 

              //判断PLL是否是系统时钟

              while(RCC_GetSYSCLKSource() != 0x08);

       }

 

       //允许TIM2的时钟

       RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);

       //允许GPIO的时钟

       RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);

 

}
*/

