//////////////////////////////////////////////////////////////////////////////////////////
#include "user_config.h"


u8 stackBuffer[STACK_BUF_SIZE];
u8 stackBufSize[STACK_BLOCK_NUM];
//u8 stack_tmp_buffer[STACK_BLOCK_SIZE];
u16 stack_index;

/* *************************************************************************

 *************************************************************************/
void ICACHE_FLASH_ATTR
msg_stack_clear(void)
{
	memset(stackBuffer,0,sizeof(stackBuffer));
	stack_index = 0;
}

/* *************************************************************************

 *************************************************************************/
void ICACHE_FLASH_ATTR
msg_stack_push(u8 input[],u8 inLen)					//向上增长
{
	if(stack_index < (STACK_BUF_SIZE))
	{
		os_memcpy(stackBuffer+stack_index,input,STACK_BLOCK_SIZE);						//数据入栈
		stackBufSize[stack_index/STACK_BLOCK_SIZE] = inLen;
		stack_index += STACK_BLOCK_SIZE;
	}
}


#if 1
/* *************************************************************************

 *************************************************************************/
void ICACHE_FLASH_ATTR
msg_stack_pop(u8 output[],u8 *outLen)								//从下面拿
{
	u8 i;
	u16 cpyIndex = 0;

	if(stack_index >= STACK_BLOCK_SIZE)
	{
		*outLen = stackBufSize[0];
		os_memcpy(output,stackBuffer,stackBufSize[0]);	//数据出栈
		memset(stackBuffer,0X00,STACK_BLOCK_SIZE); 		//清掉排出的数据区域

		ESP_DBG2("POP:");
		UART_ShowByteInCharHex(output,stackBufSize[0]);
		ESP_DBG2("\r\n");

		stackBufSize[0] = 0;

		for(cpyIndex=0;cpyIndex<((stack_index/STACK_BLOCK_SIZE)-1);cpyIndex++)			//数据下塌
		{
			os_memcpy(stackBuffer+cpyIndex*STACK_BLOCK_SIZE,stackBuffer+(cpyIndex+1)*STACK_BLOCK_SIZE,STACK_BLOCK_SIZE);	//数据出栈
			stackBufSize[cpyIndex] = stackBufSize[cpyIndex+1];
			memset(stackBuffer+(cpyIndex+1)*STACK_BLOCK_SIZE,0X00,STACK_BLOCK_SIZE); 						//清掉拷贝的数据区域
			stackBufSize[cpyIndex+1] = 0;
		}
		stack_index -= STACK_BLOCK_SIZE;
	}
	else
	{
		*outLen = 0;
	}
}

#else
void ICACHE_FLASH_ATTR
msg_stack_pop(u8 output[])								//从上面拿
{
	u16 popAddr = 0;
	u8 i;

	popAddr = stack_index-STACK_BLOCK_SIZE;

	if(stack_index >= STACK_BLOCK_SIZE)
	{
		os_memcpy(output,stackBuffer+popAddr,stackBufSize[popAddr/STACK_BLOCK_SIZE]);	//数据出栈
		memset(stackBuffer+popAddr,0X00,STACK_BLOCK_SIZE); 		//清掉排出的数据区域

		ESP_DBG2("-------------- POP --------------: \r\n");
		UART_ShowByteInCharHex(output,stackBufSize[popAddr/STACK_BLOCK_SIZE]);
		ESP_DBG2("\r\n");
		stackBufSize[popAddr/STACK_BLOCK_SIZE] = 0;
		stack_index -= STACK_BLOCK_SIZE;
	}
}
#endif

/**************************************************************************

 *************************************************************************/
void ICACHE_FLASH_ATTR
msg_stack_init(void)
{
	msg_stack_clear();
}

#if 0
/* *************************************************************************

 *************************************************************************/
void ICACHE_FLASH_ATTR
msg_stack_main(void)
{
	u16 i;
	u8 tBuf[100];
	u8 out[100];
	u8 len;

	ESP_DBG2("*********      msg_stack_main ---------stackBuffer lenght:%d         *********************************\r\n",sizeof(stackBuffer));

	memset(tBuf,0X00,sizeof(tBuf));
	memset(tBuf,0XAA,20);

	msg_stack_push(tBuf,20);
	UART_ShowByteInCharHex(stackBuffer,STACK_BUF_SIZE/10):
	ESP_DBG2("\r\n\r\n");

	memset(tBuf,0XBB,30);

	msg_stack_push(tBuf,30);
	UART_ShowByteInCharHex(stackBuffer,STACK_BUF_SIZE/10):
	ESP_DBG2("\r\n\r\n");

	memset(tBuf,0XCC,40);

	msg_stack_push(tBuf,40);

	UART_ShowByteInCharHex(stackBuffer,STACK_BUF_SIZE/10):
	ESP_DBG2("\r\n\r\n");

	memset(tBuf,0XDD,50);

	msg_stack_push(tBuf,50);

	UART_ShowByteInCharHex(stackBuffer,STACK_BUF_SIZE/10):
	ESP_DBG2("\r\n\r\n");


	for(i=0;i<5;i++)
	{
		msg_stack_pop(out,&len);
	}
}
#endif


