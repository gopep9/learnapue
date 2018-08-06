
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
#include "proxytool.h"

//#define TIME_OUT 6000000

using namespace std;
string targetIpAddress="0.0.0.0";
static pthread_mutex_t g_mutex;
static pthread_cond_t g_cond;

//struct ClientAndRemoteSocks{
//    int clientSock;
//    int remoteSock;
//    bool stopConnect;
//};

void setSignalIGN()
{
    signal(SIGPIPE, SIG_IGN);
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
    
    //    int tmpflag=fcntl(connectSock, F_GETFL);
    //    tmpflag|=O_NONBLOCK;
    //    int ret=fcntl(connectSock, F_SETFL,tmpflag);
    //    tmpflag=fcntl(remoteSock, F_GETFL);
    //    ret=fcntl(remoteSock, F_SETFL,tmpflag);
    //设置为非阻塞
    
    while(true)
    {
        int error=0;
        socklen_t len=sizeof(error);
        //        int code=getsockopt(connectSock, SOL_SOCKET, SO_ERROR, &error, &len);
        //        if(code<0||error)
        //        {
        //            break;
        //        }
        readwordnum=recv(connectSock,buf,sizeof(buf),MSG_DONTWAIT);
        //假如返回值>0正常，-1和errno为11也是正常
        if(readwordnum>0){
            write(remoteSock, buf, sizeof(buf));
        }else if((readwordnum==-1)&&(errno==EWOULDBLOCK))
        {
            //            正常
        }
        else{
            break;
        }
        error=0;
        len=sizeof(error);
        //        code=getsockopt(remoteSock, SOL_SOCKET, SO_ERROR, &error, &len);
        //        if(code<0||error)
        //        {
        //            break;
        //        }
        readwordnum=recv(remoteSock,buf,sizeof(buf),MSG_DONTWAIT);
        if(readwordnum>0){
            write(connectSock, buf, sizeof(buf));
        }else if((readwordnum==-1)&&(errno==EWOULDBLOCK))
        {
            //            正常
        }
        else{
            break;
        }
    }
    
    close(remoteSock);
    close(connectSock);
    return 0;
}

int main(int argc, const char * argv[]) {
    setSignalIGN();
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
