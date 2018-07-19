/*--------------------------------------------------------------------------*/
/**@file   	 cal_ymdhms
   @brief 	 time 	trasnsition
   @details 
   @author 	 
   @date   	
   @note   
*/
/*----------------------------------------------------------------------------*/

/******************************************************************************

*******************************************************************************/
#include "user_config.h"



/*****声明一个全局的结构体*********/
TIME_INFO time_info;

const u32 month[12] =
{
    0,
    DAY*(31),
    DAY*(31+29),
    DAY*(31+29+31),
    DAY*(31+29+31+30),
    DAY*(31+29+31+30+31),
    DAY*(31+29+31+30+31+30),
    DAY*(31+29+31+30+31+30+31),
    DAY*(31+29+31+30+31+30+31+31),
    DAY*(31+29+31+30+31+30+31+31+30),
    DAY*(31+29+31+30+31+30+31+31+30+31),
    DAY*(31+29+31+30+31+30+31+31+30+31+30)
};

void ymdhms_init(void)
{
	time_info.year 	= 	YEAR_INIT;
	time_info.month = 	MON_INIT;
	time_info.day 	= 	DAY_INIT;
	time_info.hour 	= 	HOUR_INIT;
	time_info.minute = 	MIN_INIT;
	time_info.second = 	SEC_INIT;
}

/*----------------------------------------------------------------------------*/
/**
   @brief 判断当年为平年还是闰年
   @param[in] 当年年份
   @return 闰年标志；1为闰年，0为平年
*/
/*----------------------------------------------------------------------------*/
void judge_leap_year(void)
{	//能被4整除且不能被100整除或者可以被400整除为闰年
	if(((time_info.year%4==0)&&(time_info.year%100!=0))||(time_info.year%400==0))
	{
		time_info.leap_year = LEAP_YEAR_YES;
	}
	else
	{	
		time_info.leap_year = LEAP_YEAR_NO;
	}
}

/*----------------------------------------------------------------------------*/
/**@brief 和Date2Sec函数相对，把每秒累计的count 转换成 日期
   @param [in] tm 是 yy-mm-dd-hh-mm-ss 结构体类型的时间
   @param [in] 输入秒数,输出TIME
   @return 无
   @note Sec2Date 
*/
/*----------------------------------------------------------------------------*/
void Sec2Date(TIME_INFO * tm, u32 time)
{

    u32 res = time;                                 /*总秒钟数*/      

    tm->year = 0;                                  /*年清零*/
    /*
        get year
     */
    while (1)
    {
        /*******************闰年处理*****************************************/
        if (((tm->year % 4 == 0) && (tm->year % 100 != 0)) || (tm->year % 400 == 0))
        {
            if ( res >= (YEAR + DAY))
            {
                res -= (YEAR + DAY);
                tm->year++;
            }
            else
                break;
        }
        /*********************平年处理**********************************/
        else
            if (res >= YEAR)
            {
                res -= YEAR;
                tm->year++;
            }
            else
                break;
    }
	
	tm->year += CAL_YEAR_BASE;//加上年的初始值
	
    /*
     * get month
     */
    tm->month = 12;

    while (1)
    {
        if ( ((tm->year % 4 == 0) && (tm->year % 100 != 0)) || (tm->year % 400 == 0))
        {
            if (res >= month[tm->month - 1])
            {
                res -=  month[tm->month - 1];
                break;
            }
            else

                tm->month--;
        }
        else
        {
            if (tm->month > 2)
            {
                if (res >= (month[tm->month - 1] - DAY))
                {
                    res -= (month[tm->month - 1] - DAY);
                    break;
                }
                else
                    tm->month--;
            }
            else
            {
                if (res >=month[tm->month - 1])
                {
                    res -= month[tm->month - 1];
                    break;
                }
                else
                    tm->month--;
            }
        }
    }

    /*
     * get day, the first day is 1
     */
    tm->day = (res / DAY + 1);
    res %= DAY;

    /*
     * get hour
     */
    tm->hour = res / HOUR;
    res %= HOUR;

    /*
     * get minute
     */
    tm->minute = res / MINUTE;
    res %= MINUTE;

    /*
     * get second
     */
    tm->second = res;
	
	CalWeek();
}


/*----------------------------------------------------------------------------*/
/**@brief 输入年月日,输出秒数
          
   @param 
   @return 以秒累计的数值
   @note Date2Sec
*/
/*----------------------------------------------------------------------------*/
u32 Date2Sec( TIME_INFO *tm)
{
    u32 res;                           /*总秒数*/
    u8 year,month_tmp;

    month_tmp = tm->month;             /*取结构体的月数*/
    month_tmp--;                       /*减去当前月*/
    year = tm->year;
		year -=  CAL_YEAR_BASE;// 减去年的初始值                 

    res = YEAR*year + DAY*((year+3)/4);/*计算年的总分钟数(包括闰月)*/
    res += month[month_tmp];           /*增加当年的月的大概分钟数*/
    
    /*细调当年闰月的天数*/
    if (month_tmp>1 && ((year)%4))
        res -= DAY;
    res += DAY*(tm->day-1);
    res += HOUR*tm->hour;
    res += MINUTE*tm->minute;
    res += tm->second;
    return res;

}
/*----------------------------------------------------------------------------*/
/**@brief 根据当前的日期，判断对应的星期,结果保存到相应的结构体里面
   @param 
   @return none
   @note Date2Sec
*/
/*----------------------------------------------------------------------------*/
void CalWeek(void)
{
	u16	 y_4,runs;//runs表示闰年
	u8  y_100,y_400;
	u32  AllDays;
	
	y_4=(time_info.year-1)/4;//curt.yy表示当前年份
	y_100=(u8)((time_info.year-1)/100);
	y_400=(u8)((time_info.year-1)/400);
	runs=y_4-(y_100-y_400);

	judge_leap_year();//判断闰年

	if(time_info.leap_year!=1 && time_info.month > 2)
	{//总共有多少天
		AllDays=366ul*runs+365ul*(time_info.year-runs-1)+( month[time_info.month-1] / DAY )-1+time_info.day;
	}
	else//为闰年
	{//总共有多少天
		AllDays=366ul*runs+365ul*(time_info.year-runs-1)+( month[time_info.month-1] / DAY )+time_info.day;
	}
	
	time_info.week = AllDays % 7;
}

