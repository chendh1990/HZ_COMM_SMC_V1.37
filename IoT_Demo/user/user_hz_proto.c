/******************************************************************************

*******************************************************************************/
#include "user_config.h"

u8 ReceiveBuffer[CLOUD_DATA_MAX];
u8 TransmitBuffer[CLOUD_DATA_MAX];
u8 recvMd5Buffer[16];

HZDataHead 			HZ_Data_Head;
HZDataPayload 		HZ_Data_Payload;
HZPolicyTimeControl	Policy_Time_Info;
HZPolicySunControl 	Policy_Sun_Info;
HZPolicyGPSControl 	Policy_GPS_Info;
HZPolicyBuf 		Policy_Buf;
HZCountDownBuf 		Count_Down_Info;
HZMiscBuf			HZ_Misc_Buf;

TIME_INFO 			HZ_Timestamp_Info;

LOCAL os_timer_t 	timer_trig_up_sw_sta_once;
LOCAL os_timer_t 	timer_count_down;
LOCAL os_timer_t 	timer_write_flash;

LOCAL hz_cloud_send_cb_t hz_cloud_send_callback = NULL;
LOCAL hz_cloud_restart_heartbeat_cb_t hz_cloud_restart_heartbeat_callback = NULL;

void ICACHE_FLASH_ATTR
hz_cloud_send_callback_register(hz_cloud_send_cb_t hz_cloud_send_cb)
{
	hz_cloud_send_callback = hz_cloud_send_cb;
}

void ICACHE_FLASH_ATTR
hz_cloud_restart_heartbeat_callback_register(hz_cloud_restart_heartbeat_cb_t hz_cloud_restart_heartbeat_cb)
{
	hz_cloud_restart_heartbeat_callback = hz_cloud_restart_heartbeat_cb;
}


#if 0
/* *************************************************************************
�����ַ���������תΪ����
 *************************************************************************/
u8 ICACHE_FLASH_ATTR
week_check(char *buf)
{
	if(buf[0] == 'S' && buf[1] == 'u' && buf[2] == 'n') return 7;
	if(buf[0] == 'M' && buf[1] == 'o' && buf[2] == 'n') return 1;
	if(buf[0] == 'T' && buf[1] == 'u' && buf[2] == 'e') return 2;
	if(buf[0] == 'W' && buf[1] == 'e' && buf[2] == 'n') return 3;
	if(buf[0] == 'T' && buf[1] == 'h' && buf[2] == 'u') return 4;
	if(buf[0] == 'F' && buf[1] == 'r' && buf[2] == 'i') return 5;
	if(buf[0] == 'S' && buf[1] == 'a' && buf[2] == 't') return 6;

	return 0;
}
#endif

/* *************************************************************************
�������ݰ�ͷ
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_set_data_head(u16 cmd,u8 repeat)
{
	u32 time;
	//��������
	HZ_Data_Head.cmd[0] = (cmd>>0)&0X00FF;		//0-1	����
	HZ_Data_Head.cmd[1] = (cmd>>8)&0X00FF;
	//�豸ID
/*	HZ_Data_Head.devID[0] = Device_Info.id[0];	//2-7 �豸id
	HZ_Data_Head.devID[1] = Device_Info.id[1];
	HZ_Data_Head.devID[2] = Device_Info.id[2];
	HZ_Data_Head.devID[3] = Device_Info.id[3];
	HZ_Data_Head.devID[4] = Device_Info.id[4];
	HZ_Data_Head.devID[5] = Device_Info.id[5];*/

	HZ_Data_Head.devID[0] = Device_Info.mac[0];	//2-7 �豸id  WIFI��Ʒmac��ַ���豸IDһ��
	HZ_Data_Head.devID[1] = Device_Info.mac[1];
	HZ_Data_Head.devID[2] = Device_Info.mac[2];
	HZ_Data_Head.devID[3] = Device_Info.mac[3];
	HZ_Data_Head.devID[4] = Device_Info.mac[4];
	HZ_Data_Head.devID[5] = Device_Info.mac[5];

	//�����ַ
	HZ_Data_Head.mac[0] = Device_Info.mac[0];	//mac_addr[0];
	HZ_Data_Head.mac[1] = Device_Info.mac[1];	//mac_addr[1];
	HZ_Data_Head.mac[2] = Device_Info.mac[2];	//mac_addr[2];
	HZ_Data_Head.mac[3] = Device_Info.mac[3];	//mac_addr[3];
	HZ_Data_Head.mac[4] = Device_Info.mac[4];	//mac_addr[4];
	HZ_Data_Head.mac[5] = Device_Info.mac[5];	//mac_addr[5];	//8-13	mac��ַ
	//Э��汾
	HZ_Data_Head.pVer = PROTO_VER;		//14
	//Ʒ��1������
	HZ_Data_Head.brand = BRAND;			//15

	//�豸����
	os_memcpy(HZ_Data_Head.devType,Device_Info.p,sizeof(HZ_Data_Head.devType));	//16-22 ; 16,17,18,19,20,21,22

	//�豸�̼��汾��
	HZ_Data_Head.fwVer = FW_VER;		//23
	//�ط�����
	HZ_Data_Head.repeat = repeat;		//24
	//ʱ��
	time = rtc_read_stamp();
	HZ_Data_Head.time[0] = time&0X000000FF;
	HZ_Data_Head.time[1] = (time>>8)&0X000000FF;
	HZ_Data_Head.time[2] = (time>>16)&0X000000FF;
	HZ_Data_Head.time[3] = (time>>24)&0X000000FF;
//	HZ_Data_Head.time = rtc_read_stamp();//25-28//sntp_get_current_timestamp();	��������ʱ���ǻ�ȡ����ʱ��ģ�����ʹ��RTC�Ƚϱ��գ���ʹ����������ʱ���ܻ�������

	Sec2Date(&time_info,time);

	ESP_HZ_DBG("Data Head time: %d %02d-%02d %02d:%02d:%02d \r\n",time_info.year,
																time_info.month,
																time_info.day,
																time_info.hour,
																time_info.minute,
																time_info.second );


/*	memset(realTimeStr,0,sizeof(realTimeStr));
	os_sprintf(realTimeStr,sntp_get_real_time(HZ_Data_Head.time));
	ESP_HZ_DBG("\r\nReal time: %s \r\n",realTimeStr );
	ESP_HZ_DBG("\r\nWeek: %d \r\n",week_check(realTimeStr));*/
}

/* *************************************************************************
������������
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_set_data(u16 len,u8 *payload)
{
	HZ_Data_Payload.dataLen[0] = (len>>0)&0X00FF;
	HZ_Data_Payload.dataLen[1] = (len>>8)&0X00FF;
	memset(HZ_Data_Payload.payload,0,sizeof(HZ_Data_Payload.payload));
	memcpy(HZ_Data_Payload.payload,payload,len);
}

/* *************************************************************************
 * ����Э����˵��Ҫ���ܵ��ֶ��ǹ̶��ģ�ֻ�����ݲ�ͬ
 * ���Կ��԰�MD5������װ�������Э��������ݽ��м���
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_md5_package(u8 *inBuffer,u16 len,u8 *outBuffer)
{
	char encrypt[CLOUD_DATA_MAX];
	u16 i;
	u8 index = 0;

//	ESP_DBG_FUN_START();
/////////////////////////////////////////////////////
	os_memset(encrypt,0,sizeof(encrypt));

	for(i=0;i<len;i++)
	{
		encrypt[index++] = inBuffer[i];
	}

	os_memcpy(encrypt+index,MD5_KEY,os_strlen(MD5_KEY));
	index += os_strlen(MD5_KEY);

#if 0
	ESP_HZ_DBG("key size:%d\r\n",os_strlen(MD5_KEY));
	ESP_HZ_DBG("\r\npackage encrypt char:\r\n");
	UART_ShowByteInCharHex(encrypt,index);
#endif

	MD5Digest(encrypt,index,outBuffer);

#if 0
	ESP_HZ_DBG("\r\npackage encrypt result:\r\n");
	UART_ShowByteInCharHex(outBuffer,16);
#endif

//	ESP_DBG_FUN_END();

///////////////////////////////////////////////////
}

/* *************************************************************************
 *
 *************************************************************************/

