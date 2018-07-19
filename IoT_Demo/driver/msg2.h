///////////////////////////////////////////////////////////////////////////////////
#ifndef _MSG2_H_
#define _MSG2_H_

#define NO_MSG 	0XFF


enum MSG_DEF_
{
	MSG_PC_SET = 0X60,
	MSG_TIMESTAMP,
	MSG_RTC_PRINT,
	XX1,
	XX2,
};

void put_msg_fifo(u8 msg);
void put_msg_lifo(u8 msg);
u8 get_msg2();
void flush_msg2();



#endif

