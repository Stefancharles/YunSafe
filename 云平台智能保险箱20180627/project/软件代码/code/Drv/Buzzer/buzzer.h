
#ifndef __BUZZER_H
#define __BUZZER_H

extern unsigned char buzzer_on;

void buzzerInit(void);
void buzzerOption(unsigned short cycle_us, unsigned short val);
void buzzerOpen(void);
void buzzerClose(void);

#endif

