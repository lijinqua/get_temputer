#include<stdio.h>
#include<string.h>
#include<errno.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <getopt.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/epoll.h>
#define MAXSIZE 1024

void setprocesslimit(struct rlimit *lim);
void usgeprintf(char *argv);

void usgeprintf(char *argv)
{
    printf("%s:need to input these argument:\n",argv);
    printf("-port(--p):need to input port\n");
    printf("-help(--h):if you need some help\n");
}


void setprocesslimit(struct rlimit *lim)
{   
    
    getrlimit(RLIMIT_NOFILE,lim);
    lim->rlim_cur=lim->rlim_max;
    setrlimit(RLIMIT_NOFILE,lim);
}

int main(int argc,char **argv)
{
    int fd=-1;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t  addrlen;
    int fd2=-1;
    char buf[256];
    int port;
    int on=1;
    struct epoll_event epoll_array[1024]; //自己声明在epoll_wait之后会被给返回的就绪链表，由已经准备就绪的文件描述符唤醒回调函数来将自己加入
    struct epoll_event ev;//将全部要监听的事件给加进去，与poll,select不同，他们不用就绪链表，直接加入第一个。
    int ch;
    char buf3[256];
    struct rlimit limits; 
    int max_fd=1;
    int rv;
    int rv2;
    int ep_fd=-1;
    struct option opt_long[]={
        {"port",required_argument,0,'p'},
        {"help",no_argument,0,'h'},
        {NULL,0,NULL,0}
        
    };


    while( (ch=getopt_long(argc, argv, "p:h", opt_long, NULL)) != -1 )
    {
        switch(ch)
        {   case 'p':
                port=atoi(optarg);
                break;
            case 'h':
                usgeprintf(argv[0]);
                break;
            default:
                break;
        }
    }
    if(!port)
    {
        printf("Please enter --help(--h) look messages\n");
        return -1;
    }

    //getrlimit(RLIMIT_NOFILE,&limits);
    //limits.rlim_cur=limits.rlim_max;
    //setrlimit(RLIMIT_NOFILE,&limits);
    //设置进程资源方法2.
    setprocesslimit(&limits);//设置进程资源方法1。
    printf("setrlimit:now process can open fd is:%ld\n",limits.rlim_cur);
    fd=socket(AF_INET,SOCK_STREAM,0);
    if(fd<0)
    {
        printf("socket failture:%s\n",strerror(errno));
        return -1;
    }
    //printf("create socket success\n");

    

    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=INADDR_ANY;
    server_addr.sin_port=htons(port);
    setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
    if(bind(fd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0)
    {
        printf("bind failture:%s\n",strerror(errno));
        return -2;
    }
    //printf("bind ip and port success,port:%d\n",ntohs(port));
    listen(fd,20);
    //创建epoll_create实例;
    if((ep_fd=epoll_create(485))<0)
    {
        printf("epoll create example  failture:%s\n",strerror(errno));
        return -1;
    }
    ev.data.fd=fd;
    ev.events=EPOLLIN;
    if(epoll_ctl(ep_fd,EPOLL_CTL_ADD,fd,&ev)<0) //表明监听的fd和监听的时间，同时会注册回调函数。
    {
        printf("epoll ctl add socket fd  failture:%s\n",strerror(errno));
        return -2;
    }
    while(1)
    {
        
        //对加入的文件描述符进行监听，并指明想要监听的事件
        rv=epoll_wait(ep_fd,epoll_array,1024,-1);
        if(rv<0)
        {
            printf("epoll wait failture:%s\n",strerror(errno));
            return -1;
        }
        else if(rv==0)
        {
            printf("(poll timeout...\n");
            continue;
        }
        //对就绪链表进行处理操作
        for(int i=0;i<1024;i++)
            {
                if(epoll_array[i].events&EPOLLERR||epoll_array[i].events&EPOLLHUP)
                {
                    //printf("epoll wait return ture,but not is my want\n");剩下的未满数组里面全都是这个返回值。
                    epoll_ctl(ep_fd,EPOLL_CTL_DEL,epoll_array[i].data.fd,&ev);
                    close(epoll_array[i].data.fd);
                    break;
                }
                //接下来就绪链表中的全部的fd都是真实发生了事件的;
                //就绪链表中fd有客户端连接：
                if(epoll_array[i].data.fd==fd)
                {
                    
                    printf("now have client wait conncting\n");
                    if((fd2=accept(fd,(struct sockaddr *)&client_addr,&addrlen)) <0)
                    {
                        printf("accept failture:%s\n",strerror(errno));
                        continue;
                    }
                    
                    printf("accept new client,new client port:%d,new client ip:%s,now fd2:%d\n",ntohs(client_addr.sin_port),inet_ntoa(client_addr.sin_addr),fd2);
                    ev.data.fd=fd2;//把fd2存入ev这个结构体。
                    ev.events=EPOLLIN;
                    if(epoll_ctl(ep_fd,EPOLL_CTL_ADD,fd2,&ev)<0)
                    {
                        printf("epoll ctl add client_fd to struct epoll_events failture:%s\n",strerror(errno));
                        continue;
                    }
                    //printf("accept new client,client_fd:%d\n",fd2);
                    //continue;
                    //break;
                }
            //就绪链表中fd有数据发送：
                
                if(epoll_array[i].data.fd!=fd&&epoll_array[i].data.fd>0)
                
                {
                    
                    memset(buf,0,sizeof(buf));
                    memset(buf3,0,sizeof(buf3));
                    //printf("this is commuicating fd:%d\n",epoll_array[i].data.fd); 
                   
                    rv2=(recv(epoll_array[i].data.fd,buf,sizeof(buf),0));  
                    if(rv2<0)
                    {
                        printf("recv message failture:%s,now fd2:%d\n",strerror(errno),epoll_array[i].data.fd);
                        epoll_ctl(ep_fd,EPOLL_CTL_DEL,epoll_array[i].data.fd,&ev);
                        close(epoll_array[i].data.fd);
                        continue;
                    
                    }
                    if(rv2>0)
                    {
                        snprintf(buf3,sizeof(buf3),"recive client message:%s\n",buf);
                        printf("%s\n",buf3);
                        epoll_ctl(ep_fd,EPOLL_CTL_DEL,epoll_array[i].data.fd,&ev);
                        close(epoll_array[i].data.fd);
                        continue;
                    }
                  
                    //close(epoll_array[i].data.fd);
                }
            }
        

           
    }
    close(fd);
    return 0;
}
