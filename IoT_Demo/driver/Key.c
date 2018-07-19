/******************************************************************
*key.c
*
*      
*******************************************************************/
#include "ets_sys.h"
#include "user_interface.h"
#include "os_type.h"
#include "osapi.h"
#include "gpio.h"
#include "key.h"
#include "user_esp_platform.h"
#include "gpio16.h"

u16 keyCounter ;		//按键扫描计数
u8 keyScanReady = 0;

/************************************************************
@
************************************************************/
void ICACHE_FLASH_ATTR
KeyGPIOInit(void)
{
#if (HZ_BOARD_CMB1||HZ_BOARD_CMB2)
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U,FUNC_GPIO12);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U,FUNC_GPIO14);

	gpio16_input_conf();

	GPIO_DIS_OUTPUT(12);
	GPIO_DIS_OUTPUT(14);
#endif

#if (HZ_BOARD_QCS1)

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U,FUNC_GPIO12);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U,FUNC_GPIO13);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U,FUNC_GPIO14);

	gpio16_input_conf();

	GPIO_DIS_OUTPUT(12);
	GPIO_DIS_OUTPUT(13);
	GPIO_DIS_OUTPUT(14);
#endif

}

u8 ICACHE_FLASH_ATTR
readKey(void)
{
	u8 key = 0;

#if (HZ_BOARD_QCS1)
	if(KEY_K3_INPUT == 0)
	{
		key |= KEY_K3;
		//os_printf("p");
	}
#endif
	if(KEY_K2_INPUT == 0)
	{
		key |= KEY_K2;
		//os_printf("p");
	}

	if(KEY_K1_INPUT == 0)
	{
		key |= KEY_K1;
		//os_printf("m");
	}

	if(KEY_K0_INPUT == 0)
	{
		key |= KEY_K0;
		//os_printf("e");
	}
	return key;
}


/************************************************************
@
************************************************************/
u8 ICACHE_FLASH_ATTR
KeyDetect(void)
{
  u8 key;
  
  key = readKey();

  if(key == 0X00) key = NO_KEY;
    
  return key;
}

/*----------------------------------------------------------------------------*/
/**@brief 按键检测，每10ms进行一次
@return 无
*/
/*----------------------------------------------------------------------------*/
void ICACHE_FLASH_ATTR
KeyScan(void)
{
    static u8 keyValue = 0;
	
    u8 keyTmp = NO_KEY;
    u8 key_return = NO_KEY;

    keyTmp = KeyDetect();

    if(keyTmp == NO_KEY )
    {
      if (keyCounter >= KEY_LONG_TIMES)  //长按抬键
      {
        key_return = keyValue | KEY_LONG_UP;
      }
      else if (keyCounter >= KEY_SCAN_TIMES)   //短按抬键
      {
        key_return = keyValue | KEY_SHORT_UP;
      }

      keyValue = keyTmp;
      keyCounter = 0;
    }
    else if(keyTmp != keyValue)
    {
      keyValue = keyTmp;
      keyCounter = 0;
      //PutMsg(KEY_CHANGE);
    }	
    else
    {
      keyCounter++;
      if (keyCounter == KEY_SCAN_TIMES)			//去抖
      {
        key_return = keyValue;
      }
      else if (keyCounter == KEY_LONG_TIMES)				//长按
      {
        key_return = keyValue | KEY_LONG;
      }
      else if (keyCounter == (KEY_LONG_TIMES + KEY_HOLD_TIMES) )			//连按
      {
        key_return = keyValue | KEY_HOLD;
        keyCounter = KEY_LONG_TIMES;
      }

    }
    if (key_return != NO_KEY)
    {
    	put_msg_fifo(key_return);
    	//system_os_post(USER_TASK_PRIO_2, SIG_RX, key_return);
    }

}


