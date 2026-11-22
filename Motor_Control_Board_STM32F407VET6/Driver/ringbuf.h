#ifndef __RINGBUF_H__
#define __RINGBUF_H__
#include "stdint.h"

typedef struct{
	uint8_t *buf;
	uint32_t mask;
	
	volatile uint32_t head;//生产者：中断/dma里面执行，加上volatile防止被优化
	volatile uint32_t tail;
}ringbuf_t;

void ringbuf_init(ringbuf_t *rb,uint8_t *mem,uint32_t size_pow2);

uint32_t ringbuf_used(const ringbuf_t *rb);
uint32_t ringbuf_free(const ringbuf_t *rb);

#endif