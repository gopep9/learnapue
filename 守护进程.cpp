[200~//
//  main.cpp
//  CPlusPlusTest
//
//  Created by huangzhao on 2018/4/16.
//  Copyright © 2018年 huangzhao. All rights reserved.
//

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>		/* some systems still require this */
#include <sys/stat.h>
#include <sys/termios.h>	/* for winsize */
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <stdio.h>		/* for convenience */
#include <stdlib.h>		/* for convenience */
#include <stddef.h>		/* for offsetof */
#include <string.h>		/* for convenience */
#include <unistd.h>		/* for convenience */
#include <signal.h>		/* for SIG_ERR */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <vector>
#include <string>
#include <algorithm>
#include "headset.h"
void* threadfunc(void *arg)
{

}

int daemonize()
{
    int i,fd0,fd1,fd2;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;
    umask(0);
    if(getrlimit(RLIMIT_NOFILE,&rl)<0)
    {
//        err_quit("%s: can't get file limit",cmd);
        perror("getrlimit");
    }
    //要用子进程来创建守护进程，能够保证子进程不是组长进程

    //感觉应该在这里设置sigction
    sa.sa_handler=SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags=0;
    if(sigaction(SIGHUP,&sa,NULL)<0)
    {
        perror("sigaction");
    }

    if((pid=fork())<0)
        perror("can't fork");
    else if(pid!=0)
    {
        printf("child progress:%d",pid);
        exit(0);
    }
    setsid();

    if(chdir("/")<0)
    {
        perror("chdir");
    }

    if(rl.rlim_max==RLIM_INFINITY)
        rl.rlim_max=1024;
    for(i=0;i<rl.rlim_max;i++)
        close(i);

    fd0=open("/dev/null",O_RDWR);
    fd1=dup(0);
    fd2=dup(0);
    //在这里监听请求
    int listenSock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(listenSock < 0)
    {
        perror("socket");
        exit(1);
    }
    struct sockaddr_in listenSockaddr={};
    listenSockaddr.sin_family=AF_INET;
    //监听的端口
    listenSockaddr.sin_port=htons(12354);
    listenSockaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    socklen_t len=sizeof(listenSockaddr);
    if(bind(listenSock, (struct sockaddr*)&listenSockaddr, len)<0)
    {
        perror("bind");
        exit(2);
    }
    if(listen(listenSock,5)<0)
    {
        perror("listen");
        exit(3);
    }
    struct sockaddr_in remoteSockaddr={};

    std::vector<int> socks;
    struct timeval time_out={};
    fd_set fd_read;
    int ret=0;
    while(true)
    {
        time_out.tv_sec=1;
        time_out.tv_usec=0;
        FD_ZERO(&fd_read);
        for(int i=0;i<socks.size();i++)
        {
            FD_SET(socks[i],&fd_read);
        }
        FD_SET(listenSock,&fd_read);
        ret=select(FD_SETSIZE,&fd_read,NULL,NULL,&time_out);
        if(ret<0)
            perror("select error");
        else if(ret==0)
            printf("time out\n");
        else{
            if(FD_ISSET(listenSock,&fd_read))
            {
                //接受新连接
                struct sockaddr_in remoteSockaddr={};
                socklen_t len=0;

                int connectSock=accept(listenSock,(struct sockaddr*)&remoteSockaddr,&len);
                if(connectSock<0)
                {
                    perror("accept");
                    return -1;
                }
                socks.push_back(connectSock);
            }
            for(int i=0;i<socks.size();i++)
            {
                if(FD_ISSET(socks[i],&fd_read))
                {
                    time_t currentTime=time(NULL);
                    char msg[100]={};
                    sprintf(msg,"%ld",currentTime);
                    write(socks[i],msg,strlen(msg));
                    //在这里关闭文件描述符,并且删除文件描述符
                    close(socks[i]);
                    std::vector<int>::iterator iter=
                            find(socks.begin(),socks.end(),socks[i]);
                    socks.erase(iter);
                }
            }
        }
    }
//    openlog(cmd,LOG_CONS,LOG_DAEMON);
    return 0;
}

int main(int argc, const char * argv[]) {
    // insert code here...
    daemonize();
}


