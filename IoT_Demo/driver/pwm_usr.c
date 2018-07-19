#include "os_type.h"
#include "ets_sys.h"
#include "osapi.h"
#include "pwm_usr.h"

struct _LIGHT_PARAM light_param_u;

void ICACHE_FLASH_ATTR
pwm_Init(void)
{
	uint32 io_info[][3] =
	{
		{ PWM_0_OUT_IO_MUX, PWM_0_OUT_IO_FUNC,PWM_0_OUT_IO_NUM },
		{ PWM_1_OUT_IO_MUX, PWM_1_OUT_IO_FUNC,PWM_1_OUT_IO_NUM },
		{ PWM_2_OUT_IO_MUX, PWM_2_OUT_IO_FUNC,PWM_2_OUT_IO_NUM }
	};

	light_param_u.pwm_period = 1000;
	light_param_u.pwm_duty[0] = 0;
	light_param_u.pwm_duty[1] = 0;
	light_param_u.pwm_duty[2] = 0;


#if (PWM_ENABLE == 1)
	pwm_init(light_param_u.pwm_period, light_param_u.pwm_duty, 1, io_info);
#endif
/*	light_param_u.inc = 1;
	light_param_u.pwm_duty[1] = 500;
	pwm_set_duty(light_param_u.pwm_duty[1]*22, 1);
	pwm_start();*/
	ESP_DBG("\r\n pwm_Init \r\n\r\n");
}

