////////////////////////////////////////////////
#include "user_config.h"



struct USER_RESTART_INFO User_Restart_Info;


u8 ICACHE_FLASH_ATTR
user_deal_restart (u8 ch)
{
	u8 sys_restart = 0;

	if(ch == 0)
	{
		User_Restart_Info.k0++;
		User_Restart_Info.k1 = 0;
		User_Restart_Info.k2 = 0;
		User_Restart_Info.k3 = 0;
	}
	else if(ch == 1)
	{
		User_Restart_Info.k0 = 0;
		User_Restart_Info.k1++;
		User_Restart_Info.k2 = 0;
		User_Restart_Info.k3 = 0;
	}
	else if(ch == 2)
	{
		User_Restart_Info.k0 = 0;
		User_Restart_Info.k1 = 0;
		User_Restart_Info.k2++;
		User_Restart_Info.k3 = 0;
	}
	else if(ch == 3)
	{
		User_Restart_Info.k0 = 0;
		User_Restart_Info.k1 = 0;
		User_Restart_Info.k2 = 0;
		User_Restart_Info.k3++;
	}
	else if(ch == 0XA0)
	{
		if(User_Restart_Info.k0 == 3)
		{
			sys_restart = 1;
		}
	}
	else if(ch == 0XA1)
	{
		if(User_Restart_Info.k1 == 3)
		{
			sys_restart = 1;
		}
	}
	else if(ch == 0XA2)
	{
		if(User_Restart_Info.k2 == 3)
		{
			sys_restart = 1;
		}
	}
	else if(ch == 0XA3)
	{
		if(User_Restart_Info.k3 == 3)
		{
			sys_restart = 1;
		}
	}

	if(ch >= 0XA0)
	{
		User_Restart_Info.k0 = 0;
		User_Restart_Info.k1 = 0;
		User_Restart_Info.k2 = 0;
		User_Restart_Info.k3 = 0;
	}

	return sys_restart;
}



u8 ICACHE_FLASH_ATTR
user_check_restart (u8 msk)
{
	u8 sys_restart;

	sys_restart = user_deal_restart(msk);
	ESP_DBG("sys_restart:%d \r\n",sys_restart);
	if(sys_restart)
	{
		os_printf("system_restart......\r\n");
		system_restart();
	}

	return sys_restart;
}


#if (USE_PC_PARAM_SET)
/******************************************************************************
 * FunctionName :
 * Description  :
 * Parameters   :
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_paltform_flash_record_param(void)
{
	LOCAL u16 i;

//	ESP_DBG("Flash_Test USER_PC_SET_FLASH_START_SEC:%d\r\n ",USER_PC_SET_FLASH_START_SEC);

	for(i=0;i<11;i++)			//信息，12个字节
	{
		flashBuffer[i] = dataBuffer[i+2];
	}
	flashBuffer[11] = 0XA5;
	flashBuffer[12] = 0XCC;

	ETS_UART_INTR_DISABLE();

	system_param_save_with_protect(USER_PC_SET_FLASH_START_SEC, flashBuffer, FLASH_BUF_SIZE);

	memset(flashBuffer,0,FLASH_BUF_SIZE);

	system_param_load(USER_PC_SET_FLASH_START_SEC,0,flashBuffer,FLASH_BUF_SIZE);

	ETS_UART_INTR_ENABLE();

	ESP_PC_DBG("\r\n");
	//check
	for(i=0;i<11;i++)
	{
		if(flashBuffer[i] != dataBuffer[i+2])
		{
			ESP_PC_DBG("PC set fail!\r\n");
			PCSetTransmit(0X00);
			return;
		}
		ESP_PC_DBG("%02X ",flashBuffer[i]);
	}

	ESP_PC_DBG("\r\n");

	ESP_PC_DBG("PC set done!\r\n");
	PCSetTransmit(0X01);
																	//Flash的11个字节为产品属性和滚码
	os_memcpy(Device_Info.p,flashBuffer,sizeof(Device_Info.p));		//拷贝产品属性
	os_memcpy(Device_Info.id+2,flashBuffer+7,4);					//设备ID的后面四个字节作为滚码
	peripheral_refresh_pinfo();
}
#endif

#define FLASH_TST 	0

#if FLASH_TST
void ICACHE_FLASH_ATTR
Flash_Test(void)
{
	LOCAL u16 i;


	ESP_DBG("Flash_Test USER_PC_SET_FLASH_START_SEC:%d\r\n ",USER_PC_SET_FLASH_START_SEC);

	for(i=0;i<sizeof(flashBuffer);i++)
	{
		flashBuffer[i] = 0XA5+i;
	}

	system_param_save_with_protect(USER_PC_SET_FLASH_START_SEC, flashBuffer, sizeof(flashBuffer));

	memset(flashBuffer,0,sizeof(flashBuffer));

	system_param_load(USER_PC_SET_FLASH_START_SEC,0,flashBuffer,sizeof(flashBuffer));


	for(i=0;i<sizeof(flashBuffer);i++)
	{
		ESP_DBG("%x ",flashBuffer[i]);
	}

	ESP_DBG("\r\n******************************************** \r\n ");
}
#endif

/**************************************************************************
 *
 *************************************************************************/
