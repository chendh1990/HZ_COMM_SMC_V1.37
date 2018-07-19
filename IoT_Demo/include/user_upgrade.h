#ifndef __USER_UPGRADE_H__
#define __USER_UPGRADE_H__

//#define ESP_DBG 	INFO


#define UPGRADE_USE_DNS		1			//1:从韩智云服务器升级                          0:从局域网http file server升级

typedef enum
{
	UPGRADE_NORMAL,
	UPGRADE_IS_IN_PROCCESS,
	UPGRADE_VER_OLDER_THAN_CURRENT,
	UPGRADE_UPGRADE_SUCCESS,
	UPGRADE_UPGRADE_FAIL,
	UPGRADE_UPGRADE_TIMEOUT,
	UPGRADE_UPGRADE_CANNOT_LOAD_FILE,
	UPGRADE_WIFI_DISCONN,
	UPGRADE_WRONG_URI,
}UPGRADE_STATE;


#define FLASH_EXE_APP1_START_ADDR			(0X1000)
#define FLASH_EXE_APP1_START_SEC			(0X1000/0X1000)

#define FLASH_EXE_APP2_START_ADDR			(0X81000)
#define FLASH_EXE_APP2_START_SEC			(0X81000/0X1000)

#define OTA_SEC_SIZE						USER_FLASH_SEC_SIZE

typedef struct
{
	u32 contentLen;
	s32 remainbytes;
	u32 totalbytes;
	u16 writeSec;

    uint16 front;     //队列头，对头出数据
    uint16 rear;        //队列尾.队尾插入数据
    uint16 count;       //队列计数
}OTAFIFO;


extern OTAFIFO OTAFifo;

typedef struct
{
	u8 isBusy;
	u32 *flash;
	u8 *ptr;
	u8 onNet;
	u8 start;
	u8 tcpConn;
	u8 dns_try;

	u32 flash_buffer[USER_FLASH_SEC_SIZE/4];

	struct _esp_tcp user_tcp;
	struct espconn ota_espconn;
	unsigned short remote_port;
	char host[100];
	char filename[100];
	char filepath[100];
	char userfilename[100];
	//LOCAL u8 *send_buffer = NULL;
	u8 send_buffer[512];
	os_timer_t upgrade_timer;
	os_timer_t erase_timer;
	os_timer_t dns_timer;

	os_timer_t upgrade_sus_timer;
	os_timer_t reboot_timer;
}UPGRADE;

extern UPGRADE Upgrade;

u8 http_start_upgrade(char *URL);
u8 http_gethostbyname(void);
void self_upgrade(void);
//****************************************************************************
#endif
