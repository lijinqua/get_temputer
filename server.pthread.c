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
#include<pthread.h>


void setprocesslimit(struct rlimit *lim);
void usgeprintf(char *argv);
int pthread_start(pthread_t *tid,void *pthread_work,void *arg);
void *pthread_work(void * ctx);

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
int pthread_start(pthread_t *tid,void *pthread_work,void *arg)
{   
    int rv=-1;
    pthread_attr_t pthread_attr;
    if(pthread_attr_init(&pthread_attr))
    {
        printf("init pthread  failture :%s\n",strerror(errno));
        pthread_attr_destroy(&pthread_attr);
        pthread_exit(NULL);
    }
    if (pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED))
    {
        printf("set dethche  pthread  failture :%s\n",strerror(errno));
        pthread_attr_destroy(&pthread_attr);
        pthread_exit(NULL); 
    }
    if(pthread_create(tid,&pthread_attr,pthread_work,arg))
    {
        printf("create  pthread  failture :%s\n",strerror(errno));
        pthread_attr_destroy(&pthread_attr);
        pthread_exit(NULL); 
    }
    rv=1;
    return rv;
}
void *pthread_work(void * ctx)
{
    int fd2;
    char buf[256];
    char buf3[256];
    if(!ctx)
    {
         printf("send ctx  failture:%s\n",strerror(errno));
         pthread_exit(NULL);
    }
    fd2=(int)ctx;
    //close(fd);
    while(1)
    {
        memset(buf,0,sizeof(buf));    //每次进行内存置0；
        memset(buf3,0,sizeof(buf3));
        if((recv(fd2,buf,sizeof(buf),0))<0)    //每次进行内存置0；
        {
            printf("recv message  failture:%s\n",strerror(errno));
            pthread_exit(NULL);
        }
             
        snprintf(buf3,sizeof(buf3),"recive client message:%s\n",buf);
        printf("%s\n",buf3);
        close(fd2);
        pthread_exit(NULL);      
    
    }
}

int main(int argc,char **argv)
{
    int fd=-1;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t  addrlen;
    int fd2=-1;
    pthread_t tid;
    int port;
    int on=1;
    int ch;
    
    struct rlimit limits; 
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
        pthread_exit(NULL);
    }
    // getrlimit(RLIMIT_NOFILE,&limits);
    //limits.rlim_cur=limits.rlim_max;
    //setrlimit(RLIMIT_NOFILE,&limits);
    setprocesslimit(&limits);
    printf("now process can open fd is:%ld",limits.rlim_cur);
    fd=socket(AF_INET,SOCK_STREAM,0);
    if(fd<0)
    {
        printf("socket failture:%s\n",strerror(errno));
        pthread_exit(NULL);
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
        pthread_exit(NULL);
    }
    //printf("bind ip and port success,port:%d\n",ntohs(port));
    listen(fd,20);
    //printf("success to bind port:%d\n",ntohs(port));

    printf("wait client to connect...\n");
    while(1)
    {

        if((fd2=accept(fd,(struct sockaddr *)&client_addr,&addrlen)) <0)
        {
            printf("accept failture:%s\n",strerror(errno));
            pthread_exit(NULL);;
        }
    
        printf("accept new client,new client port:%d,new client ip:%s\n",ntohs(client_addr.sin_port),inet_ntoa(client_addr.sin_addr));
        if(pthread_start(&tid,pthread_work,(void *)fd2)<0)
        {   

            printf(" pthread_start  failture:%s\n",strerror(errno));
            pthread_exit(NULL);
     
        }
    //close(fd);
    }
    close(fd);
    return 0;
}
                
        
