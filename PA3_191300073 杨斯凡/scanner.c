#include<stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
void listDir(char *path)
{
    DIR              *pDir ;
    struct dirent    *ent  ;
    char              childpath[300];

    pDir=opendir(path);
    memset(childpath,0,sizeof(childpath));
    while((ent=readdir(pDir))!=NULL)
    {

        if(ent->d_type & DT_DIR)//是目录
        {

            if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0)
                continue;
            printf("path is %s\n",path);
            printf("dname is %s\n",ent->d_name);
            sprintf(childpath,"%s/%s",path,ent->d_name);//路径+文件名
            
            printf("path:%s\n",childpath);//打印了
            if(strcmp(childpath,"//lost+found")!=0)            listDir(childpath);//再次调用

        }
        else
        {
            printf("%s\n",ent->d_name);
        }
    }

}

int main(int argc,char *argv[])
{

    listDir("/");
    return 0;
}