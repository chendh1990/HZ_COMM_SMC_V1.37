#ifndef __USER_RTC_H__
#define	__USER_RTC_H__


//****************************************************************************
#define RTC_MAGIC 0x55aaaa55

#define USE_CN_TIME_POOL 	0

#define RTC_CALI_INTERVAL 	1000 //300ms * 1000 = 300s

//****************************************************************************
typedef struct {
	uint64 time_acc;
	uint32 magic ;
	uint32 time_base;
	u8 tsFlag;
	u32 bias;
	u32 cal_stamp;
	u32 rtc_stamp;
	u32 hz_stamp;
	u8 reqStamp;
	u16 caliInterval;
}RTC_TIMER_DEMO;

extern RTC_TIMER_DEMO rtc_time;


//****************************************************************************
void user_sntp_init(void);
void rtc_print(void);

#if (USE_CN_TIME_POOL)
void rtc_tick_tock(void);
u32 rtc_read_stamp(void);
#else
void rtc_cali_when_connect_cloud(void);
u32 rtc_read_stamp(void);
void rtc_calibration_stamp(u32 cal_stamp);
void rtc_tick_tock(u32 current_stamp);
#endif
void rtc_reset_time(void);



//****************************************************************************

#endif


