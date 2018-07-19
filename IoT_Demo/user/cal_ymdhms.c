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



/*****����һ��ȫ�ֵĽṹ��*********/
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
   @brief �жϵ���Ϊƽ�껹������
   @param[in] �������
   @return �����־��1Ϊ���꣬0Ϊƽ��
*/
/*----------------------------------------------------------------------------*/
void judge_leap_year(void)
{	//�ܱ�4�����Ҳ��ܱ�100�������߿��Ա�400����Ϊ����
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
/**@brief ��Date2Sec������ԣ���ÿ���ۼƵ�count ת���� ����
   @param [in] tm �� yy-mm-dd-hh-mm-ss �ṹ�����͵�ʱ��
   @param [in] ��������,���TIME
   @return ��
   @note Sec2Date 
*/
/*----------------------------------------------------------------------------*/
void Sec2Date(TIME_INFO * tm, u32 time)
{

    u32 res = time;                                 /*��������*/      

    tm->year = 0;                                  /*������*/
    /*
        get year
     */
    while (1)
    {
        /*******************���괦��*****************************************/
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
        /*********************ƽ�괦��**********************************/
        else
            if (res >= YEAR)
            {
                res -= YEAR;
                tm->year++;
            }
            else
                break;
    }
	
	tm->year += CAL_YEAR_BASE;//������ĳ�ʼֵ
	
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
/**@brief ����������,�������
          
   @param 
   @return �����ۼƵ���ֵ
   @note Date2Sec
*/
/*----------------------------------------------------------------------------*/
u32 Date2Sec( TIME_INFO *tm)
{
    u32 res;                           /*������*/
    u8 year,month_tmp;

    month_tmp = tm->month;             /*ȡ�ṹ�������*/
    month_tmp--;                       /*��ȥ��ǰ��*/
    year = tm->year;
		year -=  CAL_YEAR_BASE;// ��ȥ��ĳ�ʼֵ                 

    res = YEAR*year + DAY*((year+3)/4);/*��������ܷ�����(��������)*/
    res += month[month_tmp];           /*���ӵ�����µĴ�ŷ�����*/
    
    /*ϸ���������µ�����*/
    if (month_tmp>1 && ((year)%4))
        res -= DAY;
    res += DAY*(tm->day-1);
    res += HOUR*tm->hour;
    res += MINUTE*tm->minute;
    res += tm->second;
    return res;

}
/*----------------------------------------------------------------------------*/
/**@brief ���ݵ�ǰ�����ڣ��ж϶�Ӧ������,������浽��Ӧ�Ľṹ������
   @param 
   @return none
   @note Date2Sec
*/
/*----------------------------------------------------------------------------*/
void CalWeek(void)
{
	u16	 y_4,runs;//runs��ʾ����
	u8  y_100,y_400;
	u32  AllDays;
	
	y_4=(time_info.year-1)/4;//curt.yy��ʾ��ǰ���
	y_100=(u8)((time_info.year-1)/100);
	y_400=(u8)((time_info.year-1)/400);
	runs=y_4-(y_100-y_400);

	judge_leap_year();//�ж�����

	if(time_info.leap_year!=1 && time_info.month > 2)
	{//�ܹ��ж�����
		AllDays=366ul*runs+365ul*(time_info.year-runs-1)+( month[time_info.month-1] / DAY )-1+time_info.day;
	}
	else//Ϊ����
	{//�ܹ��ж�����
		AllDays=366ul*runs+365ul*(time_info.year-runs-1)+( month[time_info.month-1] / DAY )+time_info.day;
	}
	
	time_info.week = AllDays % 7;
}

