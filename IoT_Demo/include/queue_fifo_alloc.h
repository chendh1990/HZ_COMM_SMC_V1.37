
#ifndef QUEUE_FIFO_ALLOC_H
#define QUEUE_FIFO_ALLOC_H
//*****************************************************

typedef enum
{
	Q_FULL,
	Q_EMPTY,
	Q_OPERATE_OK
}Q_STATE;

typedef struct  
{  
    uint16 front;     //����ͷ����ͷ������  
    uint16 rear;        //����β.��β��������  
    uint16 count;       //���м���  
    u8 *dat;
    u8 column;
    u8 size;
}QUEUE_ALLOC;

//Queue Initalize  
void QueueAllocInit(QUEUE_ALLOC *Queue , u8 *array ,u16 row,u8 column);
// Queue In  
uint8 QueueAllocIn(QUEUE_ALLOC *Queue,u8 *sdat);
// Queue Out  
uint8 QueueAllocOut(QUEUE_ALLOC *Queue,u8 *sdat);
  
#define QUEUE_ALLOC_FIFO_TEST_EN      0

#if (QUEUE_ALLOC_FIFO_TEST_EN)
void QueueAllocTest(void);
#endif
  

#endif /* QUEUE_FIFO_H */
