#include<iostream>
#include<unistd.h>
#include<pthread.h>
#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<semaphore.h>
#include <sstream>

using namespace std;

class CircleBuffer
{
	public:
		void put(int value);
		void get();
		CircleBuffer(int k);//构造函数
		int getsize();
		static void * thread_run1(void* tmp)
        {
			
            CircleBuffer *p = (CircleBuffer *)tmp;
            p->put(5);
        }
		static void * thread_run2(void* tmp)
        {
			
            CircleBuffer *p = (CircleBuffer *)tmp;
            p->get();
        }
	private:
		int size;
		int in;
		int out;
		int *arr;
		sem_t empty,full,mutex;
};

CircleBuffer::CircleBuffer(int k)
{
	size=k;
	in=0;
	out=0;
	arr=new int[size];
	sem_init(&mutex,0,1);
	sem_init(&empty,0,k);
	sem_init(&full,0,0);
}
int CircleBuffer::getsize(){
	return size;
}
void CircleBuffer::put(int value){
	sem_wait(&empty);
	sem_wait(&mutex);
	arr[in]=value;
	printf("index= %d put %d\n",in,value);
	in=(in+1)%size;
	// sleep(1);
	sem_post(&mutex);
	sem_post(&full);
}
void CircleBuffer::get(){
	int ret;
	sem_wait(&full);
	sem_wait(&mutex);
	ret=arr[out];
	printf("index= %d get %d\n",out,ret);
	out=(out+1)%size;
	// sleep(1);
	sem_post(&mutex);
	sem_post(&empty);
}


CircleBuffer * circlebuffer = new CircleBuffer(10);

int main(){
	
	 
	cout<<circlebuffer->getsize()<<"\n";

	for(int i=0;i<10;i++){
		circlebuffer->put(3-i);
		if(i==5) circlebuffer->get();
	}
	pthread_t producer[2];
	pthread_t customer[2];


	pthread_create(&customer[0],NULL,CircleBuffer::thread_run2,circlebuffer);

	pthread_create(&producer[0],NULL,CircleBuffer::thread_run1,circlebuffer);
	
	pthread_create(&customer[1],NULL,CircleBuffer::thread_run2,circlebuffer);

	pthread_create(&producer[1],NULL,CircleBuffer::thread_run1,circlebuffer);
	

	pthread_join(customer[0],NULL);
	pthread_join(producer[0],NULL);
	pthread_join(customer[1],NULL);
	pthread_join(producer[1],NULL);

	
}

