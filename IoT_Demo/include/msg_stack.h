#ifndef __MSG_STACK_H__
#define __MSG_STACK_H__


//****************************************************************************
//定义数组，数据，数据结构
#define STACK_BLOCK_SIZE       100
#define STACK_BLOCK_NUM       	100
#define STACK_BUF_SIZE       	STACK_BLOCK_SIZE*STACK_BLOCK_NUM


//****************************************************************************
void msg_stack_clear(void);
void msg_stack_push(u8 input[],u8 inLen);
void msg_stack_pop(u8 output[],u8 *outLen)	;
void msg_stack_init(void);

//****************************************************************************



//////////////////////////////////////////////////////
void msg_stack_main(void);


#endif
