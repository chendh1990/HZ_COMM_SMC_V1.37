/*--------------------------------------------------------------------------*/
/**@file   	 cal_ymdhms
   @brief 	 time 	trasnsition
   @details 
   @author 	 
   @date   	
   @note   
*/
/*----------------------------------------------------------------------------*/

/******************************************************************************

*******************************************************************************/
#include "user_config.h"



const u32  Pher_Gpio_List[LED_CHANNEL_NUM] =
{
	LED_K0_PIN_ID,
	LED_K1_PIN_ID,
	LED_K2_PIN_ID
};



const char DEV_CH_CHAR[][10] =
{
	"CH:32","CH:31","CH:30","CH:29","CH:28","CH:27","CH:26","CH:25","CH:24","CH:23","CH:22","CH:21",
	"CH:20","CH:19","CH:18","CH:17","CH:16","CH:15","CH:14","CH:13","CH:12","CH:11","CH:10","CH:9",
	"CH:8", "CH:7","CH:6","CH:5","CH:4","CH:3","CH:2","CH:1"
};

const char DEV_TYPE_STR[20][20] =
{
	DEV_TYPE_STR1,
	DEV_TYPE_STR2,
	DEV_TYPE_STR3,
	DEV_TYPE_STR4,
	DEV_TYPE_STR5,
	DEV_TYPE_STR6,
	DEV_TYPE_STR7,
	DEV_TYPE_STR8,
	DEV_TYPE_STR9,
	DEV_TYPE_STR10,
	DEV_TYPE_STR11,
	DEV_TYPE_STR12,
	DEV_TYPE_STR13,
	DEV_TYPE_STR14,
	DEV_TYPE_STR15,
	DEV_TYPE_STR16,
	DEV_TYPE_STR17,
	DEV_TYPE_STR18,
	DEV_TYPE_STR19,
	DEV_TYPE_STR20,
};


const char DEV_P_STR[][20] =
{
	DEV_P_STR1,
	DEV_P_STR2,
	DEV_P_STR3,
	DEV_P_STR4,
	DEV_P_STR5,
	DEV_P_STR6,
	DEV_P_STR7,
	DEV_P_STR8,
	DEV_P_STR9,
	DEV_P_STR10,
	DEV_P_STR11,
	DEV_P_STR12,
	DEV_P_STR13,
	DEV_P_STR14,
	DEV_P_STR15,
	DEV_P_STR16,
};


struct LED_RELAY_STATE  led_relay_state;
struct DEV_INFO Device_Info;


/*********************************************************
 * 上电的时候调用，最终以flash设置参数为准
设置设备的默认设备ID
设置默认的设备描述符
设置设备的MAC地址
 *********************************************************/
void ICACHE_FLASH_ATTR
peripheral_set_device_info(void)
{
	u32	chip_id;

	//设置设备描述符 ，  默认，最终以flash设置参数为准
	os_memset(Device_Info.info,0,sizeof(Device_Info.info));
	os_memcpy(Device_Info.info,DEV_INFO_STRING,os_strlen(DEV_INFO_STRING));

	//设置设备ID  默认，最终以flash设置参数为准
	chip_id = system_get_chip_id();
	Device_Info.id[0] = 0X00;
	Device_Info.id[1] = 0X00;
	Device_Info.id[2] = (chip_id>>24)&0X000000FF;
	Device_Info.id[3] = (chip_id>>16)&0X000000FF;
	Device_Info.id[4] = (chip_id>>8)&0X000000FF;
	Device_Info.id[5] = (chip_id>>0)&0X000000FF;

	//设置设备MAC地址
	os_memset(Device_Info.mac,0,sizeof(Device_Info.mac));
	wifi_get_macaddr(STATION_IF, Device_Info.mac);

/////////////////////////////////////////////////////////////////////////////////////
	ESP_DBG("Chip ID:%02X %02X %02X %02X %02X %02X \r\n",Device_Info.id[0],Device_Info.id[1],
																Device_Info.id[2],Device_Info.id[3],
																Device_Info.id[4],Device_Info.id[5]);
	ESP_DBG("STA MAC:{0X%02X,0X%02X,0X%02X,0X%02X,0X%02X,0X%02X} \r\n", Device_Info.mac[0],Device_Info.mac[1],
			Device_Info.mac[2],Device_Info.mac[3],Device_Info.mac[4],Device_Info.mac[5]);
	ESP_DBG("os_strlen(DEV_INFO_STRING):%d\r\n",os_strlen(DEV_INFO_STRING));
}

