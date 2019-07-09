
#include <stdio.h>
#include <string.h>
#include "stm32f10x.h"
#include "keyboard_drv.h"
#include "systick.h"
#include "LCD12864.h"
#include "timer.h"

const int32_t kbd_scan_tm = (10000/TIME2_PERIOD_US);  //10ms

#define KBD_ROW_PA_PINS	(GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4)
#define KBD_COLUMN_PB_PINS	(GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14)

uint16_t kbd_columns[KBD_COLUMN_SUM] = {GPIO_Pin_10, GPIO_Pin_11, GPIO_Pin_12, GPIO_Pin_13, GPIO_Pin_14};

static unsigned char kbd_status_buf1[KBD_ROW_SUM] [KBD_COLUMN_SUM];
static unsigned char kbd_status_buf2[KBD_ROW_SUM] [KBD_COLUMN_SUM];
static unsigned char kbd_status[KBD_ROW_SUM] [KBD_COLUMN_SUM];
static KEY_EVT_FIFO evt_fifo;


unsigned char key_code_cnvt[KBD_ROW_SUM*KBD_COLUMN_SUM + 1] = {
	KEY_INVALID,
	KBD_PROJ5,
	KEY_BACKSPACE,
	KBD_KEY6,
	KEY_ESC,
	KEY_ENTER,
	KBD_PROJ4,
	KBD_KEY3,
	KBD_KEY5,
	KBD_KEY9,
	KEY_HASH_KEY,
	KBD_PROJ3,
	KBD_KEY2,
	KBD_KEY4,
	KBD_KEY8,
	KBD_KEY0,
	KBD_PROJ2,
	KBD_KEY1,
	KBD_RIGHT,
	KBD_KEY7,
	KEY_KPASTERISK,
	KBD_PROJ1,
	KBD_UP,
	KBD_LEFT,
	KBD_DOWN,
	KBD_FUNC,
};


int kbd_init(void)
{
	//init keyboard data
	memset(kbd_status_buf1, KBD_KEY_UP, sizeof(kbd_status_buf1));
	memset(kbd_status_buf2, KBD_KEY_UP, sizeof(kbd_status_buf2));
	memset(kbd_status, KBD_KEY_UP, sizeof(kbd_status));
	memset((void*)&evt_fifo, 0, sizeof(evt_fifo));

	init_kbd_GPIO();

	return 0;
}


int init_kbd_GPIO(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);

	GPIO_InitStructure.GPIO_Pin = KBD_ROW_PA_PINS;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = KBD_COLUMN_PB_PINS;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//set all column gpios to 0
	GPIO_ResetBits(GPIOB, KBD_COLUMN_PB_PINS);

	return 0;
}


unsigned char is_kbd_fifo_empt(void)
{
	return evt_fifo.evt_r_indx == evt_fifo.evt_w_indx;
}

unsigned char kbd_fifo_len(void)
{
	if(evt_fifo.evt_w_indx > evt_fifo.evt_r_indx){
		return evt_fifo.evt_w_indx - evt_fifo.evt_r_indx;
	}else{
		return evt_fifo.evt_w_indx - evt_fifo.evt_r_indx + KEY_EVENT_BUF_SIZE;
	}
}


int put_kbd_fifo(KBD_EVENT* evt)
{
	memcpy((void*)&evt_fifo.key_evt_buf[evt_fifo.evt_w_indx], (void*)evt, sizeof(KBD_EVENT));
	evt_fifo.evt_w_indx = (++evt_fifo.evt_w_indx) % KEY_EVENT_BUF_SIZE;

	return 0;
}

int get_kbd_fifo(KBD_EVENT* evt)
{
	if(is_kbd_fifo_empt())
		return -1;
	
	memcpy((void*)evt, (void*)&evt_fifo.key_evt_buf[evt_fifo.evt_r_indx], sizeof(KBD_EVENT));
	evt_fifo.evt_r_indx = (++evt_fifo.evt_r_indx) % KEY_EVENT_BUF_SIZE;
	
	return 0;
}


