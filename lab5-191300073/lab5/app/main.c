#include "types.h"
#include "utils.h"
#include "lib.h"

union DirEntry {
	uint8_t byte[128];
	struct {
		int32_t inode;
		char name[64];
	};
};

typedef union DirEntry DirEntry;

int ls(char *destFilePath) {
	printf("ls %s\n", destFilePath);
	int i = 0;
	int fd = 0;
	int ret = 0;
	DirEntry *dirEntry = 0;
	uint8_t buffer[512 * 2];
	fd = open(destFilePath, O_READ | O_DIRECTORY);
	if (fd == -1)
		return -1;
	ret = read(fd, buffer, 512 * 2);
	while (ret != -1) {
		// TODO: Complete 'ls'.
		dirEntry = (DirEntry*)buffer;
		for(;i<8;i++){
		if(dirEntry->inode != 0){
			printf("%s ",dirEntry->name);
		}
		dirEntry+=1;
		}
		ret = read(fd,buffer,1024);
	}
	printf("\n");
	ret=close(fd);
	return 0;
}

int cat(char *destFilePath) {
	printf("cat %s\n", destFilePath);
	int fd = 0;
	int ret = 0;
	uint8_t buffer[512 * 2];
	fd = open(destFilePath, O_READ);
	if (fd == -1)
		return -1;
	ret = read(fd, buffer, 512 * 2);
	while (ret != -1) {
		// TODO: COmplete 'cat'
		printf("%s",buffer);
		ret = read(fd, buffer, 512 * 2);
	}
	ret=close(fd);
	printf("\n");
	return 0;
}

int uEntry(void) {
	int fd = 0;
	int i = 0;
	char tmp = 0;
	
	ls("/");
	ls("/boot/");
	ls("/dev/");
	ls("/usr/");

	fd = open("/usr/test", O_WRITE | O_READ | O_CREATE);
	for (; i < 26; i ++) {
		tmp = (char)(i % 26 + 'A');
		write(fd, (uint8_t*)&tmp, 1);
	}

	ls("/usr/");
	cat("/usr/test");
	printf("rm /usr/test\n");

	remove("/usr/test");
	
	ls("/usr/");
	printf("rmdir /usr/\n");
	remove("/usr/");
	
	ls("/");
	printf("create /usr/\n");
	fd = open("/usr/", O_CREATE | O_DIRECTORY);
	close(fd);

	fd = open("/usr/test", O_WRITE | O_READ | O_CREATE);
	for (i=0; i < 26; i ++) {
		tmp = (char)(i % 26 + 'A');
		write(fd, (uint8_t*)&tmp, 1);
	}

	ls("/");
	printf("> Welcome to  Ysf's  Shell\n");
	char str[100];
	char path[100]="/usr";
	while(1){
 		char ch=0;
        int index=0;
        printf("> ");
        scanf("%c", &ch);
        while(ch!='\n')
        {
            printf("%c",ch);   
            str[index]=ch;
            index++;
            scanf("%c", &ch);             
        }
        str[index]=0;
        printf("\n");
        if(stringCmp("ls",str,stringLen("ls"))==0)
        {
            ls(path);
            continue;
        }
        if(stringCmp("cat",str,stringLen("cat"))==0)
        {     
			char truepath[200];
			int i=0;
			for(;path[i]!=0;i++)	truepath[i]=path[i];
			truepath[i]='/';
			i++;
			for(;str[i-1]!=0;i++)		truepath[i]=str[i-1];
            cat(truepath);
            continue;
        }
		if(stringCmp("cat",str,stringLen("rm"))==0)
        {     
			char truepath[200];
			int i=0;
			for(;path[i]!=0;i++)	truepath[i]=path[i];
			truepath[i]='/';
			i++;
			for(;str[i-1]!=0;i++)		truepath[i]=str[i-1];
            remove(truepath);
            continue;
        }
        if(stringCmp("quit",str,stringLen("quit"))==0)
        {
            break;
        }
        else
        {
            printf("unkown command\n");
        }
	}



	exit();
	return 0;
}