/*********************************************************
上电的时候调用，使得设备有一个默认的设备类型
 默认，最终以flash设置参数为准
 *********************************************************/
void ICACHE_FLASH_ATTR
peripheral_decode_dev_p(void)
{
	char *pstr1;
	u8 i;
	u16 DEV_P_STR_L_Num;
	u16 DEV_CH_CHAR_L_Num;
	u16 DEV_TYPE_STR_L_Num;

	ESP_DBG_FUN_START();

	DEV_P_STR_L_Num = sizeof(DEV_P_STR)/sizeof(DEV_P_STR[0]);
	DEV_CH_CHAR_L_Num = sizeof(DEV_CH_CHAR)/sizeof(DEV_CH_CHAR[0]);
	DEV_TYPE_STR_L_Num = sizeof(DEV_TYPE_STR)/sizeof(DEV_TYPE_STR[0]);

	ESP_DBG("DEV_P_STR_L_Num:%d \r\n",DEV_P_STR_L_Num);
	ESP_DBG("DEV_CH_CHAR_L_Num:%d \r\n",DEV_CH_CHAR_L_Num);
	ESP_DBG("DEV_TYPE_STR_L_Num:%d \r\n",DEV_TYPE_STR_L_Num);

	memset(Device_Info.p,0,sizeof(Device_Info.p));

	//获取设备类型
	for(i=0;i<DEV_TYPE_STR_L_Num;i++)
	{
		if ((pstr1 = (char *) os_strstr(Device_Info.info, DEV_TYPE_STR[i])) != NULL)
		{
			Device_Info.p[0] = i;
			break;
		}
	}

	//获取路数
	for(i=0;i<DEV_CH_CHAR_L_Num;i++)
	{
		if((pstr1 = (char *)os_strstr(Device_Info.info, DEV_CH_CHAR[i])) != NULL)
		{
			Device_Info.p[1] = DEV_CH_CHAR_L_Num-i;
			break;
		}
	}

	//获取温度，湿度等属性
	for(i=0;i<DEV_P_STR_L_Num;i++)
	{
		if((pstr1 = (char *)os_strstr(Device_Info.info, (DEV_P_STR[i]))) != NULL)
		{
			Device_Info.p[(i/8)+2] |= (1<<(i%8));
		}
	}

//	Device_Info.p[5] = UDP_DEFAULT_PORT;	//设备的UDP端口号

	ESP_DBG("Device_Info.p:%02X %02X %02X %02X %02X  \r\n",Device_Info.p[0],
			Device_Info.p[1],Device_Info.p[2],Device_Info.p[3],Device_Info.p[4]);

	ESP_DBG_FUN_END();
}

/*********************************************************
根据产品属性，设置产品描述符
此函数会在上电读取flash参数和上位机发设置命令的时候调用
 *********************************************************/
void ICACHE_FLASH_ATTR
peripheral_refresh_pinfo(void)
{
	u16 i;
	u16 DEV_P_STR_L_Num;
	u16 DEV_CH_CHAR_L_Num;
	u16 DEV_TYPE_STR_L_Num;
	u16 strLen = 0;
	u16 strAddr = 0;
	u8 idx;

	ESP_DBG_FUN_START();

	DEV_P_STR_L_Num = sizeof(DEV_P_STR)/sizeof(DEV_P_STR[0]);
	DEV_CH_CHAR_L_Num = sizeof(DEV_CH_CHAR)/sizeof(DEV_CH_CHAR[0]);
	DEV_TYPE_STR_L_Num = sizeof(DEV_TYPE_STR)/sizeof(DEV_TYPE_STR[0]);

	memset(Device_Info.info,0,sizeof(Device_Info.info));

	os_strcat(Device_Info.info,"\"");
	idx = Device_Info.p[0];
	os_strcat(Device_Info.info,DEV_TYPE_STR[idx]);
	os_strcat(Device_Info.info,"\"");

	os_strcat(Device_Info.info,",\"");
	idx = DEV_CH_CHAR_L_Num-Device_Info.p[1];
	os_strcat(Device_Info.info,DEV_CH_CHAR[idx]);
	os_strcat(Device_Info.info,"\"");

	for(i=0;i<DEV_P_STR_L_Num;i++)
	{
		u8 bit_map;
		bit_map = (1<<(i%8));
		if(Device_Info.p[(i/8)+2]&bit_map)
		{
			os_strcat(Device_Info.info,",\"");
			os_strcat(Device_Info.info,DEV_P_STR[i]);
			os_strcat(Device_Info.info,"\"");
		}
	}
/*	for(i=0;i<sizeof(Device_Info.info);i++)
		UART_SENDBYTE(Device_Info.info[i]);*/
	ESP_DBG(Device_Info.info);
	ESP_DBG("\r\n");

	ESP_DBG_FUN_END();
}


