
#ifndef __DOOR_H
#define __DOOR_H

extern unsigned char FlagDefense;//防御标志，1-布防，0-撤防
extern unsigned char FlagLockSta;//锁舌状态，1-开锁，0-关锁
extern unsigned long int LockTimeCount;//锁舌打开时间计时

void doorInit(void);
unsigned char isDoorOpen(void);

void doorOpen(void);
	
#endif

