///////////////////////////////////////
///////////////////////////////////////
#include "user_config.h"

//定义常数
#define UDP_SEND_INTERVAL	1000
const char udp_braodcast_ip[4]={255,255,255,255};//用于存放远程IP地址

//定义UDP广播数据的FIFO
#define UDP_DATA_ROW	50
#define UDP_DATA_LEN	UDP_SCENE_EXE_BUF_SIZE
LOCAL u8 UDPFifoBuffer[UDP_DATA_ROW][UDP_DATA_LEN];
QUEUE_ALLOC UDPFifo;

//任务用到的变量
UDPTASK UDPTask;

u8 testPort[2];

void ICACHE_FLASH_ATTR user_udp_task_init(void)
{
	os_memset(&UDPTask,0,sizeof(UDPTask));

	ESP_DBG("sizeof(UDPTask.scene) : %d \r\n" , sizeof(UDPTask.scene));

	UDPTask.app_ack = UDP_APP_STATE_INIT;

	if( ((char *)os_strstr(Device_Info.info, "Scene")) == NULL)	//不是情景面板，可以初始化客户端
	{
		UDPTask.localPort = UDP_SW_LOCAL_PORT;
	}
	else
	{
		UDPTask.localPort = UDP_SCENE_LOCAL_PORT;
	}

	UDPTask.remotePort = UDP_APP_PORT;
	UDPTask.app_ack = UDP_APP_STATE_INIT;

//QueueInit(&UDPFifo);

	QueueAllocInit(&UDPFifo,&UDPFifoBuffer[0][0],UDP_DATA_ROW,UDP_DATA_LEN);

	user_udp_power_on_read_flash();

#if	(UDP_SCENE_SET_TEST)
	{
		u8 A_MAC[6] = {0X5C,0XCF,0X7F,0X44,0XE9,0X27};
		u8 B_MAC[6] = {0X5C,0XCF,0X7F,0X44,0XB2,0XFB};

		if( ((char *)os_strstr(Device_Info.info, "Scene")) != NULL)	//情景设置
		{
			testPort[0] = 1;
			testPort[1] = 2;
			user_udp_scene_set_test(0,A_MAC,B_MAC);
			testPort[0] = 1;
			testPort[1] = 2;
			user_udp_scene_set_test(1,A_MAC,B_MAC);
			testPort[0] = 1;
			testPort[1] = 2;
			user_udp_scene_set_test(2,A_MAC,B_MAC);
	}
	}
#endif
	os_timer_disarm(&UDPTask.udp_task_timer);
	os_timer_setfn(&UDPTask.udp_task_timer, (os_timer_func_t *)user_udp_task, NULL);
	os_timer_arm(&UDPTask.udp_task_timer, 1000, 1);

}


void ICACHE_FLASH_ATTR user_udp_send(int remotePort)
{//UDP发送函数
	u16 sendLen;
    UDPTask.espconn.proto.udp->remote_port=remotePort;

    sendLen = UDP_DATA_LEN;
    if(sendLen)
    {
    	ESP_DBG("\r\nUDP server发送数据: " );
    	ESP_DBG_HEX_I("",UDPTask.send_str,sendLen);
        ESP_DBG("\r\n");

        espconn_sent(&UDPTask.espconn,UDPTask.send_str,sendLen);
//        ESP_DBG("\r\nUDP server发送数据:%s\r\n" , UDPTask.ser_cli_str);
    }
    else
    {
//    	ESP_DBG("\r\nUDP server发送数据 为空 !\r\n");
    }

}

void ICACHE_FLASH_ATTR user_udp_sent_cb(void *arg)
{//发送回调函数
	ESP_DBG("\r\nUDP server发送成功！\r\n");

    if(UDPTask.app_ack == UDP_APP_STATE_SEND)
    {
        os_timer_disarm(&UDPTask.udp_app_timer);//定个时发送
        os_timer_setfn(&UDPTask.udp_app_timer,user_udp_send,UDPTask.remotePort);
        os_timer_arm(&UDPTask.udp_app_timer,UDP_SEND_INTERVAL,0);		//定1秒钟发送一次
    }
}

