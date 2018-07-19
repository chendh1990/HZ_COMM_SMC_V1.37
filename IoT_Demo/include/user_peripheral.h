#ifndef __USER_PHERH__
#define	__USER_PHERH__

//定义产品硬件类型，只能打开一个宏
#define HZ_BOARD_CMB1  1		//普通三键触摸
#define HZ_BOARD_CMB2  0		//磁吸
#define HZ_BOARD_QCS1  0		//轻触四按键

/*******************************************
"Light"
"Outlet"
"ADJ"
"Window1T"
"Window2T"
"Curtain1T"
"Curtain2T"
"Scene"
*******************************************/

#define DEV_INFO_STRING		"\"Scene\",\"CH:2\",\"Timing\""

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
//开关接的路数
#define LED_CHANNEL_NUM				3		//这个不用改，只需改上面的CH:

//上电是否只显示一次跑马
#define POW_ON_DISP_ONECE	1

#define RGB_GROUP_NUM		1

#if HZ_BOARD_CMB1
//是否使用PWM
#define PWM_ENABLE 			2		//1:PWM  2:电平驱动
//波特率设置
#define USER_BIT_RATE 		115200

//GPIO定义
#define KEY_K2_INPUT		GPIO_INPUT_GET(GPIO_ID_PIN(12))
#define KEY_K1_INPUT		GPIO_INPUT_GET(GPIO_ID_PIN(14))
#define KEY_K0_INPUT		gpio16_input_get()

#define LED_K0_PIN_ID		GPIO_ID_PIN(2)
#define LED_K1_PIN_ID		GPIO_ID_PIN(4)
#define LED_K2_PIN_ID		GPIO_ID_PIN(5)

#define PWM_PIN_ID			GPIO_ID_PIN(13)

#define WIFI_LED_OUTPUT_H	GPIO_OUTPUT_SET(GPIO_ID_PIN(15),1)
#define WIFI_LED_OUTPUT_L	GPIO_OUTPUT_SET(GPIO_ID_PIN(15),0)


/*
#define GPIO9_H				GPIO_OUTPUT_SET(GPIO_ID_PIN(9),1)
#define GPIO9_L				GPIO_OUTPUT_SET(GPIO_ID_PIN(9),0)

#define GPIO10_H			GPIO_OUTPUT_SET(GPIO_ID_PIN(10),1)
#define GPIO10_L			GPIO_OUTPUT_SET(GPIO_ID_PIN(10),0)
*/

#endif

#if HZ_BOARD_CMB2
//是否使用PWM
#define PWM_ENABLE 			0
//波特率设置
#define USER_BIT_RATE 	115200
//开关接的路数
#define LED_CHANNEL_NUM				3

//GPIO定义
#define KEY_K2_INPUT	gpio16_input_get()
#define KEY_K1_INPUT	GPIO_INPUT_GET(GPIO_ID_PIN(14))
#define KEY_K0_INPUT	GPIO_INPUT_GET(GPIO_ID_PIN(12))

#define LED_K0_PIN_ID	GPIO_ID_PIN(2)
#define LED_K1_PIN_ID	GPIO_ID_PIN(4)
#define LED_K2_PIN_ID	GPIO_ID_PIN(5)

#define WIFI_LED_OUTPUT_H	GPIO_OUTPUT_SET(GPIO_ID_PIN(13),1)
#define WIFI_LED_OUTPUT_L	GPIO_OUTPUT_SET(GPIO_ID_PIN(13),0)
#endif

#if HZ_BOARD_QCS1
//是否使用PWM
#define PWM_ENABLE 			0
//波特率设置
#define USER_BIT_RATE 		9600
//开关接的路数
#define LED_CHANNEL_NUM		4
//GPIO定义
#define KEY_K3_INPUT		gpio16_input_get()
#define KEY_K2_INPUT		GPIO_INPUT_GET(GPIO_ID_PIN(14))
#define KEY_K1_INPUT		GPIO_INPUT_GET(GPIO_ID_PIN(12))
#define KEY_K0_INPUT		GPIO_INPUT_GET(GPIO_ID_PIN(13))

#define LED_K0_PIN_ID		GPIO_ID_PIN(2)	//加了STM8的，此IO不用
#define LED_K1_PIN_ID		GPIO_ID_PIN(4)	//加了STM8的，此IO不用
#define LED_K2_PIN_ID		GPIO_ID_PIN(5)	//加了STM8的，此IO不用

