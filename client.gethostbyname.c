#include<stdio.h> 
#include <netdb.h>
#include<stdio.h>
#include<string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

char*  get_servername() 
{
        struct hostent *hostname;
        char  *p;
        
        hostname=gethostbyname("master.iot-yun.club");
        p=inet_ntoa( *(struct in_addr*)hostname->h_addr_list);
        return p;
        
}
     

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
    printf("%f\n",tem);
    return tem;
    
}

int main(int argc,char **argv)
{
    int fd1=-1;
    struct sockaddr_in client_addr;
    float tem;
    char buf[20];
    char buf2[100];
    short port=9115;
    
    char * server_ip;
    struct hostent *hostname;
    //hostname=gethostbyname("master.iot-yun.club"); //解析域名
     


    server_ip=get_servername();
    tem=get_temputer();
    //printf("p1 server_ip :%s\n",p1);

    fd1=socket(AF_INET,SOCK_STREAM,0);
    memset(&client_addr,0,sizeof(client_addr));
    client_addr.sin_family=AF_INET;
    client_addr.sin_port=htons(port);
    //client_addr.sin_addr.s_addr=((*(struct in_addr*)hostname->h_addr_list));
    //client_addr.sin_addr.s_addr = (unsigned long)p1;
    //client_addr.sin_addr.s_addr = *((unsigned long*)hostname->h_addr_list[0]);
    printf("server_ip:%s\n",server_ip);
    inet_aton(server_ip,&client_addr.sin_addr);
    printf("server_ip:%s\n",server_ip);
    


    printf("start to connecting...\n");
    printf("eran server ip:%s,server port:%d\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
     if (connect(fd1,(struct sockaddr * )&client_addr,sizeof(client_addr))<0)
     {
         printf("connect failture:%s\n",strerror(errno));
         return -1;
     }
    printf("connect success!\n");
    snprintf(buf2,sizeof(buf2),"the device currently tem is %f",tem);
    printf("currently date:%s\n",buf2);
    if( write(fd1,buf2,sizeof(buf2))<0)
    {
                   
         printf("connect failture:%s\n",strerror(errno));
         return -1;    
    }
    close(fd1);


}

