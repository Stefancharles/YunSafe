

#ifndef RTC_H
#define RTC_H

typedef void (*RTC_CALLBACK_FUNC)(uint32_t rtc_counter_val);
void rtc_init(RTC_CALLBACK_FUNC cbp);
void Time_Adjust(uint8_t hour, uint8_t minute, uint8_t second);

#endif
