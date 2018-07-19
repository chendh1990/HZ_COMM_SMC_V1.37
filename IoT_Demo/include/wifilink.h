#ifndef __WIFILINK_H__
#define __WIFILINK_H__

//****************************************************************************
/***************************************************************************
typedef enum _auth_mode {
    AUTH_OPEN           = 0,
    AUTH_WEP,
    AUTH_WPA_PSK,
    AUTH_WPA2_PSK,
    AUTH_WPA_WPA2_PSK,
    AUTH_MAX
} AUTH_MODE;
***************************************************************************/
#define WIFI_LINK_KEY		"hz#%esp*&key"
#define WIFI_AUTH_MODE		AUTH_WPA_WPA2_PSK

//****************************************************************************
struct WIFILINK_INFO
{
	u8 in;
	u8 inTmp;
	u16 pTimeOut;
};


extern struct WIFILINK_INFO WiFi_Link_Info;
typedef void (* wifilink_set_wifi_done_cb_t)(void);

//****************************************************************************
//主要应用函数
void wifilink_set_wifi_done_callback_register(wifilink_set_wifi_done_cb_t wifilink_set_wifi_done_cb);
void wifilink_set_ap_server(void);
//****************************************************************************
//超时计时器
void wifilink_pair_time_run(void);
void wifilink_pair_time_reset(void);
u8 wifilink_pair_time_out(void);
//****************************************************************************
#endif
