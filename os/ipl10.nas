; haribote-ipl
; TAB=4

CYLS	EQU		10				; define CYLS 10

		ORG		0x7c00			; 程序装载地址

; FAT12的规范

		JMP		entry
		DB		0x90
		DB		"HARIBOTE"		; 启动区的名字
		DW		512				; 每个扇区的大小，必须为512
		DB		1				; 每个簇的大小
		DW		1				; FAT其实位置(一般从第一扇区开始)
		DB		2				; FAT的个数
		DW		224				; 根目录大小
		DW		2880			; 该磁盘的大小
		DB		0xf0			; 磁盘的种类
		DW		9				; FAT的长度
		DW		18				; 一个磁道几个扇区
		DW		2				; 磁头数
		DD		0				; 不使用分区
		DD		2880			; 重写一次磁盘大小
		DB		0,0,0x29		; 意义不明？
		DD		0xffffffff		; 卷标号码
		DB		"HARIBOTEOS "	; 磁盘名称，11字节
		DB		"FAT12   "		; 磁盘格式名称，8字节
		RESB	18				; 先空出18字节

; 程序主体

entry:
		MOV		AX,0			; 初始化各寄存器
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX

; 读磁盘

		MOV		AX,0x0820
		MOV		ES,AX
		MOV		CH,0			; 柱面0
		MOV		DH,0			; 磁头0
		MOV		CL,2			; 扇区2
readloop:
		MOV		SI,0			; 记录失败的次数
retry:
		MOV		AH,0x02			; AH=0x02 : 读入磁盘
		MOV		AL,1			; 1个扇区
		MOV		BX,0
		MOV		DL,0x00			; A驱动器
		INT		0x13			; 调用BIOS
		JNC		next			; 没出错的话跳转到next
		ADD		SI,1			; SI+1
		CMP		SI,5			; SI和5比较
		JAE		error			; SI >= 5 ,错误的次数超过5了，跳转到error
		MOV		AH,0x00
		MOV		DL,0x00			; A驱动器
		INT		0x13			; 重置驱动器
		JMP		retry
next:
		MOV		AX,ES			; 把内存地址后移0x200
		ADD		AX,0x0020
		MOV		ES,AX			; ADD ES,0x020 （和前三步一起）
		ADD		CL,1			; CL+1
		CMP		CL,18			; CL和18比较
		JBE		readloop		; CL <= 18 跳转到readloop
		MOV		CL,1
		ADD		DH,1
		CMP		DH,2
		JB		readloop		; DH < 2 跳转到readloop
		MOV		DH,0
		ADD		CH,1
		CMP		CH,CYLS
		JB		readloop		; CH < CYLS 跳转到readloop

; 撉傒廔傢偭偨偺偱haribote.sys傪幚峴偩両

		MOV		[0x0ff0],CH		; CH 保存在[0x0ff0]，IPL柱面大小，IPL偑偳偙傑偱撉傫偩偺偐傪儊儌
		JMP		0xc200          ;跳到启动区开始的地方

; 显示error
error:
		MOV		SI,msg
putloop:
		MOV		AL,[SI]
		ADD		SI,1			; SI+1
		CMP		AL,0
		JE		fin
		MOV		AH,0x0e			; 
		MOV		BX,15			; 
		INT		0x10			; 调用显卡BIOS
		JMP		putloop
fin:
		HLT						; CPU休眠
		JMP		fin				
msg:
		DB		0x0a, 0x0a		; 2个换行
		DB		"load error"
		DB		0x0a			; 换行
		DB		0

		RESB	0x7dfe-$		; 填充0x00直到0x7dfe

		DB		0x55, 0xaa      ; 磁盘结束，必须，表示为启动区