/* *************************************************************************
 * �����������
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_package_trans_buffer(void)
{
	u16 len;
	char szDigest[16];
	u16 i;

//	ESP_DBG_FUN_START();

	//�������ݷ��ͻ���
	memset(TransmitBuffer,0,sizeof(TransmitBuffer));
	//��������ͷ
	len = 0;
	memcpy(TransmitBuffer+len,(u8 *)&HZ_Data_Head,sizeof(HZ_Data_Head));
	//�������ݳ���
	len += sizeof(HZ_Data_Head);
	memcpy(TransmitBuffer+len,HZ_Data_Payload.dataLen,2);
	//������������
	len += 2;
	memcpy(TransmitBuffer+len,(u8 *)&HZ_Data_Payload.payload,HZ_Data_Payload.dataLen[1]<<8|HZ_Data_Payload.dataLen[0]);

	len += HZ_Data_Payload.dataLen[1]<<8|HZ_Data_Payload.dataLen[0];
	//////////����MD5///////////////////////////////////////////
	hz_md5_package(TransmitBuffer,len,szDigest);

#if 0
	ESP_HZ_DBG("\r\nencrypt result:\r\n");
	UART_ShowByteInCharHex(szDigest,16);
	ESP_HZ_DBG("\r\n");
#endif
///////////////////////////////////////////////////
	for(i=0;i<16;i++)
	{
		HZ_Data_Payload.md5[i] = szDigest[i];
	}
	//����MD5����
	memcpy(TransmitBuffer+len,(u8 *)&HZ_Data_Payload.md5,sizeof(HZ_Data_Payload.md5));
	len += sizeof(HZ_Data_Payload.md5);

	//��ӡҪ���͵�����
	ESP_DBG_HEX_I("TransmitBuffer:\r\n" , TransmitBuffer , len);

	hz_cloud_restart_heartbeat_callback();

	//��������
	hz_cloud_send_callback(TransmitBuffer,len);

//	ESP_DBG_FUN_END();
}


/* *************************************************************************
 * ����һ������
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_up_sw_status(void)
{
	u8 buf[4];
	u16 i;


	ESP_DBG_FUN_START();

	//��������ͷ
	hz_set_data_head(HZ_CMD_UP_SW,1);
	//�������������ʵ�����ݺ����ݳ���

	memset(buf,0,sizeof(4));
	for(i=0;i<LED_CHANNEL_NUM;i++)
	{
		if(led_relay_state.led[i])
		{
			buf[i/8] |= (1<<(i%8));
		}
		else
		{
			buf[i/8] &=~(1<<(i%8));
		}
	}

	hz_set_data(4,buf);

	hz_package_trans_buffer();

	ESP_DBG_FUN_END();
}

/* *************************************************************************
 * ����һ������
 ************************************************************************
void ICACHE_FLASH_ATTR
hz_up_sw_respon(u8 channel,u8 onoff)
{
	u8 buf[6];
	u16 i;

	ESP_HZ_DBG("\r\n*****hz_up_sw_status********start*****************************\r\n");

	buf[0] = channel+1;		//��0��ʼ���ƶ��Ǵ�1��ʼ
	buf[1] = onoff;

	ESP_HZ_DBG("\r\n*****channel:%d ********onoff:%d*****************************\r\n",channel,onoff);

	//��������ͷ
	hz_set_data_head(HZ_CMD_LED_TG,1);
	//�������������ʵ�����ݺ����ݳ���
	hz_set_data(2,buf);

	hz_package_trans_buffer();
}*/

