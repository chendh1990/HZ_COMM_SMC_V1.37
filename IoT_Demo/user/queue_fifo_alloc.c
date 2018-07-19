#include "user_config.h"

#define ROW				50
#define COLUMN			1
LOCAL u8 Array[ROW][COLUMN];

//Queue Init  
void ICACHE_FLASH_ATTR QueueAllocInit(QUEUE_ALLOC *Queue , u8 *array ,u16 row,u8 column)
{  
  Queue->front = Queue->rear = 0;	//初始化时队列头队列首相连
  Queue->count = 0;   				//队列计数为0

  Queue->dat = array;

  Queue->size = row;

  Queue->column =column;

  ESP_DBG("Queue->size:%d Queue->column:%d \r\n" , Queue->size , Queue->column );
}

// Queue In  
uint8 ICACHE_FLASH_ATTR QueueAllocIn(QUEUE_ALLOC *Queue,u8 *sdat) //数据进入队列
{//一次只能入一个数据，保证队尾不会超过队头font，最多相等  

  u8 i;
  
  if((Queue->front == Queue->rear) && (Queue->count == Queue->size))
  {      // full 判断如果队列满了，Queue->front == Queue->rear时也可能是空队列  
     return Q_FULL;    //返回队列满的标志
  }
  else  
  {                    // in  

	if(Queue->dat != NULL)
	{
		for(i=0;i<Queue->column;i++)
		{
			*(Queue->dat+Queue->rear*Queue->column+i) = sdat[i];
		}
	}
    Queue->rear = (Queue->rear + 1) % Queue->size;       //队尾在增长 ,数组下标不能越界
    Queue->count = Queue->count + 1;                    
    return Q_OPERATE_OK;
  }  
}  


// Queue Out  
uint8 ICACHE_FLASH_ATTR QueueAllocOut(QUEUE_ALLOC *Queue,u8 *sdat)
{  
  
  u8 i;
  
  if((Queue->front == Queue->rear) && (Queue->count == 0))  
  {                    // empty  
    return Q_EMPTY;
  }
  else  
  {                    // out  

	if(Queue->dat != NULL)
	{
		for(i=0;i<Queue->column;i++)
		{
			sdat[i] = *(Queue->dat+Queue->front*Queue->column+i);
		}
	}

    Queue->front = (Queue->front + 1) % Queue->size;//对头也在增长，到数组                                         //                                                       //尾就重新回到数组开始
    Queue->count = Queue->count - 1;  

    return Q_OPERATE_OK;
  }  
}  

//////////////////////////////////////////////////////////  
//////////////////////////////////////////////////////////  
#if (QUEUE_ALLOC_FIFO_TEST_EN)
void ICACHE_FLASH_ATTR QueueAllocTest(void)
{  

  u8 inData[COLUMN] = {0xAA};
  u8 outData[COLUMN] = {0xBB};
  u8 i; 
  u8 k;
  u8 t;
  u8 sh;
  QUEUE_ALLOC QueueAlloc;
  
  QueueAllocInit(&QueueAlloc,&Array[0][0],ROW,COLUMN);
  
  t = 5;
  
  while(t--)
  {
    
  
  
    for(i = 0;i < 60;i++)  
    {  
      inData[0] = i;
      if(QueueAllocIn(&QueueAlloc,inData) == Q_FULL)
      {
        ESP_DBG("Queue full \r\n");
        break;  
      }
      else
      {
    	  ESP_DBG("Queue add :%d \r\n" , i);
      }
      
    }  
    for(i = 0;i < 60;i++)  
    {  
      if(QueueAllocOut(&QueueAlloc,outData) == Q_EMPTY)
      {
    	  ESP_DBG("Queue Empty \r\n");
        break; 
      }
      else
      {
        for(k=0;k<QueueAlloc.column;k++)
        {
        	ESP_DBG("%X " , outData[k]);
        }
        ESP_DBG("\r\n");
      }
    }  

  }
}  
#endif
