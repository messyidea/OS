/* 割り込み関係 */

#include "bootpack.h"
#include <stdio.h>

void init_pic(void)
/* PICｳｼｻｯ */
{
	io_out8(PIC0_IMR,  0xff  ); /* PIC0ｽ鋓ｹﾋﾐﾖﾐｶﾏ */
	io_out8(PIC1_IMR,  0xff  ); /* PIC1ｽ鋓ｹﾋﾐﾖﾐｶﾏ */

	io_out8(PIC0_ICW1, 0x11  ); /* ｱﾟﾑﾘｴ･ｷ｢ｷｽﾊｽ */
	io_out8(PIC0_ICW2, 0x20  ); /* IRQ0-7 ﾓﾉ INT20-27 ｽﾓﾊﾕ */
	io_out8(PIC0_ICW3, 1 << 2); /* PIC1 ﾓﾉ IRQ2 ﾁｬｽﾓ */
	io_out8(PIC0_ICW4, 0x01  ); /* ﾎﾞｻｺｳ蠧｣ﾊｽ */

	io_out8(PIC1_ICW1, 0x11  ); /* ｱﾟﾑﾘｴ･ｷ｢ｷｽﾊｽ */
	io_out8(PIC1_ICW2, 0x28  ); /* IRQ8-15 ﾓﾉ INT28-2fｽﾓﾊﾕ */
	io_out8(PIC1_ICW3, 2     ); /* PIC1ﾓﾉIRQ2ﾁｬｽﾓ */
	io_out8(PIC1_ICW4, 0x01  ); /* ﾎﾞｻｺｳ蠧｣ﾊｽ */

	io_out8(PIC0_IMR,  0xfb  ); /* 11111011 PIC1ﾒﾔﾍ篳ｫｲｿｽ鋓ｹ */
	io_out8(PIC1_IMR,  0xff  ); /* 11111111 ｽ鋓ｹﾋﾐﾖﾐｶﾏ */

	return;
}

void inthandler27(int *esp)
/* PIC0からの不完全割り込み対策 */
/* Athlon64X2機などではチップセットの都合によりPICの初期化時にこの割り込みが1度だけおこる */
/* この割り込み処理関数は、その割り込みに対して何もしないでやり過ごす */
/* なぜ何もしなくていいの？
	→  この割り込みはPIC初期化時の電気的なノイズによって発生したものなので、
		まじめに何か処理してやる必要がない。									*/
{
	io_out8(PIC0_OCW2, 0x67); /* IRQ-07受付完了をPICに通知 */
	return;
}
