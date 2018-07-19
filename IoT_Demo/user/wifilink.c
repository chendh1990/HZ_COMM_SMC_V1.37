////////////////////////////////////////////////
#include "user_config.h"

LOCAL wifilink_set_wifi_done_cb_t wifilink_set_wifi_done_callback = NULL;
LOCAL os_timer_t wifilink_check_wifi_timer;
LOCAL os_timer_t wifilink_time_out_timer;
LOCAL struct espconn wifilink_tcp_server;//注意在uart.c中添加声明
LOCAL struct _esp_tcp user_tcp;
LOCAL struct station_config stationConf;
struct WIFILINK_INFO WiFi_Link_Info;
LOCAL struct station_config stationInfoTmp;


int8_t ICACHE_FLASH_ATTR
wifilink_dataStrCpy(void *pDest, const void *pSrc, int8_t maxLen)
{
//	assert(pDest!=NULL && pSrc!=NULL);

  char *pTempD = pDest;
  const char *pTempS = pSrc;
  int8_t len;

  if(*pTempS != '\"')
  {
    return -1;
  }
  pTempS++;
  for(len=0; len<maxLen; len++)
  {
    if(*pTempS == '\"')
    {
      *pTempD = '\0';
      break;
    }
    else
    {
      *pTempD++ = *pTempS++;
    }
  }
  if(len == maxLen)
  {
    return -1;
  }
  return len;
}


void ICACHE_FLASH_ATTR
wifilink_set_wifi_done_callback_register(wifilink_set_wifi_done_cb_t wifilink_set_wifi_done_cb)
{
	wifilink_set_wifi_done_callback = wifilink_set_wifi_done_cb;
}


void ICACHE_FLASH_ATTR
wifilink_jump2epsplatform(void)
{
	os_timer_disarm(&wifilink_check_wifi_timer);
	os_timer_disarm(&wifilink_time_out_timer);
	wifilink_set_wifi_done_callback();			//连上wifi之后的回调函数，必须先注册
	WiFi_Link_Info.in = 0;
	user_platform_info.manual_dis = FALSE;
}


void ICACHE_FLASH_ATTR
wifilink_check_conn_wifi(void)
{
    struct ip_info ipconfig;

    wifi_get_ip_info(STATION_IF, &ipconfig);

    if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0)
    {
    	ESP_DBG("wifilink STATION_GOT_IP\r\n");
    	wifilink_jump2epsplatform();
    }
    else
    {
        /* if there are wrong while connecting to some AP, then reset mode */
        if ((wifi_station_get_connect_status() == STATION_WRONG_PASSWORD ||
                wifi_station_get_connect_status() == STATION_NO_AP_FOUND ||
                wifi_station_get_connect_status() == STATION_CONNECT_FAIL))
        {
        	ESP_DBG("wifilink STATION_WRONG_PASSWORD or STATION_NO_AP_FOUND or STATION_CONNECT_FAIL\r\n");
        	os_timer_disarm(&wifilink_check_wifi_timer);
        	wifilink_set_ap_server();
        }
    }
}


void ICACHE_FLASH_ATTR
wifilink_try_conn_wifi(void)
{
	os_timer_disarm(&wifilink_check_wifi_timer);

	ESP_DBG("wifilink_try_conn_wifi\r\n");

	wifi_set_opmode_current(STATION_MODE);
	ETS_UART_INTR_DISABLE();
	wifi_station_set_config(&stationConf);
	ETS_UART_INTR_ENABLE();
	wifi_station_connect();

	os_timer_disarm(&wifilink_check_wifi_timer);
	os_timer_setfn(&wifilink_check_wifi_timer, (os_timer_func_t *)wifilink_check_conn_wifi, 1);
	os_timer_arm(&wifilink_check_wifi_timer, 200, 1);
}


