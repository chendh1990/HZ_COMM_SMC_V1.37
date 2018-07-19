#ifndef __USER_FLASH_H__
#define __USER_FLASH_H__

/*******************************************************************************************************
gen_misc.sh����ʱSTEP1ѡ��1��STEP5ѡ��4����ֻ��ǰ��1024K�����16KΪ���������û��������á�

BOOT��ʼ��ַ��0X000
user1.bin����0X1000~0X7BFFF
user2.bin����0X81000~0XFBFFF

���16Kϵͳ������
0X3FB000���blank.bin
0X3FC000���esp_init_data_default.bin
0X3FE000���blank.bin
0X3FF000���ù�

�û����õĵ�ַΪ
0X100000~0X3FAFFF

����Ϊ
0X100~0X3FA
******************************************************************************************************/
#define USER_FLASH_SEC_SIZE						4096

//* ���������������,��������flashҪԤ��3��sector,sector1,2���౸�ݣ�sector3Ϊ��־����
#define USER_PC_SET_FLASH_START_SEC				0X300		//0X300~0X302
#define USER_POLICY_TIME_START_SEC				0X310		//0X310~0X312

#define USER_TEST_START_SEC						0X320		//0X320~0X322

//����������ֱ�Ӷ�д�ĵ�ַ
#define USER_SCENE_FLASH_START_ADDR				0X3D0000
#define USER_SCENE_FLASH_START_SEC				(0X3D0000/0X1000)

//��¼����ֱ���յĵ�ַ             /////////////////////////////////////////////////////////////////
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