#define WIFI_LED_OUTPUT_H	GPIO_OUTPUT_SET(GPIO_ID_PIN(15),1)	//加了STM8的，此IO不用
#define WIFI_LED_OUTPUT_L	GPIO_OUTPUT_SET(GPIO_ID_PIN(15),0)	//加了STM8的，此IO不用
#endif


#define ADJ_MAX_LUMIN			20			//20*30ms = 600ms

#define BEEP_TIME_LONG			2000		//2s
#define BEEP_TIME_SHORT			350			//350ms

#define SCENE_LED_ON_TIME		60			//60*10ms = 600ms
#define ADJ_LED_ON_TIME			60			//60*10ms = 600ms
#define CURTAIN_LED_ON_TIME		1500		//1500*10ms = 15000ms

//设备类型名称 顺序要跟云端一致
#define DEV_TYPE_STR1		"Light"
#define DEV_TYPE_STR2		"Outlet"
#define DEV_TYPE_STR3		"ADJ"
#define DEV_TYPE_STR4		"Window1T"
#define DEV_TYPE_STR5		"Window2T"
#define DEV_TYPE_STR6		"Curtain1T"
#define DEV_TYPE_STR7		"Curtain2T"
#define DEV_TYPE_STR8		"Scene"
#define DEV_TYPE_STR9		"XXXXX"
#define DEV_TYPE_STR10		"XXXXX"
#define DEV_TYPE_STR11		"XXXXX"
#define DEV_TYPE_STR12		"XXXXX"
#define DEV_TYPE_STR13		"XXXXX"
#define DEV_TYPE_STR14		"XXXXX"
#define DEV_TYPE_STR15		"XXXXX"
#define DEV_TYPE_STR16		"XXXXX"
#define DEV_TYPE_STR17		"XXXXX"
#define DEV_TYPE_STR18		"XXXXX"
#define DEV_TYPE_STR19		"XXXXX"
#define DEV_TYPE_STR20		"XXXXX"


//属性 顺序要跟云端一致
#define DEV_P_STR1		"Temp"
#define DEV_P_STR2		"Humidity"
#define DEV_P_STR3		"RGB"
#define DEV_P_STR4		"Current"
#define DEV_P_STR5		"Voltage"
#define DEV_P_STR6		"Consum"
#define DEV_P_STR7		"Timing"
#define DEV_P_STR8		"PM2.5"
#define DEV_P_STR9		"Infrared"
#define DEV_P_STR10		"Smoke"
#define DEV_P_STR11		"LSensor"
#define DEV_P_STR12		"Raindrop"
#define DEV_P_STR13		"Lumin"
#define DEV_P_STR14		"XXXXXXXX"
#define DEV_P_STR15		"XXXXXXXX"
#define DEV_P_STR16		"XXXXXXXX"

/******************************************************
0:	Light
1:	Plug
2:	Sensor
3:	Gateway
*******************************************************/


//设置设备ID
#define DEV_ID0		0X00
#define DEV_ID1		0X00
#define DEV_ID2		0X00
#define DEV_ID3		0X00
#define DEV_ID4		0X00
#define DEV_ID5		0X01

#define LED_P02_MSK_ON		0
#define LED_P02_MSK_OFF		1

struct LED_RELAY_STATE
{
    uint8  led[LED_CHANNEL_NUM];
    uint8  led_tmp[LED_CHANNEL_NUM];
    uint16 beepCnt;
    uint16 ledOnCnt[LED_CHANNEL_NUM];

    u8 lumin;
    u8 adjOn;
};


struct DEV_INFO
{
    uint8 info[200];
    uint8 p[7];
    uint8 mac[6];
    uint8 id[6];
};


extern struct DEV_INFO Device_Info;
extern const u32  Pher_Gpio_List[];
extern struct LED_RELAY_STATE  led_relay_state;
extern const char DEV_TYPE_STR[][20];
//////////////////////////////////////////////
void peripheral_led_state_set(void);
void peripheral_beep_n10ms(uint16 n10ms);
void peripheral_beep_state_timer_run(void);
void peripheral_init(void);
void peripheral_set_device_info(void);
void peripheral_decode_dev_p(void);
void peripheral_refresh_pinfo(void);
void peripheral_led_wifi_ctrl(u8 flag);
void peripheral_led_wifi_flash(u8 fast);
void peripheral_gpio_cmd_send(u8 mode,u8 ch,u8 onoff,u16 ms);
void peripheral_send_to_adj_controller(void);

#endif