void ICACHE_FLASH_ATTR user_udp_recv_cb(void *arg,char *pdata,unsigned short len)
{//接收回调函数
	ESP_DBG("UDP server已经接收数据：%s \r\n",pdata);//UDP接收到的数据打印出来
	ESP_DBG_HEX_I("",pdata,len);
    ESP_DBG("\r\n");
	user_udp_decode_data(pdata);

/*    if(pdata[0] == 'A' && pdata[1] == 'C' && pdata[2] == 'K')
    {
    	os_memset(UDPTask.send_str,0,sizeof(UDPTask.send_str));
    	UDPTask.app_ack = UDP_APP_STATE_ACK;
    	os_timer_disarm(&UDPTask.udp_app_timer);//定个时发送
    	//espconn_disconnect(&UDPTask.espconn);
    }*/
}


void ICACHE_FLASH_ATTR user_udp_init(void)
{
    static uint8 count=0;
    uint8 status;

	ESP_DBG("************* user_udp_init ***************\r\n");
	ESP_DBG("Local port:%d \r\n",UDPTask.localPort);
	ESP_DBG("Remote port:%d \r\n",UDP_APP_PORT);
	ESP_DBG("udp_remote_ip:%d:%d:%d:%d \r\n",udp_braodcast_ip[0],udp_braodcast_ip[1],udp_braodcast_ip[2],udp_braodcast_ip[3]);

//	wifi_set_broadcast_if(STATIONAP_MODE);//设置UDP广播的发送接口station+soft-AP模式发送
	UDPTask.espconn.type = ESPCONN_UDP;
	UDPTask.espconn.proto.udp = (esp_udp*)os_zalloc(sizeof(esp_udp));
	UDPTask.espconn.proto.udp->local_port = UDPTask.localPort;
	UDPTask.espconn.proto.udp->remote_port = UDPTask.remotePort;

	os_memcpy(&UDPTask.espconn.proto.udp->remote_ip,udp_braodcast_ip,4);

	espconn_regist_recvcb(&UDPTask.espconn,user_udp_recv_cb);//接收回调函数
	espconn_regist_sentcb(&UDPTask.espconn,user_udp_sent_cb);//发送回调函数
	espconn_create(&UDPTask.espconn);//创建UDP连接
}


u8 ICACHE_FLASH_ATTR user_udp_check_ip(void)
{
    struct ip_info ipconfig;
    uint8 wifi_state = wifi_station_get_connect_status();

    wifi_get_ip_info(STATION_IF, &ipconfig);

    if (wifi_state == STATION_GOT_IP && ipconfig.ip.addr != 0)
    {
    	return 1;
    }
    else
    {
    	return 0;
    }
}


void ICACHE_FLASH_ATTR user_udp_task(void)
{
	if(user_udp_check_ip() == 0) return;

	switch(UDPTask.app_ack)
	{
		case UDP_APP_STATE_INIT:
			user_udp_init();
			UDPTask.app_ack = UDP_APP_STATE_LOOP;
/*			os_sprintf(UDPTask.send_str,"MAC Address:"MACSTR" [%d]\r\n",MAC2STR(Device_Info.mac),UDPTask.localPort);//格式化MAC地址
			user_udp_send(UDPTask.remotePort); //发送出去
			UDPTask.stateDelay = 5;*/
			break;

		case UDP_APP_STATE_ACK:
			if(UDPTask.stateDelay)
			{
				UDPTask.stateDelay--;
				break;
			}
			UDPTask.app_ack = UDP_APP_STATE_LOOP;
			break;


		case UDP_APP_STATE_LOOP:
			break;

		default:
			break;
	}
}

