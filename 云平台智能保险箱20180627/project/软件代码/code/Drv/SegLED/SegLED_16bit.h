
#ifndef SEGLED_H
#define SEGLED_H

#define SEGLED_GPIO			GPIOC

#define SEGLED_DATA_PIN		GPIO_Pin_10
#define SEGLED_SCK_PIN		GPIO_Pin_11
#define SEGLED_RCK_PIN		GPIO_Pin_12

#define SEGLED_PINS		(SEGLED_DATA_PIN | SEGLED_RCK_PIN | SEGLED_SCK_PIN)


void init_74HC595_for_segleds(void);
void refresh_segleds(void) ;
int set_segleds_bit(uint8_t led_bit, uint8_t data);


#endif