#if (HZ_BOARD_CMB1||HZ_BOARD_CMB2)
/*********************************************************

 *********************************************************/
void ICACHE_FLASH_ATTR
peripheral_init(void)
{

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U,FUNC_GPIO15); 			//wifi led

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U,FUNC_GPIO13); 			//beep

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U,FUNC_GPIO2);				//RELAY1,LED1
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U,FUNC_GPIO4);				//RELAY2,LED2
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U,FUNC_GPIO5);				//RELAY3,LED3

//    PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA2_U,FUNC_GPIO9);				//
//    PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA3_U,FUNC_GPIO10);				//


//    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U,FUNC_GPIO3);				//RX -> GPIO

    GPIO_OUTPUT_SET(GPIO_ID_PIN(13),0);									//beep off

    peripheral_led_wifi_ctrl(0);										//wifi led off

    //所有设备关
    memset(led_relay_state.led,0,sizeof(led_relay_state.led));		//all led off
    memset(led_relay_state.led_tmp,0XFF,sizeof(led_relay_state.led_tmp));

    peripheral_led_state_set();
}
#endif

#if (HZ_BOARD_QCS1)
/*********************************************************

 *********************************************************/
void ICACHE_FLASH_ATTR
peripheral_init(void)
{

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U,FUNC_GPIO15); 					//wifi led

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U,FUNC_GPIO2);						//RELAY1,LED1
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U,FUNC_GPIO4);						//RELAY2,LED2
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U,FUNC_GPIO5);						//RELAY3,LED3

//    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U,FUNC_GPIO3);					//RX -> GPIO

//    GPIO_OUTPUT_SET(GPIO_ID_PIN(13),0);									//beep off

    peripheral_led_wifi_ctrl(0);											//wifi led off

    //所有设备关
    memset(led_relay_state.led,0,sizeof(led_relay_state.led));				//all led off
    memset(led_relay_state.led_tmp,0XFF,sizeof(led_relay_state.led_tmp));

    peripheral_led_state_set();
}
#endif


/*********************************************************

 *********************************************************/
void ICACHE_FLASH_ATTR
peripheral_led_state_set(void)
{
	u8 i;

	if(((char *)os_strstr(DEV_INFO_STRING, ("ADJ"))) != NULL)
	{

	}
	else
	{
		for(i=0;i<LED_CHANNEL_NUM;i++)
		{
			if(led_relay_state.led[i] != led_relay_state.led_tmp[i])
			{
				GPIO_OUTPUT_SET(Pher_Gpio_List[i],led_relay_state.led[i]&1);

				peripheral_gpio_cmd_send(0X00,i,led_relay_state.led[i]&1,0);

//				peripheral_send_to_adj_controller();	//发送给STM8控制器

				led_relay_state.led_tmp[i] = led_relay_state.led[i];
				///////////////////////////////////////////////////////////////////////////////////////////////////
				ESP_LTT_DBG("light:%d -> %d \r\n",i,led_relay_state.led[i]);
			}
		}
	}
}

/*********************************************************

 *********************************************************/
void ICACHE_FLASH_ATTR
peripheral_led_wifi_flash(u8 fast)
{
	static u8 flash;
	static u8 delayCnt = 0;
	u8 TIME_MSK = 0;

	if(fast)
	{
		TIME_MSK = 5;
	}
	else
	{
		TIME_MSK = 30;
	}
	if(delayCnt++ >= TIME_MSK)
	{
		delayCnt = 0;
		flash++;
		peripheral_led_wifi_ctrl(flash&1);
	}
}