#if	(UDP_SCENE_SET_TEST)
u8 scene_test_buff[2048];
void ICACHE_FLASH_ATTR user_udp_scene_set_test(u8 keyNum ,u8 *A_MAC ,u8 *B_MAC)
{

	u8 i;
	u8 startIndex;
	u8 onoff = 0X00;

	scene_test_buff[2] = keyNum;
	scene_test_buff[3] = 2;
	scene_test_buff[4] = 20;

	if(keyNum&1)
	{
		onoff = 0Xff;
	}


	//设置时间
	for(i=0;i<scene_test_buff[3];i++)
	{
		scene_test_buff[i*UDP_SCENE_TIME_BUF_SIZE+5] = 9;	//时
		scene_test_buff[i*UDP_SCENE_TIME_BUF_SIZE+6] = 10;	//分
		scene_test_buff[i*UDP_SCENE_TIME_BUF_SIZE+7] = 2;	//秒
		scene_test_buff[i*UDP_SCENE_TIME_BUF_SIZE+8] = 0XFF;	//每天都执行
	}

	//设置
	startIndex = i*UDP_SCENE_TIME_BUF_SIZE+5;
	for(i=0;i<scene_test_buff[4];i++)
	{

		if(i==0)
		{
			scene_test_buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+0] = A_MAC[0];
			scene_test_buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+1] = A_MAC[1];
			scene_test_buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+2] = A_MAC[2];
			scene_test_buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+3] = A_MAC[3];
			scene_test_buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+4] = A_MAC[4];
			scene_test_buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+5] = A_MAC[5];
		}
		else
		{
			scene_test_buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+0] = B_MAC[0];
			scene_test_buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+1] = B_MAC[1];
			scene_test_buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+2] = B_MAC[2];
			scene_test_buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+3] = B_MAC[3];
			scene_test_buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+4] = B_MAC[4];
			scene_test_buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+5] = B_MAC[5];
		}

		scene_test_buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+6] = 0XFF;
		scene_test_buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+7] = 0XFF;
		scene_test_buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+8] = 0XFF;
		scene_test_buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+9] = 0XFF;


		scene_test_buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+10] = onoff;
		scene_test_buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+11] = onoff;
		scene_test_buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+12] = onoff;
		scene_test_buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+13] = onoff;
	}


	user_udp_scene_set(scene_test_buff);
}
#endif

