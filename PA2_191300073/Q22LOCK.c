#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>

int file=0; //模拟文件
pthread_rwlock_t    lock;

void *reader()
{
    while(1){
    if(	pthread_rwlock_tryrdlock(&lock) != 0) //申请失败
		{
			printf("Reader failed to get the lock\n");
			continue;  //继续申请锁
 
		}
	else
		{
			printf("read:%d\n",file);
			sleep(1);
			pthread_rwlock_unlock(&lock); //释放锁
			
			printf("read over\n");
			
            break;
		}
    }
}

void *writer()
{
    while(1){
	if(pthread_rwlock_trywrlock(&lock) != 0)
		{
			// printf("Writer failed to get the lock\n");
			continue;
		}
		else
		{
			file++;
            printf("Write the file!\n");
			pthread_rwlock_unlock(&lock);
			printf("write over\n");
            break;
		}
    }
}

int main()
{
    // while(1){
	// pthread_t read;
	// pthread_t write;
    pthread_rwlock_init(&lock,NULL);


	// pthread_create(&read,NULL,reader,NULL);
	// pthread_create(&write,NULL,writer,NULL);

	// pthread_join(read,NULL);
	// pthread_join(write,NULL);

    // pthread_rwlock_destroy(&lock);

    // }
	//TEST1
	pthread_t write;
	pthread_t read[2];
	pthread_create(&read[0],NULL,reader,NULL);
	pthread_create(&write,NULL,writer,NULL);
	pthread_create(&read[1],NULL,reader,NULL);
	
	pthread_join(read[0],NULL);
	pthread_join(write,NULL);
	pthread_join(read[1],NULL);


}
