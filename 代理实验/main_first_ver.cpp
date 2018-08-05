
//
//  main.cpp
//  lightsocks
//
//  Created by 黄剑 on 2018/7/30.
//  Copyright © 2018年 黄剑. All rights reserved.
//

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/tcp.h>

#define TIME_OUT 6000000

using namespace std;
string targetIpAddress="0.0.0.0";
static pthread_mutex_t g_mutex;
static pthread_cond_t g_cond;

struct ClientAndRemoteSocks{
    int clientSock;
    int remoteSock;
    bool stopConnect;
};

//读取socket一行
int get_line(int sock, char *buf, int size)
{
    int i = 0;
    char c = '\0';
    int n;
    
    while ((i < size - 1) && (c != '\n'))
    {
        n = recv(sock, &c, 1, 0);
        /* DEBUG printf("%02X\n", c); */
        if (n > 0)
        {
            if (c == '\r')
            {
                n = recv(sock, &c, 1, MSG_PEEK);
                /* DEBUG printf("%02X\n", c); */
                if ((n > 0) && (c == '\n'))
                    recv(sock, &c, 1, 0);
                else
                    c = '\n';
            }
            buf[i] = c;
            i++;
        }
        else
            c = '\n';
    }
    buf[i] = '\0';
    
    return(i);
}
//根据网址和端口号获取socket
int Socket(const char *host, int clientPort)
{
    int sock;
    unsigned long inaddr;
    struct sockaddr_in ad;
    struct hostent *hp;
    
    memset(&ad, 0, sizeof(ad));
    ad.sin_family = AF_INET;
    
    inaddr = inet_addr(host);
    if (inaddr != INADDR_NONE)
        memcpy(&ad.sin_addr, &inaddr, sizeof(inaddr));
    else
    {
        hp = gethostbyname(host);
        if (hp == NULL)
            return -1;
        memcpy(&ad.sin_addr, hp->h_addr, hp->h_length);
    }
    ad.sin_port = htons(clientPort);
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        return sock;
    if (connect(sock, (struct sockaddr *)&ad, sizeof(ad)) < 0)
        return -1;
    return sock;
}


void *proxyThread(void *arg)
{
    pthread_mutex_lock(&g_mutex);
    int connectSock=*(int*)arg;
    pthread_cond_signal(&g_cond);
    pthread_mutex_unlock(&g_mutex);
    
    ssize_t readwordnum=0;
    char buf[2048]={};
    int bufIndex=0;
    memset(buf, 0, sizeof(buf));
    //可能要多搞一个线程来处理双工的链接
    pthread_t pthread_tid=0;
    //这里是获取client的包并且发送到remote，在client断开的时候断开？
    
    memset(buf, 0, sizeof(buf));
    readwordnum=get_line(connectSock, buf, sizeof(buf));
    printf("get first line:%s",buf);
    
    int methodIndex=0;
    char method[255]={};
    memset(method, 0, sizeof(255));
    while(!isspace(buf[methodIndex])&&(methodIndex<sizeof(method)-1))
    {
        method[methodIndex]=buf[methodIndex];
        methodIndex++;
    }
    bufIndex=methodIndex;
    method[methodIndex]='\0';
    
    //在这里判断是否是connect方法，假如是，返回HTTP/1.1 200 Connection Established
    //不知道为什么访问百度会访问这个
    if(strcmp(method, "CONNECT")==0)
    {
        while((readwordnum=get_line(connectSock,buf,sizeof(buf)))>0)
        {
            
        }
        const char *retMsg="HTTP/1.1 200 Connection Established";
        close(connectSock);
        return NULL;
    }
    if(strcmp(method,"GET")==0)
    {
        char secondLine[2048]={};
        int readwordnumline2=get_line(connectSock, secondLine, sizeof(secondLine));
        //获得ip地址
        string hostAddr=string(secondLine).substr(6);
        hostAddr.erase(hostAddr.end()-1);
        int remoteSock=Socket(hostAddr.c_str(),80);
        write(remoteSock, buf, readwordnum);
        write(remoteSock, secondLine,readwordnumline2);
        struct ClientAndRemoteSocks clientAndRemoteSocks={connectSock,remoteSock,false};
        struct timeval time_out={};
        fd_set fd_read;
        int ret=0;
        
        while(true)
        {
            
            time_out.tv_sec=0;
            time_out.tv_usec=TIME_OUT;
            FD_ZERO(&fd_read);
            FD_SET(connectSock,&fd_read);
            FD_SET(remoteSock,&fd_read);
            int maxSock=connectSock;
            if(remoteSock>maxSock)
            {
                maxSock=remoteSock;
            }
            maxSock++;
            ret=select(maxSock, &fd_read, NULL, NULL, NULL);
            if(-1==ret)
            {
                perror("select socket error");
            }
            else if(0==ret)
            {
                printf("select time out.\n");
                continue;
            }
            if(FD_ISSET(connectSock,&fd_read))
            {
                readwordnum=read(connectSock,buf,sizeof(buf));
                if(readwordnum>0)
                {
                    readwordnum=write(remoteSock, buf, readwordnum);
                    if(readwordnum==-1)
                    {
                        perror("send data to real server error");
                        break;
                    }
                }
                else if(readwordnum==0)
                {
                    //关闭端口，退出
                    break;
                }
                else {
                    perror("read connectSock error");
                    break;
                }
            }
            else if(FD_ISSET(remoteSock,&fd_read))
            {
                readwordnum=read(remoteSock,buf,sizeof(buf));
                if(readwordnum>0)
                {
                    readwordnum=write(connectSock, buf, readwordnum);
                    if(readwordnum==-1)
                    {
                        perror("send data to client error");
                        break;
                    }
                }
                else if(readwordnum==0)
                {
                    //关闭端口，退出
                    break;
                }
                else{
                    perror("read remoteSock error");
                    break;
                }
            }
            
        }
        close(remoteSock);
        close(connectSock);
    }
    
    return NULL;
}

int main(int argc, const char * argv[]) {
    if(argc!=2)
    {
        cout// <<"first arg is target ip address\n"
        <<"first arg is listen port\n";
        return NULL;
    }
    pthread_mutex_init(&g_mutex, NULL);
    pthread_cond_init(&g_cond, NULL);
    //    targetIpAddress=argv[1];
    int listenPort=atoi(argv[1]);
    
    int listenSock=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
    if(::bind(listenSock, (struct sockaddr*)&listenSockaddr, len)<0)
    {
        perror("bind");
        exit(2);
    }
    
    if(::listen(listenSock, 1000)<0)
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
        pthread_t pthread_tid=0;
        pthread_mutex_lock(&g_mutex);
        pthread_create(&pthread_tid, nullptr, proxyThread, &connectSock);
        pthread_cond_wait(&g_cond, &g_mutex);
        pthread_mutex_unlock(&g_mutex);
    }
}