void ICACHE_FLASH_ATTR user_udp_scene_set(u8 *buff)
{
	u8 i;
	u8 sceneBuffIndex;
	u8 startIndex;


	if(Upgrade.start)
	{
		ESP_DBG("Upgrade is in progress , no set scene\r\n");
		return;
	}


	sceneBuffIndex = buff[2]-1;//UDPTask.scene.sceneNum;

	if( (buff[0] == 0) && (buff[1] == 0) )
	{
		memset(&UDPTask.scene.sce[sceneBuffIndex],0,sizeof(UDPTask.scene.sce[sceneBuffIndex]));
		ESP_DBG("清除情景定时: 电路%d \r\n" , sceneBuffIndex);
	}
	else
	{

		ESP_DBG("user_udp_scene_set :%d \r\n" , sceneBuffIndex);

		UDPTask.scene.sce[sceneBuffIndex].cirNum = 	buff[2];

		UDPTask.scene.sce[sceneBuffIndex].timeNum = buff[3];

		UDPTask.scene.sce[sceneBuffIndex].exeNum = buff[4];


		//复制情景定时
		for(i=0;i<buff[3];i++)
		{
			UDPTask.scene.sce[sceneBuffIndex].tim[i].hour = buff[i*UDP_SCENE_TIME_BUF_SIZE+5];
			UDPTask.scene.sce[sceneBuffIndex].tim[i].minute = buff[i*UDP_SCENE_TIME_BUF_SIZE+6];
			UDPTask.scene.sce[sceneBuffIndex].tim[i].second = buff[i*UDP_SCENE_TIME_BUF_SIZE+7];
			UDPTask.scene.sce[sceneBuffIndex].tim[i].week = buff[i*UDP_SCENE_TIME_BUF_SIZE+8];
		}
		ESP_DBG("Copy exe: ");
		//复制执行设备
		startIndex = i*UDP_SCENE_TIME_BUF_SIZE+5;	//5是时间buff的其实地址
		for(i=0;i<buff[4];i++)
		{
			UDPTask.scene.sce[sceneBuffIndex].exe[i].mac[0] = buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+0];
			UDPTask.scene.sce[sceneBuffIndex].exe[i].mac[1] = buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+1];
			UDPTask.scene.sce[sceneBuffIndex].exe[i].mac[2] = buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+2];
			UDPTask.scene.sce[sceneBuffIndex].exe[i].mac[3] = buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+3];
			UDPTask.scene.sce[sceneBuffIndex].exe[i].mac[4] = buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+4];
			UDPTask.scene.sce[sceneBuffIndex].exe[i].mac[5] = buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+5];

			UDPTask.scene.sce[sceneBuffIndex].exe[i].cirBit[0] = buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+6];
			UDPTask.scene.sce[sceneBuffIndex].exe[i].cirBit[1] = buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+7];
			UDPTask.scene.sce[sceneBuffIndex].exe[i].cirBit[2] = buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+8];
			UDPTask.scene.sce[sceneBuffIndex].exe[i].cirBit[3] = buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+9];

			UDPTask.scene.sce[sceneBuffIndex].exe[i].onoff[0] = buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+10];
			UDPTask.scene.sce[sceneBuffIndex].exe[i].onoff[1] = buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+11];
			UDPTask.scene.sce[sceneBuffIndex].exe[i].onoff[2] = buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+12];
			UDPTask.scene.sce[sceneBuffIndex].exe[i].onoff[3] = buff[startIndex+i*UDP_SCENE_EXE_BUF_SIZE+13];
			ESP_DBG("%d " , startIndex+i*UDP_SCENE_EXE_BUF_SIZE+13);
		}
		ESP_DBG("\r\n");
		UDPTask.scene.sceneNum++;

	}

	ESP_DBG("UDPTask.scene.sceneNum: %d \r\n" , UDPTask.scene.sceneNum);
	ESP_DBG("_____________________________________________________\r\n");

	user_udp_flash_write();

	user_udp_flash_read();

	for(i=0;i<buff[4];i++)
	{
		ESP_DBG(": %d ---- [%02X-%02X-%02X-%02X-%02X-%02X] [%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X]\r\n" , i ,
																	UDPTask.scene.sce[sceneBuffIndex].exe[i].mac[0] ,
																	UDPTask.scene.sce[sceneBuffIndex].exe[i].mac[1] ,
																	UDPTask.scene.sce[sceneBuffIndex].exe[i].mac[2] ,
																	UDPTask.scene.sce[sceneBuffIndex].exe[i].mac[3] ,
																	UDPTask.scene.sce[sceneBuffIndex].exe[i].mac[4] ,
																	UDPTask.scene.sce[sceneBuffIndex].exe[i].mac[5] ,
																	UDPTask.scene.sce[sceneBuffIndex].exe[i].cirBit[0]	,
																	UDPTask.scene.sce[sceneBuffIndex].exe[i].cirBit[1]	,
																	UDPTask.scene.sce[sceneBuffIndex].exe[i].cirBit[2]	,
																	UDPTask.scene.sce[sceneBuffIndex].exe[i].cirBit[3]	,
																	UDPTask.scene.sce[sceneBuffIndex].exe[i].onoff[0]	,
																	UDPTask.scene.sce[sceneBuffIndex].exe[i].onoff[1]	,
																	UDPTask.scene.sce[sceneBuffIndex].exe[i].onoff[2]	,
																	UDPTask.scene.sce[sceneBuffIndex].exe[i].onoff[3]);
	}

}

