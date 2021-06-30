#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<semaphore.h>

int count=0;
int file=0;//模拟文件，writer每次加1
sem_t writeblock,mutex;

void *reader()
{


	sem_wait(&mutex);//P

	count++;

	if(count==1)	sem_wait(&writeblock);
	sem_post(&mutex);

	printf("read file,file=%d\n",file);

	sleep(1);
	printf("read over\n");
	sem_wait(&mutex);
	count--;
	if(count==0)	sem_post(&writeblock);
	sem_post(&mutex);
	
}

void *writer()
{

	sem_wait(&writeblock);
	printf("write file\n");
    file++;
	sem_post(&writeblock);
	printf("write over\n");
}

int main()
{
	//读写操作
    // while(1){
	// pthread_t read;
	// pthread_t write;

	// sem_init(&mutex,0,1);//初始化为1
	// sem_init(&writeblock,0,1);

	// pthread_create(&read,NULL,reader,NULL);
	// pthread_create(&write,NULL,writer,NULL);

	// pthread_join(read,NULL);
	// pthread_join(write,NULL);

    // }

	//TEST1
	sem_init(&mutex,0,1);//初始化为1
	sem_init(&writeblock,0,1);
	pthread_t write;
	pthread_t read[2];
	pthread_create(&read[0],NULL,reader,NULL);
	pthread_create(&write,NULL,writer,NULL);
	pthread_create(&read[1],NULL,reader,NULL);
	
	pthread_join(read[0],NULL);
	pthread_join(write,NULL);
	pthread_join(read[1],NULL);
}
