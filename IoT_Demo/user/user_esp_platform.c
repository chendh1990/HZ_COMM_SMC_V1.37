/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_esp_platform.c
 *
 * Description: The client mode configration.
 *              Check your hardware connection with the host while use this mode.
 *
 * Modification history:
 *     2014/5/09, v1.0 create this file.
*******************************************************************************/
#include "user_config.h"

#if ESP_PLATFORM

#ifdef USE_DNS
ip_addr_t esp_server_ip;
#else
//外网服务器
//const char esp_server_ip[4] = {119, 23, 33, 34};
//吴工IP
//const char esp_server_ip[4] = {192, 168, 1, 16};
//本机IP
const char esp_server_ip[4] = {192, 168, 1, 59};
#endif

LOCAL struct espconn esp_platform_tcp_client;
LOCAL struct _esp_tcp user_tcp;
LOCAL os_timer_t client_timer;
LOCAL os_timer_t heart_jump_timer;
LOCAL os_timer_t timer1;
LOCAL os_timer_t timer_message_task;


struct rst_info rtc_info;
struct USER_EPSPLATFORM_INFO user_platform_info;

LOCAL struct station_config stationInFlash;

u8 flashBuffer[FLASH_BUF_SIZE];

void user_esp_platform_check_ip(void);
void user_espconn_sent(uint8 *psent, uint16 length);

