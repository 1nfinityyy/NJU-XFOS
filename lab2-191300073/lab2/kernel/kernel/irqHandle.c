#include "x86.h"
#include "device.h"

extern int displayRow;
extern int displayCol;

extern uint32_t keyBuffer[MAX_KEYBUFFER_SIZE];
extern int bufferHead;
extern int bufferTail;


void GProtectFaultHandle(struct TrapFrame *tf);

void KeyboardHandle(struct TrapFrame *tf);

void syscallHandle(struct TrapFrame *tf);
void syscallWrite(struct TrapFrame *tf);
void syscallPrint(struct TrapFrame *tf);
void syscallRead(struct TrapFrame *tf);
void syscallGetChar(struct TrapFrame *tf);
void syscallGetStr(struct TrapFrame *tf);


void irqHandle(struct TrapFrame *tf) { // pointer tf = esp
	/*
	 * 中断处理程序
	 */
	/* Reassign segment register */
	asm volatile("movw %%ax, %%ds"::"a"(KSEL(SEG_KDATA)));
	//asm volatile("movw %%ax, %%es"::"a"(KSEL(SEG_KDATA)));
	//asm volatile("movw %%ax, %%fs"::"a"(KSEL(SEG_KDATA)));
	//asm volatile("movw %%ax, %%gs"::"a"(KSEL(SEG_KDATA)));
	switch(tf->irq) {
		// TODO: 填好中断处理程序的调用
		case 0xd:GProtectFaultHandle(tf);break;
		case 0x21: KeyboardHandle(tf);break;
		case 0x80: syscallHandle(tf);break;
		default:break;
	}
}

void GProtectFaultHandle(struct TrapFrame *tf){
	assert(0);
	return;
}

void KeyboardHandle(struct TrapFrame *tf){
	uint32_t code = getKeyCode();
	if(code == 0xe){ // 退格符
		// TODO: 要求只能退格用户键盘输入的字符串，且最多退到当行行首
		if(displayCol==0)	return;
		displayCol--;
		uint16_t data = 0| (0x0c << 8);
		int pos = (80*displayRow+displayCol)*2;
		asm volatile("movw %0, (%1)"::"r"(data),"r"(pos+0xb8000));
	}else if(code == 0x1c){ // 回车符
		// TODO: 处理回车情况
			displayCol=0;
			displayRow++;
			if(displayRow==25){
				displayRow--;
				scrollScreen();
			}
			
	}else if(code < 0x81&&code>0){ // 正常字符
		// TODO: 注意输入的大小写的实现、不可打印字符的处理
		uint16_t data = getChar(code)|(0x0c<<8);
		int pos = (80*displayRow+displayCol)*2;
		asm volatile("movw %0, (%1)"::"r"(data),"r"(pos+0xb8000));
		displayCol++;
		if(displayCol==80){//一行满了
				displayCol=0;
				displayRow++;
				if(displayRow==25){
					displayRow--;
					scrollScreen();
				}
				
			}
		putChar(data);
	}
	else if(code == 0xf||code == 0xf+0x80){//实现tab功能
		
		uint16_t data = getChar(code)|(0x0c<<8);
		int pos = (80*displayRow+displayCol)*2;
		asm volatile("movw %0, (%1)"::"r"(data),"r"(pos+0xb8000));
		displayCol++;
		if(displayCol<80){
		pos = (80*displayRow+displayCol)*2;
		asm volatile("movw %0, (%1)"::"r"(data),"r"(pos+0xb8000));
		if(displayCol>=80){//一行满了
			displayCol=0;
			displayRow++;
			if(displayRow==25){
				displayRow--;
				scrollScreen();
			}
		}
		}
		else if(displayCol>=80){//一行满了
				displayCol=0;
				displayRow++;
				if(displayRow==25){
					displayRow--;
					scrollScreen();
				}
		}
	}
	updateCursor(displayRow, displayCol);
}

void syscallHandle(struct TrapFrame *tf) {
	switch(tf->eax) { // syscall number
		case 0:
			syscallWrite(tf);
			break; // for SYS_WRITE
		case 1:
			syscallRead(tf);
			break; // for SYS_READ
		default:break;
	}
}

void syscallWrite(struct TrapFrame *tf) {
	switch(tf->ecx) { // file descriptor
		case 0:
			syscallPrint(tf);
			break; // for STD_OUT
		default:break;
	}
}

