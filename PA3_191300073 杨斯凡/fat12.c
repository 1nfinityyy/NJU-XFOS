#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <time.h>

#define BLOCKSIZE 1024
#define SIZE 1474560
#define END 0xffff
#define FREE 0
#define ROOTBLOCKNUM 2
#define MAXOPENFILE 10
#define MAXTEXT 10000  
typedef struct FCB  
{  
    char filename[8];  
    char exname[3];  
    unsigned char attribute;  
    unsigned short time;    
    unsigned short date;   
    unsigned short first; 
    unsigned long length;  
    char free;   
}fcb;  

typedef struct FAT  
{  
    unsigned short id;    
}fat;  

typedef struct USEROPEN  
{  
    char filename[8];   
    char exname[3];   
    unsigned char attribute;  
    unsigned short time;    
    unsigned short date; 
    unsigned short first;  
    unsigned long length;  
    char free;  
    unsigned short dirno;  
    int diroff; 
    char dir[80]; 
    int father;  
    int count; 
    char fcbstate; 
    char topenfile;  
}useropen;  
  
typedef struct BLOCK0  
{  
    char magic[10];  
    char information[200]; 
    unsigned short root;  
    unsigned char *startblock;

}block0;  

unsigned char *myvhard; 
useropen openfilelist[MAXOPENFILE];    
int curdir;  
char currentdir[80]; 
unsigned char* startp;  
char myfilename[] = "img.bin"; 

void startsys(); 
void my_format();    
void my_create (char *filename);   
int my_open(char *filename);  
int my_write(int fd, char *filename); 
int do_write(int fd, char *text, int len);    
int my_read (int fd, int len, char *newname);  
int do_read (int fd, int len,char *text);    
unsigned short findblock();   
int findopenfile(); 

void startsys()  
{  
    FILE *fp;
    unsigned char buf[SIZE];  
    fcb *root;  
    int i;  
    myvhard = (unsigned char *)malloc(SIZE); 
    memset(myvhard, 0, SIZE);  
    if((fp = fopen(myfilename, "r")) != NULL)  
    {  
        fread(buf, SIZE, 1, fp); 
        fclose(fp); 
        if(strcmp(((block0 *)buf)->magic, "10101010"))
        {  
            my_format();
        }  
        else  
        {  
            for(i = 0; i < SIZE; i++)  
                myvhard[i] = buf[i];  
        }  
    }  
    else  
    {  
        my_format();  
    }  

    root = (fcb *)(myvhard + 5 * BLOCKSIZE);  
    strcpy(openfilelist[0].filename, root->filename);  
    strcpy(openfilelist[0].exname, root->exname);  
    openfilelist[0].attribute = root->attribute;  
    openfilelist[0].time = root->time;  
    openfilelist[0].date = root->date;  
    openfilelist[0].first = root->first;  
    openfilelist[0].length = root->length;  
    openfilelist[0].free = root->free;  
    openfilelist[0].dirno = 5;  
    openfilelist[0].diroff = 0;  
    strcpy(openfilelist[0].dir, "\\demo\\");  
    openfilelist[0].father = 0;  
    openfilelist[0].count = 0;  
    openfilelist[0].fcbstate = 0;  
    openfilelist[0].topenfile = 1;  
    for(i = 1; i < MAXOPENFILE; i++)  
        openfilelist[i].topenfile = 0;  
    curdir = 0;  
    strcpy(currentdir, "\\demo\\");  
    startp = ((block0 *)myvhard)->startblock;  
}  

