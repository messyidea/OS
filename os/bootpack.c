/* bootpack */

#include "bootpack.h"
#include <stdio.h>

#define KEYCMD_LED		0xed
void keywin_off(struct SHEET *key_win);
void keywin_on(struct SHEET *key_win);




void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;      //hͷ�ļ��ĺ궨��
	struct FIFO32 fifo, keycmd;
	char s[40];
	int fifobuf[128], keycmd_buf[32], *cons_fifo[2];
	int mx, my, i;
	unsigned int memtotal;
	struct MOUSE_DEC mdec;      //���
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;      //������ڴ濪ʼ��ַ
	struct SHTCTL *shtctl;
    static char keytable0[0x80] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0x08, 0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0x0a, 0, 'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0x5c, 0,  0,   0,   0,   0,   0,   0,   0,   0,   0x5c, 0,  0
	};
	static char keytable1[0x80] = {
		0,   0,   '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~', '=', '~', 0x08, 0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', 0x0a, 0, 'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*', 0,   0,   '}', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   '_', 0,   0,   0,   0,   0,   0,   0,   0,   0,   '|', 0,   0
	};
	unsigned char *buf_back, buf_mouse[256], *buf_cons[2];
	struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_cons[2];
	struct TASK *task_a, *task_cons[2], *task;
	//struct TIMER *timer;
    int key_to = 0, key_shift = 0, key_leds = (binfo->leds >> 4) & 7,keycmd_wait = -1;
    struct CONSOLE *cons;
    int j, x, y, mmx = -1, mmy = -1;
	struct SHEET *sht = 0, *key_win;
    


	init_gdtidt();      // ��ʼ��gdt��idt
	init_pic();         // pic��ʼ��
	io_sti(); /* IDT/PIC��ʼ����ɣ�����CPU�ж� */
	fifo32_init(&fifo, 128, fifobuf, 0);
	init_pit(); 
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);
	io_out8(PIC0_IMR, 0xf8); /* ��������ж� */
	io_out8(PIC1_IMR, 0xef); /* ���ż����ж� */
    fifo32_init(&keycmd, 32, keycmd_buf, 0);    //����



	memtotal = memtest(0x00400000, 0xbfffffff);     //�����ڴ�����
	memman_init(memman);		//��ʼ���ڴ� 
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
	memman_free(memman, 0x00400000, memtotal - 0x00400000); 	//���� 

	init_palette();     //��ʼ����ɫ��
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny); //��ʼ��ͼ����Ϣ 
	task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 2);
    *((int *) 0x0fe4) = (int) shtctl;   //����consoleʹ��

	/* sht_back */
	sht_back  = sheet_alloc(shtctl);	//����һ��sheet 
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);     //�����ڴ� 
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); /* ����sheet */
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);     //��ʼ��,���ǻ���ʼ�˵���

    /* sht_cons �ն�*/
    for(i=0; i<2; i++) {
	sht_cons[i] = sheet_alloc(shtctl);
	buf_cons[i] = (unsigned char *) memman_alloc_4k(memman, 256 * 165);
	sheet_setbuf(sht_cons[i], buf_cons[i], 256, 165, -1); /* ��͸��ɫ */
	make_window8(buf_cons[i], 256, 165, "console", 0);
	make_textbox8(sht_cons[i], 8, 28, 240, 128, COL8_000000);
	task_cons[i] = task_alloc();
	task_cons[i]->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
	task_cons[i]->tss.eip = (int) &console_task;
	task_cons[i]->tss.es = 1 * 8;
	task_cons[i]->tss.cs = 2 * 8;
	task_cons[i]->tss.ss = 1 * 8;
	task_cons[i]->tss.ds = 1 * 8;
	task_cons[i]->tss.fs = 1 * 8;
	task_cons[i]->tss.gs = 1 * 8;
	*((int *) (task_cons[i]->tss.esp + 4)) = (int) sht_cons[i];
    *((int *) (task_cons[i]->tss.esp + 8)) = memtotal;     //��������
	task_run(task_cons[i], 2, 2); /* level=2, priority=2 */
    sht_cons[i]->task = task_cons[i];
	sht_cons[i]->flags |= 0x20;	/* �й�� */
    cons_fifo[i] = (int *) memman_alloc_4k(memman, 128 * 4);
	fifo32_init(&task_cons[i]->fifo, 128, cons_fifo[i], task_cons[i]);
    }
    

	/* sht_win_b */
	/*for (i = 0; i < 3; i++) {
		sht_win_b[i] = sheet_alloc(shtctl);
		buf_win_b = (unsigned char *) memman_alloc_4k(memman, 144 * 52);
		sheet_setbuf(sht_win_b[i], buf_win_b, 144, 52, -1); // ���ô��� 
		sprintf(s, "task_b%d", i);
		make_window8(buf_win_b, 144, 52, s, 0);
		task_b[i] = task_alloc();
		task_b[i]->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
		task_b[i]->tss.eip = (int) &task_b_main;
		task_b[i]->tss.es = 1 * 8;
		task_b[i]->tss.cs = 2 * 8;
		task_b[i]->tss.ss = 1 * 8;
		task_b[i]->tss.ds = 1 * 8;
		task_b[i]->tss.fs = 1 * 8;
		task_b[i]->tss.gs = 1 * 8;
		*((int *) (task_b[i]->tss.esp + 4)) = (int) sht_win_b[i];
		//task_run(task_b[i], 2, i + 1);
	}*/

	/* sht_win */
	/*sht_win   = sheet_alloc(shtctl);
	buf_win   = (unsigned char *) memman_alloc_4k(memman, 160 * 52);
	sheet_setbuf(sht_win, buf_win, 144, 52, -1); 
	make_window8(buf_win, 144, 52, "task_a", 1);
	make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;
	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 50);*/

	/* sht_mouse */
	sht_mouse = sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	init_mouse_cursor8(buf_mouse, 99);
	mx = (binfo->scrnx - 16) / 2; /* �м�λ�� */
	my = (binfo->scrny - 28 - 16) / 2;
	
	//�ƶ�����λ�� 
	sheet_slide(sht_back, 0, 0);
    sheet_slide(sht_cons[1], 56,  6);
    sheet_slide(sht_cons[0], 8,  2);
	/*sheet_slide(sht_win_b[0], 168,  56);
	sheet_slide(sht_win_b[1],   8, 116);
	sheet_slide(sht_win_b[2], 168, 116);*/
	//sheet_slide(sht_win,   64,  56);
	sheet_slide(sht_mouse, mx, my);
	//���䴰�ڸ߶� 
	sheet_updown(sht_back,     0);
    sheet_updown(sht_cons[1],  1);
    sheet_updown(sht_cons[0],  2);
	//sheet_updown(sht_win,      3);
	sheet_updown(sht_mouse,    3);
    key_win = sht_win;
    keywin_on(key_win);
	//sht_cons[i]->task = task_cons[i];
	//sht_cons[i]->flags |= 0x20;	/* �й�� */
    

    //Ϊ�˱���͵�ǰ����״̬��ͻ��һ��ʼ��������
    fifo32_put(&keycmd, KEYCMD_LED);
	fifo32_put(&keycmd, key_leds);

	for (;;) {
        if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) {
			/* �����������̿��������͵����ݣ��ͷ����� */
			keycmd_wait = fifo32_get(&keycmd);
			wait_KBC_sendready();
			io_out8(PORT_KEYDAT, keycmd_wait);
		}
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			task_sleep(task_a);
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
            if (key_win->flags == 0) {	/* ���봰�ڱ��ر� */
				key_win = shtctl->sheets[shtctl->top - 1];
				//cursor_c = keywin_on(key_win, sht_win, cursor_c);
                keywin_on(key_win);
			}
            
			if (256 <= i && i <= 511) { /* �������� */
                if (i < 0x80 + 256) { /* �����̱���ת��Ϊ�ַ����� */
					if (key_shift == 0) {
						s[0] = keytable0[i - 256];
					} else {
						s[0] = keytable1[i - 256];
					}
				} else {
					s[0] = 0;
				}
                if ('A' <= s[0] && s[0] <= 'Z') {	/* ������ΪӢ��ʱ */
					if (((key_leds & 4) == 0 && key_shift == 0) ||
							((key_leds & 4) != 0 && key_shift != 0)) {
						s[0] += 0x20;	/* ����д��ĸתСд��ĸ */
					}
				}
			    if (s[0] != 0) { /* һ���ַ� */
                    fifo32_put(&key_win->task->fifo, s[0] + 256);
					
				}	
               
                if (i == 256 + 0x0f) { /* Tab */
                    keywin_off(key_win);
					j = key_win->height - 1;
					if (j == 0) {
						j = shtctl->top - 1;
					}
					key_win = shtctl->sheets[j];
					keywin_on(key_win);
				}
                if (i == 256 + 0x2a) {	/* ��shift ON */
					key_shift |= 1;
				}
				if (i == 256 + 0x36) {	/* ��shift ON */
					key_shift |= 2;
				}
				if (i == 256 + 0xaa) {	/* ��shift OFF */
					key_shift &= ~1;
				}
				if (i == 256 + 0xb6) {	/* ��shift OFF */
					key_shift &= ~2;
				}
                if (i == 256 + 0x3a) {	/* CapsLock */
					key_leds ^= 4;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0x45) {	/* NumLock */
					key_leds ^= 2;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0x46) {	/* ScrollLock */
					key_leds ^= 1;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
                if (i == 256 + 0x3b && key_shift != 0) {	/* Shift+F1 */
                    task = key_win->task;
                    if (task != 0 && task->tss.ss0 != 0) {	/* Shift+F1 */
						cons_putstr0(task->cons, "\nBreak(key) :\n");
						io_cli();	/* ǿ�ƽ�������ʱ��ֹ�����л� */
						task->tss.eax = (int) &(task->tss.esp0);
						task->tss.eip = (int) asm_end_app;
						io_sti();
					}
                    
					
				}
                if (i == 256 + 0x57 && shtctl->top > 2) {	/* F11 */
					sheet_updown(shtctl->sheets[1], shtctl->top - 1);
				}
                
				if (i == 256 + 0xfa) {	/* ���̳ɹ����յ����� */
					keycmd_wait = -1;
				}
				if (i == 256 + 0xfe) {	/* ����û�гɹ����յ����� */
					wait_KBC_sendready();
					io_out8(PORT_KEYDAT, keycmd_wait);
				}
				
			} else if (512 <= i && i <= 767) { /* ������� */
				if (mouse_decode(&mdec, i - 512) != 0) {
					/* �߽紦�� */
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) {
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->scrnx - 1) {
						mx = binfo->scrnx - 1;
					}
					if (my > binfo->scrny - 1) {
						my = binfo->scrny - 1;
					}
					//sprintf(s, "(%3d, %3d)", mx, my);
					//ˢ����ʾ 
					//putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
					sheet_slide(sht_mouse, mx, my);
					if ((mdec.btn & 0x01) != 0) {
                        if (mmx < 0) {
							/* ������ͨģʽ */
							/* �����ϵ��µ�˳���������ָ���ͼ�� */
							for (j = shtctl->top - 1; j > 0; j--) {
								sht = shtctl->sheets[j];
								x = mx - sht->vx0;
								y = my - sht->vy0;
								if (0 <= x && x < sht->bxsize && 0 <= y && y < sht->bysize) {
									if (sht->buf[y * sht->bxsize + x] != sht->col_inv) {
										sheet_updown(sht, shtctl->top - 1);
                                        if (sht != key_win) {
											keywin_off(key_win);
											key_win = sht;
											keywin_on(key_win);
										}
                                        
										if (3 <= x && x < sht->bxsize - 3 && 3 <= y && y < 21) {
											mmx = mx;	/* ���봰���ƶ�ģʽ */
											mmy = my;
										}
                                        if (sht->bxsize - 21 <= x && x < sht->bxsize - 5 && 5 <= y && y < 19) {
											/* ��������x��ť */
											if ((sht->flags & 0x10) != 0) {	/* �����ǰ��������Ӧ�ó��򴰿� */
                                                task = sht->task;
												//cons = (struct CONSOLE *) *((int *) 0x0fec);
												cons_putstr0(task->cons, "\nBreak(mouse) :\n");
												io_cli();	/* ǿ�ƽ��������н�ֹ�л����� */
											    task->tss.eax = (int) &(task->tss.esp0);
												task->tss.eip = (int) asm_end_app;
												io_sti();
											}
										}
                                        
										break;
									}
								}
							}
						} else {
							/* ������ڴ����ƶ�ģʽ */
							x = mx - mmx;	/* �ƶ�������� */
							y = my - mmy;
							sheet_slide(sht, sht->vx0 + x, sht->vy0 + y);
							mmx = mx;	/* �����ƶ�֮������� */
							mmy = my;
						}
                        
					} else {
                        mmx = -1;   //ͨ��ģʽ
                    }
				}
			}
		}
	}
}



