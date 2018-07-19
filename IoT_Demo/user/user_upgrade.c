//////////////////////////////////////////////////////////////////////////////////////////
#include "user_config.h"

UPGRADE Upgrade;
//struct upgrade_server_info *server = NULL;
OTAFIFO OTAFifo;

/******************************************************
SpiFlashOpResult spi_flash_erase_sector(uint16 sec)   函数用于擦除一个扇区
SpiFlashOpResult spi_flash_write (uint32 des_addr,uint32 *src_addr, uint32 size) 将数据写入
SpiFlashOpResult spi_flash_read(uint32 src_addr,uint32 * des_addr, uint32 size)  读取数据
*******************************************************/

void ICACHE_FLASH_ATTR
self_upgrade(void)
{
	u16 i;

	if(Upgrade.isBusy)
	{
		ESP_DBG("self upgrade is already start \r\n");
		return;
	}

	Upgrade.isBusy = 1;

	Upgrade.flash = (uint32 *)os_zalloc(USER_FLASH_SEC_SIZE);

    if (system_upgrade_userbin_check() == UPGRADE_FW_BIN1)
	{
    	ESP_DBG("Erase EXE APP2 area \r\n");
    	for(i=0;i<123;i++)
    	{
    		spi_flash_erase_sector(FLASH_EXE_APP2_START_SEC+i);
    	}
    	ESP_DBG("Copy APP2 code \r\n");
		for(i=0;i<123;i++)
		{
			spi_flash_read(FLASH_APP2_START_ADDR+USER_FLASH_SEC_SIZE*i,Upgrade.flash, USER_FLASH_SEC_SIZE);

			spi_flash_write(FLASH_EXE_APP2_START_ADDR+USER_FLASH_SEC_SIZE*i,Upgrade.flash, USER_FLASH_SEC_SIZE);
		}

    }
	else if (system_upgrade_userbin_check() == UPGRADE_FW_BIN2)
    {
		ESP_DBG("Erase EXE APP1 area \r\n");
		for(i=0;i<123;i++)
		{
			spi_flash_erase_sector(FLASH_EXE_APP1_START_SEC+i);
		}

		ESP_DBG("Copy APP1 code \r\n");
		for(i=0;i<123;i++)
		{
			spi_flash_read(FLASH_APP1_START_ADDR+USER_FLASH_SEC_SIZE*i,Upgrade.flash, USER_FLASH_SEC_SIZE);

			spi_flash_write(FLASH_EXE_APP1_START_ADDR+USER_FLASH_SEC_SIZE*i,Upgrade.flash, USER_FLASH_SEC_SIZE);
		}
    }

    os_free(Upgrade.flash);

    system_upgrade_flag_set(UPGRADE_FLAG_FINISH);//设置升级的标志
    system_upgrade_reboot();//重启


	Upgrade.isBusy = 0;
}


//#define GET_STR 	"GET /%s HTTP/1.1\r\nAccept: */*\r\nHost: %s\r\nConnection: Keep-Alive\r\n\r\n"
//#define POST_STR 	"POST /%s HTTP/1.1\r\nAccept: */*\r\nContent-Length: %d\r\nContent-Type: application/x-www-form-urlencoded\r\nHost: %s\r\nConnection: Keep-Alive\r\n\r\n%s"

LOCAL void ICACHE_FLASH_ATTR OTAFifoInit(void)
{
	OTAFifo.front = OTAFifo.rear = 0;//初始化时队列头队列首相连
	OTAFifo.count = 0;   //队列计数为0
}

// Queue In
LOCAL uint8 ICACHE_FLASH_ATTR OTAFifoIn(u8 sdat) //数据进入队列
{//一次只能入一个数据，保证队尾不会超过队头font，最多相等

  u8 i;

  if((OTAFifo.front == OTAFifo.rear) && (OTAFifo.count == OTA_SEC_SIZE))
  {      // full 判断如果队列满了，Queue->front == Queue->rear时也可能是空队列
     return Q_FULL;    //返回队列满的标志
  }
  else
  {
	Upgrade.ptr[OTAFifo.rear] = sdat;
	OTAFifo.rear = (OTAFifo.rear + 1) % OTA_SEC_SIZE;       //队尾在增长 ,数组下标不能越界
	OTAFifo.count = OTAFifo.count + 1;
	return Q_OPERATE_OK;
  }
}

