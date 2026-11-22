#include "ringbuf.h"

void ringbuf_init(ringbuf_t *rb,uint8_t *mem,uint32_t size_pow2)
	{
		rb ->buf = mem;
		rb ->mask = size_pow2 - 1U;
		
		rb ->head = 0;
		rb ->tail = 0;
	}

uint32_t ringbuf_used(const ringbuf_t *rb)
	{
		return rb ->head - rb ->tail;
	}

uint32_t ringbuf_free(const ringbuf_t *rb)
	{
		uint32_t size = rb ->mask +1U;
		return size - ringbuf_used(rb);
	}