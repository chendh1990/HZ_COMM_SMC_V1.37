/**
  ******************************************************************************
  * @file    transport.c
  * $Author: �ɺ�̤ѩ $
  * $Revision: 17 $
  * $Date:: 2014-10-25 11:16:48 +0800 #$
  * @brief   ������.
  ******************************************************************************
  * @attention
  *
  *<h3><center>&copy; Copyright 2009-2012, EmbedNet</center>
  *<center><a href="http:\\www.embed-net.com">http://www.embed-net.com</a></center>
  *<center>All Rights Reserved</center></h3>
  * 
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "MQTTPacket.h"
#include "transport.h"
#include "mqtt.h"
#include "osapi.h"
#include <c_types.h>


#define Sn_MR_TCP 0
#define SOCK_OK 0



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
uint8_t domain_ip[4] = {192,168,1,55};
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/**
  * @brief  ͨ��TCP��ʽ�������ݵ�TCP������
  * @param  buf �����׵�ַ
  * @param  buflen ���ݳ���
  * @retval С��0��ʾ����ʧ��
  */
int ICACHE_FLASH_ATTR transport_sendPacketBuffer(unsigned char* buf, int buflen)
{

 // return send(SOCK_TCPS,buf,buflen);
	user_espconn_sent(buf,buflen);
	return buflen;
}
/**
  * @brief  ������ʽ����TCP���������͵�����
  * @param  buf ���ݴ洢�׵�ַ
  * @param  count ���ݻ���������
  * @retval С��0��ʾ��������ʧ��
  */
int ICACHE_FLASH_ATTR transport_getdata(unsigned char* buf, int count)
{

 // return recv(SOCK_TCPS,buf,count);
	return 0;
}


/**
  * @brief  ��һ��socket�����ӵ�������
  * @param  ��
  * @retval С��0��ʾ��ʧ��
  */
int ICACHE_FLASH_ATTR transport_open(void)
{
#if 0
  int32_t ret;
  //�½�һ��Socket���󶨱��ض˿�5000
  ret = socket(SOCK_TCPS,Sn_MR_TCP,5000,0x00);
  if(ret != SOCK_TCPS){
    printf("%d:Socket Error\r\n",SOCK_TCPS);
    while(1);
  }else{
    printf("%d:Opened\r\n",SOCK_TCPS);
  }

  //����TCP������
  ret = connect(SOCK_TCPS,domain_ip,1883);//�˿ڱ���Ϊ1883
  if(ret != SOCK_OK){
    printf("%d:Socket Connect Error\r\n",SOCK_TCPS);
    while(1);
  }else{
    printf("%d:Connected\r\n",SOCK_TCPS);
  }		
#endif
	return 0;
}
/**
  * @brief  �ر�socket
  * @param  ��
  * @retval С��0��ʾ�ر�ʧ��
  */
int ICACHE_FLASH_ATTR transport_close(void)
{
 // close(SOCK_TCPS);
  return 0;
}
