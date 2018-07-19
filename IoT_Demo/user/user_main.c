/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2014/1/1, v1.0 create this file.
*******************************************************************************/
#include "user_config.h"








///////////////////////////////////////////////////////////////////////////////////////
LOCAL struct station_config s_staconf;

void ICACHE_FLASH_ATTR
platform_web_init(void)
{
	user_esp_platform_init();

#ifdef LAN_OTA
#ifdef SERVER_SSL_ENABLE
	user_webserver_init(SERVER_SSL_PORT);
#else
	ESP_DBG("user_webserver_init\r\n");
	user_webserver_init(SERVER_PORT);
#endif
#endif
}


void ICACHE_FLASH_ATTR
user_scan_done(void *arg, STATUS status)
{
	if(status == OK)
	{
		ESP_DBG("user scan done!!!\r\n\r\n");
		platform_web_init();
	}
	else
	{
		ESP_DBG("user scan fail!!!\r\n\r\n");
		wifilink_set_ap_server();
	}
}


/*
 *
 * 结构体struct scan_config *config;带指针，传入函数的时候不用&，参数要加->而不是.
 *
 * */

void ICACHE_FLASH_ATTR
user_scan(void)
{
   struct scan_config config;

   os_memset(&config, 0, sizeof(config));

   config.ssid = s_staconf.ssid;

   wifi_station_scan(&config, user_scan_done);
}


void ICACHE_FLASH_ATTR
wifi_connect_direct(void)
{
   // Wifi configuration
   char ssid[32] = "FANTASTIC SMART";
   char password[64] = "qq123456qq";
   struct station_config stationConf;

   os_memset(stationConf.ssid, 0, 32);
   os_memset(stationConf.password, 0, 64);

   //need not mac address
   stationConf.bssid_set = 0;

   //Set ap settings
   os_memcpy(&stationConf.ssid, ssid, 32);
   os_memcpy(&stationConf.password, password, 64);

   ESP_DBG("WIFI connect direct:%s %s \r\n" , stationConf.ssid , stationConf.password);

   wifi_set_opmode_current(STATION_MODE);

   wifi_station_set_config(&stationConf);
}

/******************************************************************************
 * FunctionName : user_set_station_config
 * Description  : set the router info which ESP8266 station will connect to
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_set_station_config(void)
{
#if 0

	wifi_connect_direct();

	system_init_done_cb(platform_web_init);

#else
    wifi_set_opmode_current(STATION_MODE);
    wifi_station_get_config_default(&s_staconf);

    wifilink_set_wifi_done_callback_register(platform_web_init);

    {
#if 0
    	char ssid[32] = "ch2015";
		char password[64] = "ch201506"; 
		s_staconf.bssid_set = 0;
		os_memcpy(&s_staconf.ssid, ssid, os_strlen(ssid));
		os_memcpy(&s_staconf.password, password, os_strlen(password));
		wifi_station_set_config(&s_staconf);
#endif
    }
    if(os_strlen(s_staconf.ssid) != 0)
    {
      ESP_DBG("Startup user_scan...\r\n");
      system_init_done_cb(user_scan);
    }
    else
    {
    	/*局域网设置模式*/
    	system_init_done_cb(wifilink_set_ap_server);
    	/*smartlink方式*/
/*      ESP_DBG("smartconfig...... \r\n");
      smartconfig_set_type(SC_TYPE_ESPTOUCH); //SC_TYPE_ESPTOUCH,SC_TYPE_AIRKISS,SC_TYPE_ESPTOUCH_AIRKISS
      wifi_set_opmode(STATION_MODE);
      smartconfig_start(smartconfig_done);*/
    }
#endif
}

#define WIFI_CHANNEL	13
void ICACHE_FLASH_ATTR user_ap_mode(void)
{
    struct softap_config config;			//首先定义一个soft_config的结构体
    uint8 opmode;

    wifi_set_opmode(SOFTAP_MODE);			//设置为AP模式
    opmode = wifi_get_opmode_default();

    wifi_softap_get_config(&config);		//这个函数的参数是soft_config的结构体，记得取地址

    os_sprintf(config.ssid,"ESP AP %d",WIFI_CHANNEL);
    config.authmode = AUTH_OPEN;
    config.channel = WIFI_CHANNEL;

    wifi_softap_set_config(&config);		//完成AP模式的参数配置，记得取地址

    user_udp_task_init();
}

void ICACHE_FLASH_ATTR
user_flash_delay(void)
{
	u16 i = 5000;

	while(i--);
}