void ICACHE_FLASH_ATTR user_udp_flash_write(void)
{
	u8 i;
	u32 sceneLength;
	u8 secSize;
	u16 remainBytes;
//	u8 *ptr = (u8 *)&UDPTask.flash;

	sceneLength = sizeof(UDPTask.scene);

	secSize = sceneLength/USER_FLASH_SEC_SIZE;

	remainBytes = sceneLength%USER_FLASH_SEC_SIZE;

	ESP_DBG("*******************  Write UDP Flash  *********************** \r\n");

	ESP_DBG("secSize:%d \r\n" , secSize);
	ESP_DBG("remainBytes:%d \r\n" , remainBytes);

	UDPTask.scene.F_MSK = 0XBBCC;

//	UDPTask.scene.sceneNum = 0XAADD;

	ESP_DBG("F_MSK:%X \r\n" , UDPTask.scene.F_MSK);
	ESP_DBG("UDPTask.scene.sceneNum:%d \r\n" , UDPTask.scene.sceneNum);

	if(secSize)
	{
		for(i=0;i<secSize;i++)
		{
			ESP_DBG(":%X %X\r\n" , USER_SCENE_FLASH_START_ADDR+USER_FLASH_SEC_SIZE*i , USER_SCENE_FLASH_START_SEC+i);
			os_memcpy(UDPTask.flash,&UDPTask.scene+USER_FLASH_SEC_SIZE*i,USER_FLASH_SEC_SIZE);

			spi_flash_erase_sector(USER_SCENE_FLASH_START_SEC+i);

			spi_flash_write(USER_SCENE_FLASH_START_ADDR+USER_FLASH_SEC_SIZE*i,UDPTask.flash,USER_FLASH_SEC_SIZE);
		}

		if(remainBytes)
		{

			ESP_DBG("Write remainBytes:%X---%X---%d \r\n" ,USER_SCENE_FLASH_START_ADDR+USER_FLASH_SEC_SIZE*i,USER_SCENE_FLASH_START_SEC+i, remainBytes);

			os_memcpy(UDPTask.flash,&UDPTask.scene+USER_FLASH_SEC_SIZE*i,remainBytes);

			spi_flash_erase_sector(USER_SCENE_FLASH_START_SEC+i);

			spi_flash_write(USER_SCENE_FLASH_START_ADDR+USER_FLASH_SEC_SIZE*i,UDPTask.flash,USER_FLASH_SEC_SIZE);
		}
	}
	else
	{
		ESP_DBG(":%X %X\r\n" , USER_SCENE_FLASH_START_ADDR , USER_SCENE_FLASH_START_SEC+i);
		os_memcpy(UDPTask.flash,&UDPTask.scene,remainBytes);

		spi_flash_erase_sector(USER_SCENE_FLASH_START_SEC);

		spi_flash_write(USER_SCENE_FLASH_START_ADDR,UDPTask.flash,USER_FLASH_SEC_SIZE);
	}

	ESP_DBG("F_MSK##:%X \r\n" , UDPTask.scene.F_MSK);
	ESP_DBG("UDPTask.scene.sceneNum##:%d \r\n" , UDPTask.scene.sceneNum);
}

void ICACHE_FLASH_ATTR user_udp_flash_read(void)
{
	u8 i;
	u32 sceneLength;
	u8 secSize;
	u16 remainBytes;
//	u8 *ptr = (u8 *)&UDPTask.flash;

	sceneLength = sizeof(UDPTask.scene);

	secSize = sceneLength/USER_FLASH_SEC_SIZE;

	remainBytes = sceneLength%USER_FLASH_SEC_SIZE;

	ESP_DBG("*******************  Read UDP Flash  *********************** \r\n");

	ESP_DBG("secSize:%d \r\n" , secSize);
	ESP_DBG("remainBytes:%d \r\n" , remainBytes);

	if(secSize)
	{
		for(i=0;i<secSize;i++)
		{
			ESP_DBG(":%X \r\n" , USER_SCENE_FLASH_START_ADDR+USER_FLASH_SEC_SIZE*i);

			spi_flash_read(USER_SCENE_FLASH_START_ADDR+USER_FLASH_SEC_SIZE*i,UDPTask.flash,USER_FLASH_SEC_SIZE);
			os_memcpy(&UDPTask.scene+USER_FLASH_SEC_SIZE*i,UDPTask.flash,USER_FLASH_SEC_SIZE);
		}

		if(remainBytes)
		{
			ESP_DBG("Read remainBytes:%X---%d \r\n" , USER_SCENE_FLASH_START_ADDR+USER_FLASH_SEC_SIZE*i , remainBytes);
			spi_flash_read(USER_SCENE_FLASH_START_ADDR+USER_FLASH_SEC_SIZE*i,UDPTask.flash,USER_FLASH_SEC_SIZE);
			os_memcpy(&UDPTask.scene+USER_FLASH_SEC_SIZE*i,UDPTask.flash,remainBytes);
		}
	}
	else
	{
		ESP_DBG(":%X \r\n" , USER_SCENE_FLASH_START_ADDR);
		spi_flash_read(USER_SCENE_FLASH_START_ADDR,UDPTask.flash,USER_FLASH_SEC_SIZE);
		os_memcpy(&UDPTask.scene,UDPTask.flash,remainBytes);
	}



	ESP_DBG("F_MSK:%X \r\n" , UDPTask.scene.F_MSK);
	ESP_DBG("UDPTask.scene.sceneNum:%d \r\n" , UDPTask.scene.sceneNum);
}