void my_format()  
{  
    FILE *fp;  
    fat *fat1, *fat2;  
    block0 *blk0;  
    time_t now;  
    struct tm *nowtime;  
    fcb *root;  
    int i;  
    blk0 = (block0 *)myvhard;  
    fat1 = (fat *)(myvhard + BLOCKSIZE);  
    fat2 = (fat *)(myvhard + 3 * BLOCKSIZE);  
    root = (fcb *)(myvhard + 5 * BLOCKSIZE);  
    strcpy(blk0->magic, "10101010");  
    strcpy(blk0->information, "My FileSystem Ver 1.0 \n");  
    blk0->root = 5;  
    blk0->startblock = (unsigned char *)root;  
    for(i = 0; i < 5; i++)  
    {  
        fat1->id = END;  
        fat2->id = END;  
        fat1++;  
        fat2++;  
    }  
    fat1->id = 6;  
    fat2->id = 6;  
    fat1++;  
    fat2++;  
    fat1->id = END;  
    fat2->id = END;  
    fat1++;  
    fat2++;  
    for(i = 7; i < SIZE / BLOCKSIZE; i++)  
    {  
        fat1->id = FREE;  
        fat2->id = FREE;  
        fat1++;  
        fat2++;  
    }  
    now = time(NULL);  
    nowtime = localtime(&now);  
    strcpy(root->filename, ".");  
    strcpy(root->exname, "");  
    root->attribute = 0x28;  
    root->time = nowtime->tm_hour * 2048 + nowtime->tm_min * 32 + nowtime->tm_sec / 2;  
    root->date = (nowtime->tm_year - 80) * 512 + (nowtime->tm_mon + 1) * 32 + nowtime->tm_mday;  
    root->first = 5;  
    root->length = 2 * sizeof(fcb);  
    root->free = 1;  
    root++;  
    now = time(NULL);  
    nowtime = localtime(&now);  
    strcpy(root->filename, "..");  
    strcpy(root->exname, "");  
    root->attribute = 0x28;  
    root->time = nowtime->tm_hour * 2048 + nowtime->tm_min * 32 + nowtime->tm_sec / 2;  
    root->date = (nowtime->tm_year - 80) * 512 + (nowtime->tm_mon + 1) * 32 + nowtime->tm_mday;  
    root->first = 5;  
    root->length = 2 * sizeof(fcb);  
    root->free = 1;  
    fp = fopen(myfilename, "w");  
    fwrite(myvhard, SIZE, 1, fp);  
    fclose(fp);  
}

void my_create(char *filename)  
{  
    fcb *fcbptr;  
    fat *fat1, *fat2;  
    char *fname, *exname, text[MAXTEXT];  
    unsigned short blkno;  
    int rbn, i;  
    time_t now;  
    struct tm *nowtime;  
    fat1 = (fat *)(myvhard + BLOCKSIZE);  
    fat2 = (fat *)(myvhard + BLOCKSIZE);  
    fname = strtok(filename, ".");  
    exname = strtok(NULL, ".");  
    if(strcmp(fname, "") == 0)  
    {  
        printf("Error");  
        return;  
    }  
    if(!exname)  
    {  
        printf("Error");  
        return;  
    }  
    openfilelist[curdir].count = 0;  
    rbn = do_read(curdir, openfilelist[curdir].length, text);  
    fcbptr = (fcb *)text;  
    for(i = 0; i < rbn / sizeof(fcb); i++)  
    {  
        if(strcmp(fcbptr->filename, fname) == 0 && strcmp(fcbptr->exname, exname) == 0)  
        {  
            printf("Error\n");  
            return;  
        }  
        fcbptr++;  
    }  
    fcbptr = (fcb *)text;  
    for(i = 0; i < rbn / sizeof(fcb); i++)  
    {  
        if(fcbptr->free == 0)  
            break;  
        fcbptr++;  
    }  
    blkno = findblock();  
    if(blkno == -1)  
        return;  
    (fat1 + blkno)->id = END;  
    (fat2 + blkno)->id = END;  
    
    now = time(NULL);  
    nowtime = localtime(&now);  
    strcpy(fcbptr->filename, fname);  
    strcpy(fcbptr->exname, exname);  
    fcbptr->attribute = 0x00;  
    fcbptr->time = nowtime->tm_hour * 2048 + nowtime->tm_min * 32 + nowtime->tm_sec / 2;  
    fcbptr->date = (nowtime->tm_year - 80) * 512 + (nowtime->tm_mon + 1) * 32 + nowtime->tm_mday;  
    fcbptr->first = blkno;  
    fcbptr->length = 0;  
    fcbptr->free = 1;  
    openfilelist[curdir].count = i * sizeof(fcb);  
    do_write(curdir, (char *)fcbptr, sizeof(fcb));  
    fcbptr = (fcb *)text;  
    fcbptr->length = openfilelist[curdir].length;  
    openfilelist[curdir].count = 0;  
    do_write(curdir, (char *)fcbptr, sizeof(fcb));  
    openfilelist[curdir].fcbstate = 1; 
    
}  

