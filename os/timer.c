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
	t->next = 0; /* ???????? */
	timerctl.t0 = t; /* 指向第一个 */
	timerctl.next = 0xffffffff; /* 下个 */
	return;
}

struct TIMER *timer_alloc(void)
{
	int i;
	for (i = 0; i < MAX_TIMER; i++) {
		if (timerctl.timers0[i].flags == 0) {
			timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
			return &timerctl.timers0[i];
		}
	}
	return 0; /* ????????????? */
}

void timer_free(struct TIMER *timer)
{
	timer->flags = 0; /* ???g?p */
	return;
}

void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data)
{
	timer->fifo = fifo;
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
		/* ?擪??????? */
		timerctl.t0 = timer;
		timer->next = t; /* ????t */
		timerctl.next = timer->timeout;
		io_store_eflags(e);
		return;
	}
	/* ?????????????????T?? */
	for (;;) {
		s = t;
		t = t->next;
		if (timer->timeout <= t->timeout) {
			/* s??t????????? */
			s->next = timer; /* s?????timer */
			timer->next = t; /* timer?????t */
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
	if (timerctl.next > timerctl.count) {
		return;
	}
	timer = timerctl.t0; /* ??肠?????擪???n??timer?ɑ?? */
	for (;;) {
		/* timers??^?C?}??S?ē??????????Aflags??m?F????? */
		if (timer->timeout > timerctl.count) {
			break;
		}
		/* ?^?C???A?E?g */
		timer->flags = TIMER_FLAGS_ALLOC;
		if (timer != task_timer) {
			fifo32_put(timer->fifo, timer->data);
		} else {
			ts = 1; /* task_timer???^?C???A?E?g???? */
		}
		timer = timer->next; /* ????^?C?}???n??timer?ɑ?? */
	}
	timerctl.t0 = timer;
	timerctl.next = timer->timeout;
	if (ts != 0) {
		task_switch();
	}
	return;
}
