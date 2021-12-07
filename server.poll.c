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
#include <poll.h>

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
    struct pollfd poll_array[1024]; 
    int ch;
    char buf3[256];
    struct rlimit limits; 
    int max_fd=1;
    int rv;
    int rv2;
    int found;
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
        usgeprintf(argv[0]);
    }

    //getrlimit(RLIMIT_NOFILE,&limits);
    //limits.rlim_cur=limits.rlim_max;
    //setrlimit(RLIMIT_NOFILE,&limits);
    //设置进程资源方法2.
    setprocesslimit(&limits);//设置进程资源。
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
    //printf("success to bind port:%d\n",ntohs(port));
            
   // printf("wait client to connect...\n");
    for(int i=0;i<1024;i++)
    {
                      
             poll_array[i].fd=-1;
                  
    }
    poll_array[0].fd=fd;
    poll_array[0].events=POLLIN;
    while(1)
    {
        for(int i=0;i<1024;i++)
        {
           if(poll_array[i].fd<0)
               continue;
            
            
        
            max_fd=(max_fd>poll_array[i].fd)?(max_fd):(poll_array[i].fd);
        }
    
          //tv.tv_sec=5;
        //tv.tv_usec = 3;
        
        rv=poll(poll_array,max_fd+1,-1);//遍历全部结构体
        if(rv<0)
        {
            printf("poll failture:%s\n",strerror(errno));
            return -1;
        }
        else if(rv==0)
        {
            printf("(poll timeout...\n");
            continue;
        }
        
        if(poll_array[0].revents&POLLIN)
            {

                printf("now is waitting to accept fd\n");
                if((fd2=accept(fd,(struct sockaddr *)&client_addr,&addrlen)) <0)
                {
                    printf("accept failture:%s\n",strerror(errno));
                    continue;
                }
                found=0;
                for(int i=0;i<1024;i++)
                {
                
                    if(poll_array[i].fd<0)
                    {
                        printf("accept new client,new client port:%d,new client ip:%s,now fd2:%d\n",ntohs(client_addr.sin_port),inet_ntoa(client_addr.sin_addr),fd2);
                        printf("add fd2 to pollfd_array[i].fd\n");
                        poll_array[i].fd=fd2;//把fd2存入poll_array这个数组。
                        poll_array[i].events=POLLIN;
                        found=1;
                        break;
                    }
                }
                if(!found)
                {
                    printf("accept new client,but pollfd struct is full\n");
                    close(fd2);
                }
                  
            }
        
        //deal message
        else
        {
                for(int i=1;i<1024;i++)
               // printf("this is commuicating fd:%d\n",array[i]);
                {
                    if(poll_array[i].fd<0||!poll_array[i].revents&POLLIN)
                    {   
                        continue;
                    }
                    else
                    {    
                        memset(buf,0,sizeof(buf));
                        memset(buf3,0,sizeof(buf3));
                        printf("this is commuicating fd:%d\n",poll_array[i].fd); 
                        rv2=(recv(poll_array[i].fd,buf,sizeof(buf),0));  
                        if(rv2<0)
                        {
                            printf("recv message failture:%s,now fd2:%d\n",strerror(errno),poll_array[i].fd);
                            close(poll_array[i].fd);
                            poll_array[i].fd=-1;
                        }
                        if(rv2>0)
                        {

                            snprintf(buf3,sizeof(buf3),"recive client message:%s\n",buf);
                            printf("%s\n",buf3);
                            close(poll_array[i].fd);
                            poll_array[i].fd=-1;
                        }
                        //continue;
                    }
                }
        }

           
    }
    close(fd);
    return 0;
}
