/* ���荞�݊֌W */

#include "bootpack.h"
#include <stdio.h>

void init_pic(void)
/* PIC��ʼ�� */
{
	io_out8(PIC0_IMR,  0xff  ); /* PIC0��ֹ�����ж� */
	io_out8(PIC1_IMR,  0xff  ); /* PIC1��ֹ�����ж� */

	io_out8(PIC0_ICW1, 0x11  ); /* ���ش�����ʽ */
	io_out8(PIC0_ICW2, 0x20  ); /* IRQ0-7 �� INT20-27 ���� */
	io_out8(PIC0_ICW3, 1 << 2); /* PIC1 �� IRQ2 ���� */
	io_out8(PIC0_ICW4, 0x01  ); /* �޻���ģʽ */

	io_out8(PIC1_ICW1, 0x11  ); /* ���ش�����ʽ */
	io_out8(PIC1_ICW2, 0x28  ); /* IRQ8-15 �� INT28-2f���� */
	io_out8(PIC1_ICW3, 2     ); /* PIC1��IRQ2���� */
	io_out8(PIC1_ICW4, 0x01  ); /* �޻���ģʽ */

	io_out8(PIC0_IMR,  0xfb  ); /* 11111011 PIC1����ȫ����ֹ */
	io_out8(PIC1_IMR,  0xff  ); /* 11111111 ��ֹ�����ж� */

	return;
}

void inthandler27(int *esp)
/* PIC0����̕s���S���荞�ݑ΍� */
/* Athlon64X2�@�Ȃǂł̓`�b�v�Z�b�g�̓s���ɂ��PIC�̏��������ɂ��̊��荞�݂�1�x���������� */
/* ���̊��荞�ݏ����֐��́A���̊��荞�݂ɑ΂��ĉ������Ȃ��ł��߂��� */
/* �Ȃ��������Ȃ��Ă����́H
	��  ���̊��荞�݂�PIC���������̓d�C�I�ȃm�C�Y�ɂ���Ĕ����������̂Ȃ̂ŁA
		�܂��߂ɉ����������Ă��K�v���Ȃ��B									*/
{
	io_out8(PIC0_OCW2, 0x67); /* IRQ-07��t������PIC�ɒʒm */
	return;
}
