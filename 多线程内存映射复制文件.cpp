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
//多线程复制文件，记录锁，使用锁锁住当前的偏移数值，可能要用异步io才能在io的时候指定io的范围

int currentReadPost=0;
int fd=0;
int fd2=0;
pthread_mutex_t mutex;
long BLOCK=0;
ssize_t fileSize=0;//记录文件总大小

void *childThread(void *)
{
    while(true)
    {
        pthread_mutex_lock(&mutex);
        int startPost=currentReadPost;
        currentReadPost+=BLOCK;
        pthread_mutex_unlock(&mutex);
        //    只能使用内存映射读取文件了
        ssize_t mmapSize=0;
        if(startPost>fileSize)
        {
            break;
        }
        else if(startPost+BLOCK>fileSize)
        {
            mmapSize=fileSize-startPost;
        }else{
            mmapSize=BLOCK;
        }
        void * readAddr=mmap(NULL,mmapSize,PROT_READ,MAP_SHARED,fd,startPost);
        void * writeAddr=mmap(NULL,mmapSize,PROT_READ | PROT_WRITE,MAP_SHARED,fd2,startPost);
        if((readAddr==MAP_FAILED)||(writeAddr==MAP_FAILED))
        {
            printf("mmap error\n");
        }
        memcpy(writeAddr, readAddr, mmapSize);
        munmap(readAddr, mmapSize);
        munmap(writeAddr, mmapSize);
    }
    
    return 0;
}

int main(int argc,char *argv[])
{
    if(argc!=3)
    {
        puts("first args is source file");
        puts("second args is dest file");
        return 1;
    }
    fd=open(argv[1],O_RDWR);
    fd2=open(argv[2], O_RDWR|O_CREAT|O_EXCL,0664);
    if(fd2<0)
    {
        perror("open");
        puts("target file maybe exist");
        return 1;
    }
    if(fd<0)
    {
        perror("open");
        return 1;
    }
    struct stat sbuff={};
    stat(argv[1],&sbuff);
    fileSize=sbuff.st_size;
    if(fileSize==0)
    {
        close(fd);
        close(fd2);
        return 0;
    }
    //撑大目标文件
    if(lseek(fd2, sbuff.st_size-1,SEEK_SET )==-1)
    {
        perror("lseek");
        return -1;
    }
    
    if(write(fd2," ",1) != 1) //必须写一个空格占用新文件 不然新文件撑大不了
        perror("write error");
    
    if(lseek(fd2,0,SEEK_SET )==-1)
    {
        perror("lseek");
        return -1;
    }
    //映射的间隔必须是虚拟内存地址的倍数
    BLOCK=sysconf(_SC_PAGESIZE);
    
    pthread_mutex_init(&mutex, NULL);
    pthread_t tid=0;
    pthread_create(&tid, NULL, childThread, NULL);
    while(true)
    {
        pthread_mutex_lock(&mutex);
        int startPost=currentReadPost;
        currentReadPost+=BLOCK;
        pthread_mutex_unlock(&mutex);
        //    只能使用内存映射读取文件了
        ssize_t mmapSize=0;
        if(startPost>fileSize)
        {
            break;
        }
        else if(startPost+BLOCK>fileSize)
        {
            mmapSize=fileSize-startPost;
        }else{
            mmapSize=BLOCK;
        }
        void * readAddr=mmap(NULL,mmapSize,PROT_READ,MAP_PRIVATE,fd,startPost);
        void * writeAddr=mmap(NULL,mmapSize,PROT_READ | PROT_WRITE,MAP_SHARED,fd2,startPost);
        if((readAddr==MAP_FAILED)||(writeAddr==MAP_FAILED))
        {
            printf("mmap error\n");
        }
        memcpy(writeAddr, readAddr, mmapSize);
        munmap(readAddr, mmapSize);
        munmap(writeAddr, mmapSize);
    }
    
    pthread_join(tid, NULL);
    close(fd);
    close(fd2);
    return 0;
}
