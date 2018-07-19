#ifndef __USER_UDP_BROADCAST_H__
#define __USER_UDP_BROADCAST_H__

/***********不需要用**************************
#define UDP_SCENE_PORT_MAX 		10
#define UDP_PORT_MAX 			20
//情景面板的端口号序号
#define UDP_DEFAULT_SCENE_PORT 	0		//0~9
//普通开关的端口号序号
#define UDP_DEFAULT_PORT 		2		//0~19
*********************************************/

//远端APP的端口号(固定)
#define UDP_APP_PORT			6666
#define UDP_SCENE_LOCAL_PORT	5500
#define UDP_SW_LOCAL_PORT		5550

#define UDP_SCENE_MAX				4
#define UDP_SCENE_EXE_MAX			20
#define UDP_SCENE_TIME_MAX			20

#define UDP_SCENE_TIME_BUF_SIZE		4
#define UDP_SCENE_EXE_BUF_SIZE		14


#define UDP_SCENE_SET_TEST			0



typedef enum
{
	UDP_APP_STATE_INIT,
	UDP_APP_STATE_SEND,
	UDP_APP_STATE_ACK,
	UDP_APP_STATE_LOOP,
}UDP_APP_STATE;



typedef struct
{
	u8 hour;
	u8 minute;
	u8 second;
	u8 week;
}SCENETIME;


typedef struct
{
	u8 mac[6];
	u8 cirBit[4];
	u8 onoff[4];
}SCENEDATA;


typedef struct
{
	u8 cirNum;	//电路序号
	u8 timeNum;
	u8 exeNum;
	SCENETIME tim[UDP_SCENE_TIME_MAX];
	SCENEDATA exe[UDP_SCENE_EXE_MAX];	//14*20 = 280
}SCENECONTEXT;


typedef struct
{
	u16 F_MSK;
	u8 sceneNum;
	SCENECONTEXT sce[UDP_SCENE_MAX];
}SCENEINFO;





typedef struct
{
	ETSTimer udp_task_timer;
	struct espconn espconn;
	int remotePort;
	u32 localPort;
	char send_str[100];
	UDP_APP_STATE app_ack;
	u8 stateDelay;
	ETSTimer udp_app_timer;
	SCENEINFO scene;
	u8 remotePNum;
	u8 remoteIdx;
	u8 sceneSendStart;
	u8 sceneSendIdx;
	u32 flash[USER_FLASH_SEC_SIZE/4];
}UDPTASK;

extern UDPTASK UDPTask;


void user_udp_task_init(void);
void user_udp_init(void);
void user_udp_send(int remotePort);
u8 user_udp_check_ip(void);
void user_udp_task(void);

//情景用 ********************************************
void user_udp_scene_set(u8 *buff);

void user_udp_flash_write(void);

void user_udp_flash_read(void);

void user_udp_power_on_read_flash(void);

void user_udp_scene_manual_handle(u8 num);

void user_udp_scene_timing_handle(void);

void user_udp_scene_set_test(u8 keyNum ,u8 *A_MAC ,u8 *B_MAC);

void user_udp_pop_data(void);

void user_udp_decode_data(u8 *buff);

//void user_udp_exeSceneByList(u8 list);

#endif
