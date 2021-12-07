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
#include <getopt.h>


void usgage(char *argv);
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

void usgage(char *argv)
{
    printf("%s:you need to input this argument:\n",argv);
    printf("-i(--serverip):you need to input server_ip\n");
    printf("-p(--port):you need to input port\n");
    printf("-h(--help):you need some help\n");
        
}

int main(int argc,char **argv)
{
    int fd1=-1;
    struct sockaddr_in client_addr;
    float tem;
    char buf[20];
    char buf2[100];
    int port;
    char *server_ip;
    int   ch;
    struct option  long_option[]={
        { "serverip",required_argument,NULL,'s'},
        {"port",required_argument,NULL,'p'},
        {"help",no_argument,NULL,'h'},
        {NULL,0,NULL,0}
    };

    while( (ch=getopt_long(argc, argv, "i:p:h", long_option, NULL)) != -1 )
    {
        switch(ch)
        {   case 's':
            server_ip=optarg;
            break;
            
            case 'p':
            port=atoi(optarg);
            break;
            
            case 'h':
            usgage(argv[0]);
            return 0;

            default:
                break;
        }
                
    }
   printf("argument success\n") ;
   if(!server_ip||!port)
   {
       usgage(argv[0]);
   }

    tem=get_temputer();
    fd1=socket(AF_INET,SOCK_STREAM,0);
    memset(&client_addr,0,sizeof(client_addr));
    client_addr.sin_family=AF_INET;
    client_addr.sin_port=htons(port);
    inet_aton(server_ip,&client_addr.sin_addr);
    
    printf("the earn server ip is %s\n",inet_ntoa(client_addr.sin_addr));
    snprintf(buf2,sizeof(buf2),"the device currently tempture is %f",tem);
    
    printf("eran the port: %d\n",ntohs(client_addr.sin_port));
    printf("start to connect...\n");
     if (connect(fd1,(struct sockaddr * )&client_addr,sizeof(client_addr))<0)
     {
         printf("connect failture:%s\n",strerror(errno));
         return -1;
     }
    printf("connect success!\n");
    
    if( send(fd1,buf2,sizeof(buf2),0)<0)
    {
                   
         printf("send message failture:%s\n",strerror(errno));
         return -1;    
    }
    close(fd1);


}

