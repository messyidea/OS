/* マウス関係 */

#include "bootpack.h"

struct FIFO32 *mousefifo;
int mousedata0;

void inthandler2c(int *esp)
/* PS/2マウスからの割り込み */
{
	int data;
	io_out8(PIC1_OCW2, 0x64);	/* ﾍｨﾖｪPIC1 IRQ-12ﾊﾜﾀ�ﾍ�ｳﾉ */
	io_out8(PIC0_OCW2, 0x62);	/* ﾍｨﾖｪPIC0 IRQ-02ﾊﾜﾀ�ﾍ�ｳﾉ */
	data = io_in8(PORT_KEYDAT);
	fifo32_put(mousefifo, data + mousedata0);	//ﾎｪﾊｲﾃｴﾒｪﾏ狆ﾓ 
	return;
}

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

 
void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec)
{
	/* ｳ�ﾊｼｻｯFIFO */
	mousefifo = fifo;
	mousedata0 = data0;
	/* ｼ､ｻ�ﾊ�ｱ� */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	/* ﾋｳﾀ�ｵﾄｻｰｻ盥ｵｻﾘACK */
	mdec->phase = 0; /* ｳ�ﾊｼｻｯﾊ�ｱ麪hase */
	return;
}

//ｽ簧�mouseﾊ�ｾﾝ 
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
	if (mdec->phase == 0) {
		/* ｵﾈｴ�ﾊ�ｱ�ｵﾄ0xfaﾗｴﾌｬ */
		if (dat == 0xfa) {
			mdec->phase = 1;
		}
		return 0;
	}
	if (mdec->phase == 1) {
		/* ﾊ�ｱ�ｵﾚﾒｻﾗﾖｽﾚ */
		if ((dat & 0xc8) == 0x08) {
			/* 正しい1バイト目だった */
			mdec->buf[0] = dat;
			mdec->phase = 2;
		}
		return 0;
	}
	if (mdec->phase == 2) {
		/* ﾊ�ｱ�ｵﾚｶ�ﾗﾖｽﾚ */
		mdec->buf[1] = dat;
		mdec->phase = 3;
		return 0;
	}
	if (mdec->phase == 3) {
		/* ﾊ�ｱ�ｵﾚﾈ�ﾗﾖｽﾚ */
		mdec->buf[2] = dat;
		mdec->phase = 1;
		//ｽ簧�ﾒｻﾏｵﾁﾐﾐﾅﾏ｢ 
		mdec->btn = mdec->buf[0] & 0x07;
		mdec->x = mdec->buf[1];
		mdec->y = mdec->buf[2];
		if ((mdec->buf[0] & 0x10) != 0) {
			mdec->x |= 0xffffff00;
		}
		if ((mdec->buf[0] & 0x20) != 0) {
			mdec->y |= 0xffffff00;
		}
		mdec->y = - mdec->y; /* ﾊ�ｱ點ｷｽﾏ�ﾓ�ｻｭﾃ豺�ｺﾅﾏ犢ｴ */
		return 1;
	}
	return -1; /* ｿﾉｼﾓｿﾉｲｻｼﾓ */
}