/* *************************************************************************
 * ����һ������
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_up_resp_status(u16 resp,u16 cmd)
{
	u8 buf[6];
	u16 i;

	ESP_HZ_DBG("\r\n*****hz_up_resp_status********start*****************************\r\n");

	buf[0] = resp&0X00FF;		//��0��ʼ���ƶ��Ǵ�1��ʼ
	buf[1] = (resp>>8)&0X00FF;

	//��������ͷ
	hz_set_data_head(cmd,1);
	//�������������ʵ�����ݺ����ݳ���
	hz_set_data(2,buf);

	hz_package_trans_buffer();
}


/* *************************************************************************
 * ����һ������
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_up_resp_time_ctl_status(u8 *inbuf)
{
	u8 buf[100];
	u16 i;
	u8 len;
	u8 channel;

	ESP_HZ_DBG("\r\n*****hz_up_resp_time_ctl_status********start*****************************\r\n");

	//��������ͷ
	hz_set_data_head(HZ_CMD_UP_TIME_CTL,1);

	channel = hz_circuit_buf_to_num(inbuf);

	len = 6;
	buf[0] = channel+1;		//�洢λ���Ǵ�0��ʼ������Ǵ�1��ʼ
	buf[1] = inbuf[5];
	buf[2] = inbuf[6];
	buf[3] = inbuf[7];
	buf[4] = 0x02;
	buf[5] = inbuf[9];

	UART_ShowByteInCharHex(buf,len);
	ESP_HZ_DBG("\r\n");

	hz_set_data(len,buf);

	hz_package_trans_buffer();
}

/* *************************************************************************
 * ����һ������
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_up_resp_count_down_status(u8 channel)
{
	u8 buf[100];
	u16 i;
	u8 len;

	ESP_HZ_DBG("\r\n*****hz_up_resp_count_down_status********start*****************************\r\n");

	//��������ͷ
	hz_set_data_head(HZ_CMD_UP_CNT,1);
	if(channel == 0X00)
	{
		len = LED_CHANNEL_NUM*6;
		for(i=0;i<LED_CHANNEL_NUM;i++)
		{
			buf[i*6+0] = i+1;	//�洢λ���Ǵ�0��ʼ������Ǵ�1��ʼ
			buf[i*6+1] = Count_Down_Info.cnt_bk[i]&0X00FF;
			buf[i*6+2] = (Count_Down_Info.cnt_bk[i]>>8)&0X00FF;
			buf[i*6+3] = Count_Down_Info.cnt[i]&0X00FF;
			buf[i*6+4] = (Count_Down_Info.cnt[i]>>8)&0X00FF;
			buf[i*6+5] = Count_Down_Info.ok[i];		//1�����У�2��ɣ�3ʧ��
		}
	}
	else
	{
		channel -= 1;			//�洢λ���Ǵ�0��ʼ������Ǵ�1��ʼ
		len = 6;
		buf[0] = channel+1;		//�洢λ���Ǵ�0��ʼ������Ǵ�1��ʼ
		buf[1] = Count_Down_Info.cnt_bk[channel]&0X00FF;
		buf[2] = (Count_Down_Info.cnt_bk[channel]>>8)&0X00FF;
		buf[3] = Count_Down_Info.cnt[channel]&0X00FF;
		buf[4] = (Count_Down_Info.cnt[channel]>>8)&0X00FF;
		buf[5] = Count_Down_Info.ok[channel];		//1�����У�2��ɣ�3ʧ��

	}
	UART_ShowByteInCharHex(buf,len);
	ESP_HZ_DBG("\r\n");

	hz_set_data(len,buf);

	hz_package_trans_buffer();
}
/* *************************************************************************
 * ����һ������
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_up_resp_scene(u8 channel)
{
	u8 buf[4];
	u8 len;
	u8 i;

	ESP_DBG_FUN_START();

	buf[0] = 0X00;
	buf[1] = 0X00;
	buf[2] = 0X00;
	buf[3] = 0X00;

	//��������ͷ
	hz_set_data_head(HZ_CMD_UP_SCENE,1);

	for(i=0;i<LED_CHANNEL_NUM;i++)
	{
		if(i==channel)
		{
			buf[i/8] |= (1<<i);
		}
	}

	hz_set_data(4,buf);
	hz_package_trans_buffer();

	ESP_DBG_FUN_END();
}


/* *************************************************************************
 * ����һ������
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_up_resp_Curtain1T(u8 channel)
{
	u8 buf[4];
	u8 len;
	u8 i;

	ESP_DBG_FUN_START();

	buf[0] = 0X00;
	buf[1] = 0X00;

	buf[0] = channel+1;
	if(led_relay_state.ledOnCnt[channel])
	{
		buf[1] = 0X01;
	}

	//��������ͷ
	hz_set_data_head(HZ_CMD_UP_TCURTAIN,1);

	hz_set_data(2,buf);
	hz_package_trans_buffer();

	ESP_DBG_FUN_END();
}

/* *************************************************************************
 * ����һ������
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_up_resp_lumin(u8 channel)
{
	u8 buf[100];
	u8 len;
	u8 i;

#define HZ_LUMIN_DATA_SIZE	3

	ESP_DBG_FUN_START();

	//��������ͷ
	hz_set_data_head(HZ_CMD_UP_LUMIN,1);

	if(channel == 0X00)
	{
		len = HZ_LUMIN_DATA_SIZE*LED_CHANNEL_NUM;

		for(i=0;i<LED_CHANNEL_NUM;i++)
		{
			buf[i*HZ_LUMIN_DATA_SIZE+0] = i+1;		//�洢λ���Ǵ�0��ʼ������Ǵ�1��ʼ
			buf[i*HZ_LUMIN_DATA_SIZE+1] = HZ_Misc_Buf.lumin[i];
			buf[i*HZ_LUMIN_DATA_SIZE+2] = led_relay_state.led[1];
		}
		hz_set_data(len,buf);
		hz_package_trans_buffer();
	}
	else
	{
		len = HZ_LUMIN_DATA_SIZE;

		buf[0] = channel;						//�洢λ���Ǵ�0��ʼ������Ǵ�1��ʼ
		buf[1] = HZ_Misc_Buf.lumin[channel-1];	//�ڶ��ֽ�������ֵ
		buf[2] = led_relay_state.led[1];		//�����ֽ��ǿ���״̬
		hz_set_data(len,buf);
		hz_package_trans_buffer();
	}

	ESP_DBG_FUN_END();
}




/* *************************************************************************
 * ������������
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_accept_lumin(void)
{
	u16 i;
	u8 cloud_ctrl_idx;

	ESP_DBG_FUN_START();

	for(i=0;i<LED_CHANNEL_NUM;i++)
	{
		cloud_ctrl_idx = i/8;//LED_CHANNEL_NUM;
		if(Policy_Buf.led_ctrl_lumin[cloud_ctrl_idx]&(1<<(i%8)))
		{
			HZ_Misc_Buf.lumin[i] = Policy_Buf.led_ctrl_lumin[4];
		}
	}
	ESP_DBG_FUN_END();

	led_relay_state.lumin = HZ_Misc_Buf.lumin[1];

	peripheral_send_to_adj_controller();	//���͸�STM8������

}



/* *************************************************************************
 * ����һ������
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_up_resp_RGB(u8 channel)
{
	u8 buf[100];
	u8 len;
	u8 i;

	ESP_DBG_FUN_START();

	//��������ͷ
	hz_set_data_head(HZ_CMD_UP_RGB,1);

	if(channel == 0X00)
	{
		len = 2+RGB_GROUP_NUM*4;

		buf[0] = 0X00;		//�洢λ���Ǵ�0��ʼ������Ǵ�1��ʼ
		buf[1] = 0X00;
		for(i=0;i<RGB_GROUP_NUM;i++)
		{
			buf[2+i*4] = i+1;
			buf[3+i*4] = HZ_Misc_Buf.RGB[i][0];
			buf[4+i*4] = HZ_Misc_Buf.RGB[i][1];
			buf[5+i*4] = HZ_Misc_Buf.RGB[i][2];
		}

		hz_set_data(len,buf);
		hz_package_trans_buffer();

	}
	else
	{
		len = 6;

		buf[0] = 0X00;		//�洢λ���Ǵ�0��ʼ������Ǵ�1��ʼ
		buf[1] = 0X00;
		buf[2] = channel;
		buf[3] = HZ_Misc_Buf.RGB[channel-1][0];
		buf[4] = HZ_Misc_Buf.RGB[channel-1][1];
		buf[5] = HZ_Misc_Buf.RGB[channel-1][2];

		hz_set_data(len,buf);
		hz_package_trans_buffer();
	}

	ESP_DBG_FUN_END();
}

/* *************************************************************************
 * ������������
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_accept_RGB(void)
{
	u16 i;
	u8 cloud_ctrl_idx;

	ESP_DBG_FUN_START();

	HZ_Misc_Buf.RGB[Policy_Buf.led_ctrl_rgb[0]][0] = Policy_Buf.led_ctrl_rgb[1];
	HZ_Misc_Buf.RGB[Policy_Buf.led_ctrl_rgb[0]][1] = Policy_Buf.led_ctrl_rgb[2];
	HZ_Misc_Buf.RGB[Policy_Buf.led_ctrl_rgb[0]][2] = Policy_Buf.led_ctrl_rgb[3];

	ESP_DBG_FUN_END();
}



/* *************************************************************************
 * ����һ������
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_up_resp_Cur_Vol_Consum(u8 channel,u8 type)
{
	u8 buf[100];
	u8 len;
	u8 i;
	u16 *pbuf;
	u16 cmd;

	if(type == 0)
	{
		pbuf = HZ_Misc_Buf.cur;
		cmd = HZ_CMD_UP_CUR;
	}
	else if(type == 1)
	{
		pbuf = HZ_Misc_Buf.vol;
		cmd = HZ_CMD_UP_VOL;
	}
	else if(type == 2)
	{
		pbuf = HZ_Misc_Buf.consum;
		cmd = HZ_CMD_UP_W;
	}
	ESP_DBG_FUN_START();

	//��������ͷ
	hz_set_data_head(cmd,1);

	if(channel == 0X00)
	{
		len = LED_CHANNEL_NUM*3;
		for(i=0;i<LED_CHANNEL_NUM;i++)
		{
			buf[0+i*3] = i+1;
			buf[1+i*3] = (pbuf[i]>>8)&0X00FF;
			buf[2+i*3] = pbuf[i]&0X00FF;
		}

		hz_set_data(len,buf);
		hz_package_trans_buffer();

	}
	else
	{
		len = 3;
		buf[0] = channel;
		buf[1] = (pbuf[channel-1]>>8)&0X00FF;
		buf[2] = pbuf[channel-1]&0X00FF;

		hz_set_data(len,buf);
		hz_package_trans_buffer();
	}

	ESP_DBG_FUN_END();
}

/*
#define HZ_CMD_UP_CUR			0x020B
#define HZ_CMD_UP_VOL			0x020D
#define HZ_CMD_UP_W				0x020F
#define HZ_CMD_UP_TEMP			0x0211
#define HZ_CMD_UP_HUM			0x0213
#define HZ_CMD_UP_PM25			0x0215
#define HZ_CMD_UP_SMK			0x0217
*/
/* *************************************************************************
 * ����һ������
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_up_resp_Temp_Humi_PM25_Smoke_(u8 type)
{
	u8 buf[10];
	u8 len;
	u8 i;
	u16 pbuf;
	u16 cmd;

	if(type == 0)
	{
		pbuf = HZ_Misc_Buf.temp;
		cmd = HZ_CMD_UP_TEMP;
	}
	else if(type == 1)
	{
		pbuf = HZ_Misc_Buf.humi;
		cmd = HZ_CMD_UP_HUM;
	}
	else if(type == 2)
	{
		pbuf = HZ_Misc_Buf.pm25;
		cmd = HZ_CMD_UP_PM25;
	}
	else if(type == 3)
	{
		pbuf = HZ_Misc_Buf.smoke;
		cmd = HZ_CMD_UP_SMK;
	}
	ESP_DBG_FUN_START();

	//��������ͷ
	hz_set_data_head(cmd,1);

	len = 2;
	buf[0] = (pbuf>>8)&0X00FF;
	buf[1] = pbuf&0X00FF;

	hz_set_data(len,buf);
	hz_package_trans_buffer();

	ESP_DBG_FUN_END();
}


/* *************************************************************************
 * �������������ݵ�����
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_up_head(u16 cmd)
{
	u16 len;

	hz_set_data_head(cmd,1);

	memset(TransmitBuffer,0,sizeof(TransmitBuffer));
	len = 0;
	memcpy(TransmitBuffer+len,(u8 *)&HZ_Data_Head,sizeof(HZ_Data_Head));
	len += sizeof(HZ_Data_Head);

	UART_ShowByteInCharHex(TransmitBuffer,len);
	ESP_HZ_DBG("\r\n");

	hz_cloud_send_callback(TransmitBuffer,len);
}

/* *************************************************************************
 * ����һ������
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_up_signin_status(void)
{
	ESP_DBG_FUN_START();

	hz_up_head(HZ_CMD_SIGNIN);

	ESP_DBG_FUN_END();
}




/* *************************************************************************
 * ����һ������
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_up_login_status(void)
{

	ESP_DBG_FUN_START();

	hz_up_head(HZ_CMD_LOGIN);

	hz_trig_up_sw_once();

	ESP_DBG_FUN_END();
}

/* *************************************************************************
 * ����������
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_send_heartbeat(void)
{

	ESP_DBG_FUN_START();

	hz_up_head(HZ_CMD_HEARTBEAT);

	ESP_DBG_FUN_END();
}


/* *************************************************************************
 * ��ȡʱ���
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_request_timestamp(void)
{

	ESP_DBG_FUN_START();

	hz_up_head(HZ_CMD_TIMESTAMP);

	ESP_DBG_FUN_END();
}


/* *************************************************************************
 * �жϽ��յ������ݵ�MAC��ַ���豸ID�Ƿ�ƥ��
 *************************************************************************/
