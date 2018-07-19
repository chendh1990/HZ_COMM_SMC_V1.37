#ifndef __USER_DEVICE_H__
#define __USER_DEVICE_H__


struct USER_EPSPLATFORM_INFO {
	u8 cloud_connect;
	u8 manual_dis;
	u8 dispOnce;
	u8 w_timeout;
};


struct USER_RESTART_INFO {
	u8 k0;
	u8 k1;
	u8 k2;
	u8 k3;
};


#define ESP_CHECK_IP_TIME		500
#define ESP_CHECK_DNS_TIME		1000
#define ESP_RECON_TIME			1000
/*#define SIG_RX  		0
#define TEST_QUEUE_LEN 	4
os_event_t *testQueue;*/

/////////////////////////////////////////////////
//////////////////////////////////////////////////
extern struct USER_EPSPLATFORM_INFO user_platform_info;
extern u8 flashBuffer[];

void user_esp_platform_create_task(void);

#endif