void kbd_scan(void)
{
	int i;
	int j;
	static unsigned char(*kbd_now)[KBD_COLUMN_SUM] = kbd_status_buf1;
	static unsigned char(*kbd_prev)[KBD_COLUMN_SUM] = kbd_status_buf2;
	static unsigned char(*p_tmp)[KBD_COLUMN_SUM];
	static unsigned char kbd_same_cnt[KBD_ROW_SUM] [KBD_COLUMN_SUM] = {{0}};
	static unsigned char key_evt_flag = 0;
	static unsigned char act_flag = 0;
	KBD_EVENT evt;
	unsigned char chg_flag = 0;
	unsigned long kbd_data;

	#ifdef KBD_HOLD_KEY_SCAN_ON
	static unsigned char hold_key_cnt = KBD_SAMPLE_COUNT_FOR_HOLD_KEY;
	#endif
	

	if (key_evt_flag == 0 && act_flag == 0) {
		//set all column gpios to 0
		GPIO_ResetBits(GPIOB, KBD_COLUMN_PB_PINS);

		if ((GPIO_ReadInputData(GPIOA) & 0x1f) == 0x1f) {

			//no key dow
			return;
		}
	}

	//read the now status
	for(i = 0; i < KBD_ROW_SUM; i++) {
		//set all columns to 1
		GPIO_SetBits(GPIOB, KBD_COLUMN_PB_PINS);

		//set column(0~4 data
		 GPIO_ResetBits(GPIOB, kbd_columns[i]);
			
		//delay 1 us, make the signal stable
		delay_us(1);

		kbd_data = GPIO_ReadInputData(GPIOA);
		for(j = 0; j < KBD_COLUMN_SUM; j++) {
			kbd_now[i][j] = (kbd_data >> j) & 0x01;
		}
	}

#if 0
	printf("\r\n\r\n");
	for(i = 0; i < KBD_ROW_SUM; i++) {
		for(j = 0; j < KBD_COLUMN_SUM; j++) {
			printf("%d  ",kbd_now[i][j]);
		}
		printf("\r\n");
	}
	printf("\r\n\r\n");
#endif

	for(i = 0; i < KBD_ROW_SUM; i++) {
		for(j = 0; j < KBD_COLUMN_SUM; j++) {

			//now status is same with prior status, then calculate the same times
			if (kbd_now[i][j] == kbd_prev[i][j]) {

				//keyboard status is not changed, set the same count to MAX
				if ( ++kbd_same_cnt[i][j] > KBD_SAMPLE_COUNT_FOR_HOLD_KEY ) {
					kbd_same_cnt[i][j] = KBD_SAMPLE_COUNT_FOR_HOLD_KEY;
				}

				//the same count must greater than KBD_SAMPLE_COUNT to guarantee the status is effective
				if ( (kbd_same_cnt[i][j] >= KBD_SAMPLE_COUNT) && (kbd_now[i][j] != kbd_status[i][j]) ) {

					//keep the real status
					kbd_status[i][j] = kbd_now[i][j];

					//keep key event cout
					if (kbd_status[i][j] == KBD_KEY_DONW) {
						key_evt_flag++;
					} else {
						key_evt_flag--;
						#ifdef KBD_HOLD_KEY_SCAN_ON
						hold_key_cnt = KBD_SAMPLE_COUNT_FOR_HOLD_KEY;
						#endif
					}

					evt.key_no = i*KBD_COLUMN_SUM + j + 1;
					evt.key_event = kbd_status[i][j];

					//put the keyboard evnet to buffer
					evt.key_no = key_code_cnvt[evt.key_no];
					put_kbd_fifo(&evt);
					
					//printf("keyboard event key:%d event: %d\n", evt.key_no, evt.key_event);

				}
				#ifdef KBD_HOLD_KEY_SCAN_ON
				else if ( (kbd_same_cnt[i][j] >= hold_key_cnt) && (kbd_now[i][j] == KBD_KEY_DONW) ){

					evt.key_no = i*KBD_COLUMN_SUM + j + 1;
					evt.key_event = KBD_KEY_HOLD;

					//put the keyboard evnet to buffer
					evt.key_no = key_code_cnvt[evt.key_no];
					put_kbd_fifo(&evt);

					//keyboard status is changed, then count the new status
					kbd_same_cnt[i][j] = 1;
					hold_key_cnt = KBD_SAMPLE_COUNT * 2;
					
					//printf("keyboard event key:%d event: %d\r\n", evt.key_no, evt.key_event);
				}
				#endif
			} else {

				//keyboard status is changed, then count the new status
				kbd_same_cnt[i][j] = 1;

				chg_flag = 1;
			}
		}
	}

	if (chg_flag) {
		//keep the now status
		p_tmp = kbd_now;
		kbd_now = kbd_prev;
		kbd_prev = p_tmp;

		//keyboard has action
		act_flag = 1;
	} else {
		//keyboard not has  action
		act_flag = 0;
	}
}

void print_key_inf(KBD_EVENT* key)
{
	char str[LCD_COLUMN_NUM+1];

	memset(str, 0, sizeof(str));
	lcd_clr_row(0);
	
	if(key->key_no <= 9){
		if(key->key_event == KBD_KEY_DONW){
			sprintf(str, "Key %d down", key->key_no);
		}else{
			sprintf(str, "Key %d up", key->key_no);
		}
		lcd_write(0, 0, str, strlen(str));
	}else{
		switch(key->key_no){
		case KBD_UP :
			if(key->key_event == KBD_KEY_DONW){
				sprintf(str, "UP KEY down");
			}else{
				sprintf(str, "UP KEY up");
			}
			break;
		case KBD_DOWN :
			if(key->key_event == KBD_KEY_DONW){
				sprintf(str, "DOWN KEY down");
			}else{
				sprintf(str, "DOWN KEY up");
			}
			break;
		case KBD_LEFT :
			if(key->key_event == KBD_KEY_DONW){
				sprintf(str, "LEFT KEY down");
			}else{
				sprintf(str, "LEFT KEY up");
			}
			break;
		case KBD_RIGHT :
			if(key->key_event == KBD_KEY_DONW){
				sprintf(str, "RIGHT KEY down");
			}else{
				sprintf(str, "RIGHT KEY up");
			}
			break;
		case KBD_PROJ1 :
			if(key->key_event == KBD_KEY_DONW){
				sprintf(str, "PROJ1 KEY down");
			}else{
				sprintf(str, "PROJ1 KEY up");
			}
			break;
		case KBD_PROJ2 :
			if(key->key_event == KBD_KEY_DONW){
				sprintf(str, "PROJ2 KEY down");
			}else{
				sprintf(str, "PROJ2 KEY up");
			}
			break;
		case KBD_PROJ3 :
			if(key->key_event == KBD_KEY_DONW){
				sprintf(str, "PROJ3 KEY down");
			}else{
				sprintf(str, "PROJ3 KEY up");
			}
			break;
		case KBD_PROJ4 :
			if(key->key_event == KBD_KEY_DONW){
				sprintf(str, "PROJ4 KEY down");
			}else{
				sprintf(str, "PROJ4 KEY up");
			}
			break;
		case KBD_PROJ5 :
			if(key->key_event == KBD_KEY_DONW){
				sprintf(str, "PROJ5 KEY down");
			}else{
				sprintf(str, "PROJ5 KEY up");
			}
			break;
		case KBD_FUNC :
			if(key->key_event == KBD_KEY_DONW){
				sprintf(str, "FUNC KEY down");
			}else{
				sprintf(str, "FUNC KEY up");
			}
			break;
		case KEY_HASH_KEY :
			if(key->key_event == KBD_KEY_DONW){
				sprintf(str, "HASH KEY down");
			}else{
				sprintf(str, "HASH KEY up");
			}
			break;
		case KEY_KPASTERISK :
			if(key->key_event == KBD_KEY_DONW){
				sprintf(str, "KPASTERISK KEY down");
			}else{
				sprintf(str, "KPASTERISK KEY up");
			}
			break;
		case KEY_BACKSPACE :
			if(key->key_event == KBD_KEY_DONW){
				sprintf(str, "BACKSPACE KEY down");
			}else{
				sprintf(str, "BACKSPACE KEY up");
			}
			break;
		case KEY_ENTER :
			if(key->key_event == KBD_KEY_DONW){
				sprintf(str, "ENTER KEY down");
			}else{
				sprintf(str, "ENTER KEY up");
			}
			break;
		case KEY_ESC :
			if(key->key_event == KBD_KEY_DONW){
				sprintf(str, "ESC KEY down");
			}else{
				sprintf(str, "ESC KEY up");
			}
			break;
		default:
			break;
		}
		lcd_write(0, 0, str, strlen(str));
	}
}