void ICACHE_FLASH_ATTR
wifilink_send_dev_info(void)
{
	u8 send_buf[100];

	os_memset(send_buf,0,sizeof(send_buf));

	os_sprintf(send_buf,"%s,\"MAC:%02X%02X%02X%02X%02X%02X\"\r\n",Device_Info.info,Device_Info.mac[0],Device_Info.mac[1],
			Device_Info.mac[2],Device_Info.mac[3],Device_Info.mac[4],Device_Info.mac[5]);
	ESP_DBG("\r\nsend_buf:%d|||%s\r\n",os_strlen(send_buf),send_buf);

	espconn_sent(&wifilink_tcp_server, send_buf, os_strlen(send_buf));
}


void ICACHE_FLASH_ATTR
wifilink_server_recvcb(void *arg, char *pusrdata, unsigned short length)
{
    struct espconn *pespconn = arg;
    u16 i;

    ESP_DBG("user_esp_platform_recv_cb:%s\r\n",pusrdata);

    if(pusrdata[0]=='g'&&pusrdata[1]=='e'&&pusrdata[2]=='t'&&pusrdata[3]=='i'&&pusrdata[4]=='n'&&pusrdata[5]=='f'&&pusrdata[6]=='o' )
    {
    	wifilink_send_dev_info();
    }

	if(pusrdata[0]=='s'&&pusrdata[1]=='j'&&pusrdata[2]=='a'&&pusrdata[3]=='p'&&pusrdata[4]=='=')   	//远程设置连接路由器,UDP远程发送sjap="ssid","password"
	{
		int8_t len;

		wifilink_pair_time_reset();

		pusrdata+=5;
		len = wifilink_dataStrCpy(&stationConf.ssid, pusrdata, 32);
		if(len != -1)
		{
			pusrdata+= (len+3);
			len = wifilink_dataStrCpy(&stationConf.password, pusrdata, 64);
		}
		if(len != -1)
		{
			espconn_sent(&wifilink_tcp_server, "Accept passsword\r\n", os_strlen("Accept passsword\r\n"));
			os_timer_disarm(&wifilink_check_wifi_timer);
			os_timer_setfn(&wifilink_check_wifi_timer, (os_timer_func_t *)wifilink_try_conn_wifi, 1);
			os_timer_arm(&wifilink_check_wifi_timer, 200, 1);
		}
	}
}

void ICACHE_FLASH_ATTR wifilink_server_sentcb(void *arg)
{
	ESP_DBG("wifilink Send Successful！\r\n");
}

void ICACHE_FLASH_ATTR wifilink_server_disconcb(void *arg)
{
	ESP_DBG("wifilink Connection break！\r\n");
}

void ICACHE_FLASH_ATTR wifilink_server_listen(void *arg) //注册 TCP 连接成功建立后的回调函数
{
	u8 send_buf[100];

	ESP_DBG("wifilink server_listen！\r\n");
    struct espconn *pespconn = arg;

	espconn_regist_recvcb(pespconn, wifilink_server_recvcb); 		//接收
	espconn_regist_sentcb(pespconn, wifilink_server_sentcb); 		//发送
	espconn_regist_disconcb(pespconn, wifilink_server_disconcb); 	//断开

	wifilink_send_dev_info();
}

void ICACHE_FLASH_ATTR wifilink_server_reconcb(void *arg, sint8 err) //注册 TCP 连接发生异常断开时的回调函数，可以在回调函数中进行重连
{
	ESP_DBG("Err sta：\r\n", err); //用来输出十进制整数
}

void wifilink_Inter_InitTCP(uint32_t Local_port)
{

//	wifilink_tcp_server = (struct espconn *)os_zalloc(sizeof(struct espconn));
	wifilink_tcp_server.proto.tcp = &user_tcp;
	wifilink_tcp_server.type = ESPCONN_TCP; //设置类型为TCP协议
	wifilink_tcp_server.state = ESPCONN_NONE;									//状态
	wifilink_tcp_server.proto.tcp->local_port = Local_port; //本地端口

	//注册连接成功回调函数和重新连接回调函数
	espconn_regist_connectcb(&wifilink_tcp_server, wifilink_server_listen); 		//注册 TCP 连接成功建立后的回调函数
	espconn_regist_reconcb(&wifilink_tcp_server, wifilink_server_reconcb); 			//注册 TCP 连接发生异常断开时的回调函数，可以在回调函数中进行重连
	espconn_accept(&wifilink_tcp_server); 									//创建 TCP server，建立侦听
	espconn_regist_time(&wifilink_tcp_server, 180, 0); //设置超时断开时间 单位：秒，最大值：7200 秒
	//如果超时时间设置为 0，ESP8266 TCP server 将始终不会断开已经不与它通信的 TCP client，不建议这样使用。
}