bool ICACHE_FLASH_ATTR
hz_check_dev_mac_id(void)
{
	u8 i;
	bool result;

	result = TRUE;

/*	for(i=0;i<sizeof(Device_Info.id);i++)
	{
		if(ReceiveBuffer[i+HZ_DATA_HEAD_ID_START_IDX] != Device_Info.id[i])
		{
			result = FALSE;
			break;
		}
	}*/

	for(i=0;i<sizeof(Device_Info.mac);i++)
	{
		if(ReceiveBuffer[i+HZ_DATA_HEAD_MAC_START_IDX] != Device_Info.mac[i])
		{
			result = FALSE;
			break;
		}
	}

	return result;
}


/* *************************************************************************
 * �ж�MD5�Ƿ���ȷ
 *************************************************************************/
bool ICACHE_FLASH_ATTR
hz_check_md5(void)
{
	bool result;
	u8 index = 0;
	char szDigest[16];
	u16 dataLen;
	u16 i;

	////////////////////////////////////////////////
	//hz_md5_fill_test();
/*	ESP_HZ_DBG("\r\n*****hz_check_md5******test test test test test test test testtesttesttesttest**********************\r\n");
	u8 testBuf[] =
	{
			0X00,	0X01,	0XAA,	0X55,	0XBB,	0X66,	0XCC,	0X77,	0X00,	0X00,
			0X00,	0X00,	0X00,	0X00,	0X00,	0X00,	0X00,	0X00,	0X00,	0X00,
			0X00,	0X00,	0X00,	0X9D,	0XA9,	0XFA,	0X58,	0X05,	0X00,	0X01,
			0X00,	0X00,	0X00,	0X01,	0X9B,	0XE0,	0XCE,	0X95,	0XA7,	0X9A,
			0XAF,	0X52,	0X6C,	0XC3,	0X18,	0X88,	0X69,	0XC2,	0X02,	0XF1
	};
	os_memcpy(ReceiveBuffer,testBuf,sizeof(testBuf));*/

	ESP_DBG_FUN_START();
	ESP_HZ_DBG("sizeof(u32):%d \r\n",sizeof(u32));
	ESP_HZ_DBG("sizeof(u16):%d \r\n",sizeof(u16));
	ESP_HZ_DBG("sizeof(HZDataPayload):%d \r\n",sizeof(HZDataPayload));
	ESP_HZ_DBG("sizeof(HZPolicyTimeControl):%d \r\n",sizeof(HZPolicyTimeControl));
	ESP_HZ_DBG("sizeof(HZPolicySunControl):%d \r\n",sizeof(HZPolicySunControl));
	ESP_HZ_DBG("sizeof(HZPolicyGPSControl):%d \r\n",sizeof(HZPolicyGPSControl));
	ESP_HZ_DBG("sizeof(HZPolicyBuf):%d \r\n",sizeof(HZPolicyBuf));
	////////////////////////////////////////////////

	dataLen = ReceiveBuffer[HZ_DATA_HEAD_DATA_LEN_H_IDX];
	dataLen <<= 8;
	dataLen |= ReceiveBuffer[HZ_DATA_HEAD_DATA_LEN_H_IDX-1];

	ESP_HZ_DBG("dataLen:%d\r\n",dataLen);

	if(dataLen > (CLOUD_DATA_MAX-20))
	{
		ESP_HZ_DBG("\r\n Clound data error ---- dataLen too large!!! \r\n");
		return FALSE;
	}


	//�������յ���MD5
	for(i=0;i<16;i++)
	{
		recvMd5Buffer[i] = ReceiveBuffer[i+sizeof(HZ_Data_Head)+dataLen+2];
		ESP_HZ_DBG("%02X ",recvMd5Buffer[i]);
	}

	//������յ������ݵ�MD5
	/////////////////////////////////////////////////////
	hz_md5_package(ReceiveBuffer,sizeof(HZ_Data_Head)+dataLen+2,szDigest);

	ESP_HZ_DBG("\r\nhz_check_md5 encrypt result:\r\n");
	UART_ShowByteInCharHex(szDigest,16);


	ESP_HZ_DBG("\r\n");
////�ж������Ƿ�һ��//////////////////////////
	result = TRUE;
	for(i=0;i<16;i++)
	{
		if(recvMd5Buffer[i] != szDigest[i])
		{
			result = FALSE;
		}
	}

	ESP_HZ_DBG("check md5 result: %d \r\n",result);
	ESP_DBG_FUN_END();
	return result;
}