void task_b_main(struct SHEET *sht_win_b)
{
	struct FIFO32 fifo;
	struct TIMER *timer_1s;
	int i, fifobuf[128], count = 0, count0 = 0;
	char s[12];

	fifo32_init(&fifo, 128, fifobuf, 0);
	timer_1s = timer_alloc();
	timer_init(timer_1s, &fifo, 100);
	timer_settime(timer_1s, 100);

	for (;;) {
		count++;
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (i == 100) {
				sprintf(s, "%11d", count - count0);
				putfonts8_asc_sht(sht_win_b, 24, 28, COL8_000000, COL8_C6C6C6, s, 11);
				count0 = count;
				timer_settime(timer_1s, 100);
			}
		}
	}
}

void keywin_off(struct SHEET *key_win)
{
	change_wtitle8(key_win, 0);
	if ((key_win->flags & 0x20) != 0) {
		fifo32_put(&key_win->task->fifo, 3); /* �R���\�[���̃J�[�\��OFF */
	}
	return;
}

void keywin_on(struct SHEET *key_win)
{
	change_wtitle8(key_win, 1);
	if ((key_win->flags & 0x20) != 0) {
		fifo32_put(&key_win->task->fifo, 2); /* �R���\�[���̃J�[�\��ON */
	}
	return;
}



