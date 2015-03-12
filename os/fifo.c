/* FIFO���C�u���� */

#include "bootpack.h"

#define FLAGS_OVERRUN		0x0001

void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task)
/* FIFO��ʼ����bufΪ�Ѿ�������ڴ棬FIFO32��һ�����ݽṹ */
{
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size; /* ʣ�� */
	fifo->flags = 0;
	fifo->p = 0; /* β */
	fifo->q = 0; /* ͷ */
	fifo->task = task; /* �f�[�^���������Ƃ��ɋN�����^�X�N */
	return;
}

int fifo32_put(struct FIFO32 *fifo, int data)
/* �����ݷ��뵽FIFO */
{
	if (fifo->free == 0) {
		/* ���� */
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->p] = data;
	fifo->p++;
	if (fifo->p == fifo->size) {
		fifo->p = 0;		//ѭ������
	}
	fifo->free--;
	if (fifo->task != 0) {
		if (fifo->task->flags != 2) { /* �^�X�N���Q�Ă����� */
			task_run(fifo->task, -1, 0); /* �N�����Ă����� */
		}
	}
	return 0;
}

int fifo32_get(struct FIFO32 *fifo)
/* ��FIFO�л��һ��int */
{
	int data;
	if (fifo->free == fifo->size) {
		/* Ϊ�� */
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
/* ���ڻ�ʣ�¶��ٿռ� */
{
	return fifo->size - fifo->free;
}