LOCAL void ICACHE_FLASH_ATTR
upgrade_timeout_callback(void)
{
	ESP_DBG("\r\n\r\nupgrade_timeout_callback \r\n");
	Upgrade.start = 0;
	os_timer_disarm(&Upgrade.upgrade_timer);
}


LOCAL void ICACHE_FLASH_ATTR
upgrade_reboot_callback(void)
{
	ESP_DBG("\r\n\r\nupgrade_reboot_callback \r\n");
    system_upgrade_flag_set(UPGRADE_FLAG_FINISH);//设置升级的标志
    system_upgrade_reboot();//重启
}


LOCAL void ICACHE_FLASH_ATTR
upgrade_sus_callback(void)
{
	ESP_DBG("\r\n\r\nupgrade_sus_callback \r\n");
	os_timer_disarm(&Upgrade.upgrade_sus_timer);
/*
    system_upgrade_flag_set(UPGRADE_FLAG_FINISH);//设置升级的标志
    system_upgrade_reboot();//重启
*/
    hz_up_resp_upgrade(UPGRADE_UPGRADE_SUCCESS);
	os_timer_disarm(&Upgrade.reboot_timer);
	os_timer_setfn(&Upgrade.reboot_timer, (os_timer_func_t *)upgrade_reboot_callback, NULL);
	os_timer_arm(&Upgrade.reboot_timer, 1000, 0);		//1000ms

}


LOCAL void ICACHE_FLASH_ATTR
http_ota_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
//    char *pstr = NULL;
//    LOCAL char pbuffer[1024 * 2] = {0};
    struct espconn *pespconn = arg;