/* *************************************************************************
 * ����ʱ���ƵĶ�ʱ��������
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_shut_cnt_down(u8 channel)
{
	Policy_Buf.led_cnt_down[channel/8] &=~(1<<(channel%8));
	Count_Down_Info.cnt[channel] = 0;
}


void ICACHE_FLASH_ATTR
hz_initial_count_down(void)
{
	memset(&Policy_Buf.led_cnt_down,0,sizeof(Policy_Buf.led_cnt_down));
	memset(&Count_Down_Info,0,sizeof(Count_Down_Info));
}


void ICACHE_FLASH_ATTR
hz_timer_up_sw_once_isr(void)
{
	os_timer_disarm(&timer_trig_up_sw_sta_once);
	hz_up_sw_status();
}


void ICACHE_FLASH_ATTR
hz_trig_up_sw_once(void)
{
	os_timer_disarm(&timer_trig_up_sw_sta_once);
	os_timer_setfn(&timer_trig_up_sw_sta_once, (os_timer_func_t *)hz_timer_up_sw_once_isr, NULL);
	os_timer_arm(&timer_trig_up_sw_sta_once, 500, 0);
}





/* *************************************************************************
 * ���ڵ���ʱ���ƣ���ֱ�ӿ��Ƶ�buffer
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_deal_curtain_onoff(u8 *buf)
{
	u8 onoff;
	u8 channel;

	channel = buf[0];
	onoff = buf[1];

	ESP_HZ_DBG("TCurtain Cloud Ctrl ---- [channel:%d] [onoff:%d] \r\n",channel,onoff);

	TimerCurtainCloudControl(channel-1,onoff);
}


/* *************************************************************************
 * ���ڵ���ʱ���ƣ���ֱ�ӿ��Ƶ�buffer
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_deal_buf_onoff(u8 *buf,u8 need_up_now)
{
	u8 cloud_ctrl_idx;
	u8 i;
	u8 led_relay_tmp[LED_CHANNEL_NUM];
	u8 need_up = FALSE;
	u8 ctlbuf[10];

	ESP_DBG("****************\r\n");
	for(i=0;i<5;i++)
	{
		ESP_DBG("%02X ",buf[i]);
	}
	ESP_DBG("\r\n");

	os_memcpy(led_relay_tmp,led_relay_state.led,LED_CHANNEL_NUM);

	if(buf[4] == 0X01)
	{
		for(i=0;i<LED_CHANNEL_NUM;i++)			//Policy_Buf.led_ctrl4���ֽڴ����·������һ�ֽڵ�0λΪ��һ·���ڶ�λΪ�ڶ�·���ڶ��ֽڵĵ�һ·Ϊ��9·...
		{										//led_relay_state.led[0]Ϊ��һ·�Ŀ���״̬��led_relay_state.led[1]Ϊ�ڶ�·�Ŀ���״̬...
			cloud_ctrl_idx = i/8;//LED_CHANNEL_NUM;
			if(buf[cloud_ctrl_idx]&(1<<(i%8)))
			{
				led_relay_state.led[i] = 0X01;
			}
		}
	}
	else
	{
		for(i=0;i<LED_CHANNEL_NUM;i++)
		{
			cloud_ctrl_idx = i/8;//LED_CHANNEL_NUM;
			if(buf[cloud_ctrl_idx]&(1<<(i%8)))
			{
				led_relay_state.led[i] = 0X00;
			}
		}
	}

	for(i=0;i<LED_CHANNEL_NUM;i++)	//�ж��Ƿ���Ҫ�ϴ�״̬
	{
		if(led_relay_tmp[i] != led_relay_state.led[i])
		{
			need_up = TRUE;
			ESP_DBG("CLOUD TOG LED:%d -> %d \r\n",i,led_relay_state.led[i]);
			if( ((char *)os_strstr(DEV_INFO_STRING, "ADJ")) != NULL )
			{
				peripheral_send_to_adj_controller();	//���͸�STM8������
			}

			if( (((char *)os_strstr(DEV_INFO_STRING, "Curtain1T")) != NULL) || (((char *)os_strstr(DEV_INFO_STRING, "Window1T")) != NULL) )
			{
				ctlbuf[0] = i+1;
				ctlbuf[1] = led_relay_state.led[i];
				hz_deal_curtain_onoff(ctlbuf);
			}
		}
	}
	if(need_up_now == 1)
	{
		hz_up_sw_status();
	}
	else if(need_up_now == 0)
	{
		if(need_up)		//����ƶ˿��Ƶ�״̬����ǰ״̬��һ���ģ���ô���ϴ����ر���
		{
			hz_trig_up_sw_once();
		}
	}
}




#if 0
/* *************************************************************************
 * ����ʱ���ƵĶ�ʱ��������
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_count_down_isr(void)
{
	u8 i;

	for(i=0;i<LED_CHANNEL_NUM;i++)
	{
		if(Count_Down_Info.cnt[i])
		{
			Count_Down_Info.cnt[i]--;
			if(Count_Down_Info.cnt[i] == 1)
			{
				led_relay_state.led[i] = Count_Down_Info.onoff[i];
				hz_up_sw_status(i,led_relay_state.led[i]);
				Count_Down_Info.ok[i] = 2;		//���
				ESP_HZ_DBG("count done:%d %d \r\n",i,Count_Down_Info.cnt[i]);
			}
		}
	}
}
#else
/* *************************************************************************
 * ����ʱ���ƵĶ�ʱ��������
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_count_down_isr(void)
{
	LOCAL u8 i = 0;

	if(Count_Down_Info.cnt[i])
	{
		Count_Down_Info.cnt[i]--;
		if(Count_Down_Info.cnt[i] == 1)
		{
			led_relay_state.led[i] = Count_Down_Info.onoff[i];
//			hz_up_sw_status();
			Count_Down_Info.ok[i] = 2;					//���
			hz_up_resp_count_down_status(i+1);			//�ϱ�����ʱ״̬
			ESP_HZ_DBG("count done:%d %d \r\n",i,Count_Down_Info.cnt[i]);
		}
	}
	if(i < LED_CHANNEL_NUM-1)
	{
		i++;
	}
	else
	{
		i = 0;
	}
}
#endif

/* *************************************************************************
 * ����ȡ������ʱ�жϣ�����ʱʱ��Ϊ0��ʱ��
 *************************************************************************/
void hz_accept_count_down_time(u8 *buf)
{
	u8 i;

	for(i=0;i<LED_CHANNEL_NUM;i++)
	{
		if(buf[i/8]&(1<<(i%8)))				//�ж���·��Ҫ��¼
		{
			Count_Down_Info.cnt[i] = buf[6]<<8|buf[5];		//���ֽ���ǰ
			Count_Down_Info.cnt_bk[i] = Count_Down_Info.cnt[i];
			Count_Down_Info.onoff[i] = buf[4];
			Count_Down_Info.ok[i] = 1;					//������
			if(Count_Down_Info.cnt[i] == 0)
			{
				Count_Down_Info.ok[i] = 2;				//�յ��Ķ�ʱʱ��Ϊ0���
			}
			ESP_HZ_DBG("accept cnt down:%d %d \r\n",i,Count_Down_Info.cnt[i]);
		}
	}
}



/* *************************************************************************
 * �����ʱ����
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_clear_policy_time(void)
{
	u16 i;
	u16 j;

	ESP_DBG_FUN_START();

	for(i=0;i<HZ_TIME_CTRL_BUF_SIZE;i++)
	{
		for(j=0;j<HZ_TIME_CTRL_BUF_GROUP_SIZE;j++)
		{
			Policy_Time_Info.buf[i][j] = 0;
		}
	}

	ESP_DBG_FUN_END();
}


void ICACHE_FLASH_ATTR
hz_up_resp_upgrade(u8 status)
{
	u8 buf[2];
	u8 len;
	u8 i;

	ESP_DBG_FUN_START();

	buf[0] = status;

	//��������ͷ
	hz_set_data_head(HZ_CMD_UP_UPGRADE,1);

	hz_set_data(1,buf);
	hz_package_trans_buffer();

	ESP_DBG_FUN_END();
}

/* *************************************************************************
 * ��������
 *************************************************************************/