/******************************************************************************
 * FunctionName : user_esp_platform_reconnect
 * Description  : reconnect with host after get ip
 * Parameters   : pespconn -- the espconn used to reconnect with host
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_reconnect(struct espconn *pespconn)
{
    ESP_DBG("user_esp_platform_reconnect\n");

    user_platform_info.cloud_connect = FALSE;

    user_esp_platform_check_ip();
}

/******************************************************************************
 * FunctionName : user_esp_platform_discon_cb
 * Description  : disconnect successfully with the host
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_discon_cb(void *arg)
{
    struct espconn *pespconn = arg;

    ESP_DBG("user_esp_platform_discon_cb\n");

    user_platform_info.cloud_connect = FALSE;

    if (pespconn == NULL)
    {
        return;
    }

    pespconn->proto.tcp->local_port = espconn_port();

    if(user_platform_info.manual_dis == FALSE)
    {
    	user_esp_platform_reconnect(pespconn);
    }
}

/******************************************************************************
 * FunctionName : user_esp_platform_discon
 * Description  : A new incoming connection has been disconnected.
 * Parameters   : espconn -- the espconn used to disconnect with host
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_discon(struct espconn *pespconn)
{
    ESP_DBG("user_esp_platform_discon\n");

    user_platform_info.cloud_connect = FALSE;

    espconn_disconnect(pespconn);
}

/******************************************************************************
 * FunctionName : user_esp_platform_sent_cb
 * Description  : Data has been sent successfully and acknowledged by the remote host.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_sent_cb(void *arg)
{
    struct espconn *pespconn = arg;
    user_platform_info.cloud_connect = TRUE;

    ESP_DBG("user_esp_platform_sent_cb\n");
}


/******************************************************************************
 * FunctionName : user_esp_platform_recv_cb
 * Description  : Processing the received data from the server
 * Parameters   : arg -- Additional argument to pass to the callback function
 *                pusrdata -- The received data (or NULL when the connection has been closed!)
 *                length -- The length of received data
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
    char *pstr = NULL;
//    LOCAL char pbuffer[1024 * 2] = {0};
    struct espconn *pespconn = arg;
    u16 i;

    ESP_DBG("user_esp_platform_recv_cb : length -> %d \r\n",length);
    UART_ShowByteInCharHex(pusrdata,length);
    ESP_DBG("\r\n");
    if(length < CLOUD_DATA_MAX)
    {
    	memcpy(ReceiveBuffer,pusrdata,length);
    	hz_decode_data();
    }
    else
    {
    	ESP_DBG("length too long!!\r\n");
    }

}


/******************************************************************************
 * FunctionName : user_esp_platform_recon_cb
 * Description  : The connection had an error and is already deallocated.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_recon_cb(void *arg, sint8 err)
{
    struct espconn *pespconn = (struct espconn *)arg;

    ESP_DBG("user_esp_platform_recon_cb\n");

    user_platform_info.cloud_connect = FALSE;

    os_timer_disarm(&client_timer);
    os_timer_setfn(&client_timer, (os_timer_func_t *)user_esp_platform_reconnect, pespconn);
    os_timer_arm(&client_timer, ESP_RECON_TIME, 0);
}


/******************************************************************************
 * FunctionName :
 * Description  :
 * Parameters   :
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_heart_jump(void)
{
	if(user_platform_info.cloud_connect == TRUE)
	{
		hz_send_heartbeat();
	}
}

/******************************************************************************
 * FunctionName : user_esp_platform_connect_cb
 * Description  : A new incoming connection has been connected.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_connect_cb(void *arg)
{
    struct espconn *pespconn = arg;

    ESP_DBG("user_esp_platform_connect_cb\n");

    espconn_regist_recvcb(pespconn, user_esp_platform_recv_cb);
    espconn_regist_sentcb(pespconn, user_esp_platform_sent_cb);


    os_timer_disarm(&heart_jump_timer);
    os_timer_setfn(&heart_jump_timer, (os_timer_func_t *)user_esp_platform_heart_jump, pespconn);
    os_timer_arm(&heart_jump_timer, BEACON_TIME, 1);

    hz_up_login_status();

    user_platform_info.cloud_connect = TRUE;
}

/******************************************************************************
 * FunctionName : user_esp_restart_heartbeat_timer
 * Description  :
 * Parameters   :
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_restart_heartbeat_timer(void)
{
    os_timer_disarm(&heart_jump_timer);
    os_timer_setfn(&heart_jump_timer, (os_timer_func_t *)user_esp_platform_heart_jump, NULL);
    os_timer_arm(&heart_jump_timer, BEACON_TIME, 1);
}

/******************************************************************************
 * FunctionName : user_esp_platform_connect
 * Description  : The function given as the connect with the host
 * Parameters   : espconn -- the espconn used to connect the connection
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_connect(struct espconn *pespconn)
{
    ESP_DBG("user_esp_platform_connect\n");

#ifdef CLIENT_SSL_ENABLE
    espconn_secure_connect(pespconn);
#else
    espconn_connect(pespconn);
#endif
}

#ifdef USE_DNS
/******************************************************************************
 * FunctionName : user_esp_platform_dns_found
 * Description  : dns found callback
 * Parameters   : name -- pointer to the name that was looked up.
 *                ipaddr -- pointer to an ip_addr_t containing the IP address of
 *                the hostname, or NULL if the name could not be found (or on any
 *                other error).
 *                callback_arg -- a user-specified callback argument passed to
 *                dns_gethostbyname
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
    struct espconn *pespconn = (struct espconn *)arg;

    if (ipaddr == NULL)
    {
        ESP_DBG("user_esp_platform_dns_found NULL\n");
        return;
    }

    ESP_DBG("DNS:%s PORT:%d \r\n",ESP_DOMAIN,ESP_DOMAIN_PORT);

    ESP_DBG("user_esp_platform_dns_found %d.%d.%d.%d\n",
            *((uint8 *)&ipaddr->addr), *((uint8 *)&ipaddr->addr + 1),
            *((uint8 *)&ipaddr->addr + 2), *((uint8 *)&ipaddr->addr + 3));

    if (esp_server_ip.addr == 0 && ipaddr->addr != 0)
    {
        os_timer_disarm(&client_timer);
        esp_server_ip.addr = ipaddr->addr;
        os_memcpy(pespconn->proto.tcp->remote_ip, &ipaddr->addr, 4);

        pespconn->proto.tcp->local_port = espconn_port();

#ifdef CLIENT_SSL_ENABLE
        pespconn->proto.tcp->remote_port = 8443;
#else
        pespconn->proto.tcp->remote_port = ESP_DOMAIN_PORT;
#endif

        espconn_regist_connectcb(pespconn, user_esp_platform_connect_cb);
        espconn_regist_disconcb(pespconn, user_esp_platform_discon_cb);
        espconn_regist_reconcb(pespconn, user_esp_platform_recon_cb);
        user_esp_platform_connect(pespconn);
    }
}

/******************************************************************************
 * FunctionName : user_esp_platform_dns_check_cb
 * Description  : 1s time callback to check dns found
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_dns_check_cb(void *arg)
{
    struct espconn *pespconn = arg;

    ESP_DBG("user_esp_platform_dns_check_cb\n");

    if (wifi_station_get_connect_status() != STATION_GOT_IP)
    {
        os_timer_setfn(&client_timer, (os_timer_func_t *)user_esp_platform_check_ip, NULL);
        os_timer_arm(&client_timer, ESP_CHECK_IP_TIME, 0);
    	return;
    }

    espconn_gethostbyname(pespconn, ESP_DOMAIN, &esp_server_ip, user_esp_platform_dns_found);

    os_timer_arm(&client_timer, ESP_CHECK_DNS_TIME, 0);
}


/******************************************************************************
 * FunctionName :
 * Description  :
 * Parameters   :
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_start_dns(struct espconn *pespconn)
{
    esp_server_ip.addr = 0;
    espconn_gethostbyname(pespconn, ESP_DOMAIN, &esp_server_ip, user_esp_platform_dns_found);

    os_timer_disarm(&client_timer);
    os_timer_setfn(&client_timer, (os_timer_func_t *)user_esp_platform_dns_check_cb, pespconn);
    os_timer_arm(&client_timer, ESP_CHECK_DNS_TIME, 0);
}
#endif


/******************************************************************************
 * FunctionName : user_esp_platform_check_ip
 * Description  : espconn struct parame init when get ip addr
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_check_ip(void)
{
    struct ip_info ipconfig;
    uint8 wifi_state = wifi_station_get_connect_status();

    os_timer_disarm(&client_timer);

    wifi_get_ip_info(STATION_IF, &ipconfig);

    ESP_DBG("user_esp_platform_check_ip:%d \r\n",wifi_state);

    if (wifi_state == STATION_GOT_IP && ipconfig.ip.addr != 0)
    {
    	Upgrade.onNet = 1;

#if (USE_CN_TIME_POOL)
//使用网络服务器获取时间
#else
//使用韩智服务器获取时间
    	rtc_cali_when_connect_cloud();
#endif
//    	http_test_main();
#if 1
    	user_sntp_init();
//********************************************************************
    	esp_platform_tcp_client.proto.tcp = &user_tcp;
    	esp_platform_tcp_client.type = ESPCONN_TCP;
    	esp_platform_tcp_client.state = ESPCONN_NONE;

#ifdef USE_DNS
        user_esp_platform_start_dns(&esp_platform_tcp_client);
#else
        ESP_DBG("esp_server_ip:%d.%d.%d.%d:%d\r\n",esp_server_ip[0],esp_server_ip[1],esp_server_ip[2],esp_server_ip[3],ESP_DOMAIN_PORT);
        os_memcpy(esp_platform_tcp_client.proto.tcp->remote_ip, esp_server_ip, 4);
        esp_platform_tcp_client.proto.tcp->local_port = espconn_port();

#ifdef CLIENT_SSL_ENABLE
        esp_platform_tcp_client.proto.tcp->remote_port = 8443;
#else
        esp_platform_tcp_client.proto.tcp->remote_port = ESP_DOMAIN_PORT;
#endif
        espconn_regist_connectcb(&esp_platform_tcp_client, user_esp_platform_connect_cb);
        espconn_regist_disconcb(&esp_platform_tcp_client, user_esp_platform_discon_cb);
        espconn_regist_reconcb(&esp_platform_tcp_client, user_esp_platform_recon_cb);
        user_esp_platform_connect(&esp_platform_tcp_client);
#endif
#endif


    }
    else
    {
        /* if there are wrong while connecting to some AP, then reset mode    连不上路由器时           */
        if ((wifi_station_get_connect_status() == STATION_WRONG_PASSWORD ||
                wifi_station_get_connect_status() == STATION_NO_AP_FOUND ||
                wifi_station_get_connect_status() == STATION_CONNECT_FAIL))
        {

           // user_esp_platform_reset_mode();
        	ESP_DBG("esp platform STATION_WRONG_PASSWORD or STATION_NO_AP_FOUND or STATION_CONNECT_FAIL\r\n");
			user_platform_info.cloud_connect = FALSE;


			if(user_platform_info.w_timeout++ > 50)
			{
				user_platform_info.w_timeout = 0;

			    wifi_set_opmode_current(STATION_MODE);
			    wifi_station_get_config_default(&stationInFlash);
				wifi_station_set_config(&stationInFlash);
				wifi_station_connect();

	            os_timer_setfn(&client_timer, (os_timer_func_t *)user_esp_platform_check_ip, NULL);
	            os_timer_arm(&client_timer, ESP_CHECK_IP_TIME, 0);
/*				user_platform_info.manual_dis = TRUE;			//这里执行完之后会调用esp_paltform_init
				rtc_reset_time();
				os_timer_disarm(&heart_jump_timer);
				os_timer_disarm(&client_timer);
				wifilink_set_ap_server();*/
			}
			else		//没有超时，继续检测
			{
	            os_timer_setfn(&client_timer, (os_timer_func_t *)user_esp_platform_check_ip, NULL);
	            os_timer_arm(&client_timer, ESP_CHECK_IP_TIME, 0);
			}

        }
        else//继续检测
        {
            os_timer_setfn(&client_timer, (os_timer_func_t *)user_esp_platform_check_ip, NULL);
            os_timer_arm(&client_timer, ESP_CHECK_IP_TIME, 0);
        }
    }
}


