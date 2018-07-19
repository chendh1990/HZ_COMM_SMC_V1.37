//////////////////////////////////////////////////////////////////////////////////////////
#include "user_config.h"

/*****************************************************************

******************************************************************/

void ICACHE_FLASH_ATTR
UART_ShowByteInCharHex(u8 *buf,u16 len)
{
#define LINE_NUM	31

	u16 i,j;
	u8 blockSize;
	u8 remainbytes;

	if(len == 0) return;

	blockSize = len/LINE_NUM;
	remainbytes = len%LINE_NUM;

	for(j=0;j<blockSize;j++)
	{
		for(i=0;i<LINE_NUM;i++)
		{
			ESP_DBG_HEX("%02X ",buf[i+(j*LINE_NUM)]);
		}
		ESP_DBG_HEX("|\r\n");
	}

	if(remainbytes)
	{
		for(i=0;i<remainbytes;i++)
		{
			ESP_DBG_HEX("%02X ",buf[i+(blockSize*LINE_NUM)]);
		}
		ESP_DBG_HEX("|\r\n");
	}

}


void ICACHE_FLASH_ATTR
UART_ShowByteInCharHex_WithItem(char *item , u8 *buf,u16 len)
{
	ESP_DBG_HEX(item);

	UART_ShowByteInCharHex(buf,len);
}
