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
//多进程复制文件

int currentReadPost=0;
pthread_mutex_t mutex;
#define BLOCK 4096
ssize_t fileSize=0;//记录文件总大小


ssize_t safe_read(int fd,char *vptr,size_t n)
{
    size_t nleft;
    ssize_t nread;
    char *ptr;
    
    ptr=vptr;
    nleft=n;
    
    while(nleft > 0)
    {
        if((nread = read(fd,ptr,nleft)) < 0)
        {
            if(errno == EINTR) //被信号中断，重读
                nread = 0;
            else //出错
                return -1;
        }
        else if(nread == 0) //EOF
            break;
        
        nleft -= nread;
        ptr += nread;
    }
    return (n-nleft);
}

ssize_t safe_write(int fd, const char *vptr, size_t n)
{
    size_t  nleft;
    ssize_t nwritten;
    const char *ptr;
    
    ptr = vptr;
    nleft = n;
    
    while(nleft > 0)
    {
        if((nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if(nwritten < 0 && errno == EINTR) //被信号中断，重写
                nwritten = 0;
            else //error
                return -1;
        }
        nleft -= nwritten;
        ptr   += nwritten;
    }
    return(n);
}


int main(int argc,char *argv[])
{
    if(argc!=3)
    {
        puts("first args is source file");
        puts("second args is dest file");
        return 1;
    }
    
    key_t shmKey=ftok("/Users/huangjian/xcodeproject/CPlusPlusTest/CPlusPlusTest/main.cpp",0);
    
    ssize_t shmSize=sizeof(pthread_mutex_t)+8;
    
    int shmHandle=shmget(shmKey, shmSize, IPC_CREAT|0777);
    
    //在子进程之前连接共享内存到地址
    void *share=shmat(shmHandle, NULL, NULL);
    if(share==(void*)-1)
    {
        perror("shmat");
        return 1;
    }
    //清空缓存区
    memset(share, 0, shmSize);
    
    pthread_mutex_t *mutexShare=static_cast<pthread_mutex_t *>(share);
    int *currentPost=reinterpret_cast<int *>(reinterpret_cast<ssize_t>(share)+sizeof(pthread_mutex_t));
    *currentPost=0;
    pthread_mutexattr_t attr={};
    int ret=pthread_mutexattr_init(&attr);
    if(ret<0)
    {
        perror("pthread_mutexattr_init");
        return 1;
    }
    ret=pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    if(ret<0)
    {
        perror("pthread_mutexattr_setpshared");
        return 1;
    }
    ret=pthread_mutex_init(mutexShare, &attr);
    if(ret<0)
    {
        perror("pthread_mutex_init");
        return 1;
    }
    pthread_mutexattr_destroy(&attr);
    
    pid_t pid=fork();
    if(pid<0)
    {
        perror("fork");
        return 1;
    }
    //父进程和子进程进行一样的操作
    //为了不让文件描述符指向同一个文件表，在fork之后才open
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
    struct stat sbuff={};
    stat(argv[1],&sbuff);
    if(sbuff.st_size==0)
    {
        close(srcfd);
        close(dstfd);
        return 0;
    }
    fileSize=sbuff.st_size;
    ftruncate(dstfd, sbuff.st_size);//两个进程都设置下文件大小
    //使用共享内存同步文件偏移
    while(true)
    {
        pthread_mutex_lock(mutexShare);
        ssize_t startPost=*currentPost;
        *currentPost=*currentPost+BLOCK;
        pthread_mutex_unlock(mutexShare);
        ssize_t copySize=0;
        if(startPost>=fileSize)
        {
            break;
        }
        else if(startPost+BLOCK>fileSize)
        {
            copySize=fileSize-startPost;
        }
        else
        {
            copySize=BLOCK;
        }
        
        lseek(srcfd, startPost, SEEK_SET);
        lseek(dstfd, startPost, SEEK_SET);
        char buf[BLOCK]={};
        safe_read(srcfd, buf, copySize);
        safe_write(dstfd, buf, copySize);
    }
    close(srcfd);
    close(dstfd);
    if(pid==0)
    {
        
        
        //分离共享内存
        shmdt(share);
    }
    else
    {
        
        waitpid(pid, NULL, NULL);
        pthread_mutex_destroy(mutexShare);
        shmdt(share);
        ret=shmctl(shmHandle, IPC_RMID, NULL);
        if(ret<0)
        {
            perror("shmctl IPC_RMID");
            return 1;
        }
    }
    return 0;
}
