#include<stdio.h> 
#include<string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
     

float get_temputer()
{    
        
    struct dirent *direntp;
    DIR *dirp=NULL;
    char buf[100]="/sys/bus/w1/devices/";
    char buf2[50];
    int fd=-1;
    int res=-1;
    char *p4=NULL;
    float tem;
    
    if((dirp=opendir(buf))==NULL)
    {
        printf("open des_name failture:%s",strerror(errno));
        return -1;
    }
                                                      
    while((direntp=readdir(dirp))!=NULL)
    {
        if(strstr(direntp->d_name,"28-"))
        {
            strcpy(buf2,direntp->d_name);
            //strncat(buf2,direntp->d_name,sizeof(buf2));
            
            break;
        }
    }
    closedir(dirp);
    
    strcat(buf,buf2);
    strcat(buf,"/w1_slave");
    if((fd=open(buf,O_RDONLY))<0)
        {
            printf("open   failture:%s",strerror(errno));
            return -3;
        }
    if((res=read(fd,buf,sizeof(buf)))<0)
        {
            printf("read   failture:%s",strerror(errno));
            return -4;
        }              
    close(fd);
    
                                                 
    p4=strstr(buf,"t=");
    p4+=2;
    tem=atof(p4)/1000;
    //printf("%f\n",tem);
    return tem;
    
}

int main(int argc,char **argv)
{
    
    float tem;
    tem=get_temputer();
    printf("the device current temputer is %f\n",tem);
}
