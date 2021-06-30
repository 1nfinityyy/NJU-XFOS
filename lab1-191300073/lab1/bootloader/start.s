/* Real Mode Hello World */
/*.code16

.global start
start:
	movw %cs, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss
	movw $0x7d00, %ax
	movw %ax, %sp # setting stack pointer to 0x7d00
	pushw $37 #字符串长度
	pushw $message
	callw displayHello

message:
	.string "Hello, World in real mode in Part1.1!\n\0"

displayHello:
	pushw %bp
	movw 4(%esp), %bp	#要加4 不然就会乱码 可能是返回错误
	movw 6(%esp),%cx #16位下int 2个字节 char 1个  必须改成esp 不然报错 stackoverflow上 解释原因是会被覆盖
	movw $0x1320,%ax #光标位置
	movw $0x50,%dx #字符位置 放到了第二行 50是试出来的555
	movw $0x2,%bx #前景设置为绿色 背景黑色	颜色见https://blog.csdn.net/lindorx/article/details/83957903
	int $0x10
	popw %bp
	ret
loop:
	jmp loop*/




/* Protected Mode Hello World */
/*.code16

.global start
start:
	movw %cs, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss
	cli	#关中断
	inb $0x92, %al #启动A20
	orb $0x02, %al
	outb %al, $0x92
	lgdt gdtDesc #加载GDTR
	movl %cr0,%eax #开启保护模式
	movl $0x1,%eax
	movl %eax,%cr0 #CR0的0位设置为1
	ljmp $0x08, $start32 #切到保护模式

.code32
start32:
	movw $0x10, %ax # setting data segment selector
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %ss
	movw $0x18, %ax # setting graphics data segment selector
	movw %ax, %gs
	pushl $42 #字符串长度
	pushl $message
	calll displayHello
	
	

message:
	.string "Hello, World in Protected mode in Part1.2!\n\0"


displayHello:
	movl 4(%esp), %ebx
	movl 8(%esp), %ecx
	movl $((80*6+0)*2), %edi 				#每行80个字符 6行即可 改到中央
	movb $0x0f, %ah 						#白色字体
nextchar:
	movb (%ebx), %al
	movw %ax, %gs:(%edi)
	addl $2, %edi
	incl %ebx
	loopnz nextchar 						# loopnz decrease ecx by 1
	ret

loop32:
	jmp loop32

.p2align 2
gdt: 
	#GDT definition here	汇编存储顺序是反的 https://bbs.csdn.net/topics/80507125

	.word	0x0,0x0
	.byte	0x0,0x0,0x0,0x0

	.word	0xffff,0x0 	#代码段描述符
	.byte	0x0,0x9a,0xcf,0x0

	.word	0xffff,0x0	#数据段描述符
	.byte	0x0,0x92,0xcf,0x0

	.word	0xffff,0x8000 
	.byte	0x0b,0x92,0xcf,0

gdtDesc: 
	#gdtDesc definition here
	.word (gdtDesc - gdt -1)
	.long gdt	
*/

/* Protected Mode Loading Hello World APP */
.code16

.global start
start:
	movw %cs, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss
	#TODO: Protected Mode Here	上面初始化
	cli	#关中断
	inb $0x92, %al #启动A20
	orb $0x02, %al
	outb %al, $0x92
	lgdt gdtDesc #加载GDTR
	movl %cr0,%eax #开启保护模式
	movl $0x1,%eax
	movl %eax,%cr0 #CR0的0位设置为1
	ljmp $0x08, $start32 #切到保护模式


.code32
start32:
	movw $0x10, %ax # setting data segment selector
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %ss
	movw $0x18, %ax # setting graphics data segment selector
	movw %ax, %gs
	
	movl $0x8000, %eax # setting esp
	movl %eax, %esp
	jmp bootMain # jump to bootMain in boot.c

.p2align 2


gdt: 
	#GDT definition here	汇编存储顺序是反的 https://bbs.csdn.net/topics/80507125
#	.word 0x0,0x0,0x0,0x0		#第一项为NULL

#	.word 0xffff,0x0	#代码段 00cf9a00 0000ffff
#	.word 0x9a00,0x00cf

#	.word 0xffff,0x0		#数据段
#	.word 0x9200,0x00cf
	
#	.word 0xffff,0x8000	#图像段
#	.word 0x920b,0x00cf

	.word	0x0,0x0
	.byte	0x0,0x0,0x0,0x0

	.word	0xffff,0x0 	#代码段描述符
	.byte	0x0,0x9a,0xcf,0x0

	.word	0xffff,0x0	#数据段描述符
	.byte	0x0,0x92,0xcf,0x0

	.word	0xffff,0x8000 
	.byte	0x0b,0x92,0xcf,0

gdtDesc: 
	#gdtDesc definition here
	.word (gdtDesc - gdt -1)
	.long gdt