//    u16 i;
    u16 i;
    char *strLenAddr;
    char contentLenStr[20];
    u16 proStartAddr = 0;
    u8 gotCodeHead = 0;
    u8 lastWrite = 0;

    ESP_DBG("http_ota_recv_cb : length -> %d \r\n",length);

    if(Upgrade.start == 0)
    {
    	if(((char *)os_strstr(pusrdata,"404 Not Found")) != NULL)
    	{
    		ESP_DBG("404 Not Found!!!!!!\r\n");
    		return;
    	}

        strLenAddr = (char *)os_strstr(pusrdata,"Content-Length: ");

        os_memset(contentLenStr,0,sizeof(contentLenStr));

        if(strLenAddr != NULL)
        {
        	Upgrade.ptr = (u8 *)Upgrade.flash_buffer;

        	for(i=0;i<length;i++)
    		{
        		uart_tx_one_char_no_wait(UART1,pusrdata[i]);
    		}

        	ESP_DBG("Got content length :\r\n");

        	for(i=0;i<10;i++)
        	{
        		contentLenStr[i] = strLenAddr[i+16];
        		if(contentLenStr[i] == '\r' || contentLenStr[i] == '\n')
        		{
        			contentLenStr[i] = '\0';
        			break;
        		}
        	}
        	OTAFifo.contentLen = atoi(contentLenStr);

        	ESP_DBG("%s ---- %d \r\n" ,contentLenStr ,OTAFifo.contentLen);
        }

		for(i=0;i<length;i++)
		{//EA 04 00 02 04 00 10 40
			if((pusrdata[i] == 0XEA)&&
				(pusrdata[i+1] == 0X04)&&
				(pusrdata[i+2] == 0X00)&&
				(pusrdata[i+4] == 0X04)&&
				(pusrdata[i+5] == 0X00)&&
				(pusrdata[i+6] == 0X10)&&
				(pusrdata[i+7] == 0X40))
			{
				proStartAddr = i;
				gotCodeHead = 1;
				ESP_DBG("proStartAddr:%d \r\n" ,proStartAddr);
				break;
			}
		}

		if(gotCodeHead)
		{
			OTAFifo.remainbytes = OTAFifo.contentLen;

			OTAFifoInit();

			OTAFifo.writeSec = 0;

			for(i=0;i<(length-proStartAddr);i++)
			{
				OTAFifoIn(pusrdata[proStartAddr+i]);
			}

			OTAFifo.remainbytes -= (length-proStartAddr);

			OTAFifo.totalbytes = (length-proStartAddr);

			ESP_DBG("remainbytes:%d \r\n" , OTAFifo.remainbytes);

			ESP_DBG("totalbytes:%d \r\n" , OTAFifo.totalbytes);

			Upgrade.start = 1;

			os_timer_disarm(&Upgrade.upgrade_timer);
			os_timer_setfn(&Upgrade.upgrade_timer, (os_timer_func_t *)upgrade_timeout_callback, NULL);
			os_timer_arm(&Upgrade.upgrade_timer, 120000, 0);		//120秒
		}

    }
    else
    {

    	for(i=0;i<length;i++)
    	{
    		if((OTAFifoIn(pusrdata[i]) == Q_FULL))
			{
				if (system_upgrade_userbin_check() == UPGRADE_FW_BIN1)
				{
					ESP_DBG("Write addr :%X count:%X \r\n" , FLASH_EXE_APP2_START_ADDR+USER_FLASH_SEC_SIZE*OTAFifo.writeSec ,OTAFifo.count );
					spi_flash_write(FLASH_EXE_APP2_START_ADDR+USER_FLASH_SEC_SIZE*OTAFifo.writeSec,Upgrade.flash_buffer, USER_FLASH_SEC_SIZE);
				}
				else
				{
					ESP_DBG("Write addr :%X count:%X \r\n" , FLASH_EXE_APP1_START_ADDR+USER_FLASH_SEC_SIZE*OTAFifo.writeSec ,OTAFifo.count );
					spi_flash_write(FLASH_EXE_APP1_START_ADDR+USER_FLASH_SEC_SIZE*OTAFifo.writeSec,Upgrade.flash_buffer, USER_FLASH_SEC_SIZE);
				}
    			OTAFifoInit();
    			OTAFifo.writeSec++;
    			i--;		//塞不进，退一步重新塞
			}
    	}

    	OTAFifo.remainbytes -= length;

    	OTAFifo.totalbytes += length;

    	if(OTAFifo.remainbytes > 0)
    	{

    	}
    	else if(OTAFifo.remainbytes == 0)
        {

			if (system_upgrade_userbin_check() == UPGRADE_FW_BIN1)
			{
				ESP_DBG("Last Write addr :%X count:%X \r\n" , FLASH_EXE_APP2_START_ADDR+USER_FLASH_SEC_SIZE*OTAFifo.writeSec ,OTAFifo.count );
				spi_flash_write(FLASH_EXE_APP2_START_ADDR+USER_FLASH_SEC_SIZE*OTAFifo.writeSec,Upgrade.flash_buffer, USER_FLASH_SEC_SIZE);
			}
			else
			{
				ESP_DBG("Last Write addr :%X count:%X \r\n" , FLASH_EXE_APP1_START_ADDR+USER_FLASH_SEC_SIZE*OTAFifo.writeSec ,OTAFifo.count);
				spi_flash_write(FLASH_EXE_APP1_START_ADDR+USER_FLASH_SEC_SIZE*OTAFifo.writeSec,Upgrade.flash_buffer, USER_FLASH_SEC_SIZE);
			}


        	ESP_DBG("\r\n\r\nUpgrade done!!!!!!!!!!!!!!\r\n");
        	Upgrade.start = 0;


			os_timer_disarm(&Upgrade.upgrade_sus_timer);
			os_timer_setfn(&Upgrade.upgrade_sus_timer, (os_timer_func_t *)upgrade_sus_callback, NULL);
			os_timer_arm(&Upgrade.upgrade_sus_timer, 100, 0);		//100ms

/*
            system_upgrade_flag_set(UPGRADE_FLAG_FINISH);//设置升级的标志
            system_upgrade_reboot();//重启
*/
        }
        else
        {
        	ESP_DBG("\r\n\r\nUpgrade error:total bytes not fit !!!!!!!!!!!!!!\r\n");
        	Upgrade.start = 0;
        	os_timer_disarm(&Upgrade.upgrade_timer);
        }

        ESP_DBG("remainbytes:%d \r\n" , OTAFifo.remainbytes);

        ESP_DBG("totalbytes:%d \r\n" , OTAFifo.totalbytes);
    }

/*    for(i=0;i<length;i++)
    {
    	UART_SENDBYTE(pusrdata[i]);
    }*/
//    ESP_DBG("%s \r\n" , pusrdata);

#if 0
    ESP_DBG("======================================================\r\n" );

    for(i=0;i<length;i++)
    {
    	ESP_DBG("%02X " , pusrdata[i]);
    }
    ESP_DBG("\r\n" );
    ESP_DBG("======================================================\r\n" );
