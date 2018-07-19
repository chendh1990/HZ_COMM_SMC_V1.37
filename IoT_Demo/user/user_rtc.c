/*--------------------------------------------------------------------------*/
/**@file   	 user_rtc
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

RTC_TIMER_DEMO rtc_time;

/* *************************************************************************
 * 复位RTC信息
 *************************************************************************/
void ICACHE_FLASH_ATTR rtc_reset_time(void)
{
	ESP_DBG("\r\nrtc_reset_time!\r\n\r\n");
	memset(&rtc_time,0,sizeof(rtc_time));
//	system_rtc_mem_write(64, &rtc_time, sizeof(rtc_time));
}

#if (USE_CN_TIME_POOL)
/* *************************************************************************
 * RTC读取时间戳
 *************************************************************************/
u32 ICACHE_FLASH_ATTR rtc_read_stamp(void)
{
	uint32 rtc_t1;
	uint32 cal1;
	u32 rtc_timestamp;
	u32 current_stamp;

	rtc_t1 = system_get_rtc_time();									//获取当前计数
	cal1 = system_rtc_clock_cali_proc();							//获取校准周期

	rtc_time.time_acc += (((uint64)(rtc_t1 - rtc_time.time_base)) *( (uint64)((cal1*1000)>>12))) ;
	rtc_time.time_acc = ((rtc_time.time_acc/10000000)/100);			//把累加值转换为秒

	if((rtc_time.time_acc%60) == 0)						//每60秒从网络更新时间戳
	{
		current_stamp = sntp_get_current_timestamp();
		if(current_stamp)
		{
			rtc_time.bias = rtc_time.time_acc;
			rtc_time.rtc_stamp = current_stamp;
			ESP_DBG("\r\nsntp_stamp:%lld \r\n",current_stamp);
			ESP_DBG("rtc_time.bias:%d \r\n",rtc_time.bias);
		}
	}

	rtc_timestamp = rtc_time.time_acc-rtc_time.bias+rtc_time.rtc_stamp;

	if((rtc_time.time_acc%60) == 0)
	{
		ESP_DBG("rtc time acc : %lld \r\n",rtc_time.time_acc);
		ESP_DBG("RTC Time:%s \r\n",sntp_get_real_time(rtc_timestamp));
	}


	return rtc_timestamp;
}


/* *************************************************************************
 * 第一次上电的时候RTC执行的操作
 *************************************************************************/
void ICACHE_FLASH_ATTR rtc_tick_tock(void)
{
	uint32 rtc_t1;
	uint32 cal1;
	uint32 current_stamp;
	u32 rtc_timestamp;

	if(!user_platform_info.cloud_connect) return;

	if(rtc_time.tsFlag && (rtc_time.magic == RTC_MAGIC)) return;

//	system_rtc_mem_read(64, &rtc_time, sizeof(rtc_time));
	if(rtc_time.magic!=RTC_MAGIC)						//冷启动，设置一个时间基准
	{
		rtc_time.magic = RTC_MAGIC;
		rtc_time.time_acc= 0;							//累加为0
		rtc_time.time_base = system_get_rtc_time();		//设置一个时间基准
//		system_rtc_mem_write(64, &rtc_time, sizeof(rtc_time));
		ESP_DBG("\r\n\r\ncold set up time base : %d \r\n",rtc_time.time_base);
	}


	rtc_t1 = system_get_rtc_time();						//获取当前计数
	cal1 = system_rtc_clock_cali_proc();				//获取校准周期

	rtc_time.time_acc += (((uint64)(rtc_t1 - rtc_time.time_base)) *( (uint64)((cal1*1000)>>12))) ;
	rtc_time.time_acc = ((rtc_time.time_acc/10000000)/100);	//把累加值转换为秒
	ESP_DBG("rtc time acc : %lld \r\n",rtc_time.time_acc);

	if(!rtc_time.tsFlag)
	{
		current_stamp = sntp_get_current_timestamp();
		if(current_stamp)
		{
			rtc_time.tsFlag = 1;
			rtc_time.bias = rtc_time.time_acc;
			rtc_time.rtc_stamp = current_stamp;
			ESP_DBG("\r\nGet current_stamp:%lld \r\n",current_stamp);
			ESP_DBG("rtc_time.bias:%d \r\n",rtc_time.bias);
		}
	}
	else
	{
		rtc_timestamp = rtc_time.time_acc-rtc_time.bias+rtc_time.rtc_stamp;
		ESP_DBG("RTC Time:%s \r\n",sntp_get_real_time(rtc_timestamp));
	}
}

#else


#if 0
/* *************************************************************************
 * RTC读取时间戳
 *************************************************************************/