void ICACHE_FLASH_ATTR
wifilink_set_ap_mode(void)
{
	struct softap_config apConfig;
	struct ip_info info;
	uint8 mac_ddr[8];
	uint8 ssid_tmp[32];

	os_memset(mac_ddr,0,sizeof(mac_ddr));
	wifi_get_macaddr(SOFTAP_IF, mac_ddr);
	ESP_DBG("AP MAC:%2X%2X%2X%2X%2X%2X \r\n", mac_ddr[0],mac_ddr[1],mac_ddr[2],mac_ddr[3],mac_ddr[4],mac_ddr[5]);

	os_memset(Device_Info.mac,0,sizeof(Device_Info.mac));
	wifi_get_macaddr(STATION_IF, Device_Info.mac);
	ESP_DBG("STA MAC:%2X%2X%2X%2X%2X%2X \r\n", Device_Info.mac[0],Device_Info.mac[1],
			Device_Info.mac[2],Device_Info.mac[3],Device_Info.mac[4],Device_Info.mac[5]);

	wifi_set_opmode_current(SOFTAP_MODE);    //设置为AP模式，不保存到 flash

	os_memset(ssid_tmp,0,sizeof(ssid_tmp));
	os_sprintf(ssid_tmp, "ESP_HZ_%02X%02X%02X%02X%02X%02X", Device_Info.mac[0],Device_Info.mac[1],
			Device_Info.mac[2],Device_Info.mac[3],Device_Info.mac[4],Device_Info.mac[5]);

	ESP_DBG("AP SSID:%s \r\n",ssid_tmp);

	apConfig.ssid_len = 7+12;						//设置ssid长度
	os_strcpy(apConfig.ssid, ssid_tmp);	    		//设置ssid名字
	os_strcpy(apConfig.password, WIFI_LINK_KEY);	//设置密码
	apConfig.authmode = AUTH_OPEN;// AUTH_WPA_WPA2_PSK;      	//设置加密模式
	apConfig.beacon_interval = 100;            		//信标间隔时槽100 ~ 60000 ms
	apConfig.channel = 6;                     		//通道号1 ~ 13
	apConfig.max_connection = 4;               		//最大连接数
	apConfig.ssid_hidden = 0;                  		//隐藏SSID

	IP4_ADDR(&info.ip, 192, 168, 4, 1);      		//设置本地IP
	IP4_ADDR(&info.gw, 192, 168, 4, 1);
	IP4_ADDR(&info.netmask, 255, 255, 255, 0);
	wifi_set_ip_info(SOFTAP_IF, &info);
	ETS_UART_INTR_DISABLE();
	wifi_softap_set_config_current(&apConfig);		//设置 WiFi soft-AP 接口配置，不保存到 flash
	ETS_UART_INTR_ENABLE();


/*	wifi_softap_dhcps_stop();
	wifi_softap_dhcps_start();*/
}


void ICACHE_FLASH_ATTR
wifilink_pair_time_run(void)
{
	if(WiFi_Link_Info.pTimeOut < 50000)
	{
		WiFi_Link_Info.pTimeOut++;
	}
}

void ICACHE_FLASH_ATTR
wifilink_pair_time_reset(void)
{
	WiFi_Link_Info.pTimeOut = 0;
}


#define INT_2MINU_MSK	(2*60)

u8 ICACHE_FLASH_ATTR
wifilink_pair_time_out(void)
{

	if(WiFi_Link_Info.pTimeOut > INT_2MINU_MSK)
	{
		return TRUE;
	}

	return FALSE;
}