void ICACHE_FLASH_ATTR user_udp_power_on_read_flash(void)
{
	user_udp_flash_read();

	if(UDPTask.scene.F_MSK != 0XBBCC)
	{
		os_memset(&UDPTask.scene,0,sizeof(UDPTask.scene));
		UDPTask.scene.F_MSK = 0XBBCC;
		user_udp_flash_write();
		user_udp_flash_read();
	}
}



void ICACHE_FLASH_ATTR user_udp_exeSceneByList(u8 list)
{
	u8 e;
	u8 inData[UDP_DATA_LEN];
	u8 QRet;
	u8 repeat = 3;

	ESP_DBG("user_udp_exeSceneByList : %d \r\n" , list);


	while(repeat--)
	{
		for(e=0;e<UDPTask.scene.sce[list].exeNum;e++)
		{

			inData[0] = UDPTask.scene.sce[list].exe[e].mac[0];
			inData[1] = UDPTask.scene.sce[list].exe[e].mac[1];
			inData[2] = UDPTask.scene.sce[list].exe[e].mac[2];
			inData[3] = UDPTask.scene.sce[list].exe[e].mac[3];
			inData[4] = UDPTask.scene.sce[list].exe[e].mac[4];
			inData[5] = UDPTask.scene.sce[list].exe[e].mac[5];

			inData[6] = UDPTask.scene.sce[list].exe[e].cirBit[0];
			inData[7] = UDPTask.scene.sce[list].exe[e].cirBit[1];
			inData[8] = UDPTask.scene.sce[list].exe[e].cirBit[2];
			inData[9] = UDPTask.scene.sce[list].exe[e].cirBit[3];

			inData[10] = UDPTask.scene.sce[list].exe[e].onoff[0];
			inData[11] = UDPTask.scene.sce[list].exe[e].onoff[1];
			inData[12] = UDPTask.scene.sce[list].exe[e].onoff[2];
			inData[13] = UDPTask.scene.sce[list].exe[e].onoff[3];

			ESP_DBG(": %d ---- [%02X-%02X-%02X-%02X-%02X-%02X]\r\n" , e , inData[0] ,inData[1],inData[2],inData[3],inData[4],inData[5]);

			QRet = QueueAllocIn(&UDPFifo,inData);
	/*		if(QRet == QUEUE_OPERATE_OK)
			{
				ESP_DBG("QueueIn OK \r\n");
			}
			else
			{
				ESP_DBG("QueueIn Err \r\n");
			}*/

		}
	}
}


void ICACHE_FLASH_ATTR user_udp_decode_data(u8 *buff)
{
	u8 i;
	u8 cmd[8];
	u8 channel;
	u8 onoff;

	if( ((char *)os_strstr(Device_Info.info, "Scene")) != NULL) return;	//情景不需要解析数据

	//判断是否是自己的数据包
	if( (buff[0] !=Device_Info.mac[0]) ||
		(buff[1] !=Device_Info.mac[1]) ||
		(buff[2] !=Device_Info.mac[2]) ||
		(buff[3] !=Device_Info.mac[3]) ||
		(buff[4] !=Device_Info.mac[4]) ||
		(buff[5] !=Device_Info.mac[5]) )
	{
		ESP_DBG("Not my data , return!\r\n");
		return;
	}

	os_memcpy(cmd,buff+6,8);


	if( (((char *)os_strstr(Device_Info.info, "Curtain1T")) != NULL) || (((char *)os_strstr(Device_Info.info, "Window1T")) != NULL))
	{
		if(cmd[0]&BIT(0))
		{
			channel = 0;
			if(cmd[4]&BIT(0))
			{
				onoff = 1;
			}
			else
			{
				onoff = 0;
			}
		}
		else if(cmd[0]&BIT(2))
		{
			channel = 2;
			if(cmd[4]&BIT(2))
			{
				onoff = 1;
			}
			else
			{
				onoff = 0;
			}
		}
		TimerCurtainCloudControl(channel,onoff);	//带缓存的控制，要先判断其他电路是否打开，再决定是立即打开当前通道还是延迟打开当前通道
		hz_up_resp_Curtain1T(channel);
	}
	else if( ((char *)os_strstr(DEV_INFO_STRING, "ADJ")) != NULL )
	{
		if(cmd[0]&BIT(1))
		{
			if(cmd[4]&BIT(1))
			{
				led_relay_state.led[1] = 1;
			}
			else
			{
				led_relay_state.led[1] = 0;
			}
			peripheral_send_to_adj_controller();	//发送给STM8控制器
			hz_up_resp_lumin(1);
		}
	}
	else
	{
		for(i=0;i<LED_CHANNEL_NUM;i++)
		{
			if(cmd[i/8] & (1<<i))		//cmd[0~3]是有效位标志
			{
				if(cmd[i/8+4] & (1<<i))	//cmd[4~7]是表示开关
				{
					led_relay_state.led[i] = 1;
					ESP_DBG("led[%d] on \r\n" , i);
				}
				else
				{
					led_relay_state.led[i] = 0;
					ESP_DBG("led[%d] off \r\n" , i);
				}
			}
		}
		hz_up_sw_status();
	}
}

