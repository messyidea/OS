/* �}�E�X�֌W */

#include "bootpack.h"

struct FIFO32 *mousefifo;
int mousedata0;

void inthandler2c(int *esp)
{
	int data;
	io_out8(PIC1_OCW2, 0x64);	/* ֪ͨPIC1 IRQ-12������� */
	io_out8(PIC0_OCW2, 0x62);	/* ֪ͨPIC0 IRQ-02������� */
	data = io_in8(PORT_KEYDAT);
	fifo32_put(mousefifo, data + mousedata0);	//ΪʲôҪ��� 
	return;
}

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

 
void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec)
{
	/* ��ʼ��FIFO */
	mousefifo = fifo;
	mousedata0 = data0;
	/* ������� */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	/* ˳���Ļ��᷵��ACK */
	mdec->phase = 0; /* ��ʼ�����phase */
	return;
}

//����mouse���� 
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
	if (mdec->phase == 0) {
		/* �ȴ�����0xfa״̬ */
		if (dat == 0xfa) {
			mdec->phase = 1;
		}
		return 0;
	}
	if (mdec->phase == 1) {
		/* ����һ�ֽ� */
		if ((dat & 0xc8) == 0x08) {
			mdec->buf[0] = dat;
			mdec->phase = 2;
		}
		return 0;
	}
	if (mdec->phase == 2) {
		/* ���ڶ��ֽ� */
		mdec->buf[1] = dat;
		mdec->phase = 3;
		return 0;
	}
	if (mdec->phase == 3) {
		/* �������ֽ� */
		mdec->buf[2] = dat;
		mdec->phase = 1;
		//����һϵ����Ϣ 
		mdec->btn = mdec->buf[0] & 0x07;
		mdec->x = mdec->buf[1];
		mdec->y = mdec->buf[2];
		if ((mdec->buf[0] & 0x10) != 0) {
			mdec->x |= 0xffffff00;
		}
		if ((mdec->buf[0] & 0x20) != 0) {
			mdec->y |= 0xffffff00;
		}
		mdec->y = - mdec->y; /* ���y�����뻭������෴ */
		return 1;
	}
	return -1; /* �ɼӿɲ��� */
}
