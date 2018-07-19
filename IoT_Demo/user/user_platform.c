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

	for(i=0;i<11;i++)			//��Ϣ��12���ֽ�
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
																	//Flash��11���ֽ�Ϊ��Ʒ���Ժ͹���
	os_memcpy(Device_Info.p,flashBuffer,sizeof(Device_Info.p));		//������Ʒ����
	os_memcpy(Device_Info.id+2,flashBuffer+7,4);					//�豸ID�ĺ����ĸ��ֽ���Ϊ����
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
 *�ϵ��ʱ�������
 *�����׶Σ�1-��������ָʾ������ 2-WIFI����˸ 3-ȫ���ƹ�
 *֮��user_platform_info.dispOnce = 2���������д˺���
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
		if(cycle > 1)									//�׶�1���������ص�����
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
		else if(cycle == 1)								//�׶�2��WIFI����˸
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
		else											//�׶�3��ȫ�ƹ�
		{
			user_platform_info.dispOnce = 2;
			peripheral_led_wifi_ctrl(0);				//WIFI�ƹ�
			memset(led_relay_state.led,0,sizeof(led_relay_state.led));		//all led off
		}

	}
}


void ICACHE_FLASH_ATTR led_display_work(void)
{
/*
 * ���ڶ�Ϊ������WIFI����ģʽ���ƿ�������2���ӳ�ʱ��2�������޲������˳�����Ҫ�û��Լ����³�����������ģʽ
 * ��������ģʽʱ����ָֻʾ����״̬
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


	if(user_platform_info.dispOnce == 2)			//�ȵ��������
	{
		if(WiFi_Link_Info.in)
		{
			peripheral_led_wifi_flash(1);			//����ģʽ������
		}
		else
		{
			if(user_platform_info.cloud_connect == TRUE) 	//�����ƶ�
			{
				peripheral_led_wifi_ctrl(0);		//���濪��״̬
			}
			else
			{
				peripheral_led_wifi_ctrl(1);		//����
				//peripheral_led_wifi_flash(0);		//δ���ƶ�����
			}
		}

		if(WiFi_Link_Info.inTmp != WiFi_Link_Info.in)
		{
			WiFi_Link_Info.inTmp = WiFi_Link_Info.in;
			peripheral_gpio_cmd_send(0X06,0X00,WiFi_Link_Info.in,0X00);
		}
#if 0 	//�������֪����������״̬
		wifi_conn_sta = wifi_station_get_connect_status();
		if(wifi_conn_sta == STATION_GOT_IP)				//����·�����ˣ��Ʋ���
		{
			peripheral_led_wifi_on();
		}
		else
		{
			if(WiFi_Link_Info.in)
			{
				peripheral_led_wifi_flash(1);			//����ģʽ������
			}
			else
			{
				peripheral_led_wifi_flash(0);			//���ú��ˣ�����û����·����������
			}

		}
#endif
	}


}

/* *************************************************************************
 * �����ƴ�����
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