/* *************************************************************************
 * 云端升级时，需要调用这些操作
 *************************************************************************/
void ICACHE_FLASH_ATTR
user_ota_update_disable_misc(void)
{
	rtc_reset_time();
	user_platform_info.cloud_connect = FALSE;
	user_platform_info.manual_dis = TRUE;						//这里执行完之后会重启调用esp_paltform_init
	user_esp_platform_discon(&esp_platform_tcp_client);
	os_timer_disarm(&client_timer);
	os_timer_disarm(&heart_jump_timer);
	os_timer_disarm(&timer1);
}


/**************************************************************************
 *
 *************************************************************************/
void ICACHE_FLASH_ATTR
cloud_data_process (void)
{
	LOCAL u8 delayCnt = 0;
	u8 len;
	u8 outBuf[STACK_BLOCK_SIZE];

	if(++delayCnt >= 3)		//300ms处理一次发送
	{
		delayCnt = 0;
		msg_stack_pop(outBuf, &len);

		if(len > 0)
		{
			espconn_sent(&esp_platform_tcp_client,outBuf,len);
		}
	}
}


void ICACHE_FLASH_ATTR
user_jump2wifilink_task (void)
{
	user_platform_info.cloud_connect = FALSE;
	user_platform_info.manual_dis = TRUE;			//这里执行完之后会调用esp_paltform_init
	rtc_reset_time();
	user_esp_platform_discon(&esp_platform_tcp_client);
	os_timer_disarm(&heart_jump_timer);
	os_timer_disarm(&client_timer);
	ESP_DBG("\r\n*******************user manual set wifi********************\r\n");
	wifilink_set_ap_server();
}


