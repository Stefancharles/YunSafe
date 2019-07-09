/**
  ******************************************************************************
  * @file    LCD12864.h
  * @author  newlab edu
  ******************************************************************************
**/
#ifndef LCD_HC12864C
#define LCD_HC12864C


#define GPIO_PD0	GPIO_Pin_0
#define GPIO_PD1	GPIO_Pin_1
#define GPIO_PD2	GPIO_Pin_2
#define GPIO_PD3	GPIO_Pin_3
#define GPIO_PD4 	GPIO_Pin_4
#define GPIO_PD5 	GPIO_Pin_5
#define GPIO_PD7 	GPIO_Pin_7
#define GPIO_PD8	GPIO_Pin_8
#define GPIO_PD9	GPIO_Pin_9
#define GPIO_PD10	GPIO_Pin_10
#define GPIO_PD11	GPIO_Pin_11
#define GPIO_PD12	GPIO_Pin_12
#define GPIO_PD13	GPIO_Pin_13
#define GPIO_PD14	GPIO_Pin_14
#define GPIO_PD15	GPIO_Pin_15


#define GPIO_PE7  	GPIO_Pin_7
#define GPIO_PE8  	GPIO_Pin_8
#define GPIO_PE9  	GPIO_Pin_9
#define GPIO_PE10 	GPIO_Pin_10
#define GPIO_PE11 	GPIO_Pin_11
#define GPIO_PE12 	GPIO_Pin_12
#define GPIO_PE13 	GPIO_Pin_13
#define GPIO_PE14 	GPIO_Pin_14
#define GPIO_PE15 	GPIO_Pin_15


#define LCD_BL		GPIO_PD7  	//CS
#define LCD_RST		GPIO_PD10	//D15
#define LCD_CS2		GPIO_PD9	//D14
#define LCD_CS1		GPIO_PD8	//D13
#define LCD_E   	GPIO_PE7	//D4
#define LCD_RW		GPIO_PD1	//D3
#define LCD_RS		GPIO_PD0	//D2
#define LCD_DATA0	GPIO_PE8	//D5
#define LCD_DATA1	GPIO_PE9	//D6
#define LCD_DATA2	GPIO_PE10	//D7
#define LCD_DATA3	GPIO_PE11	//D8
#define LCD_DATA4	GPIO_PE12	//D9
#define LCD_DATA5	GPIO_PE13	//D10
#define LCD_DATA6	GPIO_PE14	//D11
#define LCD_DATA7	GPIO_PE15	//D12

#define CONCTROLLER_CS1		1
#define CONCTROLLER_CS2		2

#define LCD_PAGE_ROW_NUM		8

#define CHAR_ROW_PIXEL			16
#define CHAR_COLUMN_PIXEL		8
#define LCD_ROW_PIXEL_SUM			64
#define LCD_COLUMN_PIXEL_SUM		128
#define LCD_ROW_NUM				(LCD_ROW_PIXEL_SUM/CHAR_ROW_PIXEL)
#define LCD_COLUMN_NUM				(LCD_COLUMN_PIXEL_SUM/CHAR_COLUMN_PIXEL)

void show_logo(void);
int lcd_write(unsigned char row, unsigned char column, char* data, unsigned char len);
void lcd_init (void);
void lcd_clr(void);
void lcd_clr_row(unsigned char row);

#endif