/********************************************************************
须放在peripheral_set_device_info();和peripheral_decode_dev_p();后面
********************************************************************/
void ICACHE_FLASH_ATTR
user_flash_read_param_power_on(void)
{
	LOCAL u16 i;

	ESP_DBG_FUN_START();

	user_platform_info.dispOnce = 0;

	memset(flashBuffer,0,FLASH_BUF_SIZE);

//	ETS_UART_INTR_DISABLE();

	system_param_load(USER_POLICY_TIME_START_SEC,0,flashBuffer,FLASH_BUF_SIZE);

	ESP_DBG("Power on read F_MASK:%02X %02X %02X %02X \r\n",flashBuffer[F_MASK_START_ADDR],
															flashBuffer[F_MASK_START_ADDR+1],
															flashBuffer[F_MASK_START_ADDR+2],
															flashBuffer[F_MASK_START_ADDR+3]);

	if((flashBuffer[F_MASK_START_ADDR] != F_MASK0)
	|| (flashBuffer[F_MASK_START_ADDR+1] != F_MASK1)
	|| (flashBuffer[F_MASK_START_ADDR+2] != F_MASK2)
	|| (flashBuffer[F_MASK_START_ADDR+3] != F_MASK3))
	{
		ESP_DBG("Power on <<RESET>> hz_clear_policy_time \r\n");
		memset(flashBuffer,0,FLASH_BUF_SIZE);
		flashBuffer[F_MASK_START_ADDR+0] = F_MASK0;
		flashBuffer[F_MASK_START_ADDR+1] = F_MASK1;
		flashBuffer[F_MASK_START_ADDR+2] = F_MASK2;
		flashBuffer[F_MASK_START_ADDR+3] = F_MASK3;
		//保存定时数据到Flash
		system_param_save_with_protect(USER_POLICY_TIME_START_SEC,flashBuffer,FLASH_BUF_SIZE);
		hz_clear_policy_time();
	}
	else
	{
		ESP_DBG("Power on <<LOAD>> Policy_Time_Info.buf \r\n");
		system_param_load(USER_POLICY_TIME_START_SEC,0,Policy_Time_Info.buf,sizeof(Policy_Time_Info.buf));
#if POW_ON_DISP_ONECE
		user_platform_info.dispOnce = 2;
#endif
	}

//	ETS_UART_INTR_ENABLE();

	ESP_DBG("Power on -> Policy_Time_Info.buf: \r\n");
#if 1
	for(i=0;i<HZ_TIME_CTRL_BUF_SIZE;i++)
	{
		UART_ShowByteInCharHex(Policy_Time_Info.buf[i],9);
		ESP_DBG("\r\n");
	}
#endif
	user_flash_delay();

/**************************************************************************************
	memset(flashBuffer,0,FLASH_BUF_SIZE);

	system_param_load(USER_PC_SET_FLASH_START_SEC,0,flashBuffer,FLASH_BUF_SIZE);

	if((flashBuffer[11] == 0XA5) && (flashBuffer[12] == 0XCC))				//如果设置过产品属性，则改变它
	{
		ESP_DBG("Power on load param: \r\n");

		UART_ShowByteInCharHex(flashBuffer,11);
		ESP_DBG("\r\n");
		os_memcpy(Device_Info.p,flashBuffer,sizeof(Device_Info.p));		//拷贝产品属性
		os_memcpy(Device_Info.id+2,flashBuffer+7,4);					//设备ID的后面四个字节作为滚码

		ESP_DBG("Device_Info.p: \r\n");
		UART_ShowByteInCharHex(Device_Info.p,7);

		ESP_DBG("\r\n");
		ESP_DBG("Device_Info.id: \r\n");
		UART_ShowByteInCharHex(Device_Info.id,6);
		ESP_DBG("\r\n");
		ESP_DBG("roll code:%d\r\n",Device_Info.id[5]<<24|Device_Info.id[4]<<16|Device_Info.id[3]<<8|Device_Info.id[2]<<0);

		peripheral_refresh_pinfo();
	}
***************************************************************************************/

	ESP_DBG("-----------------------------------------\r\n");
	ESP_DBG("Product real information:%s \r\n",Device_Info.info);
	ESP_DBG("-----------------------------------------\r\n");
	ESP_DBG_FUN_END();
}


