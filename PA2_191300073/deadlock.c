#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<semaphore.h>

int count=0;
int file=0;//模拟文件，writer每次加1
sem_t writeblock_1,writeblock_2,mutex;

void *reader()
{
    while(1){
	printf("read file,file=%d\n",file);
    sleep(5);
    }
}

void *writer_1()
{
    while(1){
	sem_wait(&writeblock_1);
    sleep(2);
    sem_wait(&writeblock_2);
	printf("write file\n");
    file++;
	sem_post(&writeblock_1);
    sem_post(&writeblock_2);
    sleep(2);
    }
}

void *writer_2()
{
    while(1){
	sem_wait(&writeblock_2);
    sleep(2);
    sem_wait(&writeblock_1);
	printf("write file\n");
    file++;
	sem_post(&writeblock_2);
    sem_post(&writeblock_1);
    sleep(2);
    }
}


int main()
{
	pthread_t read_1;
    pthread_t write_1;
	pthread_t write_2;

	sem_init(&mutex,0,1);//初始化为1
	sem_init(&writeblock_1,0,1);
    sem_init(&writeblock_2,0,1);

	
    pthread_create(&write_1,NULL,writer_1,NULL);
	pthread_create(&write_2,NULL,writer_2,NULL);
    pthread_create(&read_1,NULL,reader,NULL);

	pthread_join(read_1,NULL);
    pthread_join(write_1,NULL);
	pthread_join(write_2,NULL);

}
