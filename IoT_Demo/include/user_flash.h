#ifndef __USER_FLASH_H__
#define __USER_FLASH_H__

/*******************************************************************************************************
gen_misc.sh编译时STEP1选择1，STEP5选择4，则只有前面1024K和最后16K为程序区，用户不可乱用。

BOOT起始地址：0X000
user1.bin区域0X1000~0X7BFFF
user2.bin区域0X81000~0XFBFFF

最后16K系统参数：
0X3FB000存放blank.bin
0X3FC000存放esp_init_data_default.bin
0X3FE000存放blank.bin
0X3FF000不用管

用户可用的地址为
0X100000~0X3FAFFF

扇区为
0X100~0X3FA
******************************************************************************************************/
#define USER_FLASH_SEC_SIZE						4096

//* 定义带保护的区域,带保护的flash要预留3个sector,sector1,2互相备份，sector3为标志区域
#define USER_PC_SET_FLASH_START_SEC				0X300		//0X300~0X302
#define USER_POLICY_TIME_START_SEC				0X310		//0X310~0X312

#define USER_TEST_START_SEC						0X320		//0X320~0X322

//不带保护，直接读写的地址
#define USER_SCENE_FLASH_START_ADDR				0X3D0000
#define USER_SCENE_FLASH_START_SEC				(0X3D0000/0X1000)

//烧录工具直接烧的地址             /////////////////////////////////////////////////////////////////
#define FLASH_APP1_START_ADDR				0X200000			//0x200000~0x27FFFF
//#define FLASH_APP1_START_SEC				(0X200000/0X1000)
#define FLASH_APP2_START_ADDR				0X280000			//0x280000~0x2FFFFF
//#define FLASH_APP2_START_SEC				(0X280000/0X1000)

//****************************************************************************
#define FLASH_PROTECT_TEST				0


void hz_write_time_policy_to_flash(void);

//****************************************************************************
#endif


//esp_platform_saved_param