void ICACHE_FLASH_ATTR
USER_DISPLAY_BANNER(void)
{

//*********************************************************************************************
	os_printf("\r\n\r\n\r\n\r\n");
	os_printf("-----------------------------------------\r\n");
	os_printf("www.szhanzhi.cn\r\n");
	os_printf("Copyright 2017 Fantastic Smart Corporation. All Rights Reserved.\r\n");
	os_printf("-----------------------------------------\r\n");
//*********************************************************************************************
	ESP_DBG("\r\n");
	ESP_DBG("*******************ESP8266 FW Start************************\r\n");
	ESP_DBG("\r\n*********************\r\nIOT Version: V1.%d \r\n*********************\r\n",FW_VER);
    ESP_DBG("Compile time:%s %s\r\n",__DATE__,__TIME__);
    ESP_DBG("***********************************************************\r\n");
    ESP_DBG("Prodcut information default:%s \r\n",DEV_INFO_STRING);
    ESP_DBG("***********************************************************\r\n\r\n\r\n");

    {
    	u16 i;
    	u8 *Buffer;
    	u8 k = 3;

    	RingbufferInit();

#if (QUEUE_ALLOC_FIFO_TEST_EN)
    	QueueAllocTest();
#endif

    	Buffer = (u8 *)UDPTask.flash;

#if (FLASH_PROTECT_TEST)
    	for(k=1;k<4;k++)
    	{
    		Buffer[0] = k*10+k;
			Buffer[1] = 0X55;
			Buffer[2] = 0X66;
			Buffer[3] = 0XBB;
			Buffer[4] = 0XCC;
			Buffer[5] = 0XDD;

			system_param_save_with_protect(USER_TEST_START_SEC, Buffer, USER_FLASH_SEC_SIZE);

			spi_flash_read(USER_TEST_START_SEC*USER_FLASH_SEC_SIZE,UDPTask.flash, USER_FLASH_SEC_SIZE);


			ESP_DBG("&&&&&&&&&& flash wit protect 0 &&&&&&&&&&\r\n");
			for(i=0;i<32;i++)
			{
				ESP_DBG("%02X " , Buffer[i]);
			}
			ESP_DBG("\r\n");


			spi_flash_read((USER_TEST_START_SEC+1)*USER_FLASH_SEC_SIZE,UDPTask.flash, USER_FLASH_SEC_SIZE);


			ESP_DBG("&&&&&&&&&& flash wit protect 1 &&&&&&&&&&\r\n");
			for(i=0;i<32;i++)
			{
				ESP_DBG("%02X " , Buffer[i]);
			}
			ESP_DBG("\r\n");


			spi_flash_read((USER_TEST_START_SEC+2)*USER_FLASH_SEC_SIZE,UDPTask.flash, USER_FLASH_SEC_SIZE);


			ESP_DBG("&&&&&&&&&& flash wit protect 2 &&&&&&&&&&\r\n");
			for(i=0;i<32;i++)
			{
				ESP_DBG("%02X " , Buffer[i]);
			}
			ESP_DBG("\r\n");


			spi_flash_read((USER_TEST_START_SEC+3)*USER_FLASH_SEC_SIZE,UDPTask.flash, USER_FLASH_SEC_SIZE);


			ESP_DBG("&&&&&&&&&& flash wit protect 3 &&&&&&&&&&\r\n");
			for(i=0;i<32;i++)
			{
				ESP_DBG("%02X " , Buffer[i]);
			}
			ESP_DBG("\r\n");
    	}
#endif

    	for(i=0;i<10;i++)
    	{
    		if(i == 5)
    		{
    			break;
    		}
    	}

    	ESP_DBG("&&&&&&&&&& : %d \r\n" , i);

    	for(i=0;i<10;i++)
    	{
    		if(i == 9)
    		{
    			break;
    		}
    	}

    	ESP_DBG("&&&&&&&&&& : %d \r\n" , i);

    	for(i=0;i<10;i++)
    	{
    	}

    	ESP_DBG("&&&&&&&&&& : %d \r\n" , i);
    }


//    ESP_DBG2("chip id:%X \r\n",system_get_chip_id());
//    ESP_DBG2("**********************************************\r\n");
}



/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{


	uart_init(USER_BIT_RATE,USER_BIT_RATE);

	if( ((char *)os_strstr(DEV_INFO_STRING, "ADJ")) != NULL)	//调光开关用
	{
		uart_init(19200,19200);
	}

	USER_DISPLAY_BANNER();
	//------------------------------------------------------------------------------

    peripheral_set_device_info();

    peripheral_decode_dev_p();

    user_flash_read_param_power_on();	//必须放在peripheral_set_device_info();和peripheral_decode_dev_p();后面

    MD5_main();

    KeyGPIOInit();

    peripheral_init();

    pwm_Init();

    msg_stack_clear();

    tttt();
	// 检测当前运行的 userbin
    if (system_upgrade_userbin_check() == UPGRADE_FW_BIN1)
	{
    	ESP_DBG("\r\n===========================UPGRADE_FW_BIN1============================\r\n");
    }
	else if (system_upgrade_userbin_check() == UPGRADE_FW_BIN2)
    {
		ESP_DBG("\r\n===========================UPGRADE_FW_BIN2============================\r\n");
    }

	hz_initial_count_down();

	user_platform_info.cloud_connect = FALSE;

	user_platform_info.w_timeout = 0;

	WiFi_Link_Info.in = 0;

	user_udp_task_init();

	user_esp_platform_create_task();

	// 使设备连接上路由器
//	user_ota_connect_to_router(AP_SSID, AP_PASSWD);
//	pwm_Init();
	user_set_station_config();

//	user_ap_mode();
}