void ICACHE_FLASH_ATTR user_udp_pop_data(void)
{
	u8 outData[UDP_DATA_LEN];
	u8 i;
	static u8 dly = 0;

	if(dly++ < 10) return;	//10*10ms=100ms
	dly = 0;

	if(QueueAllocOut(&UDPFifo,outData) == Q_OPERATE_OK)
	{
/**********************************************************************************
		os_sprintf(UDPTask.send_str,"Port:%d [%02X %02X %02X %02X] [%02X %02X %02X %02X] \r\n" ,
				LocalPortTable[outData[0]] ,outData[1],outData[2],outData[3],outData[4],outData[5],outData[6],outData[7],outData[8] );		//格式化MAC地址
**********************************************************************************/
		for(i=0;i<UDP_DATA_LEN;i++)
		{
			UDPTask.send_str[i] = outData[i];
		}
		UDPTask.send_str[i++] = 'A';
		user_udp_send(UDP_SW_LOCAL_PORT); //发送出去
	}

}


void ICACHE_FLASH_ATTR user_udp_scene_timing_handle(void)
{
	u8 i;
	u8 t;
	u8 week_msk;

	if( ((char *)os_strstr(Device_Info.info, "Scene")) == NULL) return;

	if(UDPTask.app_ack != UDP_APP_STATE_LOOP) return;

	week_msk = 0X00;
	if(time_info.week == 1) week_msk = 0X80;
	if(time_info.week == 2) week_msk = 0X40;
	if(time_info.week == 3) week_msk = 0X20;
	if(time_info.week == 4) week_msk = 0X10;
	if(time_info.week == 5) week_msk = 0X08;
	if(time_info.week == 6) week_msk = 0X04;
	if(time_info.week == 0) week_msk = 0X02;

	for(i=0;i<UDPTask.scene.sceneNum;i++)
	{
		for(t=0;t<UDP_SCENE_TIME_MAX;t++)
		{
			if((UDPTask.scene.sce[i].tim[t].hour == time_info.hour) &&
					(UDPTask.scene.sce[i].tim[t].minute == time_info.minute) &&
					(UDPTask.scene.sce[i].tim[t].second == time_info.second) &&
					(UDPTask.scene.sce[i].tim[t].week&week_msk))
			{
				user_udp_exeSceneByList(i);
			}
		}

	}
}


void ICACHE_FLASH_ATTR user_udp_scene_manual_handle(u8 num)
{

	u8 i;
	u8 t;

	if( ((char *)os_strstr(Device_Info.info, "Scene")) == NULL) return;

	if(UDPTask.app_ack != UDP_APP_STATE_LOOP) return;

	ESP_DBG("user_udp_scene_manual_handle : %d \r\n" , num);

	user_udp_exeSceneByList(num);

/*	for(i=0;i<UDPTask.scene.sceneNum;i++)
	{
		for(t=0;t<UDP_SCENE_TIME_MAX;t++)
		{
			if((UDPTask.scene.sce[i].cirNum == num))
			{
				user_udp_exeSceneByList(i);
			}
		}

	}*/

}




