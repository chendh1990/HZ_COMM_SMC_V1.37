/**
  ******************************************************************************
  * @file    transport.c
  * $Author: 飞鸿踏雪 $
  * $Revision: 17 $
  * $Date:: 2014-10-25 11:16:48 +0800 #$
  * @brief   主函数.
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
  * @brief  通过TCP方式发送数据到TCP服务器
  * @param  buf 数据首地址
  * @param  buflen 数据长度
  * @retval 小于0表示发送失败
  */
int ICACHE_FLASH_ATTR transport_sendPacketBuffer(unsigned char* buf, int buflen)
{

 // return send(SOCK_TCPS,buf,buflen);
	user_espconn_sent(buf,buflen);
	return buflen;
}
/**
  * @brief  阻塞方式接收TCP服务器发送的数据
  * @param  buf 数据存储首地址
  * @param  count 数据缓冲区长度
  * @retval 小于0表示接收数据失败
  */
int ICACHE_FLASH_ATTR transport_getdata(unsigned char* buf, int count)
{

 // return recv(SOCK_TCPS,buf,count);
	return 0;
}


/**
  * @brief  打开一个socket并连接到服务器
  * @param  无
  * @retval 小于0表示打开失败
  */
int ICACHE_FLASH_ATTR transport_open(void)
{
#if 0
  int32_t ret;
  //新建一个Socket并绑定本地端口5000
  ret = socket(SOCK_TCPS,Sn_MR_TCP,5000,0x00);
  if(ret != SOCK_TCPS){
    printf("%d:Socket Error\r\n",SOCK_TCPS);
    while(1);
  }else{
    printf("%d:Opened\r\n",SOCK_TCPS);
  }

  //连接TCP服务器
  ret = connect(SOCK_TCPS,domain_ip,1883);//端口必须为1883
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
  * @brief  关闭socket
  * @param  无
  * @retval 小于0表示关闭失败
  */
int ICACHE_FLASH_ATTR transport_close(void)
{
 // close(SOCK_TCPS);
  return 0;
}
