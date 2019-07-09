
#ifndef ADC_H
#define ADC_H


//MEMS RANK1~RANK3
#define ADC1_CH10_RANK1		1
#define ADC1_CH11_RANK2		2
#define ADC1_CH12_RANK3		3
#define ADC1_CH13_RANK4		4
#define ADC1_CH14_RANK5		5

#define ADC1_CH10_RANK1_NOTIFY	0x01
#define ADC1_CH11_RANK2_NOTIFY	0x02
#define ADC1_CH12_RANK3_NOTIFY	0x04
#define ADC1_CH13_RANK4_NOTIFY	0x08
#define ADC1_CH14_RANK5_NOTIFY	0x10


#define MEMS_THRESHOLD_DEFAULT		50  //ADC value

#define MEMS_ALERT_TRIGER	5


typedef void (*ADC_CALLBACK_FUNC)(uint8_t nofify_rank);


void ADC1_init(ADC_CALLBACK_FUNC nofify_callbk);
uint16_t get_ADC1_result(unsigned char rank);
void start_ADC1(void);
void set_ADC1_update_flag(void);
uint16_t get_ADC1_voltege_mv(unsigned char rank);
void set_MEMS_static_ADC_value(int16_t rk1, int16_t rk2, int16_t rk3);



#endif