/*********************************************************

 *********************************************************/
void ICACHE_FLASH_ATTR
peripheral_led_wifi_ctrl(u8 flag)
{
	if(((char *)os_strstr(DEV_INFO_STRING, ("ADJ"))) != NULL)
	{
		if(flag)
		{
			GPIO_OUTPUT_SET(LED_K2_PIN_ID,0);
		}
		else
		{
			GPIO_OUTPUT_SET(LED_K2_PIN_ID,1);
		}
	}
	else
	{
		if(flag)
		{
			WIFI_LED_OUTPUT_H;
		}
		else
		{
			WIFI_LED_OUTPUT_L;
		}
	}


//	peripheral_gpio_cmd_send(0X04,0X00,flag&1,0);
/*
  	if(flag)
	{
		light_param_u.pwm_duty[1] = 500;
	}
	else
	{
		light_param_u.pwm_duty[1] = 0;
	}
	pwm_set_duty(light_param_u.pwm_duty[1]*22, 1);
	pwm_start();
*/
}


/*********************************************************

 *********************************************************/
void ICACHE_FLASH_ATTR
peripheral_beep_n10ms(uint16 n10ms)
{
#if (PWM_ENABLE)
	led_relay_state.beepCnt = n10ms/10;
#if (PWM_ENABLE == 1)
	pwm_set_duty(500*22,0);
	pwm_start();
#else
	GPIO_OUTPUT_SET(PWM_PIN_ID,1);
#endif

#endif
	peripheral_gpio_cmd_send(0X03,0X00,0X00,n10ms);

}

/*********************************************************

 *********************************************************/
void ICACHE_FLASH_ATTR
peripheral_beep_state_timer_run(void)
{
#if (PWM_ENABLE)

	if(led_relay_state.beepCnt)
	{
		led_relay_state.beepCnt--;
		if(led_relay_state.beepCnt == 1)
		{
#if (PWM_ENABLE == 1)
			pwm_set_duty(0,0);
			pwm_start();
#else
			GPIO_OUTPUT_SET(PWM_PIN_ID,0);
#endif
		}
	}

#endif
}


/*********************************************************

 *********************************************************/
void ICACHE_FLASH_ATTR
peripheral_gpio_cmd_send(u8 mode,u8 ch,u8 onoff,u16 ms)
{
	u8 sendBuffer[5];
	u8 i;

	sendBuffer[0] = 0XFA;
	sendBuffer[1] = 0X55;
	sendBuffer[2] = mode;

	if(mode == 3)
	{
		sendBuffer[3] = (ms>>8)&0X00FF;
		sendBuffer[4] = (ms>>0)&0X00FF;
	}
	else
	{
		sendBuffer[3] = ch;
		sendBuffer[4] = onoff;
	}

/*	ESP_DBG("\r\n");
	for(i=0;i<5;i++)
	{
		UART_SENDBYTE(sendBuffer[i]);
	}
	ESP_DBG("\r\n");*/
}


#define PC_MSG_SIZE     6
void peripheral_send_to_adj_controller(void)
{
  u8 sendbuff[PC_MSG_SIZE];
  u8 i;
  u8 cs = 0;

  sendbuff[0] = 0XFA;
  sendbuff[1] = 0X55;

  sendbuff[2] = 0X01;
  sendbuff[3] = led_relay_state.lumin;
  sendbuff[4] = led_relay_state.led[1];

  for(i=0;i<PC_MSG_SIZE-1;i++)
  {
    cs += sendbuff[i];
  }

  sendbuff[PC_MSG_SIZE-1] = cs;



  ESP_DBG("\r\n");
  //Send to STM8
  for(i=0;i<PC_MSG_SIZE;i++)
  {
	  UART_SENDBYTE(sendbuff[i]);
  }
  ESP_DBG("\r\n");

  ESP_DBG("Send to STM8:");

  for(i=0;i<PC_MSG_SIZE;i++)
  {
	 ESP_DBG("%02X " , sendbuff[i]);
  }
  ESP_DBG("\r\n");

}


