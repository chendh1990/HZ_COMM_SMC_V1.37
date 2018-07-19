#ifndef __USER_HZ_PROTO_H__
#define __USER_HZ_PROTO_H__

/*******************     ˵��       *****************************************
 δ�����տ���
 δ����γ�ȿ���
 ***************************************************************************/

#define ESP_DBG 			os_printf

#define ESP_HZ_DBG 			os_printf

#define ESP_DBG2 			os_printf

#define ESP_PC_DBG			os_printf

#define ESP_LTT_DBG			os_printf

#define ESP_DBG_HEX 		os_printf

#define ESP_DBG_HEX_NO_I 	UART_ShowByteInCharHex

#define ESP_DBG_HEX_I 		UART_ShowByteInCharHex_WithItem

#define UART_SENDBYTE(x)		uart_tx_one_char(UART0,x)


#define ESP_DBG_FUN_START() 	ESP_HZ_DBG("[FUNTION:------------<%s> START 			-------------------]\r\n",__FUNCTION__)
#define ESP_DBG_FUN_END() 		ESP_HZ_DBG("[FUNTION:------------<%s>   END 			-------------------]\r\n",__FUNCTION__)


#define USE_SMARTCONFIG		1

//////////////////////////////////////////////
#define MD5_KEY		"xseFscdd!23"
//#define MD5_KEY		""

//////////////////////////////////////////////
#define F_MASK0					0XA0
#define F_MASK1					0XA1
#define F_MASK2					0XA2
#define F_MASK3					0XB3
#define F_MASK_START_ADDR		1010
///////////////////////////////////////////////////
#define PROTO_VER				0X01		//Э��汾
#define BRAND					0X00		//Ʒ��
#define FW_VER					49			//ESP�̼��汾

////////////device->cloud    response///////////////////////////////////////////////
#define HZ_CMD_RESP_OK			0x0000
#define HZ_CMD_RESP_FAIL		0x0001

////////////cloud->device    cmd///////////////////////////////////////////////
#define HZ_CMD_LED_TG			0x0100
#define HZ_CMD_LED_CNTDOWN		0x0101
#define HZ_CMD_LED_LUMIN		0x0102
#define HZ_CMD_LED_RGB			0x0103
#define HZ_CMD_LED_CURTAIN		0x0105
#define HZ_CMD_LED_POLICY		0x0104
#define HZ_CMD_SET_SCENE		0x0106
#define HZ_CMD_UPGRADE			0x0107


/*
#define HZ_CMD_REQ_POLICY		0x0200
#define HZ_CMD_REQ_SW			0x0202
#define HZ_CMD_REQ_LUMIN		0x0204
#define HZ_CMD_REQ_RGB			0x0207
#define HZ_CMD_REQ_CNT			0x0208
#define HZ_CMD_REQ_CUR			0x020A
#define HZ_CMD_REQ_VOL			0x020C
#define HZ_CMD_REQ_CONSUM		0x020E
#define HZ_CMD_REQ_TEMP			0x0210
#define HZ_CMD_REQ_HUM			0x0212
#define HZ_CMD_REQ_PM25			0x0214
#define HZ_CMD_REQ_SMK			0x0216
*/

////////////device->cloud    cmd///////////////////////////////////////////////
#define HZ_CMD_HEARTBEAT		0x0000
#define HZ_CMD_SIGNIN			0x0001
#define HZ_CMD_LOGIN			0x0002
#define HZ_CMD_TIMESTAMP		0x0003

