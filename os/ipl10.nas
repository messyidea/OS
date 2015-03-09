; haribote-ipl
; TAB=4

CYLS	EQU		10				; define CYLS 10

		ORG		0x7c00			; ����װ�ص�ַ

; FAT12�Ĺ淶

		JMP		entry
		DB		0x90
		DB		"HARIBOTE"		; ������������
		DW		512				; ÿ�������Ĵ�С������Ϊ512
		DB		1				; ÿ���صĴ�С
		DW		1				; FAT��ʵλ��(һ��ӵ�һ������ʼ)
		DB		2				; FAT�ĸ���
		DW		224				; ��Ŀ¼��С
		DW		2880			; �ô��̵Ĵ�С
		DB		0xf0			; ���̵�����
		DW		9				; FAT�ĳ���
		DW		18				; һ���ŵ���������
		DW		2				; ��ͷ��
		DD		0				; ��ʹ�÷���
		DD		2880			; ��дһ�δ��̴�С
		DB		0,0,0x29		; ���岻����
		DD		0xffffffff		; ������
		DB		"HARIBOTEOS "	; �������ƣ�11�ֽ�
		DB		"FAT12   "		; ���̸�ʽ���ƣ�8�ֽ�
		RESB	18				; �ȿճ�18�ֽ�

; ��������

entry:
		MOV		AX,0			; ��ʼ�����Ĵ���
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX

; ������

		MOV		AX,0x0820
		MOV		ES,AX
		MOV		CH,0			; ����0
		MOV		DH,0			; ��ͷ0
		MOV		CL,2			; ����2
readloop:
		MOV		SI,0			; ��¼ʧ�ܵĴ���
retry:
		MOV		AH,0x02			; AH=0x02 : �������
		MOV		AL,1			; 1������
		MOV		BX,0
		MOV		DL,0x00			; A������
		INT		0x13			; ����BIOS
		JNC		next			; û����Ļ���ת��next
		ADD		SI,1			; SI+1
		CMP		SI,5			; SI��5�Ƚ�
		JAE		error			; SI >= 5 ,����Ĵ�������5�ˣ���ת��error
		MOV		AH,0x00
		MOV		DL,0x00			; A������
		INT		0x13			; ����������
		JMP		retry
next:
		MOV		AX,ES			; ���ڴ��ַ����0x200
		ADD		AX,0x0020
		MOV		ES,AX			; ADD ES,0x020 ����ǰ����һ��
		ADD		CL,1			; CL+1
		CMP		CL,18			; CL��18�Ƚ�
		JBE		readloop		; CL <= 18 ��ת��readloop
		MOV		CL,1
		ADD		DH,1
		CMP		DH,2
		JB		readloop		; DH < 2 ��ת��readloop
		MOV		DH,0
		ADD		CH,1
		CMP		CH,CYLS
		JB		readloop		; CH < CYLS ��ת��readloop

; �ǂݏI������̂�haribote.sys�����s���I

		MOV		[0x0ff0],CH		; CH ������[0x0ff0]��IPL�����С��IPL���ǂ��܂œǂ񂾂̂�������
		JMP		0xc200          ;������������ʼ�ĵط�

; ��ʾerror
error:
		MOV		SI,msg
putloop:
		MOV		AL,[SI]
		ADD		SI,1			; SI+1
		CMP		AL,0
		JE		fin
		MOV		AH,0x0e			; 
		MOV		BX,15			; 
		INT		0x10			; �����Կ�BIOS
		JMP		putloop
fin:
		HLT						; CPU����
		JMP		fin				
msg:
		DB		0x0a, 0x0a		; 2������
		DB		"load error"
		DB		0x0a			; ����
		DB		0

		RESB	0x7dfe-$		; ���0x00ֱ��0x7dfe

		DB		0x55, 0xaa      ; ���̽��������룬��ʾΪ������
