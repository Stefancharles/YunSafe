
#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#define KBD_HOLD_KEY_SCAN_ON


#define KEY_EVENT_BUF_SIZE		20

#define KBD_ROW_SUM		5	/* number of row */
#define KBD_COLUMN_SUM	5	/* number of column */
#define KBD_SAMPLE_COUNT	2	/* for debounce */
#define KBD_SAMPLE_COUNT_FOR_HOLD_KEY	50	/* for debounce */

//define key event
enum {
	KBD_KEY_DONW=0,
	KBD_KEY_UP=1,
	KBD_KEY_HOLD=2,
};



/* keyboard event struct */
typedef struct {
	unsigned char key_no;
	unsigned char key_event;
}KBD_EVENT;


typedef struct {	
	KBD_EVENT key_evt_buf[KEY_EVENT_BUF_SIZE];
	short evt_r_indx;
	short evt_w_indx;
}KEY_EVT_FIFO;

//define KEY num
enum KBD {
	KBD_KEY0 = 0,
	KBD_KEY1,
	KBD_KEY2,
	KBD_KEY3,
	KBD_KEY4,
	KBD_KEY5,
	KBD_KEY6,
	KBD_KEY7,
	KBD_KEY8,
	KBD_KEY9,
	KBD_UP,
	KBD_DOWN,
	KBD_LEFT,
	KBD_RIGHT,
	KBD_PROJ1,
	KBD_PROJ2,
	KBD_PROJ3,
	KBD_PROJ4,
	KBD_PROJ5,
	KBD_FUNC,
	KEY_HASH_KEY,
	KEY_KPASTERISK,
	KEY_BACKSPACE,
	KEY_ENTER,
	KEY_ESC,
	KEY_INVALID
};

int kbd_init(void);
void kbd_scan(void);
int init_kbd_GPIO(void);
int get_kbd_fifo(KBD_EVENT* evt);
void print_key_inf(KBD_EVENT* key);

#endif /* _KEYBOARD_H_ */

