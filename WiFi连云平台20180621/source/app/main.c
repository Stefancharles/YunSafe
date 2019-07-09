
#include "stm32f103_config.h"
#include "stm32App.h"

int main(void)
{	
	delay_init();
	delay_ms(100);//延时100Ms待系统稳定
	USART1_Init(BAUD_RATE);
	UART4_Init(BAUD_RATE);
	Buzzer_Init();//或LED_Init();IO口都是PA8
	TIM2_Init();
	printf("Cortex-M3操作WiFi通讯模块连接云平台\r\n");
	HalBuzzerBlink (HAL_BUZZER_0, 1, 50, 500);

	userApp();
}




