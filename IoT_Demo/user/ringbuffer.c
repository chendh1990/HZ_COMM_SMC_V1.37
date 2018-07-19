/**
 * Project page: https://github.com/wangrn/ringbuffer
 * Copyright (c) 2013 Wang Ruining <https://github.com/wangrn>
 * @date 2013/01/16 13:33:20
 * @brief   a simple ringbuffer, DO NOT support dynamic expanded memory
 */
#include "user_config.h"

rb_t RB;

u8 RingBuffer[100];

void ICACHE_FLASH_ATTR RingbufferInit(void)
{

	u8 buffer[10] = {'0','1','2','3','4','5','6','7','8','9'};


	RB.rbCapacity = sizeof(RingBuffer);
	RB.rbBuff = RingBuffer;

	rbCreate(&RB);

	ESP_DBG("rbCanRead:%ld \r\n" , rbCanRead(&RB));

	ESP_DBG("rbCanWrite:%ld \r\n" , rbCanWrite(&RB));

	if(rbWrite(&RB,buffer, sizeof(buffer)) < 0 )
	{
		ESP_DBG("write error \r\n");
	}

	memset(buffer,0,sizeof(buffer));

	if(rbRead(&RB,buffer,sizeof(buffer)) < 0)
	{
		ESP_DBG("read error \r\n");
	}

	ESP_DBG_HEX_I("Ringbuffer:",buffer,sizeof(buffer));
}

int8_t ICACHE_FLASH_ATTR rbCreate(rb_t* rb)
{
    if(NULL == rb)
    {
        return -1;
    }

    rb->rbHead = rb->rbBuff;
    rb->rbTail = rb->rbBuff;
    return 0;
}

int8_t ICACHE_FLASH_ATTR rbDelete(rb_t* rb)
{
    if(NULL == rb)
    {
        return -1;
    }

    rb->rbBuff = NULL;
    rb->rbHead = NULL;
    rb->rbTail = NULL;
    rb->rbCapacity = 0;
		return 0;
}

int32_t ICACHE_FLASH_ATTR rbCapacity(rb_t *rb)
{
    if(NULL == rb)
    {
        return -1;
    }

    return rb->rbCapacity;
}

int32_t ICACHE_FLASH_ATTR rbCanRead(rb_t *rb)
{
    if(NULL == rb)
    {
        return -1;
    }

    if (rb->rbHead == rb->rbTail)
    {
        return 0;
    }

    if (rb->rbHead < rb->rbTail)
    {
        return rb->rbTail - rb->rbHead;
    }

    return rbCapacity(rb) - (rb->rbHead - rb->rbTail);
}

int32_t ICACHE_FLASH_ATTR rbCanWrite(rb_t *rb)
{
    if(NULL == rb)
    {
        return -1;
    }

    return rbCapacity(rb) - rbCanRead(rb);
}

int32_t ICACHE_FLASH_ATTR rbRead(rb_t *rb, void *data, size_t count)
{
    int32_t copySz = 0;

    if(NULL == rb)
    {
        return -1;
    }

    if(NULL == data)
    {
        return -1;
    }

    if (rb->rbHead < rb->rbTail)
    {
        copySz = min(count, rbCanRead(rb));
        memcpy(data, rb->rbHead, copySz);
        rb->rbHead += copySz;
        return copySz;
    }
    else
    {
        if (count < rbCapacity(rb)-(rb->rbHead - rb->rbBuff))
        {
            copySz = count;
            memcpy(data, rb->rbHead, copySz);
            rb->rbHead += copySz;
            return copySz;
        }
        else
        {
            copySz = rbCapacity(rb) - (rb->rbHead - rb->rbBuff);
            memcpy(data, rb->rbHead, copySz);
            rb->rbHead = rb->rbBuff;
            copySz += rbRead(rb, (char*)data+copySz, count-copySz);
            return copySz;
        }
    }
}

int32_t ICACHE_FLASH_ATTR rbWrite(rb_t *rb, const void *data, size_t count)
{
    int32_t tailAvailSz = 0;

    if((NULL == rb)||(NULL == data))
    {
        return -1;
    }

    if (count >= rbCanWrite(rb))
    {
        return -2;
    }

    if (rb->rbHead <= rb->rbTail)
    {
        tailAvailSz = rbCapacity(rb) - (rb->rbTail - rb->rbBuff);
        if (count <= tailAvailSz)
        {
            memcpy(rb->rbTail, data, count);
            rb->rbTail += count;
            if (rb->rbTail == rb->rbBuff+rbCapacity(rb))
            {
                rb->rbTail = rb->rbBuff;
            }
            return count;
        }
        else
        {
            memcpy(rb->rbTail, data, tailAvailSz);

            rb->rbTail = rb->rbBuff;

            return tailAvailSz + rbWrite(rb, (char*)data+tailAvailSz, count-tailAvailSz);
        }
    }
    else
    {
        memcpy(rb->rbTail, data, count);
        rb->rbTail += count;
        return count;
    }
}
