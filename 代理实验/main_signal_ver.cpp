#include "cppheadset.h"
#include "headset.h"
#include "proxytool.h"

//尝试使用多进程实现代理，不用select，每个线程监听一个端口并且发送给对应的端口
//使用信号量，共享内存，信号等在两个对应的发送接收线程之间同步

static void handleSIGUSR(int){
    exit(0);
}

int proxyThread(int connectSock)
{
    ssize_t readwordnum=0;
    char buf[2048]={};
    int bufIndex=0;
    memset(buf, 0, sizeof(buf));
    
    readwordnum=get_line(connectSock,buf,sizeof(buf));
    printf("get first line:%s",buf);
    char method[100]={};
    char request[1024]={};
    char httpver[100]={};
    
    sscanf(buf,"%s %s %s",method,request,httpver);
    char line1[2048]={};
    strcpy(line1,buf);
    
    //第二行
    readwordnum=get_line(connectSock,buf,sizeof(buf));
    printf("get secode line:%s",buf);
    char hostHead[100]={};
    char hostAddr[1024]={};
    sscanf(buf,"%s %s",hostHead,hostAddr);
    
    int remoteSock=0;
    if(!strcmp(method,"CONNECT")){
        remoteSock=Socket(hostAddr,443);
    }else{
        remoteSock=Socket(hostAddr,80);
    }
    write(remoteSock,line1,strlen(line1));
    write(remoteSock,buf,readwordnum);
    
    //在这里fork一个子进程处理服务器返回的信息，传递到客户端
    
    //设置信号处理函数
    
    signal(SIGUSR1,handleSIGUSR);
    pid_t pid=fork();
    if(pid<0){
        perror("fork");
    }
    else if(pid==0)
    {
        //子进程
        while(true){
            readwordnum=read(remoteSock,buf,sizeof(buf));
            if(readwordnum<=0)
            {
                kill(getppid(),SIGUSR1);
                break;
            }
            write(connectSock,buf,readwordnum);
        }
    }
    else {
        //父进程
        while(true){
            readwordnum=read(connectSock,buf,sizeof(buf));
            if(readwordnum<=0)
            {
                kill(pid,SIGUSR1);
                waitpid(pid,NULL,NULL);
                break;
            }
            write(remoteSock,buf,readwordnum);
        }
        
    }
    close(remoteSock);
    close(connectSock);
    return 0;
}

int main(int argc,char *argv[])
{
    if(argc!=2)
    {
        std::cout// <<"first arg is target ip address\n"
        <<"first arg is listen port\n";
        return NULL;
    }
    int listenPort=atoi(argv[1]);
    
    int listenSock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(listenSock<0)
    {
        perror("socket");
        exit(1);
    }
    
    struct sockaddr_in listenSockaddr={};
    listenSockaddr.sin_family=AF_INET;
    listenSockaddr.sin_port=htons(listenPort);
    listenSockaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    socklen_t len=sizeof(listenSockaddr);
    if(::bind(listenSock,(struct sockaddr*)&listenSockaddr,len)<0)
    {
        perror("bind");
        exit(2);
    }
    if(::listen(listenSock,100)<0)
    {
        perror("listen");
        exit(3);
    }
    
    
    struct sockaddr_in remoteSockaddr={};
    while(true){
        int connectSock=accept(listenSock,(struct sockaddr*)&remoteSockaddr, &len);
        std::cout<<"receive file or document from ip address: "<<inet_ntoa(remoteSockaddr.sin_addr)<<"\nand port: "<<ntohs(remoteSockaddr.sin_port)<<"\n";
        if(connectSock<0)
        {
            perror("accept");
            return -1;
        }
        //新开一个进程处理
        pid_t pid=fork();
        if(pid<0)
        {
            perror("fork error");
        }
        else if(pid==0)
        {
            //第一个子进程，使用两次fork大法
            if((pid=fork())<0)
            {
                perror("fork second error");
            }else if(pid>0)
            {
                exit(0);
            }
            proxyThread(connectSock);
            exit(0);
        }
        else
        {
            close(connectSock);
            waitpid(pid,NULL,NULL);
        }
    }
    printf("done!\n");
    return 0;
}

