//////////////////////////////////////////////////////////////////////////////////////////
#include "user_config.h"

//定义数组，数据，数据结构
#define UART_BUF_SIZE       20

u8 dataBuffer[UART_BUF_SIZE];
u8 transBuffer[UART_BUF_SIZE];

void PCSetTransmit(u8 dat)
{
	u8 i;
	u8 cs = 0;

	memset(transBuffer,0,sizeof(transBuffer));

	transBuffer[0] = 0XFA;
	transBuffer[1] = 0X55;
	transBuffer[2] = dat;
	transBuffer[13] = 0XFA+0X55+dat;

	for(i=0;i<14;i++)
	{
		UART_SENDBYTE(transBuffer[i]);
	}
}


/************************************************************
@
某次测量结果为流量： 10L/min， O2 浓度： 50%。
那么返回的结果 （O2 = 50.0% = 0x01F4 (16 进制)， （FLOW） = 10.0 L/min= 0x0064(16 进制)
16 09 01 01 F4 00 64 00 D2 00 00 B5
************************************************************/
void PCSetReceive(u8 UART_BUF)
{
  static u8 uartRXIndex = 0;
  u8 RecOneFrame;
  u8 cs;
  u8 i;

  
  //时刻获取数据
  RecOneFrame = 0;
  dataBuffer[uartRXIndex++] = UART_BUF;
  
  if(uartRXIndex &&(dataBuffer[0] != 0XFA))
     uartRXIndex = 0;
  else if((uartRXIndex>1)&&(dataBuffer[1] != 0X55))
     uartRXIndex = 0;
  else if((uartRXIndex > 3)&&(uartRXIndex == 4))		//2+7+4+1 = 14
  {
    RecOneFrame = 1;
  }
                  
  if(RecOneFrame)
  {
    cs = 0;
    for(i=0;i<uartRXIndex-1;i++)
    {
      cs += dataBuffer[i];
    }
    
    if(cs != dataBuffer[uartRXIndex-1])  
    {
      ESP_DBG("\ninv");
      uartRXIndex = 0;
      return;
    }
	switch (dataBuffer[2])
	{
		case 0x01:
    		put_msg_fifo(MSG_PC_SET);
			break;
		case 0x02:
   	 		put_msg_fifo(MSG_RTC_PRINT);
			break;
		default:
			break;
	}
    //system_os_post(USER_TASK_PRIO_2, SIG_RX, MSG_PC_SET);
    uartRXIndex = 0;		
  }

}


