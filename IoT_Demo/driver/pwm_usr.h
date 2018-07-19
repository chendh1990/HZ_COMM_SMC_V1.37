#ifndef __PWM_USR_H__
#define __PWM_USR_H__


#define PWM_0_OUT_IO_MUX 	PERIPHS_IO_MUX_MTCK_U
#define PWM_0_OUT_IO_NUM 	13
#define PWM_0_OUT_IO_FUNC  	FUNC_GPIO13

#define PWM_1_OUT_IO_MUX 	PERIPHS_IO_MUX_MTCK_U
#define PWM_1_OUT_IO_NUM 	13
#define PWM_1_OUT_IO_FUNC  	FUNC_GPIO13

/*
#define PWM_1_OUT_IO_MUX 	PERIPHS_IO_MUX_MTDO_U
#define PWM_1_OUT_IO_NUM 	15
#define PWM_1_OUT_IO_FUNC  	FUNC_GPIO15
*/

#define PWM_2_OUT_IO_MUX 	PERIPHS_IO_MUX_MTCK_U
#define PWM_2_OUT_IO_NUM 	13
#define PWM_2_OUT_IO_FUNC  	FUNC_GPIO13

struct _LIGHT_PARAM {
    u32 pwm_period;
    u32 pwm_duty[8];
    u8 inc;
};

extern struct _LIGHT_PARAM light_param_u;


void pwm_Init(void);


#endif
