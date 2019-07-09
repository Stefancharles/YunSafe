#ifndef _PUBLIC_H_
#define _PUBLIC_H_

#include "stm32f103_config.h"

uint8_t lrc_chk(uint8_t *buf, uint8_t len);
uint8_t rev_datapage(uint8_t *buf, uint8_t *len);
void send_datapag(uint8_t *buf);
void set_mem(uint8_t *buf, uint8_t len, uint8_t val);
void uartsend(uint8_t *buf, uint8_t len);

#endif

