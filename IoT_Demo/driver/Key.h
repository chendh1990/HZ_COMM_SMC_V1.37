/******************************************************************
KEY.H  file
作者：
建立日期: 
修改日期: 
版本：
版权所有，盗版必究。
Copyright(C) 
All rights reserved            
*******************************************************************/

#ifndef __KEY_H__
#define __KEY_H__

#define KEY_SCAN_TIMES		(2)  //按键防抖的扫描次数
#define KEY_SHORT_TIMES		(5)
#define KEY_LONG_TIMES		(300)	//长按键的次数
#define KEY_HOLD_TIMES      (50)    //连按的频率次数


#define NO_KEY          0xff
							 
#define KEY_LONG		0x10
#define KEY_HOLD		0x20
#define KEY_SHORT_UP    0x30
#define KEY_LONG_UP     0x40

#define KEY_K2			0X01

#define PR_K2		KEY_K2
#define PS_K2		(KEY_SHORT_UP|KEY_K2)
#define PL_K2		(KEY_LONG|KEY_K2)
#define PU_K2		(KEY_LONG_UP|KEY_K2)
#define PH_K2		(KEY_HOLD|KEY_K2)

#define KEY_K1			0X02

#define PR_K1		KEY_K1
#define PS_K1		(KEY_SHORT_UP|KEY_K1)
#define PL_K1		(KEY_LONG|KEY_K1)
#define PU_K1		(KEY_LONG_UP|KEY_K1)
#define PH_K1		(KEY_HOLD|KEY_K1)

#define KEY_K0			0X04

#define PR_K0		KEY_K0
#define PS_K0		(KEY_SHORT_UP|KEY_K0)
#define PL_K0		(KEY_LONG|KEY_K0)
#define PU_K0		(KEY_LONG_UP|KEY_K0)
#define PH_K0		(KEY_HOLD|KEY_K0)

#define KEY_K3			0X08

#define PR_K3		KEY_K3
#define PS_K3		(KEY_SHORT_UP|KEY_K3)
#define PL_K3		(KEY_LONG|KEY_K3)
#define PU_K3		(KEY_LONG_UP|KEY_K3)
#define PH_K3		(KEY_HOLD|KEY_K3)

extern u8 keyScanReady;

void KeyScan(void);
void KeyGPIOInit(void);

#endif