#endif
}

LOCAL void ICACHE_FLASH_ATTR
http_ota_sent_cb(void *arg)
{
    struct espconn *pespconn = arg;

    ESP_DBG("http_ota_sent_cb\n");
}


LOCAL void ICACHE_FLASH_ATTR
http_send(char *buf)
{
	ESP_DBG("Send:%s \r\n" , buf);

	espconn_sent(&Upgrade.ota_espconn,buf,os_strlen(buf));
}

LOCAL void ICACHE_FLASH_ATTR
http_ota_connect_cb(void *arg)
{
    struct espconn *pespconn = arg;

    ESP_DBG("http_ota_connect_cb\n");

    espconn_regist_recvcb(pespconn, http_ota_recv_cb);
    espconn_regist_sentcb(pespconn, http_ota_sent_cb);

    Upgrade.tcpConn = 1;
//    http_send("Hello http \r\n");
    http_send(Upgrade.send_buffer);
}


LOCAL void ICACHE_FLASH_ATTR
http_ota_discon_cb(void *arg)
{
    struct espconn *pespconn = arg;

    ESP_DBG("http_ota_discon_cb\n");

    Upgrade.tcpConn = 0;
}


LOCAL void ICACHE_FLASH_ATTR
http_ota_recon_cb(void *arg, sint8 err)
{
//    struct espconn *pespconn = (struct espconn *)arg;

    ESP_DBG("http_ota_recon_cb\n");

//    espconn_connect((struct espconn *)arg);

}


void ICACHE_FLASH_ATTR
http_parse_request_url(char *URL,char *host,char *filename,unsigned short *port)
{
    char *PA;
    char *PB;
    memset(host,0,sizeof(host));
    memset(filename,0,sizeof(filename));
    *port=0;
    if(!(*URL)) return;
    PA=URL;

    if(!strncmp(PA,"http://",strlen("http://")))
    {
        PA=URL+strlen("http://");
    }

    if(!strncmp(PA,"https://",strlen("https://")))
    {
        PA=URL+strlen("https://");
    }
    //以上是把PA的地址赋为域名的首地址,例如abc.domain.cnPA指向a
    PB=strchr(PA,'/');	//PB指向域名末尾的/

    if(PB)	//存在域名末尾的/
    {
        memcpy(host,PA,strlen(PA)-strlen(PB));
        if(PB+1)
        {
            memcpy(filename,PB+1,strlen(PB-1));
            filename[strlen(PB)-1]=0;
        }
        host[strlen(PA)-strlen(PB)]=0;
    }
    else
    {
        memcpy(host,PA,strlen(PA));
        host[strlen(PA)]=0;
    }


    PA=strchr(host,':');

    if(PA)
        *port=atoi(PA+1);
    else
        *port=80;
}

void ICACHE_FLASH_ATTR
http_parse_request_path(char *filename,char *path)
{
    char *PA;
    char *PB;


    memset(path,0,sizeof(path));

    if(!(*filename)) return;

    PA=filename;

    for(;;)
    {
        PB=strchr(PA,'/');

        if(PB)	//存在域名末尾的/
        {
        	PA = PB+1;
        }
        else
        {
        	memcpy(path,filename,strlen(filename)-strlen(PA));
        	break;
        }
    }

}


#if 0
LOCAL void http_ota_upgrade_rsp(void *arg)
{
    struct upgrade_server_info *server = arg;
    if (server->upgrade_flag == true)
	{
    	ESP_DBG("user_esp_platform_upgrade_successful\n");
		ESP_DBG("system_upgrade_reboot\n");
		system_upgrade_reboot();
    }
	else
    {
        ESP_DBG("user_esp_platform_upgrade_failed\n");
    }

    os_free(server->url);
    server->url = NULL;
    os_free(server);
    server = NULL;
}
#endif





