//
//  main.cpp
//  CPlusPlusTest
//
//  Created by huangzhao on 2018/4/16.
//  Copyright © 2018年 huangzhao. All rights reserved.
//

#include <iostream>
#include <string>
#include "headset.h"
//多进程复制文件，记录锁，使用锁锁住当前的偏移数值，可能要用异步io才能在io的时候指定io的范围

int currentReadPost=0;
int fd=0;
int fd2=0;
pthread_mutex_t mutex;
long BLOCK=0;
ssize_t fileSize=0;//记录文件总大小

int main(int argc,char *argv[])
{
    if(argc!=3)
    {
        puts("first args is source file");
        puts("second args is dest file");
        return 1;
    }
    pid_t pid=fork();
    if(pid<0)
    {
        perror("fork");
        return 1;
    }
    //父进程和子进程进行一样的操作
    int srcfd=open(argv[1], O_RDWR);
    if(srcfd<0)
    {
        perror("open");
        return 1;
    }
    int dstfd=open(argv[2], O_RDWR|O_CREAT,0664);
    if(dstfd<0)
    {
        perror("open");
        return 1;
    }
    if(pid==0)
    {
        int srcfd=open(argv[1],)
    }
    else
    {
        
    }
    return 0;
}
