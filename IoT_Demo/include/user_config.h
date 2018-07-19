#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__
//*******c语言系统库
#include   <stdio.h>
#include   <stdlib.h>
#include   <time.h>
#include   <string.h>
#include   <stdarg.h>
//********ESP库
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "smartconfig.h"
#include "espconn.h"
#include "upgrade.h"
//*******算法
#include "cal_ymdhms.h"
#include   "md5.h"
//****驱动
#include "uart.h"
#include "Key.h"
#include "pwm_usr.h"
#include "gpio.h"
#include "gpio16.h"
#include "pwm.h"
#include "msg2.h"

//******应用
#include "uart_printf.h"
#include "user_flash.h"
#include "user_peripheral.h"
#include "tttt.h"
#include "user_hz_proto.h"
#include "user_esp_platform.h"
#include "user_rtc.h"
#include "wifilink.h"
#include "pc_set.h"
#include "msg_stack.h"
#include "queue_fifo_alloc.h"
#include "user_udp_broadcast.h"
#include "user_button_deal.h"
#include "user_platform.h"
#include "user_upgrade.h"
#include "ringBuffer.h"
//////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////

#define ESP_PLATFORM        1

#define USE_OPTIMIZE_		PRINTF

#if ESP_PLATFORM

//#define SERVER_SSL_ENABLE
//#define CLIENT_SSL_ENABLE
//#define UPGRADE_SSL_ENABLE

#define USE_TEST

#define USE_DNS
/////////////////////////////////////////////////////////////
#ifdef USE_DNS
//#define ESP_DOMAIN      "sz1.testdev.szhanzhi.cn"
#ifndef USE_TEST
#define ESP_DOMAIN		"internal.dev.szhanzhi.cn"
#else
#define ESP_DOMAIN 		"sz1.testdev.szhanzhi.cn"
#endif

//#define ESP_DOMAIN		"dev22.szhanzhi.cn"
//#define ESP_DOMAIN      "cqy97988.123nat.com"
//#define ESP_DOMAIN      "iot.espressif.cn"
#endif
#ifndef USE_TEST
#define ESP_DOMAIN_PORT      	9997
#else
#define ESP_DOMAIN_PORT      	10097
#endif

#define BEACON_TIME     		((20)*(1000))	//XX*1000ms == XXs

//#define SOFTAP_ENCRYPT

#ifdef SOFTAP_ENCRYPT
#define PASSWORD	"v*%W>L<@i&Nxe!"
#endif


/*
#define AP_CACHE           0

#if AP_CACHE
#define AP_CACHE_NUMBER    5
#endif
*/

#endif

#endif