u32 ICACHE_FLASH_ATTR rtc_read_stamp(void)
{
	uint32 rtc_t1;
	uint32 cal1;
	u32 rtc_timestamp;
	u32 current_stamp;

	rtc_t1 = system_get_rtc_time();									//获取当前计数
	cal1 = system_rtc_clock_cali_proc();							//获取校准周期

	rtc_time.time_acc += (((uint64)(rtc_t1 - rtc_time.time_base)) *( (uint64)((cal1*1000)>>12))) ;
	rtc_time.time_acc = ((rtc_time.time_acc/10000000)/100);			//把累加值转换为秒

	rtc_timestamp = rtc_time.time_acc-rtc_time.bias+rtc_time.rtc_stamp;

	return rtc_timestamp;
}


/* *************************************************************************
 * RTC校准时间戳
 *************************************************************************/
void ICACHE_FLASH_ATTR rtc_calibration_stamp(u32 current_stamp)
{
	uint32 rtc_t1;
	uint32 cal1;
	u32 rtc_timestamp;

	rtc_t1 = system_get_rtc_time();									//获取当前计数
	cal1 = system_rtc_clock_cali_proc();							//获取校准周期

	rtc_time.time_acc += (((uint64)(rtc_t1 - rtc_time.time_base)) *( (uint64)((cal1*1000)>>12))) ;
	rtc_time.time_acc = ((rtc_time.time_acc/10000000)/100);			//把累加值转换为秒

	rtc_time.bias = rtc_time.time_acc;
	rtc_time.rtc_stamp = current_stamp;
	ESP_DBG("\r\nsntp_stamp:%lld \r\n",current_stamp);
	ESP_DBG("rtc_time.bias:%d \r\n",rtc_time.bias);

	rtc_timestamp = rtc_time.time_acc-rtc_time.bias+rtc_time.rtc_stamp;

	ESP_DBG("rtc time acc : %lld \r\n",rtc_time.time_acc);
	ESP_DBG("RTC Time:%s \r\n",sntp_get_real_time(rtc_timestamp));
}



/* *************************************************************************
 * 第一次上电的时候RTC执行的操作
 *************************************************************************/
void ICACHE_FLASH_ATTR rtc_tick_tock(u32 current_stamp)
{
	uint32 rtc_t1;
	uint32 cal1;
	u32 rtc_timestamp;

	if(!user_platform_info.cloud_connect) return;

	if(rtc_time.tsFlag && (rtc_time.magic == RTC_MAGIC)) return;

//	system_rtc_mem_read(64, &rtc_time, sizeof(rtc_time));
	if(rtc_time.magic!=RTC_MAGIC)						//冷启动，设置一个时间基准
	{
		rtc_time.magic = RTC_MAGIC;
		rtc_time.time_acc= 0;							//累加为0
		rtc_time.time_base = system_get_rtc_time();		//设置一个时间基准
//		system_rtc_mem_write(64, &rtc_time, sizeof(rtc_time));
		ESP_DBG("\r\n\r\ncold set up time base : %d \r\n",rtc_time.time_base);
	}


	rtc_t1 = system_get_rtc_time();						//获取当前计数
	cal1 = system_rtc_clock_cali_proc();				//获取校准周期

	rtc_time.time_acc += (((uint64)(rtc_t1 - rtc_time.time_base)) *( (uint64)((cal1*1000)>>12))) ;
	rtc_time.time_acc = ((rtc_time.time_acc/10000000)/100);	//把累加值转换为秒
	ESP_DBG("rtc time acc : %lld \r\n",rtc_time.time_acc);

	if(!rtc_time.tsFlag)
	{
		rtc_time.tsFlag = 1;
		rtc_time.bias = rtc_time.time_acc;
		rtc_time.rtc_stamp = current_stamp;
		ESP_DBG("\r\nGet current_stamp:%lld \r\n",current_stamp);
		ESP_DBG("rtc_time.bias:%d \r\n",rtc_time.bias);

	}
	else
	{
		rtc_timestamp = rtc_time.time_acc-rtc_time.bias+rtc_time.rtc_stamp;
		ESP_DBG("RTC Time:%s \r\n",sntp_get_real_time(rtc_timestamp));
	}
}


void ICACHE_FLASH_ATTR rtc_cali_when_connect_cloud(void)
{
	rtc_time.caliInterval = (RTC_CALI_INTERVAL-10);	//10*300ms = 3000ms
}

void ICACHE_FLASH_ATTR rtc_cali_interval_run(void)
{//300ms调用一次
	if(rtc_time.caliInterval++ >= RTC_CALI_INTERVAL)	//300000
	{
		rtc_time.caliInterval = 0;
		rtc_time.reqStamp = 1;
		hz_request_timestamp();
	}
}
#else
/* *************************************************************************
 * RTC读取时间戳
 *************************************************************************/
u32 ICACHE_FLASH_ATTR rtc_read_stamp(void)
{
	uint32 rtc_t;
	uint32 cal;
	u64 rtc_timestamp;

	rtc_t = system_get_rtc_time();								//获取当前计数
	cal = system_rtc_clock_cali_proc();							//获取校准周期

	rtc_time.time_acc += (((uint64)(rtc_t - rtc_time.time_base)) *( (uint64)((cal*1000)>>12)));
	
	rtc_time.rtc_stamp = ((rtc_time.time_acc/10000000)/100);			//把累加值转换为秒

	rtc_timestamp = rtc_time.rtc_stamp - rtc_time.bias + rtc_time.cal_stamp;

	rtc_time.time_base = rtc_t;

	return rtc_timestamp;
}


