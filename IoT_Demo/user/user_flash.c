//////////////////////////////////////////////////////////////////////////////////////////
#include "user_config.h"

/*****************************************************************
不带保护
spi_flash_erase_sector(FLASH_EXE_APP2_START_SEC+i);
spi_flash_read(FLASH_APP2_START_ADDR+USER_FLASH_SEC_SIZE*i,Upgrade.flash, USER_FLASH_SEC_SIZE);
spi_flash_write(FLASH_EXE_APP2_START_ADDR+USER_FLASH_SEC_SIZE*i,Upgrade.flash, USER_FLASH_SEC_SIZE);

带保护的，不需要擦除
system_param_save_with_protect(USER_PC_SET_FLASH_START_SEC, flashBuffer, FLASH_BUF_SIZE);
system_param_load(USER_PC_SET_FLASH_START_SEC,0,flashBuffer,FLASH_BUF_SIZE);
******************************************************************/


void ICACHE_FLASH_ATTR hz_write_time_policy_to_flash(void)
{
	memset(flashBuffer,0,FLASH_BUF_SIZE);
	os_memcpy(flashBuffer,Policy_Time_Info.buf,sizeof(Policy_Time_Info.buf));
	flashBuffer[F_MASK_START_ADDR+0] = F_MASK0;
	flashBuffer[F_MASK_START_ADDR+1] = F_MASK1;
	flashBuffer[F_MASK_START_ADDR+2] = F_MASK2;
	flashBuffer[F_MASK_START_ADDR+3] = F_MASK3;
	ETS_UART_INTR_DISABLE();
	system_param_save_with_protect(USER_POLICY_TIME_START_SEC, flashBuffer, FLASH_BUF_SIZE);
	ETS_UART_INTR_ENABLE();
}










