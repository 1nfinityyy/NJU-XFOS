#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

int A[10];
 
/* 归并函数 */
void merge(int left, int middle, int right)
{
    int i, j, k;
    int len1 = middle - left + 1;
    int len2= right - middle;
    int first[len1], second[len2]; 

    for (i = 0; i < len1; i++) first[i] = A[left + i]; 
    for (j = 0; j < len2; j++) second[j] = A[middle + 1 + j];

    i = 0;
    j = 0;
    k = left;
    while (i < len1 && j < len2) 
    {
        if (first[i] <= second[j]) A[k++] = first[i++];
        else A[k++] = second[j++];
    }
    while (i < len1) A[k++] = first[i++];
    while (j < len2) A[k++] = second[j++];
    
}
 

void Merge_Sort(void* arg) 
{
    int *argu = (int*)arg;
    int left = argu[0];
    int right = argu[1];
 
    if (left < right) 
    {       
        pthread_t pid1;
        pthread_t pid2;
 
        int arg1[2];
        int arg2[2];
 
        int middle;
        middle = (left + (right - 1)) / 2;
        arg1[0] = left;
        arg1[1] = middle;
        arg2[0] = middle + 1;
        arg2[1] = right;
 
        pthread_create(&pid1, NULL, Merge_Sort, arg1);
        pthread_create(&pid2, NULL, Merge_Sort, arg2);
        // Merge_Sort(arg1);
        // Merge_Sort(arg2);
 
        pthread_join(pid1, NULL);
        pthread_join(pid2, NULL);

        merge(left, middle, right);
    }
}
 
int main()
{
    srand((unsigned)time(NULL));
    for(int i=0;i<10;i++)   A[i]=rand()%20;
    int arg[2];
    arg[0] = 0;
    arg[1] =10;
 
    pthread_t pid;
    pthread_create(&pid, NULL, Merge_Sort, arg);
    pthread_join(pid, NULL);
 
    for (int i = 0; i <10; i++)     printf("%d ", A[i]);
}
