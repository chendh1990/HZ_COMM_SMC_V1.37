/**
  ******************************************************************************
  * @file    transport.h
  * $Author: 飞鸿踏雪 $
  * $Revision: 17 $
  * $Date:: 2012-07-06 11:16:48 +0800 #$
  * @brief   以太网收发相关函数包装.
  ******************************************************************************
  * @attention
  *
  *<h3><center>&copy; Copyright 2009-2012, EmbedNet</center>
  *<center><a href="http:\\www.embed-net.com">http://www.embed-net.com</a></center>
  *<center>All Rights Reserved</center></h3>
  * 
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TRANSPORT_H
#define __TRANSPORT_H
/* Includes ------------------------------------------------------------------*/

/* Exported Functions --------------------------------------------------------*/


#define SOCK_TCPS         0
#define SOCK_DNS          1
#define DATA_BUF_SIZE   2048

int transport_sendPacketBuffer(unsigned char* buf, int buflen);
int transport_getdata(unsigned char* buf, int count);
int transport_open(void);
int transport_close(void);



#endif /* __MAIN_H */

/*********************************END OF FILE**********************************/
