/* ?^?C?}??W */

#include "bootpack.h"

#define PIT_CTRL	0x0043
#define PIT_CNT0	0x0040

struct TIMERCTL timerctl;

#define TIMER_FLAGS_ALLOC		1	/* ?m???????? */
#define TIMER_FLAGS_USING		2	/* ?^?C?}???? */

void init_pit(void)
{
	int i;
	struct TIMER *t;
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);	//中断频率100hz 
	io_out8(PIT_CNT0, 0x2e);
	timerctl.count = 0;
	for (i = 0; i < MAX_TIMER; i++) {
		timerctl.timers0[i].flags = 0; /* 未使用 */
	}
	t = timer_alloc(); /* 分配一个time */
	t->timeout = 0xffffffff;	//守护的，保证不会没有计时器运行 
	t->flags = TIMER_FLAGS_USING;
	t->next = 0; /* 末尾 */
	timerctl.t0 = t; /* 指向第一个 */
	timerctl.next = 0xffffffff; /* 只有哨兵，下个超时就是自己 */
	return;
}

struct TIMER *timer_alloc(void)
{
	int i;
	for (i = 0; i < MAX_TIMER; i++) {
		if (timerctl.timers0[i].flags == 0) {
			timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
			timerctl.timers0[i].flags2 = 0;
			return &timerctl.timers0[i];
		}
	}
	return 0; /* 没找到 */
}

void timer_free(struct TIMER *timer)
{
	timer->flags = 0; /* 释放计时器 */
	return;
}

void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data)
{
	timer->fifo = fifo;	//标记 
	timer->data = data;
	return;
}

void timer_settime(struct TIMER *timer, unsigned int timeout)
{
	int e;
	struct TIMER *t, *s;
	timer->timeout = timeout + timerctl.count;
	timer->flags = TIMER_FLAGS_USING;
	e = io_load_eflags();
	io_cli();
	t = timerctl.t0;
	if (timer->timeout <= t->timeout) {
		/* 插入最前面的情况 */
		timerctl.t0 = timer;
		timer->next = t; /* 下面一个是t */
		timerctl.next = timer->timeout;
		io_store_eflags(e);
		return;
	}
	/* 寻找插入位置 */
	for (;;) {
		s = t;
		t = t->next;
		if (timer->timeout <= t->timeout) {
			/* 插入到s和t之间 */
			s->next = timer; /* s下个是timer */
			timer->next = t; /* timer下个是t */
			io_store_eflags(e);
			return;
		}
	}
}

void inthandler20(int *esp)
{
	struct TIMER *timer;
	char ts = 0;
	io_out8(PIC0_OCW2, 0x60);	/* 把IRQ-00信号接收结束的信息通知PIC */
	timerctl.count++;
	if (timerctl.next > timerctl.count) {	//还没到下个时刻，结束 
		return;
	}
	timer = timerctl.t0; /* 起始的位置 */
	for (;;) {
		/* timers定时器都处于动作中，不用确认flag */
		if (timer->timeout > timerctl.count) {
			break;
		}
		/* 超时 */
		timer->flags = TIMER_FLAGS_ALLOC;
		if (timer != task_timer) {
			fifo32_put(timer->fifo, timer->data);
		} else {
			ts = 1; /* task_timer超时切换 */
		}
		timer = timer->next; /* 下一个定时器地址给timer */
	}
	timerctl.t0 = timer;	//现在第一个是timer了 
	timerctl.next = timer->timeout;
	if (ts != 0) {
		task_switch();
	}
	return;
}


int timer_cancel(struct TIMER *timer)
{
	int e;
	struct TIMER *t;
	e = io_load_eflags();
	io_cli();	/* 设置的过程中禁止改变定时器状态 */
	if (timer->flags == TIMER_FLAGS_USING) {	/* 是否需要取消 */
		if (timer == timerctl.t0) {
			/* 第一个定时器的取消处理 */
			t = timer->next;
			timerctl.t0 = t;
			timerctl.next = t->timeout;
		} else {
			/* 非第一个定时器的取消处理 */
			/* 找到timer前一个定时器 */
			t = timerctl.t0;
			for (;;) {
				if (t->next == timer) {
					break;
				}
				t = t->next;
			}
			t->next = timer->next; /* 将 之前的timer的下个 指向 timer的下个 */
		}
		timer->flags = TIMER_FLAGS_ALLOC;
		io_store_eflags(e);
		return 1;	/* 取消处理成功 */
	}
	io_store_eflags(e);
	return 0; /* 不需要取消处理 */
}

void timer_cancelall(struct FIFO32 *fifo)
{
	int e, i;
	struct TIMER *t;
	e = io_load_eflags();
	io_cli();	/* 设定的时候禁止改变定时器状态 */
	for (i = 0; i < MAX_TIMER; i++) {
		t = &timerctl.timers0[i];
		if (t->flags != 0 && t->flags2 != 0 && t->fifo == fifo) {
			timer_cancel(t);
			timer_free(t);
		}
	}
	io_store_eflags(e);
	return;
}