int my_open(char *filename)  
{  
    fcb *fcbptr;  
    char *fname, exname[4], *str, text[MAXTEXT];  
    int rbn, fd, i;  
    fname = strtok(filename, ".");  
    str = strtok(NULL, ".");  
    if(str)  
        strcpy(exname, str);  
    else  
        strcpy(exname, "");  
    for(i = 0; i < MAXOPENFILE; i++)  
    {  
        if(strcmp(openfilelist[i].filename, fname) == 0 && strcmp(openfilelist[i].exname, exname) == 0 && i != curdir)  
        {  
            printf("Error,the file is already open.\n");  
            return -1;  
        }  
    }  
    exname[3]=0;
    openfilelist[curdir].count = 0;  
    rbn = do_read(curdir, openfilelist[curdir].length, text);  
    fcbptr = (fcb *)text;  
    for(i = 0; i < rbn / sizeof(fcb); i++)  
    {  
        
        if(strcmp(fcbptr->filename, fname) == 0 && strcmp(fcbptr->exname, exname) == 0)  
            break;  
        fcbptr++;  
    }  
    if(i == rbn / sizeof(fcb))  
    {  
        printf("Error,the file is not exist.\n");  
        return -1;  
    }  
    fd = findopenfile();  
    if(fd == -1)  
        return -1;  
    strcpy(openfilelist[fd].filename, fcbptr->filename);  
    strcpy(openfilelist[fd].exname, fcbptr->exname);  
    openfilelist[fd].attribute = fcbptr->attribute;  
    openfilelist[fd].time = fcbptr->time;  
    openfilelist[fd].date = fcbptr->date;  
    openfilelist[fd].first = fcbptr->first;  
    openfilelist[fd].length = fcbptr->length;  
    openfilelist[fd].free = fcbptr->free;  
    openfilelist[fd].dirno = openfilelist[curdir].first;  
    openfilelist[fd].diroff = i;  
    strcpy(openfilelist[fd].dir, openfilelist[curdir].dir);  
    strcat(openfilelist[fd].dir, filename);  
    if(fcbptr->attribute & 0x20)  
        strcat(openfilelist[fd].dir, "\\");  
    openfilelist[fd].father = curdir;  
    openfilelist[fd].count = 0;  
    openfilelist[fd].fcbstate = 0;  
    openfilelist[fd].topenfile = 1;  
    return fd;  
}  

int my_write(int fd, char *filenname)  
{  
    fat *fat1, *fat2, *fatptr1, *fatptr2;  
    int  len, ll, tmp;  
    char text[MAXTEXT];
    FILE *fp; 
	if((fp=fopen(filenname,"r"))==NULL) {
		printf("cannot open file/n");
	} 
    int length=0;
	while(!feof(fp)) {
		if(fgets(text+length,128,fp)!=NULL)
        length=strlen(text);
	}
	fclose(fp);
    printf("FIle is : \n%s",text);
    unsigned short blkno;  
    fat1 = (fat *)(myvhard + BLOCKSIZE);  
    fat2 = (fat *)(myvhard + 3 * BLOCKSIZE);  
    if(fd < 0 || fd >= MAXOPENFILE)  
    {  
        printf("The file is not exist!\n");  
        return -1;  
    }  
 
            blkno = openfilelist[fd].first;  
            fatptr1 = fat1 + blkno;  
            fatptr2 = fat2 + blkno;  
            blkno = fatptr1->id;  
            fatptr1->id = END;  
            fatptr2->id = END;  
            while(blkno != END)  
            {  
                fatptr1 = fat1 + blkno;  
                fatptr2 = fat2 + blkno;  
                blkno = fatptr1->id;  
                fatptr1->id = FREE;  
                fatptr2->id = FREE;  
            }  
            openfilelist[fd].count = 0;  
            openfilelist[fd].length = 0;  
    
    ll = 0;  
    len = strlen(text);  
    text[len++] = '\n';  
    text[len] = '\0';  
    tmp = do_write(fd, text, len);  
    if(tmp != -1)  
            ll += tmp;   
    return ll;  
}  
  
