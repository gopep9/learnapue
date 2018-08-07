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
//内存映射拷贝文件
int main(int argc,char *argv[])
{
    if(argc!=3)
    {
        puts("first args is source file");
        puts("second args is dest file");
        return 1;
    }
    int fd=open(argv[1],O_RDWR);
    int fd2=open(argv[2], O_RDWR|O_CREAT|O_EXCL,0664);
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
    
    //撑大目标文件
    if(lseek(fd2, sbuff.st_size-1,SEEK_SET )==-1)
    {
        perror("lseek");
        return -1;
    }
    
    if(write(fd2," ",1) != 1) //必须写一个空格占用新文件 不然新文件撑大不了
        perror("write error");
    
    void *share=mmap(NULL,sbuff.st_size,PROT_READ,MAP_PRIVATE,fd,0);
    if(share==MAP_FAILED)
    {
        perror("map error");
    }
    void *dst;
    if((dst = mmap(0,sbuff.st_size,PROT_READ | PROT_WRITE,MAP_SHARED,fd2,0)) ==MAP_FAILED)
        perror("mmap error for output");
    memcpy(dst, share, sbuff.st_size);
    munmap(share,sbuff.st_size);
    munmap(dst,sbuff.st_size);//映射完记得关掉映射，不然源文件内容不会被改变
    return 0;
}