void ICACHE_FLASH_ATTR
hz_decode_data(void)
{
	u16 cmd;
	u16 dataLen;
	u16 i;
	u16 j;
	u8 cpyIndex = 0;
	u32 stamp_tmp;
	u16 policyDataLen;
	u8 dataContent[500];
	s8 timezone = 0;

	dataLen = ReceiveBuffer[HZ_DATA_HEAD_DATA_LEN_H_IDX];
	dataLen <<= 8;
	dataLen |= ReceiveBuffer[HZ_DATA_HEAD_DATA_LEN_H_IDX-1];

	if(!hz_check_dev_mac_id() || !hz_check_md5()) return;

	cmd = ReceiveBuffer[1];
	cmd <<= 8;
	cmd |= ReceiveBuffer[0];

	stamp_tmp = ReceiveBuffer[28];
	stamp_tmp <<= 8;
	stamp_tmp |= ReceiveBuffer[27];
	stamp_tmp <<= 8;
	stamp_tmp |= ReceiveBuffer[26];
	stamp_tmp <<= 8;
	stamp_tmp |= ReceiveBuffer[25];


	timezone = (s8)ReceiveBuffer[21];
//	timezone = 8;					//Ҫ����ʱ��

	stamp_tmp += (timezone*HOUR);

	Sec2Date(&HZ_Timestamp_Info,stamp_tmp);

	ESP_LTT_DBG("Cloud data in time:[%ld:%d] %d %02d-%02d %02d:%02d:%02d \r\n",stamp_tmp,timezone,
																	HZ_Timestamp_Info.year,
																	HZ_Timestamp_Info.month,
																	HZ_Timestamp_Info.day,
																	HZ_Timestamp_Info.hour,
																	HZ_Timestamp_Info.minute,
																	HZ_Timestamp_Info.second );

#if (USE_CN_TIME_POOL == 0)
	rtc_time.hz_stamp = stamp_tmp;
	put_msg_fifo(MSG_TIMESTAMP);
#endif
/*******************************�������Ӧid**************************/
	HZ_Data_Head.reqOrResId[0] = ReceiveBuffer[29];
	HZ_Data_Head.reqOrResId[1] = ReceiveBuffer[30];
	switch(cmd)
	{
		case HZ_CMD_LED_TG://�·����ؿ���
			ESP_DBG("cmd == HZ_CMD_LED_TG\r\n");
			Policy_Buf.led_ctrl[0] = ReceiveBuffer[HZ_DATA_PLY_START_IDX+0];
			Policy_Buf.led_ctrl[1] = ReceiveBuffer[HZ_DATA_PLY_START_IDX+1];
			Policy_Buf.led_ctrl[2] = ReceiveBuffer[HZ_DATA_PLY_START_IDX+2];
			Policy_Buf.led_ctrl[3] = ReceiveBuffer[HZ_DATA_PLY_START_IDX+3];
			Policy_Buf.led_ctrl[4] = ReceiveBuffer[HZ_DATA_PLY_START_IDX+4];
			peripheral_beep_n10ms(BEEP_TIME_SHORT);
			hz_deal_buf_onoff(Policy_Buf.led_ctrl,0);		//���������ϱ����ص�״̬
			hz_up_resp_status(HZ_CMD_RESP_OK,cmd);			//���سɹ���Ӧ
			break;

		case HZ_CMD_LED_CNTDOWN://�·�����ʱ����
			ESP_HZ_DBG("cmd == HZ_CMD_LED_CNTDOWN\r\n");
			Policy_Buf.led_cnt_down[0] = ReceiveBuffer[HZ_DATA_PLY_START_IDX+0];
			Policy_Buf.led_cnt_down[1] = ReceiveBuffer[HZ_DATA_PLY_START_IDX+1];
			Policy_Buf.led_cnt_down[2] = ReceiveBuffer[HZ_DATA_PLY_START_IDX+2];
			Policy_Buf.led_cnt_down[3] = ReceiveBuffer[HZ_DATA_PLY_START_IDX+3];
			Policy_Buf.led_cnt_down[4] = ReceiveBuffer[HZ_DATA_PLY_START_IDX+4];
			Policy_Buf.led_cnt_down[5] = ReceiveBuffer[HZ_DATA_PLY_START_IDX+5];
			Policy_Buf.led_cnt_down[6] = ReceiveBuffer[HZ_DATA_PLY_START_IDX+6];
			if( ((char *)os_strstr(Device_Info.info, "Timing")) != NULL)		//�ж�ʱ����
			{
				hz_accept_count_down_time(Policy_Buf.led_cnt_down);
				os_timer_disarm(&timer_count_down);
				os_timer_setfn(&timer_count_down, (os_timer_func_t *)hz_count_down_isr, 1);
				os_timer_arm(&timer_count_down, 1000/LED_CHANNEL_NUM, 1);
				hz_up_resp_status(HZ_CMD_RESP_OK,cmd);
			}
			else																//û�ж�ʱ����
			{
				os_timer_disarm(&timer_count_down);
				hz_up_resp_status(HZ_CMD_RESP_FAIL,cmd);
			}
			break;

		case HZ_CMD_LED_LUMIN://�·����ȿ���
			ESP_HZ_DBG("cmd == HZ_CMD_LED_LUMIN\r\n");
			Policy_Buf.led_ctrl_lumin[0] = ReceiveBuffer[HZ_DATA_PLY_START_IDX+0];
			Policy_Buf.led_ctrl_lumin[1] = ReceiveBuffer[HZ_DATA_PLY_START_IDX+1];
			Policy_Buf.led_ctrl_lumin[2] = ReceiveBuffer[HZ_DATA_PLY_START_IDX+2];
			Policy_Buf.led_ctrl_lumin[3] = ReceiveBuffer[HZ_DATA_PLY_START_IDX+3];
			Policy_Buf.led_ctrl_lumin[4] = ReceiveBuffer[HZ_DATA_PLY_START_IDX+4];
			hz_accept_lumin();
			hz_up_resp_status(HZ_CMD_RESP_OK,cmd);
			break;


		case HZ_CMD_LED_RGB://�·�RGB����
			ESP_HZ_DBG("cmd == HZ_CMD_LED_RGB\r\n");
			Policy_Buf.led_ctrl_rgb[0] = ReceiveBuffer[HZ_DATA_PLY_START_IDX+0];
			Policy_Buf.led_ctrl_rgb[1] = ReceiveBuffer[HZ_DATA_PLY_START_IDX+1];
			Policy_Buf.led_ctrl_rgb[2] = ReceiveBuffer[HZ_DATA_PLY_START_IDX+2];
			Policy_Buf.led_ctrl_rgb[3] = ReceiveBuffer[HZ_DATA_PLY_START_IDX+3];
			hz_accept_RGB();
			hz_up_resp_status(HZ_CMD_RESP_OK,cmd);
			break;

		case HZ_CMD_LED_POLICY://�·�LED���Կ���
			ESP_HZ_DBG("cmd == HZ_CMD_LED_POLICY\r\n");
			if(ReceiveBuffer[HZ_DATA_PLY_TYPE_IDX] == HZ_POLICY_TIME)	//������ʼ��ַHZ_DATA_PLY_TIME_DATA_IDX
			{
				ESP_HZ_DBG("type HZ_POLICY_TIME\r\n");
				cpyIndex = 0;
				memset(&Policy_Time_Info,0,sizeof(Policy_Time_Info));
				os_memcpy(&Policy_Time_Info,ReceiveBuffer+HZ_DATA_PLY_START_IDX,dataLen);
/*				hz_add_time_policy(ReceiveBuffer+HZ_DATA_PLY_TIME_DATA_IDX);*/
/*				policyDataLen = (ReceiveBuffer[HZ_DATA_PLY_START_IDX+2]<<8)|ReceiveBuffer[HZ_DATA_PLY_START_IDX+1];*/
				policyDataLen = (Policy_Time_Info.dataLen[1]<<8)|Policy_Time_Info.dataLen[0];
				ESP_HZ_DBG("policyDataLen:%d \n" , policyDataLen);
				if(policyDataLen == 1)		//��ʱ�������ݳ���Ϊ1
				{
					hz_clear_policy_time();
				}
				ESP_HZ_DBG("Policy_Time_Info.buf: \r\n");
				for(i=0;i<HZ_TIME_CTRL_BUF_SIZE;i++)
				{
					UART_ShowByteInCharHex(Policy_Time_Info.buf[i],HZ_TIME_CTRL_BUF_GROUP_SIZE);
					ESP_HZ_DBG("\r\n");
				}

				//���涨ʱ���ݵ�Flash
				hz_write_time_policy_to_flash();

				ESP_HZ_DBG("\r\n");
			}
			if(ReceiveBuffer[HZ_DATA_PLY_TYPE_IDX] == HZ_POLICY_SUN)
			{
				ESP_HZ_DBG("type HZ_POLICY_SUN\r\n");
				os_memcpy(&Policy_Sun_Info,ReceiveBuffer+HZ_DATA_PLY_START_IDX,dataLen);
			}
			if(ReceiveBuffer[HZ_DATA_PLY_TYPE_IDX] == HZ_POLICY_GPS)
			{
				ESP_HZ_DBG("type HZ_POLICY_GPS\r\n");
				os_memcpy(&Policy_GPS_Info,ReceiveBuffer+HZ_DATA_PLY_START_IDX,dataLen);
			}
			hz_up_resp_status(HZ_CMD_RESP_OK,cmd);
			break;


		case HZ_CMD_SET_SCENE:	//�·������龰
			ESP_HZ_DBG("HZ_CMD_SET_SCENE\r\n");
			memset(dataContent,0,sizeof(dataContent));
			for(i=0;i<dataLen;i++)	//��������ĳ���
			{
				dataContent[i] = ReceiveBuffer[HZ_DATA_PLY_START_IDX+i];
				ESP_HZ_DBG("%02X " , dataContent[i]);
			}
			ESP_HZ_DBG("\r\n");
			user_udp_scene_set(dataContent);
			break;

		case HZ_CMD_LED_CURTAIN:
			ESP_HZ_DBG("HZ_CMD_LED_CURTAIN\r\n");
			dataContent[0] = ReceiveBuffer[HZ_DATA_PLY_START_IDX];
			dataContent[1] = ReceiveBuffer[HZ_DATA_PLY_START_IDX+1];
			hz_deal_curtain_onoff(dataContent);
			hz_up_resp_status(HZ_CMD_RESP_OK,cmd);			//���سɹ���Ӧ
			break;


		case HZ_CMD_UPGRADE:
			ESP_HZ_DBG("HZ_CMD_UPGRADE\r\n");
			memset(dataContent,0,sizeof(dataContent));
			for(i=0;i<dataLen;i++)	//��������ĳ���
			{
				dataContent[i] = ReceiveBuffer[HZ_DATA_PLY_START_IDX+i];
				UART_SENDBYTE(dataContent[i]);
			}
			ESP_HZ_DBG("\r\n");
			hz_up_resp_upgrade(http_start_upgrade(dataContent));
			break;

//***********************          ����������        **************************************************//
/*		case HZ_CMD_REQ_POLICY://������ԣ�ȥ������
			ESP_HZ_DBG("cmd == HZ_CMD_REQ_POLICY\r\n");
			break;

		case HZ_CMD_REQ_SW://���󿪹�״̬
			ESP_HZ_DBG("cmd == HZ_CMD_REQ_SW\r\n");
			hz_up_sw_status();
			break;

		case HZ_CMD_REQ_LUMIN://��������
			hz_up_resp_lumin(ReceiveBuffer[HZ_DATA_PLY_START_IDX+0]);
			ESP_HZ_DBG("cmd == HZ_CMD_REQ_LUMIN\r\n");
			break;


		case HZ_CMD_REQ_RGB://����RGB
			hz_up_resp_RGB(ReceiveBuffer[HZ_DATA_PLY_START_IDX+0]);
			ESP_HZ_DBG("cmd == HZ_CMD_REQ_RGB\r\n");
			break;

		case HZ_CMD_REQ_CNT://���󵹼�ʱ״̬
			ESP_HZ_DBG("cmd == HZ_CMD_REQ_CNT\r\n");
			hz_up_resp_count_down_status(ReceiveBuffer[HZ_DATA_PLY_START_IDX+0]);
			break;

		case HZ_CMD_REQ_CUR://�������
			hz_up_resp_Cur_Vol_Consum(ReceiveBuffer[HZ_DATA_PLY_START_IDX+0],0);
			ESP_HZ_DBG("cmd == HZ_CMD_REQ_CUR\r\n");
			break;

		case HZ_CMD_REQ_VOL://�����ѹ
			hz_up_resp_Cur_Vol_Consum(ReceiveBuffer[HZ_DATA_PLY_START_IDX+0],1);
			ESP_HZ_DBG("cmd == HZ_CMD_REQ_VOL\r\n");
			break;

		case HZ_CMD_REQ_CONSUM://���󹦺�
			hz_up_resp_Cur_Vol_Consum(ReceiveBuffer[HZ_DATA_PLY_START_IDX+0],2);
			ESP_HZ_DBG("cmd == HZ_CMD_REQ_CONSUM\r\n");
			break;

		case HZ_CMD_REQ_TEMP://�����¶�
			hz_up_resp_Temp_Humi_PM25_Smoke_(0);
			ESP_HZ_DBG("cmd == HZ_CMD_REQ_TEMP\r\n");
			break;

		case HZ_CMD_REQ_HUM://����ʪ��
			hz_up_resp_Temp_Humi_PM25_Smoke_(1);
			ESP_HZ_DBG("cmd == HZ_CMD_REQ_HUM\r\n");
			break;

		case HZ_CMD_REQ_PM25://����PM2.5
			hz_up_resp_Temp_Humi_PM25_Smoke_(2);
			ESP_HZ_DBG("cmd == HZ_CMD_REQ_PM25\r\n");
			break;

		case HZ_CMD_REQ_SMK://��������״̬
			hz_up_resp_Temp_Humi_PM25_Smoke_(3);
			ESP_HZ_DBG("cmd == HZ_CMD_REQ_SMK\r\n");
			break;
*/

		default:
			break;
	}
}