//#define LONG_TIME_TEST

#ifdef LONG_TIME_TEST

void ICACHE_FLASH_ATTR
LongTimeTest(void)
{
	LOCAL u16 timeCnt = 0;

	timeCnt++;

	if(timeCnt >= 300)
	{
		put_msg_fifo(PS_K2);
		put_msg_fifo(PS_K1);
		put_msg_fifo(PS_K0);
		timeCnt = 0;

	}

}
#endif


/**************************************************************************
 *上电的时候灯跑马
 *三个阶段：1-三个开关指示灯跑马 2-WIFI灯闪烁 3-全部灯关
 *之后user_platform_info.dispOnce = 2不会再运行此函数
 *************************************************************************/
void ICACHE_FLASH_ATTR
led_display_poweron(void)
{
	LOCAL u8 dispChannel;
	LOCAL u8 cycle;

	if(user_platform_info.dispOnce == 0)
	{
		dispChannel = 0;
		cycle = 4;
		user_platform_info.dispOnce = 1;
	}
	else if(user_platform_info.dispOnce == 1)
	{
		if(cycle > 1)									//阶段1：三个开关灯跑马
		{
			user_toggle_led_state(dispChannel);
			if(dispChannel < LED_CHANNEL_NUM-1)
			{
				dispChannel++;
			}
			else
			{
				cycle--;
				dispChannel = 0;
			}
		}
		else if(cycle == 1)								//阶段2：WIFI灯闪烁
		{
			if(dispChannel < 5)
			{
				dispChannel++;
			}
			else
			{
				cycle--;
				dispChannel = 0;
			}
			peripheral_led_wifi_ctrl(dispChannel&1);
		}
		else											//阶段3：全灯关
		{
			user_platform_info.dispOnce = 2;
			peripheral_led_wifi_ctrl(0);				//WIFI灯关
			memset(led_relay_state.led,0,sizeof(led_relay_state.led));		//all led off
		}

	}
}


void ICACHE_FLASH_ATTR led_display_work(void)
{
/*
 * 现在定为，进入WIFI配置模式，灯快闪，有2分钟超时，2分钟内无操作，退出，需要用户自己重新长按进入配置模式
 * 不是配置模式时，灯只指示开关状态
 *
 */
/*	static u8 flash1;
	static u8 flash2;
	flash1++;

	if((flash1%10) == 0)
	{
		flash2++;
		if(flash2&1)
		{
			GPIO9_H;
			GPIO10_H;
		}
		else
		{
			GPIO9_L;
			GPIO10_L;
		}
	}*/


	if(user_platform_info.dispOnce == 2)			//等到跑马完成
	{
		if(WiFi_Link_Info.in)
		{
			peripheral_led_wifi_flash(1);			//配置模式，快闪
		}
		else
		{
			if(user_platform_info.cloud_connect == TRUE) 	//连上云端
			{
				peripheral_led_wifi_ctrl(0);		//跟随开关状态
			}
			else
			{
				peripheral_led_wifi_ctrl(1);		//常亮
				//peripheral_led_wifi_flash(0);		//未连云端慢闪
			}
		}

		if(WiFi_Link_Info.inTmp != WiFi_Link_Info.in)
		{
			WiFi_Link_Info.inTmp = WiFi_Link_Info.in;
			peripheral_gpio_cmd_send(0X06,0X00,WiFi_Link_Info.in,0X00);
		}
#if 0 	//这里可以知道各种连接状态
		wifi_conn_sta = wifi_station_get_connect_status();
		if(wifi_conn_sta == STATION_GOT_IP)				//连上路由器了，灯不闪
		{
			peripheral_led_wifi_on();
		}
		else
		{
			if(WiFi_Link_Info.in)
			{
				peripheral_led_wifi_flash(1);			//配置模式，快闪
			}
			else
			{
				peripheral_led_wifi_flash(0);			//配置好了，但是没连上路由器，慢闪
			}

		}
#endif
	}


}

/* *************************************************************************
 * 呼吸灯处理函数
 *************************************************************************/
void ICACHE_FLASH_ATTR
led_pwm_breath(void)
{
	if(light_param_u.inc)
	{
		if(light_param_u.pwm_duty[1] < 990)
		{
			light_param_u.pwm_duty[1]+=5;
		}
		else
		{
			light_param_u.inc = 0;
		}
	}
	else
	{
		if(light_param_u.pwm_duty[1] > 6)
		{
			light_param_u.pwm_duty[1]-=5;
		}
		else
		{
			light_param_u.inc = 1;
		}
	}
	pwm_set_duty(light_param_u.pwm_duty[1]*22, 1);
	pwm_start();
}



