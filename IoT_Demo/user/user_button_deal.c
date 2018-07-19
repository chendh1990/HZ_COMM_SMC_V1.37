////////////////////////////////////////////////
#include "user_config.h"


LOCAL os_timer_t timer_curtain;


/******************************************************************************
 * FunctionName :
 * Description  :
 * Parameters   :
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_led_on_run (void)
{
	u8 i;

	//中间键为开关，两边按下灯亮一段时间后关闭
	if( (((char *)os_strstr(Device_Info.info, "ADJ")) != NULL) || (((char *)os_strstr(Device_Info.info, "Curtain1T")) != NULL) || (((char *)os_strstr(Device_Info.info, "Window1T")) != NULL))
	{
		for(i=0;i<LED_CHANNEL_NUM;i++)
		{
			if(i != 1)
			{
				if(led_relay_state.ledOnCnt[i])
				{
					led_relay_state.ledOnCnt[i]--;
					led_relay_state.led[i] = 1;
				}
				else
				{
					led_relay_state.led[i] = 0;
				}
			}

		}
	}

	//情景面板，所有按键都是按下短时间内亮一下
	if( ((char *)os_strstr(Device_Info.info, "Scene")) != NULL)
	{
		for(i=0;i<LED_CHANNEL_NUM;i++)
		{
			if(led_relay_state.ledOnCnt[i])
			{
				led_relay_state.ledOnCnt[i]--;
				led_relay_state.led[i] = 1;
			}
			else
			{
				led_relay_state.led[i] = 0;
			}

		}
	}
}


void ICACHE_FLASH_ATTR
TimerCurtainCallback (u8 channel)
{
	os_timer_disarm(&timer_curtain);
	led_relay_state.ledOnCnt[channel] = CURTAIN_LED_ON_TIME;	//旁边两个键亮一会
}

void ICACHE_FLASH_ATTR
TimerCurtainManualControl(u8 channel)
{
	u8 i;
	u8 needDelay = 0;

	if(led_relay_state.ledOnCnt[channel] == 0)	//如果此灯关闭，则打开
	{
		for(i=0;i<LED_CHANNEL_NUM;i++)
		{
			if(i != channel)
			{
				if(led_relay_state.ledOnCnt[i])
				{
					led_relay_state.ledOnCnt[i] = 0;
					needDelay = 1;
				}
			}
		}

		if(needDelay)	//如果其他灯打开，先把其他灯关了，然后延迟再打开此灯
		{
			os_timer_disarm(&timer_curtain);
			os_timer_setfn(&timer_curtain, (os_timer_func_t *)TimerCurtainCallback, channel);
			os_timer_arm(&timer_curtain, 1000, 0);
		}
		else			//如果其他灯没打开，立即打开
		{
			led_relay_state.ledOnCnt[channel] = CURTAIN_LED_ON_TIME;	//旁边两个键亮一会
		}
	}
	else				//如果此灯打开，则关闭
	{
		led_relay_state.ledOnCnt[channel] = 0;
	}
}

void ICACHE_FLASH_ATTR
TimerCurtainCloudControl(u8 channel,u8 onoff)
{
	u8 i;
	u8 needDelay = 0;


	if(onoff)
	{
		for(i=0;i<LED_CHANNEL_NUM;i++)
		{
			if(i != channel)
			{
				if(led_relay_state.ledOnCnt[i])
				{
					led_relay_state.ledOnCnt[i] = 0;
					needDelay = 1;
				}
			}
		}
		if(needDelay)	//如果其他灯打开，先把其他灯关了，然后延迟再打开此灯
		{
			os_timer_disarm(&timer_curtain);
			os_timer_setfn(&timer_curtain, (os_timer_func_t *)TimerCurtainCallback, channel);
			os_timer_arm(&timer_curtain, 1000, 0);
		}
		else			//如果其他灯没打开，立即打开
		{
			os_timer_disarm(&timer_curtain);
			led_relay_state.ledOnCnt[channel] = CURTAIN_LED_ON_TIME;	//旁边两个键亮一会
		}
	}
	else				//如果此灯打开，则关闭
	{
		led_relay_state.ledOnCnt[channel] = 0;
	}

}

/******************************************************************************
 * FunctionName :
 * Description  :
 * Parameters   :
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_toggle_led_state (u8 channel)
{


	if( ((char *)os_strstr(DEV_INFO_STRING, ("ADJ"))) != NULL)		//调光开关    ##########################################
	{

		if(channel == 1)									//中间键开关
		{
			if(led_relay_state.led[channel])
			{
				led_relay_state.led[channel] = 0;
			}
			else
			{
				led_relay_state.led[channel] = 1;
			}
		}
		else
		{
			if(led_relay_state.led[1])
			{
				if(channel == 2)
				{
					if(led_relay_state.lumin < ADJ_MAX_LUMIN)
					{
						led_relay_state.lumin++;
					}
					led_relay_state.ledOnCnt[0] = ADJ_LED_ON_TIME;		//旁边两个键亮一会
				}
				else if(channel == 0)
				{
					if(led_relay_state.lumin > 0)
					{
						led_relay_state.lumin--;
					}
					led_relay_state.ledOnCnt[2] = ADJ_LED_ON_TIME;		//旁边两个键亮一会
				}
			}
		}

		peripheral_send_to_adj_controller();	//发送给STM8控制器
	}
	else if( ((char *)os_strstr(DEV_INFO_STRING, "Scene")) != NULL)		//情景面板   ##########################################
	{
		led_relay_state.ledOnCnt[channel] = SCENE_LED_ON_TIME;		//旁边两个键亮一会
	}
	else if( (((char *)os_strstr(DEV_INFO_STRING, "Curtain1T")) != NULL) || (((char *)os_strstr(DEV_INFO_STRING, "Window1T")) != NULL) )		//单轨窗帘   ##########################################
	{
		TimerCurtainManualControl(channel);
	}
	else													//普通开关    #########################################
	{
		if(led_relay_state.led[channel])
		{
			led_relay_state.led[channel] = 0;
		}
		else
		{
			led_relay_state.led[channel] = 1;
		}
	}
}



/**************************************************************************
 *
 *************************************************************************/