#define HZ_CMD_UP_PLY			0x0201
#define HZ_CMD_UP_SW			0x0203
#define HZ_CMD_UP_TCURTAIN		0x0204
#define HZ_CMD_UP_LUMIN			0x0205
#define HZ_CMD_UP_SCENE			0x0206
#define HZ_CMD_UP_RGB			0x0207
#define HZ_CMD_UP_CNT			0X0209
#define HZ_CMD_UP_CUR			0x020B
#define HZ_CMD_UP_VOL			0x020D
#define HZ_CMD_UP_W				0x020F
#define HZ_CMD_UP_TIME_CTL		0x0210
#define HZ_CMD_UP_TEMP			0x0211
#define HZ_CMD_UP_HUM			0x0213
#define HZ_CMD_UP_PM25			0x0215
#define HZ_CMD_UP_SMK			0x0217
#define HZ_CMD_UP_UPGRADE		0x0218

////////////policy type////////////////////////////////
#define HZ_POLICY_TIME			0X00
#define HZ_POLICY_SUN			0X01
#define HZ_POLICY_GPS			0X02

//////////// ����ͷ��Ϣ  ////////////////////////////////
#define HZ_DATA_HEAD_DATA_LEN_H_IDX			32									//���ݳ��ȸ��ֽڣ���ַ
#define HZ_DATA_PLY_TIME_DATA_IDX			(HZ_DATA_HEAD_DATA_LEN_H_IDX+5)		//���Կ��Ʒ�ʽ��ʱ�䣬���գ�GPS���������ݿ�ʼ��ַ
#define HZ_DATA_PLY_TYPE_IDX				(HZ_DATA_HEAD_DATA_LEN_H_IDX+4)		//���Կ��Ʒ�ʽ��ʱ�䣬���գ�GPS����ʼ��ַ
#define HZ_DATA_PLY_START_IDX				(HZ_DATA_HEAD_DATA_LEN_H_IDX+1)
#define HZ_DATA_HEAD_ID_START_IDX			2									//�豸ID��ʼ��ַ
#define HZ_DATA_HEAD_MAC_START_IDX			8									//�����ַ��ʼ��ַ

//////////// �������������С���� ////////////////////////////
#define HZ_TIME_CTRL_BUF_SIZE					100								//��ʱ���Կ��Ƶ�����
#define HZ_TIME_CTRL_BUF_GROUP_SIZE				10								//��ʱ���Կ���ÿ��������ֽ���
#define HZ_SUN_CTRL_BUF_SIZE					2								//���ղ�����������

//�ƶ�TCP����ͨ�Ż����С
#define CLOUD_DATA_MAX						((HZ_TIME_CTRL_BUF_SIZE*HZ_TIME_CTRL_BUF_GROUP_SIZE)+100)		//���ݲ��Կ��Ƶ���������趨�����buffer

#define FLASH_BUF_SIZE			1024

#define HZ_GIVE_POLICY_TEST		0

typedef struct _HZ_DATA_HEAD_
{
	u8 cmd[2];		//2
	u8 devID[6];	//6
	u8 mac[6];		//6
	u8 pVer;		//1
	u8 brand;		//1
	u8 devType[7];	//7
	u8 fwVer;		//1
	u8 repeat;		//1
	u8 time[4];		//4
	u8 reqOrResId[2];
}HZDataHead;

typedef struct _HZ_DATA_PAYLOAD_
{
	u8 dataLen[2];
	u8 payload[128];
	u8 md5[16];
}HZDataPayload;

typedef struct _HZ_POLICY_TIME_CONTROL_
{
	u8 policy;		//||,&&,Priority
	u8 dataLen[2];
	u8 type;		//0X00ʱ�� 0X01���� 0X02��γ
	u8 buf[HZ_TIME_CTRL_BUF_SIZE][HZ_TIME_CTRL_BUF_GROUP_SIZE];	//ʱ��---------0-3�����Ƶĵ�·��4:on or off ��5-7:ʱ���� 8�����ڼ�
}HZPolicyTimeControl;

typedef struct _HZ_POLICY_SUN_CONTROL_
{
	u8 policy;		//||,&&,Priority
	u8 dataLen[2];
	u8 type;		//0X00ʱ�� 0X01���� 0X02��γ
	u8 buf[HZ_SUN_CTRL_BUF_SIZE][6];		//0-3���Ƶĵ�· 4-5���ڼ�
}HZPolicySunControl;

