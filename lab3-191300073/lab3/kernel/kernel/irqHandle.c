#include "x86.h"
#include "device.h"

extern TSS tss;
extern ProcessTable pcb[MAX_PCB_NUM];
extern int current;

extern int displayRow;
extern int displayCol;

extern int turn;
extern int inside[4];


void GProtectFaultHandle(struct StackFrame *sf);
void timerHandle(struct StackFrame *sf);
void syscallHandle(struct StackFrame *sf);
void syscallFork(struct StackFrame *sf);
void syscallSleep(struct StackFrame *sf);
void syscallExit(struct StackFrame *sf);
void syscallWrite(struct StackFrame *sf);
void syscallPrint(struct StackFrame *sf);


void irqHandle(struct StackFrame *sf) { // pointer sf = esp
	/* Reassign segment register */
	asm volatile("movw %%ax, %%ds"::"a"(KSEL(SEG_KDATA)));
	/*TODO Save esp to stackTop */
	uint32_t tmpStackTop = pcb[current].stackTop;
	pcb[current].prevStackTop = pcb[current].stackTop;
	pcb[current].stackTop = (uint32_t)sf;

	switch(sf->irq) {
		case -1:
			break;
		case 0xd:
			GProtectFaultHandle(sf);
			break;
		case 0x20:
			timerHandle(sf);
			break;
		case 0x80:
			syscallHandle(sf);
			break;
		default:assert(0);
	}
	/*TODO Recover stackTop */
	pcb[current].stackTop = tmpStackTop;

}

void GProtectFaultHandle(struct StackFrame *sf) {
	assert(0);
	return;
}

void timerHandle(struct StackFrame *sf){
	int has=0;
	for(int i=0;i<MAX_PCB_NUM;i++){
		if(pcb[i].state==STATE_BLOCKED){
			pcb[i].sleepTime-=1;
			if(pcb[i].sleepTime==0){
				pcb[i].state=STATE_RUNNABLE;
				pcb[i].timeCount=0;
			}
		}
	}
	if(pcb[current].state==STATE_RUNNING){
		pcb[current].timeCount+=1;
		if(pcb[current].timeCount<MAX_TIME_COUNT)	return;
		pcb[current].timeCount=0;
		pcb[current].state=STATE_RUNNABLE;
	}
	for(int i=(current+1)%MAX_PCB_NUM;i!=current;i=(i+1)%MAX_PCB_NUM){
		if(pcb[i].state==STATE_RUNNABLE&&i!=0){
			current = i;	
			pcb[current].state = STATE_RUNNING;
			pcb[current].timeCount = 0;
			has=1;
			break;
		}
	}
	if(has==0){//没有处于 RUNNABLE 状态的进程可供切换
	current=0;
	pcb[current].state = STATE_RUNNING;
	}
	asm volatile("movl %0, %%esp"::"m"(pcb[current].stackTop));//将esp指向当前进程 一条指令即可
	pcb[current].stackTop = pcb[current].prevStackTop;
	tss.esp0 = (uint32_t)&(pcb[current].stackTop);

	asm volatile("popl %gs");
	asm volatile("popl %fs");
	asm volatile("popl %es");
	asm volatile("popl %ds");
	asm volatile("popal");
	asm volatile("addl $8, %esp");
	asm volatile("iret");
}

void syscallHandle(struct StackFrame *sf) {
	switch(sf->eax) { // syscall number
		case 0:
			syscallWrite(sf);
			break; // for SYS_WRITE
		/*TODO Add Fork,Sleep... */
		case 1://fork
			syscallFork(sf);
			break;
		case 3:
			syscallSleep(sf);
			break;
		case 4:
			syscallExit(sf);
			break;
		default:assert(0);
	}
}

void syscallWrite(struct StackFrame *sf) {
	switch(sf->ecx) { // file descriptor
		case 0:
			syscallPrint(sf);
			break; // for STD_OUT
		default:break;
	}
}