void ICACHE_FLASH_ATTR
user_press_deal (u8 channel)
{
	u8 wifi_conn_sta;

	wifi_conn_sta = wifi_station_get_connect_status();

//	ESP_DBG2("wifi_conn_sta:%d \r\n",wifi_conn_sta);

	user_toggle_led_state(channel);
	hz_shut_cnt_down(channel);

	if( (user_platform_info.cloud_connect == TRUE) && (wifi_conn_sta == STATION_GOT_IP) )
	{
		user_esp_restart_heartbeat_timer();

		if( ((char *)os_strstr(DEV_INFO_STRING, "ADJ")) != NULL)	//调光开关
		{


			if(led_relay_state.lumin != HZ_Misc_Buf.lumin[0])
			{
				HZ_Misc_Buf.lumin[0] = led_relay_state.lumin;
				hz_up_resp_lumin(1);
			}

			if(led_relay_state.adjOn != led_relay_state.led[1])
			{
				led_relay_state.adjOn = led_relay_state.led[1];
				hz_up_resp_lumin(1);
			}

		}
		else if( ((char *)os_strstr(DEV_INFO_STRING, "Scene")) != NULL)	//情景面板
		{
			hz_up_resp_scene(channel);
		}
		else if( (((char *)os_strstr(DEV_INFO_STRING, "Curtain1T")) != NULL) || (((char *)os_strstr(DEV_INFO_STRING, "Window1T")) != NULL) )	//
		{
			hz_up_resp_Curtain1T(channel);
		}
		else	//普通开关
		{
			hz_up_sw_status();
		}
	}
}