u8 ICACHE_FLASH_ATTR hz_check_buf_null(u8 *buf , u8 len)
{
	u8 i;

	for(i=0;i<len;i++)
	{
		if(buf[i])
		{
			return 1;
		}
	}

	return 0;
}


void ICACHE_FLASH_ATTR hz_add_time_policy(u8 *timebuf)
{
	u8 i;

	for(i=0;i<HZ_TIME_CTRL_BUF_SIZE;i++)
	{
		if(hz_check_buf_null(Policy_Time_Info.buf[i],HZ_TIME_CTRL_BUF_GROUP_SIZE) == 0)
		{
			os_memcpy(Policy_Time_Info.buf[i],timebuf,HZ_TIME_CTRL_BUF_GROUP_SIZE);
			break;
		}
	}
}

#if HZ_GIVE_POLICY_TEST

#if 0

/* *************************************************************************
 *hz_test_give_one_policy
 *************************************************************************/
void ICACHE_FLASH_ATTR hz_test_give_one_policy(void)
{

	u8 hour = 14;
	u8 minute = 18;
	u8 testBuf[] =
	{
		0X04,	0X01,	0XA0,	0XA1,	0XA2,	0XA3,	0XA4,	0XA5,	0X5C,	0XCF,
		0X7F,	0XF8,	0X76,	0XD1,	0X01,	0X00,	0X00,	0X03,	0X03,	0X00, 0X00, 0X00,
		0X00,	0X15,	0X01,	0X9D,	0XA9,	0XFA,	0X58
	};

	u8 test2Buf[100];
	u8 flag = 0;
	u8 i;
	u8 len = 10*5;

	memset(ReceiveBuffer,0,sizeof(ReceiveBuffer));

	memset(test2Buf,0,sizeof(test2Buf));

	test2Buf[0] = (len+4)&0X00FF;
	test2Buf[1] = ((len+4)>>8)&0X00FF;
	test2Buf[2] = 0X00;
	test2Buf[3] = (len)&0X00FF;
	test2Buf[4] = (len>>8)&0X00FF;
	test2Buf[5] = 0X00;

	for(i=0;i<(10*5);i++)
	{

		if(i%10 == 0)
		{
			test2Buf[i+6+0] = 0XFF;
			test2Buf[i+6+1] = 0XFF;
			test2Buf[i+6+2] = 0XFF;
			test2Buf[i+6+3] = 0xFF;
			test2Buf[i+6+4] = flag&1;
			test2Buf[i+6+5] = hour;
			test2Buf[i+6+6] = minute+flag;
			test2Buf[i+6+7] = 30;
			test2Buf[i+6+8] = 0XFF;
			test2Buf[i+6+9] = 0xFF;

			flag++;
		}
	}
	ESP_HZ_DBG("!!!!!!!!hz_test_give_one_policy!!!!!!!!!!\r\n");

	os_memcpy(ReceiveBuffer,testBuf,sizeof(testBuf));
	os_memcpy(ReceiveBuffer+sizeof(testBuf),test2Buf,len+6);
	u8 szDigest[16];

	UART_ShowByteInCharHex(ReceiveBuffer,sizeof(testBuf)+len+6);
	ESP_HZ_DBG("\r\n");
	hz_md5_package(ReceiveBuffer,sizeof(testBuf)+len+6,szDigest);

	os_memcpy(ReceiveBuffer+sizeof(testBuf)+len+6,szDigest,16);

	UART_ShowByteInCharHex(ReceiveBuffer,sizeof(testBuf)+len+6+16);
	ESP_HZ_DBG("\r\n");
	ESP_HZ_DBG("!!!!!!!!hz_test_give_one_policy end****!!!!!!!!!!\r\n");

	hz_decode_data();
}


#endif

/* *************************************************************************
 *hz_test_give_one_policy
 *************************************************************************/