/* *************************************************************************
 * RTC校准时间戳
 *************************************************************************/
void ICACHE_FLASH_ATTR rtc_calibration_stamp(u32 cal_stamp)
{
	uint32 rtc_t;
	uint32 cal;
	u32 rtc_timestamp;

	rtc_t = system_get_rtc_time();								//获取当前计数
	cal = system_rtc_clock_cali_proc();							//获取校准周期

	rtc_time.time_acc += (((uint64)(rtc_t - rtc_time.time_base)) *( (uint64)((cal*1000)>>12))) ;
	rtc_time.rtc_stamp  = ((rtc_time.time_acc/10000000)/100);			//把累加值转换为秒

	rtc_time.bias = rtc_time.rtc_stamp;
	rtc_time.cal_stamp = cal_stamp;
	rtc_time.time_base = rtc_t;
	
	ESP_DBG("\r\ncal_stamp:%lld \r\n",cal_stamp);
	ESP_DBG("rtc_time.bias:%d \r\n",rtc_time.bias);

	rtc_timestamp = rtc_time.rtc_stamp - rtc_time.bias + rtc_time.cal_stamp;

	ESP_DBG("rtc time rtc_stamp : %lld \r\n",rtc_time.rtc_stamp);
	ESP_DBG("RTC Time:%s \r\n",sntp_get_real_time(rtc_timestamp));
}

/* *************************************************************************
 * 第一次上电的时候RTC执行的操作
 *************************************************************************/
void ICACHE_FLASH_ATTR rtc_tick_tock(u32 cal_stamp)
{
	uint32 rtc_t;
	uint32 cal;
	u32 rtc_timestamp;

	if(!user_platform_info.cloud_connect) return;

	if(rtc_time.tsFlag && (rtc_time.magic == RTC_MAGIC)) return;

//	system_rtc_mem_read(64, &rtc_time, sizeof(rtc_time));
	if(rtc_time.magic!=RTC_MAGIC)						//冷启动，设置一个时间基准
	{
		rtc_time.magic = RTC_MAGIC;
		rtc_time.time_acc= 0;							//累加为0
		rtc_time.time_base = system_get_rtc_time();		//设置一个时间基准
//		system_rtc_mem_write(64, &rtc_time, sizeof(rtc_time));
		ESP_DBG("\r\n\r\ncold set up time base : %d \r\n",rtc_time.time_base);
	}


	rtc_t = system_get_rtc_time();						//获取当前计数
	cal = system_rtc_clock_cali_proc();				//获取校准周期

	rtc_time.time_acc += (((uint64)(rtc_t - rtc_time.time_base)) *( (uint64)((cal*1000)>>12))) ;
	rtc_time.rtc_stamp = ((rtc_time.time_acc/10000000)/100);	//把累加值转换为秒
	ESP_DBG("rtc rtc_stamp : %lld \r\n",rtc_time.rtc_stamp);

	if(!rtc_time.tsFlag)
	{
		rtc_time.tsFlag = 1;
		rtc_time.bias = rtc_time.rtc_stamp;
		rtc_time.cal_stamp = cal_stamp;
		ESP_DBG("\r\nGet cal_stamp:%lld \r\n",cal_stamp);
		ESP_DBG("rtc_time.bias:%d \r\n",rtc_time.bias);

	}
	else
	{
		rtc_timestamp = rtc_time.rtc_stamp - rtc_time.bias + rtc_time.cal_stamp;
		ESP_DBG("RTC Time:%s \r\n",sntp_get_real_time(rtc_timestamp));
	}
}


void ICACHE_FLASH_ATTR rtc_cali_when_connect_cloud(void)
{
	rtc_time.caliInterval = (RTC_CALI_INTERVAL-10);	//10*300ms = 3000ms
}

void ICACHE_FLASH_ATTR rtc_cali_interval_run(void)
{//300ms调用一次
	if(rtc_time.caliInterval++ >= RTC_CALI_INTERVAL)	//300000
	{
		rtc_time.caliInterval = 0;
		rtc_time.reqStamp = 1;
		hz_request_timestamp();
	}
}

#endif

#endif

void ICACHE_FLASH_ATTR
rtc_print(void)
{
	u32 rtc_timestamp;
	rtc_timestamp = rtc_read_stamp();
	Sec2Date(&time_info,rtc_timestamp);
	ESP_DBG("RTC Time:%d %02d-%02d %02d:%02d:%02d \r\n",time_info.year,
														time_info.month,
														time_info.day,
														time_info.hour,
														time_info.minute,
														time_info.second );

}

void ICACHE_FLASH_ATTR
user_sntp_init(void)
{
	sntp_setservername(0,"0.cn.pool.ntp.org");
	sntp_setservername(1,"1.tw.pool.ntp.org");
	sntp_setservername(2,"2.cn.pool.ntp.org");
	sntp_init();
}