/**************************************************************************
 *
 *************************************************************************/
void ICACHE_FLASH_ATTR
message_task (void)
{
	u8 msg;
	u8 i;
	u8 sys_restart;

	cloud_data_process();

	msg = get_msg2();

	switch(msg)
	{

#if (USE_PC_PARAM_SET)
		case MSG_PC_SET:
			ESP_PC_DBG("MSG_PC_SET\r\n");
			user_paltform_flash_record_param();
			break;
#endif

#if (USE_CN_TIME_POOL == 0)
		case MSG_TIMESTAMP:
			ESP_PC_DBG("MSG_TIMESTAMP\r\n");
			if(rtc_time.reqStamp)
			{
				rtc_time.reqStamp = 0;
				rtc_calibration_stamp(rtc_time.hz_stamp);
			}
			rtc_tick_tock(rtc_time.hz_stamp);
			break;
#endif	
		case MSG_RTC_PRINT:
			ESP_PC_DBG("MSG_RTC_PRINT\r\n");
			rtc_print();
			break;
		case PR_K3:
		case PR_K2:
		case PR_K1:
		case PR_K0:
//			http_start_upgrade("http://www.szhanzhi.cn/download/Light.CH3.VER033.user2.bin");
			/*http_start_upgrade*/("http://szhanzhi.cn/download/wifiFireware/Light.CH3.VER036.user2.bin");
			//self_upgrade();
			peripheral_beep_n10ms(BEEP_TIME_SHORT);
//			peripheral_beep_n10ms(30);
			break;

		case PS_K3:
			ESP_DBG("PS_K3\n");
			user_press_deal(3);
			user_deal_restart(3);
			user_udp_scene_manual_handle(3);
//			peripheral_beep_n10ms(BEEP_TIME_SHORT);
			break;

		case PS_K2:
			ESP_DBG("PS_K2\n");
			user_press_deal(2);
			user_deal_restart(2);
			user_udp_scene_manual_handle(2);
//			peripheral_beep_n10ms(BEEP_TIME_SHORT);
			break;

		case PS_K1:
			ESP_DBG("PS_K1\n");
			user_press_deal(1);
			user_deal_restart(1);
			user_udp_scene_manual_handle(1);
//			peripheral_beep_n10ms(BEEP_TIME_SHORT);
			break;

		case PS_K0:
			ESP_DBG("PS_K0\n");
			user_press_deal(0);
			user_deal_restart(0);
			user_udp_scene_manual_handle(0);
//			peripheral_beep_n10ms(BEEP_TIME_SHORT);
			break;


#ifdef USER_MANUAL_DIS_TEST
		case PL_MEM:
			ESP_DBG("PL_MEM\n");
			ESP_DBG("user manual disconnect\n");
			user_platform_info.cloud_connect = FALSE;
			user_platform_info.manual_dis = TRUE;
			os_timer_disarm(&heart_jump_timer);
			user_esp_platform_discon(&user_conn);
			peripheral_beep_n10ms(20);
			break;

		case PL_PWR:
			ESP_DBG("PL_PWR\n");
			ESP_DBG("user manual reconnect\n");
			user_platform_info.manual_dis = FALSE;
			user_esp_platform_reconnect(&user_conn);
/*				ESP_DBG("system_restart\n");
			rtc_reset_time();
			system_restart();*/
			peripheral_beep_n10ms(20);
			break;
#endif

#if HZ_GIVE_POLICY_TEST
		case PL_K2:
			hz_test_give_one_policy(0X01,5);
			hz_test_give_one_policy(0X02,10);
			hz_test_give_one_policy(0X04,15);
			break;
#endif


		case PL_K0:
			if(user_check_restart(0XA0))
			{
				break;
			}
			peripheral_beep_n10ms(BEEP_TIME_LONG);
			if(((char *)os_strstr(DEV_INFO_STRING, ("ADJ"))) == NULL)
			{
				user_jump2wifilink_task();
			}
			ESP_DBG("PL_K0\n");
			break;

		case PL_K1:
			if(user_check_restart(0XA1))
			{
				break;
			}
			peripheral_beep_n10ms(BEEP_TIME_LONG);
			user_jump2wifilink_task();
			ESP_DBG("PL_K1\n");
			break;

		case PL_K2:
			if(user_check_restart(0XA2))
			{
				break;
			}
			peripheral_beep_n10ms(BEEP_TIME_LONG);
			if(((char *)os_strstr(DEV_INFO_STRING, ("ADJ"))) == NULL)
			{
				user_jump2wifilink_task();
			}
			ESP_DBG("PL_K2\n");
			break;

		case PL_K3:
			if(user_check_restart(0XA3))
			{
				break;
			}
			peripheral_beep_n10ms(BEEP_TIME_LONG);
			user_jump2wifilink_task();
			ESP_DBG("PL_K3\n");
			break;

		case PH_K2:
			ESP_DBG("PH_K2\n");
			user_press_deal(2);
			break;
			
		case PH_K0:
			ESP_DBG("PH_K0\n");
			user_press_deal(0);
			break;
		default:
			break;

	}
}



