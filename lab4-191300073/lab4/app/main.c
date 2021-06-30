#include "lib.h"
#include "types.h"

int uEntry(void) {
	// For lab4.1
	// Test 'scanf' 
	int dec = 0;
	int hex = 0;
	char str[6];
	char cha = 0;
	int ret = 0;
	while(1){
		printf("Input:\" Test %%c Test %%6s %%d %%x\"\n");
		ret = scanf(" Test %c Test %6s %d %x", &cha, str, &dec, &hex);
		printf("Ret: %d; %c, %s, %d, %x.\n", ret, cha, str, dec, hex);
		printf("Ret is %d\n",ret);
		if (ret == 4)
			break;
	}
	
	// For lab4.2
	// Test 'Semaphore'
	int i = 4;
	ret=0;

	sem_t sem;
	printf("Father Process: Semaphore Initializing.\n");
	ret = sem_init(&sem, 2);
	if (ret == -1) {
		printf("Father Process: Semaphore Initializing Failed.\n");
		exit();
	}

	ret = fork();
	if (ret == 0) {
		while( i != 0) {
			i --;
			printf("Child Process: Semaphore Waiting.\n");
			sem_wait(&sem);
			printf("Child Process: In Critical Area.\n");
		}
		printf("Child Process: Semaphore Destroying.\n");
		sem_destroy(&sem);
		exit();
	}
	else if (ret != -1) {
		while( i != 0) {
			i --;
			printf("Father Process: Sleeping.\n");
			sleep(128);
			printf("Father Process: Semaphore Posting.\n");
			sem_post(&sem);
		}
		printf("Father Process: Semaphore Destroying.\n");
		sem_destroy(&sem);
		exit();
	}
	return 0;

	// For lab4.3
	// TODO: You need to design and test the philosopher problem.
	// Note that you can create your own functions.
	// Requirements are demonstrated in the guide.
	/*int i;
	int ret;
	sem_t Fork[5];
	for(i=0;i<5;i++){
		ret=sem_init(&(Fork[i]), 1);
		if(ret==-1)	printf("Failed\n");
		printf("Successfully init!\n");
	}

	for(i = 0; i < 4; ++i){
		ret = fork();
		if(ret == -1){
			printf("Error!\n");  
			return -1;  
		}
		if(ret == 0){
			break;
		}
	}

	int pid = 0;
	while(1){
		pid = getpid();
		printf("Philosopher %d: think\n", pid);
		sleep(128);  //Between think and eat
		if(pid%2 == 0){
			sem_wait(&Fork[pid]);
			sleep(128);
			sem_wait(&Fork[(pid+1)%5]); 
		} 
		else {
			sem_wait(&Fork[(pid+1)%5]); 
			sleep(128);
			sem_wait(&Fork[pid]);
		}
		printf("Philosopher %d: eat\n", pid);
		sleep(128);  //Between P-V operation
		sem_post(&Fork[pid]); 
		sleep(128);
		sem_post(&Fork[(pid+1)%5]); 
	}
	exit();*/

	// customers
	/*int i;
	int ret;
	sem_t mutex, full, empty;
	int size=5;
	ret=sem_init(&mutex, 1);
	if(ret==-1)	printf("Failed\n");
	printf("Successfully init!\n");
	ret=sem_init(&full, 0);
	if(ret==-1)	printf("Failed\n");
	printf("Successfully init!\n");
	ret=sem_init(&empty, size);
	if(ret==-1)	printf("Failed\n");
	printf("Successfully init!\n");

	for(i = 0; i < 4; i++){
		ret = fork();
		if(ret == -1){
			printf("Error!\n");  
			return -1;  
		}
		if(ret == 0){
			break;
		}
	}
	if(getpid() == 1){ 
		while(1){
			sem_wait(&full);
			sleep(128);
			sem_wait(&mutex);
			printf("Consumer : consume\n");
			sleep(128);
			sem_post(&mutex);
			sleep(128);
			sem_post(&empty);
			
		}
		
	}
	else if(getpid()>1 && getpid()<=5){
		while(1){
			sem_wait(&empty);
			sleep(128);
			sem_wait(&mutex);
			printf("Producer %d: produce\n", getpid());
			sleep(128);
			sem_post(&mutex);
			sleep(128);
			sem_post(&full);
			
		}
	}
	return 0;*/
/*	int ret=0;
	int count=0;
	write(3,0);
	sem_t lock,mutex;
	ret=sem_init(&mutex, 1);
	if(ret==-1)	printf("Failed\n");

	ret=sem_init(&lock, 1);
	if(ret==-1)	printf("Failed\n");

	for(int i = 0; i < 5; i++){ 
		ret = fork();
		if(ret == -1){
			printf("Error!\n");  
			return -1;  
		}
		if(ret == 0){
			break;
		}
	}
	if(getpid() >= 1 && getpid() <= 3){
		while(1){
			sem_wait(&mutex);
			count=read(3);
			count+=1;
			write(3,count);
			if(count == 1){
				sem_wait(&lock);
			}
			sleep(128);
			sem_post(&mutex);
			int id = getpid();
			printf("Reader %d: read, total %d reader\n", id, count);

			sem_wait(&mutex);
			sleep(128);
			count=read(3);
			count-=1;
			write(3,count);
			if(count == 0){
				sem_post(&lock);
				sleep(128);
			}
			sem_post(&mutex);
		}
	}
	else if(getpid() >= 4 && getpid() <= 6){
		while(1){
			sem_wait(&lock);
			int id = getpid();		
			printf("Writer %d: write\n", id);
			sleep(128);
			sem_post(&lock);
		}
	}
	return 0;
	*/
}