void syscallPrint(struct TrapFrame *tf) {
	int sel =USEL(SEG_UDATA); //TODO: segment selector for user data, need further modification
	char *str = (char*)tf->edx;
	int size = tf->ebx;
	int i = 0;
	int pos = 0;
	char character = 0;
	uint16_t data = 0;
	asm volatile("movw %0, %%es"::"m"(sel));
	for (i = 0; i < size; i++) {
		asm volatile("movb %%es:(%1), %0":"=r"(character):"r"(str+i));
		// TODO: 完成光标的维护和打印到显存,每次留下最后一行，参考Vscode
		if(character=='\n'){
			displayCol=0;
			displayRow++;
			if(displayRow==25){
				displayRow--;
				scrollScreen();
			}
			
		}
		else{
		//打印字符 仿照Vga
		data = character| (0x0c << 8);
		pos = (80*displayRow+displayCol)*2;
		asm volatile("movw %0, (%1)"::"r"(data),"r"(pos+0xb8000));
		displayCol++;
		if(displayCol==80){//一行满了
				displayCol=0;
				displayRow++;
				if(displayRow==25){
					displayRow--;
					scrollScreen();
				}
				
			}
		}
	}
	
	updateCursor(displayRow, displayCol);
}

void syscallRead(struct TrapFrame *tf){
	switch(tf->ecx){ //file descriptor
		case 0:
			syscallGetChar(tf);
			break; // for STD_IN
		case 1:
			syscallGetStr(tf);
			break; // for STD_STR
		default:break;
	}
}

void syscallGetChar(struct TrapFrame *tf){
	// TODO: 自由实现
	int sel = USEL(SEG_UDATA);
	uint16_t press[10000];
	int i=0;
	uint32_t code=0;
	uint32_t temp=0;
	int start=displayCol;
	int pos;
	uint16_t data;
	code=0;
	asm volatile("movw %0, %%es"::"m"(sel));
	while(code!=0x1c+0x80){
		code=getKeyCode();
		if(code<0x81&&code!=0&&code!=0x1c&&code!=0xe){
			temp=code;
			
			data=getChar(temp)|(0x0c<<8);
			pos = (80*displayRow+displayCol)*2;
			asm volatile("movw %0, (%1)"::"r"(data),"r"(pos+0xb8000));
			press[i]=data;
			i++;
			displayCol++;
			if(displayCol==80){//一行满了
				displayCol=0;
				displayRow++;
				if(displayRow==25){
					displayRow--;
					scrollScreen();
				}
			}
			updateCursor(displayRow, displayCol);
			while(getKeyCode()==temp);	
		}
		else if(code==0xe){
		temp=code;
		while(getKeyCode()==temp);	
		if(displayCol>start){
		displayCol--;
		uint16_t data = 0| (0x0c << 8);
		int pos = (80*displayRow+displayCol)*2;
		asm volatile("movw %0, (%1)"::"r"(data),"r"(pos+0xb8000));
		updateCursor(displayRow, displayCol);
		}
		if(i>0)	i--;
		}
	}
	displayCol=0;//回车
	displayRow++;
	if(displayRow==25){
		displayRow--;
		scrollScreen();
	}
	if(i>0)	i--;
	tf->eax=press[i];
}

void syscallGetStr(struct TrapFrame *tf){
	// TODO: 自由实现
	int sel = USEL(SEG_UDATA);
	char *str = (char*)tf->edx;
	int size = tf->ebx;
	int i = 0;
	uint32_t code=0;
	uint32_t temp=0;
	uint16_t character;
	int start=displayCol;
	i=0;
	asm volatile("movw %0, %%es"::"m"(sel));
	while(i<size-1) {
	while (code!=temp+0x80) {
		if(code<0x81&&code!=0){
			temp=code;
			}
		code=getKeyCode();
	}
		if(temp==0x1c)	break;
		if(temp<0x81&&temp!=0&&temp!=0xe){
			character=(getChar(temp)|(0x0c<<8));
			asm volatile("movb %0, %%es:(%1)"::"r"(character),"r"(str+i));
			int pos = (80*displayRow+displayCol)*2;
			asm volatile("movw %0, (%1)"::"r"(character),"r"(pos+0xb8000));
			displayCol++;
			if(displayCol==80){//一行满了
				displayCol=0;
				displayRow++;
				if(displayRow==25){
					displayRow--;
					scrollScreen();
				}
			}
			updateCursor(displayRow, displayCol);
			i++;
		}
		if(temp==0xe){
		if(displayCol>start){
		displayCol--;
		uint16_t data = 0| (0x0c << 8);
		int pos = (80*displayRow+displayCol)*2;
		asm volatile("movw %0, (%1)"::"r"(data),"r"(pos+0xb8000));
		character=(getChar(0)|(0x0c<<8));
		i--;
		asm volatile("movb %0, %%es:(%1)"::"r"(character),"r"(str+i));
		}
		updateCursor(displayRow, displayCol);
		}
		code=0;
		temp=0;
	}
	character='\0';
	displayCol=0;
	displayRow++;
	if(displayRow==25){
		displayRow--;
		scrollScreen();
	}
	asm volatile("movb %0, %%es:(%1)"::"r"(character),"r"(str+i));
}