int do_write(int fd, char *text, int len)  
{  
    fat *fat1, *fat2, *fatptr1, *fatptr2;  
    unsigned char *buf, *blkptr;  
    unsigned short blkno, blkoff;  
    int i, ll;  
    fat1 = (fat *)(myvhard + BLOCKSIZE);  
    fat2 = (fat *)(myvhard + 3 * BLOCKSIZE);  
    buf = (unsigned char *)malloc(BLOCKSIZE);  
    if(buf == NULL)  
    {  
        printf("malloc failed!\n");  
        return -1;  
    }  
    blkno = openfilelist[fd].first;  
    blkoff = openfilelist[fd].count;  
    fatptr1 = fat1 + blkno;  
    fatptr2 = fat2 + blkno;  
    while(blkoff >= BLOCKSIZE)  
    {  
        blkno = fatptr1->id;  
        if(blkno == END)  
        {  
            blkno = findblock();  
            if(blkno == -1)  
            {  
                free(buf);  
                return -1;  
            }  
            fatptr1->id = blkno;  
            fatptr2->id = blkno;  
            fatptr1 = fat1 + blkno;  
            fatptr2 = fat2 + blkno;  
            fatptr1->id = END;  
            fatptr2->id = END;  
        }  
        else  
        {  
            fatptr1 = fat1 + blkno;  
            fatptr2 = fat2 + blkno;  
        }  
        blkoff = blkoff - BLOCKSIZE;  
    }  
  
    ll = 0;  
    while(ll < len)  
    {  
        blkptr = (unsigned char *)(myvhard + blkno * BLOCKSIZE);  
        for(i = 0; i < BLOCKSIZE; i++)  
            buf[i] = blkptr[i];  
        for(;blkoff < BLOCKSIZE; blkoff++)  
        {  
            buf[blkoff] = text[ll++];  
            openfilelist[fd].count++;  
            if(ll == len)  
                break;  
        }  
        for(i = 0; i < BLOCKSIZE; i++)  
            blkptr[i] = buf[i];  
        if(ll < len)  
        {  
            blkno = fatptr1->id;  
            if(blkno == END)  
            {  
                blkno = findblock();  
                if(blkno == -1)  
                    break;  
                fatptr1->id = blkno;  
                fatptr2->id = blkno;  
                fatptr1 = fat1 + blkno;  
                fatptr2 = fat2 + blkno;  
                fatptr1->id = END;  
                fatptr2->id = END;  
            }  
            else  
            {  
                fatptr1 = fat1 + blkno;  
                fatptr2 = fat2 + blkno;  
            }  
            blkoff = 0;  
            }  
    }  
    if(openfilelist[fd].count > openfilelist[fd].length)  
        openfilelist[fd].length = openfilelist[fd].count;  
    openfilelist[fd].fcbstate = 1;  
    free(buf);  
    return ll;  
}  
int my_read(int fd, int len,char *newname)  
{  
    char text[MAXTEXT];  
    int ll;  
    if(fd < 0 || fd >= MAXOPENFILE)  
    {  
        return -1;  
    }  
    openfilelist[fd].count = 0;  
    
    ll = do_read(fd, len, text);  
    if(ll != -1){
        FILE *file = fopen(newname, "wb+");
        if(file == NULL)
        {
            printf("open error!\n");
            return 0;
        }
        fwrite(text, strlen(text), 1, file);
        fclose(file);
    }
    return ll;  
}  
int do_read(int fd, int len, char *text)  
{  
    fat *fat1, *fatptr;  
    unsigned char *buf, *blkptr;  
    unsigned short blkno, blkoff;  
    int i, ll;  
    fat1 = (fat *)(myvhard + BLOCKSIZE);  
    buf = (unsigned char *)malloc(BLOCKSIZE);  
    if(buf == NULL)  
    {  
        printf("malloc failed!\n");  
        return -1;  
    }  
    blkno = openfilelist[fd].first;  
    blkoff = openfilelist[fd].count;  
    if(blkoff >= openfilelist[fd].length)  
    {  
        puts("Read out of range!");  
        free(buf);  
        return -1;  
    }  
    fatptr = fat1 + blkno;  
    while(blkoff >= BLOCKSIZE)  
    {  
        blkno = fatptr->id;  
        blkoff = blkoff - BLOCKSIZE;  
        fatptr = fat1 + blkno;  
    }  
    ll = 0;  
    while(ll < len)  
    {  
        blkptr = (unsigned char *)(myvhard + blkno * BLOCKSIZE);  
        for(i = 0; i < BLOCKSIZE; i++)  
            buf[i] = blkptr[i];  
        for(; blkoff < BLOCKSIZE; blkoff++)  
        {  
            text[ll++] = buf[blkoff];  
            openfilelist[fd].count++;  
            if(ll == len || openfilelist[fd].count == openfilelist[fd].length)  
                break;  
        }  
        if(ll < len && openfilelist[fd].count != openfilelist[fd].length)  
        {  
            blkno = fatptr->id;  
            if(blkno == END)  
                break;  
            blkoff = 0;  
            fatptr = fat1 + blkno;  
        }  
    }  
    text[ll] = '\0';  
    free(buf); 
    return ll;  
}   
unsigned short findblock()  
{  
    unsigned short i;  
    fat *fat1, *fatptr;  
    fat1 = (fat *)(myvhard + BLOCKSIZE);  
    for(i = 7; i < SIZE / BLOCKSIZE; i++)  
    {  
        fatptr = fat1 + i;  
        if(fatptr->id == FREE)  
            return i;  
    }  
    printf("Error,Can't find free block!\n");  
    return -1;  
}  
int findopenfile()  
{  
    int i;  
    for(i = 0; i < MAXTEXT; i++)  
    {  
        if(openfilelist[i].topenfile == 0)  
            return i;  
    }  
    return -1;  
}
void make_file(){
    startsys();
}
int  copy_file(char *path, char* filename){
    char acname[100];
    char acname2[100];
    int i;
    for(i=0;filename[i]!=0;i++) {acname[i]=filename[i];acname2[i]=filename[i];}
    acname[i]=0;
    acname2[i]=0;
    startsys();
    my_create(filename);  
    int fd;
    fd=my_open(acname);
    if(fd==10)  printf("find file error");
    else{
         my_write(fd,acname2);
    }
    return fd;
}
void copy_out(char* filename){
    char name[10];
    int len=strlen(filename);
    int last=len-4;
    for(;filename[last]!='/';last--);
    last++;
    int j=0;
    for(;filename[last+j]!=0;j++)  name[j]=filename[j+last];
    name[j]=0;
    char acname[100];
    char acname2[100];
    int i;
    for(i=0;i<strlen(name);i++) {acname[i]=name[i];acname2[i]=name[i];}
    acname[i]=0;
    acname2[i]=0;
    startsys();
    int fd;
    fd=copy_file("/demo/",acname);
    my_read(fd,openfilelist[fd].length,acname2);
}


int main(int argc,char*argv[])
{
    if(argc<3 ||(argc>6)){
        printf("error\n");
        return 0;
    }
    else if (strcmp(argv[1],"-f")==0){
        make_file();
        printf("Successfully create!\n");
    }
    else if(strcmp(argv[1],"-mi")==0){
        copy_file(argv[3],argv[4]);      
    }
    else if(strcmp(argv[1],"-mo")==0){
        copy_out(argv[3]);
    }
    return  0;
}
