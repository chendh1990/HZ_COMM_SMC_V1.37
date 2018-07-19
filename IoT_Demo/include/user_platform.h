#ifndef __USER_PLATFORM_H__
#define __USER_PLATFORM_H__

u8 user_deal_restart (u8 ch);

void led_pwm_breath(void);

void led_display_poweron(void);

void led_display_work(void);

u8 user_check_restart (u8 msk);


#endif