void ICACHE_FLASH_ATTR hz_test_give_one_policy(u8 ch,u8 time)
{

	u8 testBuf[] =
	{
		0X01,	0X01,	0XA0,	0XA1,	0XA2,	0XA3,	0XA4,	0XA5,	0X5C,	0XCF,
		0X7F,	0XF8,	0X71,	0XFD,	0X01,	0X00,	0X00,	0X03,	0X03,	0X00, 0X00, 0X00,
		0X00,	0X15,	0X01,	0X9D,	0XA9,	0XFA,	0X58
	};

	u8 test2Buf[100];
	u8 flag = 0;
	u8 i;
	u8 len = 7;

	memset(ReceiveBuffer,0,sizeof(ReceiveBuffer));

	memset(test2Buf,0,sizeof(test2Buf));

	test2Buf[0] = (len)&0X00FF;
	test2Buf[1] = ((len)>>8)&0X00FF;
	test2Buf[2] = ch;			//0
	test2Buf[3] = 0X00;			//1
	test2Buf[4] = 0X00;			//2
	test2Buf[5] = 0X00;			//3
	test2Buf[6] = 0X00;			//4 onoff
	test2Buf[7] = 0X00;			//5
	test2Buf[8] = time;			//6

	ESP_HZ_DBG("!!!!!!!!hz_test_give_one_policy!!!!!!!!!!\r\n");

	os_memcpy(ReceiveBuffer,testBuf,sizeof(testBuf));
	os_memcpy(ReceiveBuffer+sizeof(testBuf),test2Buf,len+2);
	u8 szDigest[16];

	UART_ShowByteInCharHex(ReceiveBuffer,sizeof(testBuf)+len+2);
	ESP_HZ_DBG("\r\n");
	hz_md5_package(ReceiveBuffer,sizeof(testBuf)+len+2,szDigest);

	os_memcpy(ReceiveBuffer+sizeof(testBuf)+len+2,szDigest,16);

	UART_ShowByteInCharHex(ReceiveBuffer,sizeof(testBuf)+len+2+16);
	ESP_HZ_DBG("\r\n");
	ESP_HZ_DBG("!!!!!!!!hz_test_give_one_policy end****!!!!!!!!!!\r\n");

	hz_decode_data();
}

#endif


u8 ICACHE_FLASH_ATTR hz_circuit_buf_to_num(u8 *buf)
{
	u8 i;
	u8 circuit = 0XFE;

	for(i=0;i<8*4;i++)
	{
		if(buf[i/8]&BIT(i%8))		//buf[0-3] i:0-7
		{
			circuit = i;
			break;
		}
	}

	return circuit;
}

void ICACHE_FLASH_ATTR hz_write_flash_isr(void)
{
	ESP_HZ_DBG("Single ctrl update flash \r\n");
	hz_write_time_policy_to_flash();
}

/**********************************************************
HZ_Data_Head.time = rtc_read_stamp();//20-23//sntp_get_current_timestamp();	��������ʱ���ǻ�ȡ����ʱ��ģ�����ʹ��RTC�Ƚϱ��գ���ʹ����������ʱ���ܻ�������
memset(realTimeStr,0,sizeof(realTimeStr));
os_sprintf(realTimeStr,sntp_get_real_time(HZ_Data_Head.time));
ESP_HZ_DBG("\r\nReal time: %s \r\n",realTimeStr );
ESP_HZ_DBG("\r\nWeek: %d \r\n",week_check(realTimeStr));
**********************************************************/
/* *************************************************************************
 * ʵʱ��ȡʱ�䣬ִ�ж�ʱ��������
 *************************************************************************/
void ICACHE_FLASH_ATTR hz_policy_run(void)
{
	u32 rtc_timestamp;
	u8 week_msk;
	u8 *pBuffer;
	u8 i;
	static u8 lastminute = 0;
	static u8 lastsecond = 0;

//	hz_policy_fill_test();

	if(!rtc_time.tsFlag || (rtc_time.magic != RTC_MAGIC)) return; //RTCû�����úã�����

	rtc_timestamp = rtc_read_stamp();//20-23//sntp_get_current_timestamp();	��������ʱ���ǻ�ȡ����ʱ��ģ�����ʹ��RTC�Ƚϱ��գ���ʹ����������ʱ���ܻ�������
	Sec2Date(&time_info,rtc_timestamp);

	if(lastsecond == time_info.second)
	{
		return;
	}
	lastsecond = time_info.second;

	week_msk = 0X00;
	if(time_info.week == 1) week_msk = 0X80;
	if(time_info.week == 2) week_msk = 0X40;
	if(time_info.week == 3) week_msk = 0X20;
	if(time_info.week == 4) week_msk = 0X10;
	if(time_info.week == 5) week_msk = 0X08;
	if(time_info.week == 6) week_msk = 0X04;
	if(time_info.week == 0) week_msk = 0X02;

	//ʱ��---------0-3�����Ƶĵ�·��4:on or off ��5-7:ʱ���� 8�����ڼ�
	for(i=0;i<HZ_TIME_CTRL_BUF_SIZE;i++)
	{
		pBuffer = Policy_Time_Info.buf[i];
		if(pBuffer[8] & week_msk)
		{
			if((time_info.hour == pBuffer[5]) && (time_info.minute == pBuffer[6]) && (time_info.minute != lastminute))
			{
				ESP_HZ_DBG("\r\nPolicy Ctrl: %d %02d-%02d %02d:%02d:%02d week:%d\r\n",time_info.year,
																			time_info.month,
																			time_info.day,
																			time_info.hour,
																			time_info.minute,
																			time_info.second,
																			time_info.week);
				hz_deal_buf_onoff(pBuffer,2);		//��ʱ���ƣ���Ҫ�����ϱ�����״̬
				hz_up_resp_time_ctl_status(pBuffer);
			}
			/*
			if((time_info.hour == pBuffer[5]) && (time_info.minute == pBuffer[6]) && (time_info.second == pBuffer[7]))
			{

				ESP_HZ_DBG("\r\nPolicy Ctrl: %d %02d-%02d %02d:%02d:%02d week:%d\r\n",time_info.year,
																			time_info.month,
																			time_info.day,
																			time_info.hour,
																			time_info.minute,
																			time_info.second,
																			time_info.week);
				hz_deal_buf_onoff(pBuffer,2);		//��ʱ���ƣ���Ҫ�����ϱ�����״̬
				hz_up_resp_time_ctl_status(pBuffer);
			}*/
		}

		//2018.1.5���ӵ��μ�ʱ���ܣ������λΪ1ʱ��ִֻ��һ��
		if(pBuffer[8] & 0X01)
		{
			if((time_info.hour == pBuffer[5]) && (time_info.minute == pBuffer[6]) && (time_info.minute != lastminute))
			{
				ESP_HZ_DBG("\r\nPolicy Single Ctrl: %d %02d-%02d %02d:%02d:%02d week:%d\r\n",time_info.year,
																			time_info.month,
																			time_info.day,
																			time_info.hour,
																			time_info.minute,
																			time_info.second,
																			time_info.week);
				hz_deal_buf_onoff(pBuffer,2);		//��ʱ���ƣ���Ҫ�����ϱ�����״̬
				hz_up_resp_time_ctl_status(pBuffer);
				pBuffer[8] &=~0X01;
				os_timer_disarm(&timer_write_flash);
				os_timer_setfn(&timer_write_flash, (os_timer_func_t *)hz_write_flash_isr, NULL);
				os_timer_arm(&timer_write_flash, 5000, 0);
			}
			/*
			if((time_info.hour == pBuffer[5]) && (time_info.minute == pBuffer[6]) && (time_info.second == pBuffer[7]))
			{

				ESP_HZ_DBG("\r\nPolicy Single Ctrl: %d %02d-%02d %02d:%02d:%02d week:%d\r\n",time_info.year,
																			time_info.month,
																			time_info.day,
																			time_info.hour,
																			time_info.minute,
																			time_info.second,
																			time_info.week);
				hz_deal_buf_onoff(pBuffer,2);		//��ʱ���ƣ���Ҫ�����ϱ�����״̬
				hz_up_resp_time_ctl_status(pBuffer);
				pBuffer[8] &=~0X01;
				os_timer_disarm(&timer_write_flash);
				os_timer_setfn(&timer_write_flash, (os_timer_func_t *)hz_write_flash_isr, NULL);
				os_timer_arm(&timer_write_flash, 5000, 0);
			}*/
		}

	}

	lastminute = time_info.minute;
}