void ICACHE_FLASH_ATTR http_ota_dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
    struct espconn *pespconn = (struct espconn *)arg;


    if (ipaddr == NULL)
    {
        ESP_DBG("http_ota_dns_found NULL\n");

        if(Upgrade.dns_try++ < 5)
        {
        	os_timer_disarm(&Upgrade.dns_timer);								//启动定时器循环发送消息
        	os_timer_setfn(&Upgrade.dns_timer, (os_timer_func_t *)http_gethostbyname, NULL);
        	os_timer_arm(&Upgrade.dns_timer, 1000, 0);
        }
        return;
    }

    ESP_DBG("http_ota_dns_found %d.%d.%d.%d\n",
            *((uint8 *)&ipaddr->addr), *((uint8 *)&ipaddr->addr + 1),
            *((uint8 *)&ipaddr->addr + 2), *((uint8 *)&ipaddr->addr + 3));

    if (ipaddr->addr != 0)
    {

        os_memcpy(pespconn->proto.tcp->remote_ip, &ipaddr->addr, 4);

        pespconn->proto.tcp->local_port = espconn_port();

        pespconn->proto.tcp->remote_port = Upgrade.remote_port;			//80端口

#if 0
//这里是用库的upgrade
        server->pespconn = pespconn;
        server->port = remote_port;												// 修改 服务器的 端口号   80
        server->check_cb = http_ota_upgrade_rsp;
        server->check_times = 120000;
        os_memcpy(server->ip, pespconn->proto.tcp->remote_ip, 4);

        os_sprintf(server->url,"GET /%s HTTP/1.1\r\nHost: "IPSTR":%d\r\n"OTA_HEADER_BUF"",
               filename,
               IP2STR(server->ip),
               server->port);

        os_sprintf(server->url,"GET /%s HTTP/1.1\r\nHost: "IPSTR":%d\r\nAccept: */*\r\nConnection: Keep-Alive\r\n\r\n",
               filename,
               IP2STR(server->ip),
               server->port);
        ESP_DBG("server->url:\r\n%s \r\n" , server->url);


        if (system_upgrade_start(server) == false)
    	{
            ESP_DBG("upgrade is already started\n");
        }

#else
        //自己从网页拉取文件并写入flash
        espconn_regist_connectcb(pespconn, http_ota_connect_cb);
        espconn_regist_disconcb(pespconn, http_ota_discon_cb);
        espconn_regist_reconcb(pespconn, http_ota_recon_cb);
        espconn_connect(pespconn);
#endif
    }
}

u8 ICACHE_FLASH_ATTR http_gethostbyname(void)
{
	struct ip_addr addr;
#if (UPGRADE_USE_DNS)

    ESP_DBG("espconn_gethostbyname:%s \r\n",Upgrade.host);

	espconn_gethostbyname(&Upgrade.ota_espconn,Upgrade.host, &addr,http_ota_dns_found);

#else
	ota_espconn.proto.tcp->remote_ip[0] = 192;
	ota_espconn.proto.tcp->remote_ip[1] = 168;
	ota_espconn.proto.tcp->remote_ip[2] = 1;
	ota_espconn.proto.tcp->remote_ip[3] = 55;

	ota_espconn.proto.tcp->local_port = espconn_port();

	ota_espconn.proto.tcp->remote_port = remote_port;			//80端口

	//自己从网页拉取文件并写入flash
	espconn_regist_connectcb(&ota_espconn, http_ota_connect_cb);
	espconn_regist_disconcb(&ota_espconn, http_ota_discon_cb);
	espconn_regist_reconcb(&ota_espconn, http_ota_recon_cb);
	espconn_connect(&ota_espconn);
#endif
}


u8 ICACHE_FLASH_ATTR http_upgrade_erase_task(void)
{
	u8 i;

	os_timer_disarm(&Upgrade.erase_timer);

    if (system_upgrade_userbin_check() == UPGRADE_FW_BIN1)
	{
		ESP_DBG("Erase EXE APP2 area \r\n");
		for(i=0;i<123;i++)
		{
			spi_flash_erase_sector(FLASH_EXE_APP2_START_SEC+i);
			system_soft_wdt_feed();
			ESP_DBG("%d ",i);
		}
		ESP_DBG("\r\n");
	}
	else if (system_upgrade_userbin_check() == UPGRADE_FW_BIN2)
	{
		ESP_DBG("Erase EXE APP1 area \r\n");
		for(i=0;i<123;i++)
		{
			spi_flash_erase_sector(FLASH_EXE_APP1_START_SEC+i);
			system_soft_wdt_feed();
			ESP_DBG("%d ",i);
		}
		ESP_DBG("\r\n");
	}

    Upgrade.dns_try = 0;

	os_timer_disarm(&Upgrade.dns_timer);								//启动定时器循环发送消息
	os_timer_setfn(&Upgrade.dns_timer, (os_timer_func_t *)http_gethostbyname, NULL);
	os_timer_arm(&Upgrade.dns_timer, 50, 0);
}


