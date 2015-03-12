/* FIFOライブラリ */

#include "bootpack.h"

#define FLAGS_OVERRUN		0x0001

void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task)
/* FIFOｳ�ﾊｼｻｯ｣ｬbufﾎｪﾒﾑｾｭｷﾖﾅ莊ﾄﾄﾚｴ譽ｬFIFO32ﾊﾇﾒｻｸ�ﾊ�ｾﾝｽ盪ｹ */
{
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size; /* ﾊ｣ﾓ� */
	fifo->flags = 0;
	fifo->p = 0; /* ﾎｲ */
	fifo->q = 0; /* ﾍｷ */
	fifo->task = task; /* データが入ったときに起こすタスク */
	return;
}

int fifo32_put(struct FIFO32 *fifo, int data)
/* ｰﾑﾊ�ｾﾝｷﾅﾈ�ｵｽFIFO */
{
	if (fifo->free == 0) {
		/* ﾂ�ﾁﾋ */
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->p] = data;
	fifo->p++;
	if (fifo->p == fifo->size) {
		fifo->p = 0;		//ﾑｭｻｷｶﾓﾁﾐ
	}
	fifo->free--;
	if (fifo->task != 0) {
		if (fifo->task->flags != 2) { /* タスクが寝ていたら */
			task_run(fifo->task, -1, 0); /* 起こしてあげる */
		}
	}
	return 0;
}

int fifo32_get(struct FIFO32 *fifo)
/* ｴﾓFIFOﾖﾐｻ�ｵﾃﾒｻｸ�int */
{
	int data;
	if (fifo->free == fifo->size) {
		/* ﾎｪｿﾕ */
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
/* ﾏﾖﾔﾚｻｹﾊ｣ﾏﾂｶ猖ﾙｿﾕｼ� */
{
	return fifo->size - fifo->free;
}