/* *************************************************************************
 * 定时器1处理函数
 *************************************************************************/
void ICACHE_FLASH_ATTR
timer1_isr(void)
{
	LOCAL u16 heartJumpCnt;
	LOCAL u8 send_timeout;
	u8 wifi_conn_sta = 0;
//	LOCAL u8 secCnt = 0;
	KeyScan();

//	led_pwm_breath();

	user_udp_pop_data();

	user_led_on_run();

	if(heartJumpCnt++ >= 30)			//30*10ms == 300ms
	{
		heartJumpCnt = 0;
#if (USE_CN_TIME_POOL)
		rtc_tick_tock();
#else
		rtc_cali_interval_run();
#endif
		hz_policy_run();
		led_display_poweron();

	}
	peripheral_beep_state_timer_run();

#ifdef LONG_TIME_TEST
	LongTimeTest();
#endif

	peripheral_led_state_set();

	led_display_work();
/*
	if(++secCnt > 100)
	{
		secCnt = 0;
		rtc_print();
	}
*/
}



/**************************************************************************
 *
 *************************************************************************/
void ICACHE_FLASH_ATTR
user_espconn_sent(uint8 *psent, uint16 length)
{
	u8 transBuf[STACK_BLOCK_SIZE];

	memset(transBuf,0X00,sizeof(transBuf));

	os_memcpy(transBuf,psent,length);

	msg_stack_push(transBuf,length);
}

