.code32

.global start	#在这输出hello world
start:
	pushl $13
	pushl $message
	calll displayStr
loop:
	jmp loop

message:
#	.string "Hello, World in Part1.3!\n\0"
	.string "Hello, World!\n\0"

displayStr:
	movl 4(%esp), %ebx
	movl 8(%esp), %ecx
	movl $((80*5+0)*2), %edi
	movb $0x0c, %ah
nextChar:
	movb (%ebx), %al
	movw %ax, %gs:(%edi)
	addl $2, %edi
	incl %ebx
	loopnz nextChar # loopnz decrease ecx by 1
	ret
