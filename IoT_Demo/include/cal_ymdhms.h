#ifndef __CAL_YMDHM_H__
#define	__CAL_YMDHM_H__


#define YEAR_INIT 	2012
#define MON_INIT	1
#define DAY_INIT	1
#define HOUR_INIT	8
#define MIN_INIT	0
#define SEC_INIT	0

#define CAL_YEAR_BASE 1970


#define YEAR		(DAY*365)
#define DAY			(HOUR*24)
#define HOUR		(MINUTE*60)
#define MINUTE		60L 

#define LEAP_YEAR_YES	1
#define LEAP_YEAR_NO	0

typedef struct TIME_INFO___
{
    u8 leap_year;
	u8 second;
    u8 minute;
    u8 hour;
    u16 day;
    u8 month;
    u16 year;
	u8 week;
} TIME_INFO; ///<时间结构体

//声明变量
extern TIME_INFO time_info;

//声明函数
extern void ymdhms_init(void) ;//large;
extern u32 Date2Sec( TIME_INFO *tm) ;//large;
extern void Sec2Date(TIME_INFO * tm, u32 time) ;//large ;
extern void judge_leap_year(void) ;//large;
extern void CalWeek(void);//large;

#endif