void syscallFork(struct StackFrame *sf){
	// putChar('F');
	int new=1;
	for(;new<MAX_PCB_NUM;new++){
		if(pcb[new].state==STATE_DEAD)	break;
	}
	if(new==MAX_PCB_NUM){//fork失败
		pcb[current].regs.eax=-1;
	}
	else{
		// enableInterrupt();
		for(int i=0;i<0x100000;i++){
		*(uint8_t *)(i + (new + 1) * 0x100000) = *(uint8_t *)(i + (current + 1) * 0x100000);//拷贝内存
		// if(i%(16*16*16)==0)	asm volatile("int $0x20"); //XXX Testing irqTimer during syscall
		}
		// disableInterrupt();
		for(int i=0;i<MAX_STACK_SIZE;i++)	pcb[new].stack[i]=pcb[current].stack[i];//拷贝栈
		pcb[new].stackTop=(uint32_t)&(pcb[new].regs);
		pcb[new].prevStackTop=(uint32_t)&(pcb[new].stackTop);
		pcb[new].state=STATE_RUNNABLE;
		pcb[new].timeCount=0;
		pcb[new].sleepTime=0;
		pcb[new].pid=new;
		for(int i=0;i<32;i++)	pcb[new].name[i]=pcb[current].name[i];
		pcb[new].regs.edi = pcb[current].regs.edi;
		pcb[new].regs.esi = pcb[current].regs.esi;
		pcb[new].regs.ebp = pcb[current].regs.ebp;
		pcb[new].regs.xxx = pcb[current].regs.xxx;
		pcb[new].regs.ebx = pcb[current].regs.ebx;
		pcb[new].regs.edx = pcb[current].regs.edx;
		pcb[new].regs.ecx = pcb[current].regs.ecx;
		pcb[new].regs.irq = pcb[current].regs.irq;
		pcb[new].regs.error = pcb[current].regs.error;
		pcb[new].regs.eip = pcb[current].regs.eip;
		pcb[new].regs.eflags = pcb[current].regs.eflags;
		pcb[new].regs.esp = pcb[current].regs.esp;
		pcb[new].regs.ss = USEL(2+2*new);
		pcb[new].regs.cs = USEL(1+2*new);
		pcb[new].regs.ds = USEL(2+2*new);
		pcb[new].regs.es = USEL(2+2*new);
		pcb[new].regs.fs = USEL(2+2*new);
		pcb[new].regs.gs = USEL(2+2*new);
		pcb[current].regs.eax=new;
		pcb[new].regs.eax=0;
	}
}

void syscallSleep(struct StackFrame *sf){
	// putChar('S');
	uint32_t time = sf->ecx;
	if(time>0){
	pcb[current].state=STATE_BLOCKED;
	pcb[current].sleepTime=time;
	asm volatile("int $0x20");
	}
}

void syscallExit(struct StackFrame *sf){
	// putChar('E');
	pcb[current].state=STATE_DEAD;
	asm volatile("int $0x20");
}

void syscallPrint(struct StackFrame *sf) {
	/*int t;
	for(t=0;t<MAX_PCB_NUM;t++){
		if(pcb[t].state==STATE_RUNNING)	break;
	}
	inside[t]=1;
	turn=t;
	while((inside[(t+1)%4]==1||inside[(t+2)%4]==1||inside[(t+3)%4]==1)&&turn==t){
		asm volatile("int $0x20");
	}*/
	int sel = sf->ds; //TODO segment selector for user data, need further modification
	char *str = (char*)sf->edx;
	int size = sf->ebx;
	int i = 0;
	int pos = 0;
	char character = 0;
	uint16_t data = 0;
	asm volatile("movw %0, %%es"::"m"(sel));
	for (i = 0; i < size; i++) {
		asm volatile("movb %%es:(%1), %0":"=r"(character):"r"(str+i));
		if(character == '\n') {
			displayRow++;
			displayCol=0;
			if(displayRow==25){
				displayRow=24;
				displayCol=0;
				scrollScreen();
			}
		}
		else {
			data = character | (0x0c << 8);
			pos = (80*displayRow+displayCol)*2;
			asm volatile("movw %0, (%1)"::"r"(data),"r"(pos+0xb8000));
			displayCol++;
			if(displayCol==80){
				displayRow++;
				displayCol=0;
				if(displayRow==25){
					displayRow=24;
					displayCol=0;
					scrollScreen();
				}
			}
		}
		// updateCursor(displayRow, displayCol);
		// asm volatile("int $0x20"); //XXX Testing irqTimer during syscall
		//asm volatile("int $0x20":::"memory"); //XXX Testing irqTimer during syscall
	}
	
	updateCursor(displayRow, displayCol);
	// inside[t]=0;
	//TODO take care of return value
	return;
}