/*
 http://yisenh.com/uploads/soft/user1.4096.new.4.bin
 */

u8 ICACHE_FLASH_ATTR http_start_upgrade(char *URL)
{

	u8 i,j;
	u8 cannot_upgrade = UPGRADE_NORMAL;
	char *pstr1;

	ESP_DBG("**********************************************************\r\n" );

	ESP_DBG("\r\n**** http_start_upgrade--URL:%s***** \r\n" , URL);


	//判断是否可以升级
	if(wifi_station_get_connect_status() != STATION_GOT_IP)
	{
		ESP_DBG("UPGRADE_WIFI_DISCONN \r\n");
		cannot_upgrade = UPGRADE_WIFI_DISCONN;
	}

	if(Upgrade.tcpConn)
	{
		http_send(Upgrade.send_buffer);
		ESP_DBG("UPGRADE_NORMAL \r\n");
		cannot_upgrade = UPGRADE_NORMAL;
	}

	if(Upgrade.start)
	{
		ESP_DBG("UPGRADE_IS_IN_PROCCESS \r\n");
		cannot_upgrade = UPGRADE_IS_IN_PROCCESS;
	}

	if(cannot_upgrade)
	{
		ESP_DBG("********************************************************************************\r\n");
		return cannot_upgrade;
	}

	ESP_DBG("**********************************************************\r\n" );
	//解析链接域名，文件名称
    memset(Upgrade.userfilename,0,sizeof(Upgrade.userfilename));

    http_parse_request_url(URL , Upgrade.host , Upgrade.filename , &Upgrade.remote_port);

    ESP_DBG("filename: %s \r\n" , Upgrade.filename);

#if (UPGRADE_USE_DNS)

    pstr1 = strstr(Upgrade.filename,"user");
    if(pstr1)
    {
        if (system_upgrade_userbin_check() == UPGRADE_FW_BIN1)
    	{
        	*(pstr1+4) = '2';
        }
    	else if (system_upgrade_userbin_check() == UPGRADE_FW_BIN2)
        {
    		*(pstr1+4) = '1';
        }
    }
    else
    {
    	ESP_DBG("UPGRADE_WRONG_URI \r\n");
    	return UPGRADE_WRONG_URI;
    }

    os_memcpy(Upgrade.userfilename,Upgrade.filename,strlen(Upgrade.filename));

#else
	strcat(userfilename,"upgrade/");
    if (system_upgrade_userbin_check() == UPGRADE_FW_BIN1)
	{
    	strcat(userfilename,"user2.4096.new.4.bin");
    }
	else if (system_upgrade_userbin_check() == UPGRADE_FW_BIN2)
    {
		strcat(userfilename,"user1.4096.new.4.bin");
    }
#endif
    ESP_DBG("**********************************************************\r\n" );
	ESP_DBG("host: %s \r\n" , Upgrade.host);
	ESP_DBG("userfilename: %s \r\n" , Upgrade.userfilename);
	ESP_DBG("port: %d \r\n" , Upgrade.remote_port);

	ESP_DBG("**********************************************************\r\n" );

    ESP_DBG("**********************************************************\r\n" );

	os_memset(Upgrade.send_buffer,0,512);

	os_sprintf(Upgrade.send_buffer,"GET /%s HTTP/1.1\r\nAccept: */*\r\nHost: %s\r\nConnection: Keep-Alive\r\n\r\n",Upgrade.userfilename,Upgrade.host);


	Upgrade.ota_espconn.type = ESPCONN_TCP;
	Upgrade.ota_espconn.state = ESPCONN_NONE;
	Upgrade.ota_espconn.proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
	Upgrade.ota_espconn.proto.tcp->local_port = espconn_port();
	Upgrade.ota_espconn.proto.tcp->remote_port = 80;


	ESP_DBG("********************************************************************************\r\n");

	os_timer_disarm(&Upgrade.erase_timer);								//启动定时器循环发送消息
	os_timer_setfn(&Upgrade.erase_timer, (os_timer_func_t *)http_upgrade_erase_task, NULL);
	os_timer_arm(&Upgrade.erase_timer, 50, 0);

	return cannot_upgrade;
}