extern void user_scan_done(void *arg, STATUS status);
void ICACHE_FLASH_ATTR
wifilink_time_out_timer_isr(void)
{
	struct scan_config config;

	os_memset(&config, 0, sizeof(config));

	config.ssid = stationInfoTmp.ssid;

	wifilink_pair_time_run();
	if(wifilink_pair_time_out())
	{

    	ESP_DBG("\r\n|-------------wifilink time out return to esp platform !!! -----------------|\r\n");
    	ESP_DBG("Connect to previous AP:%s --- %s \r\n",stationInfoTmp.ssid,stationInfoTmp.password);
    	ESP_DBG("|---------------------------------------------------------------------------|\r\n\r\n");
    	//超时的话，连回原来的路由器
    	ESP_DBG("smartconfig_stop \r\n");
    	smartconfig_stop();

    	wifi_set_opmode_current(STATION_MODE);


    	ETS_UART_INTR_DISABLE();
    	wifi_station_set_config(&stationInfoTmp);
    	ETS_UART_INTR_ENABLE();
    	wifi_station_disconnect();
    	wifi_station_connect();

    	//结束wifilink的定时器
    	wifilink_jump2epsplatform();
	}
}


#if (USE_SMARTCONFIG)
void ICACHE_FLASH_ATTR
smartconfig_done(sc_status status, void *pdata)
{
    switch(status)
    {
        case SC_STATUS_WAIT:
            ESP_DBG("SC_STATUS_WAIT\n");
            break;
        case SC_STATUS_FIND_CHANNEL:
            ESP_DBG("SC_STATUS_FIND_CHANNEL\n");
            break;
        case SC_STATUS_GETTING_SSID_PSWD:
            ESP_DBG("SC_STATUS_GETTING_SSID_PSWD\n");
			sc_type *type = pdata;
            if (*type == SC_TYPE_ESPTOUCH) {
                ESP_DBG("SC_TYPE:SC_TYPE_ESPTOUCH\n");
            } else {
                ESP_DBG("SC_TYPE:SC_TYPE_AIRKISS\n");
            }
            break;
        case SC_STATUS_LINK:
            ESP_DBG("SC_STATUS_LINK\n");
            struct station_config *sta_conf = pdata;

	        wifi_station_set_config(sta_conf);
	        wifi_station_disconnect();
	        wifi_station_connect();
            break;
        case SC_STATUS_LINK_OVER:
            ESP_DBG("SC_STATUS_LINK_OVER\n");
            if (pdata != NULL) {
                uint8 phone_ip[4] = {0};

                os_memcpy(phone_ip, (uint8*)pdata, 4);
                ESP_DBG("Phone ip: %d.%d.%d.%d\n",phone_ip[0],phone_ip[1],phone_ip[2],phone_ip[3]);
            }
            smartconfig_stop();

        	//结束wifilink的定时器
        	os_timer_disarm(&wifilink_check_wifi_timer);
        	os_timer_disarm(&wifilink_time_out_timer);
        	wifilink_set_wifi_done_callback();			//连上wifi之后的回调函数，必须先注册
        	WiFi_Link_Info.in = 0;
        	user_platform_info.manual_dis = FALSE;
            break;
    }

}
#endif

void ICACHE_FLASH_ATTR
wifilink_set_ap_server(void)
{
	ESP_DBG("\r\n*****************wifilink start*****************\r\n");

	WiFi_Link_Info.in = 1;

	user_platform_info.w_timeout = 0;

	wifi_station_get_config_default(&stationInfoTmp);	//先备份旧路由信息

	wifilink_pair_time_reset();


#if (USE_SMARTCONFIG)
	ESP_DBG("smartconfig...... \r\n");
	smartconfig_stop();
	smartconfig_set_type(SC_TYPE_ESPTOUCH); //SC_TYPE_ESPTOUCH,SC_TYPE_AIRKISS,SC_TYPE_ESPTOUCH_AIRKISS
	wifi_set_opmode(STATION_MODE);
	smartconfig_start(smartconfig_done);
#else
	wifilink_set_ap_mode();
	wifilink_Inter_InitTCP(8266); //本地端口
#endif

	os_timer_disarm(&wifilink_time_out_timer);
	os_timer_setfn(&wifilink_time_out_timer, (os_timer_func_t *)wifilink_time_out_timer_isr, 1);
	os_timer_arm(&wifilink_time_out_timer, 1000, 1);

}

