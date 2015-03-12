/* FIFO儔僀僽儔儕 */

#include "bootpack.h"

#define FLAGS_OVERRUN		0x0001

void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task)
/* FIFO初始化，buf为已经分配的内存，FIFO32是一个数据结构 */
{
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size; /* 剩余 */
	fifo->flags = 0;
	fifo->p = 0; /* 尾 */
	fifo->q = 0; /* 头 */
	fifo->task = task; /* 僨乕僞偑擖偭偨偲偒偵婲偙偡僞僗僋 */
	return;
}

int fifo32_put(struct FIFO32 *fifo, int data)
/* 把数据放入到FIFO */
{
	if (fifo->free == 0) {
		/* 满了 */
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->p] = data;
	fifo->p++;
	if (fifo->p == fifo->size) {
		fifo->p = 0;		//循环队列
	}
	fifo->free--;
	if (fifo->task != 0) {
		if (fifo->task->flags != 2) { /* 僞僗僋偑怮偰偄偨傜 */
			task_run(fifo->task, -1, 0); /* 婲偙偟偰偁偘傞 */
		}
	}
	return 0;
}

int fifo32_get(struct FIFO32 *fifo)
/* 从FIFO中获得一个int */
{
	int data;
	if (fifo->free == fifo->size) {
		/* 为空 */
		return -1;
	}
	data = fifo->buf[fifo->q];
	fifo->q++;
	if (fifo->q == fifo->size) {
		fifo->q = 0;
	}
	fifo->free++;
	return data;
}

int fifo32_status(struct FIFO32 *fifo)
/* 现在还剩下多少空间 */
{
	return fifo->size - fifo->free;
}