/**************************************************************************
 *
 *************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_create_task(void)
{
 //   testQueue=(os_event_t *)os_malloc(sizeof(os_event_t)*TEST_QUEUE_LEN);
 //   system_os_task(test_task,USER_TASK_PRIO_2,testQueue,TEST_QUEUE_LEN);

	os_timer_disarm(&timer1);
	os_timer_setfn(&timer1, (os_timer_func_t *)timer1_isr, 1);
	os_timer_arm(&timer1, 10, 1);

	os_timer_disarm(&timer_message_task);
	os_timer_setfn(&timer_message_task, (os_timer_func_t *)message_task, 1);
	os_timer_arm(&timer_message_task, 100, 1);

	user_platform_info.cloud_connect = FALSE;
	user_platform_info.manual_dis = FALSE;

}


/******************************************************************************
 * FunctionName : user_esp_platform_init
 * Description  : device parame init based on espressif platform
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_init(void)
{
	struct rst_info *rtc_info = system_get_rst_info();

	ESP_DBG("reset reason: %x\n", rtc_info->reason);

	if (rtc_info->reason == REASON_WDT_RST ||
		rtc_info->reason == REASON_EXCEPTION_RST ||
		rtc_info->reason == REASON_SOFT_WDT_RST)
	{
		if (rtc_info->reason == REASON_EXCEPTION_RST)
		{
			ESP_DBG("Fatal exception (%d):\n", rtc_info->exccause);
		}
		ESP_DBG("epc1=0x%08x, epc2=0x%08x, epc3=0x%08x, excvaddr=0x%08x, depc=0x%08x\n",
				rtc_info->epc1, rtc_info->epc2, rtc_info->epc3, rtc_info->excvaddr, rtc_info->depc);
	}

	os_timer_disarm(&client_timer);
	os_timer_setfn(&client_timer, (os_timer_func_t *)user_esp_platform_check_ip, 1);
	os_timer_arm(&client_timer, ESP_CHECK_IP_TIME, 0);


	hz_cloud_send_callback_register(user_espconn_sent);									//韩智云发送需要调用的本文件函数
	hz_cloud_restart_heartbeat_callback_register(user_esp_restart_heartbeat_timer);		//韩智云发送需要调用的本文件函数

    pwm_Init();

    peripheral_beep_n10ms(BEEP_TIME_SHORT);

	ESP_DBG("esp_platform_init done\r\n\r\n");

	msg_stack_init();
}

#endif