typedef struct _HZ_POLICY_GPS_CONTROL_
{
	u8 policy;		//||,&&,Priority
	u8 dataLen[2];
	u8 type;		//0X00ʱ�� 0X01���� 0X02��γ
	u8 buf0[4];		//���Ƶĵ�·
}HZPolicyGPSControl;

typedef struct _HZ_POLICY_BUF_
{
	u8 led_ctrl[5];
	u8 led_cnt_down[7];
	u8 led_ctrl_lumin[6];
	u8 led_ctrl_rgb[4];
	u8 led_ctrl_policy[100];
}HZPolicyBuf;

typedef struct _HZ_CNTDOWN_BUF_
{
	u16 cnt_bk[LED_CHANNEL_NUM];
	u16 cnt[LED_CHANNEL_NUM];
	u8 onoff[LED_CHANNEL_NUM];
	u8 ok[LED_CHANNEL_NUM];
}HZCountDownBuf;

typedef struct _HZ_MISC_BUF_
{
	u8 lumin[LED_CHANNEL_NUM];
	u8 RGB[LED_CHANNEL_NUM][3];
	u16 cur[LED_CHANNEL_NUM];
	u16 vol[LED_CHANNEL_NUM];
	u16 consum[LED_CHANNEL_NUM];
	u16 temp;
	u16 humi;
	u16 pm25;
	u16 smoke;
}HZMiscBuf;

////�ص�������������
typedef void (* hz_cloud_send_cb_t)(uint8 *psent, uint16 length);
typedef void (* hz_cloud_restart_heartbeat_cb_t)(void);

///////////////////////////////////////////////////////////////
bool hz_check_md5(void);
void hz_md5_package(u8 *inBuffer,u16 len,u8 *outBuffer);
void hz_up_resp_status(u16 resp,u16 cmd);
void hz_up_sw_respon(u8 channel,u8 onoff);
void hz_up_resp_count_down_status(u8 channel);
void hz_add_time_policy(u8 *timebuf);
void hz_up_resp_time_ctl_status(u8 *inbuf);
u8 hz_circuit_buf_to_num(u8 *buf);


//��Щ���ⲿ��Ҫ���õĺ���
void hz_send_heartbeat(void);					//���͵�������
void hz_up_signin_status(void);				//����ע������
void hz_up_login_status(void);				//���͵�½����
void hz_request_timestamp(void);					//����ʱ���
void hz_policy_run(void);						//����ʵʱ��ѯ����
void hz_decode_data(void);					//�������յ�������
void hz_shut_cnt_down(u8 channel);			//ֹͣ����ʱ
void hz_initial_count_down(void);				//
void hz_trig_up_sw_once(void);
void hz_up_sw_status(void);					//�ϴ�����״̬
void hz_clear_policy_time(void);
void hz_up_resp_scene(u8 channel);
void hz_up_resp_Curtain1T(u8 channel);
u8 hz_check_buf_null(u8 *buf , u8 len);


//ע�᱾�ļ���Ҫ���õ��ⲿ����
void hz_cloud_send_callback_register(hz_cloud_send_cb_t hz_cloud_send_cb);
void hz_cloud_restart_heartbeat_callback_register(hz_cloud_restart_heartbeat_cb_t hz_cloud_restart_heartbeat_cb);

#if HZ_GIVE_POLICY_TEST
void hz_test_give_one_policy(u8 ch,u8 time);
#endif

//////////////////////////////////////////////////////////////
extern u8 ReceiveBuffer[];
extern HZPolicyBuf Policy_Buf;
extern TIME_INFO HZ_Timestamp_Info;
extern HZPolicyTimeControl	Policy_Time_Info;
extern HZMiscBuf HZ_Misc_Buf;

#endif

