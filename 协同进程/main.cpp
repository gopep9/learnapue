//
//  main.cpp
//  CPlusPlusTest
//
//  Created by huangzhao on 2018/4/16.
//  Copyright © 2018年 huangzhao. All rights reserved.
//

#include <iostream>
#include <string>
#include <unistd.h>
#include "headset.h"
//加法器，读取输入，并且相加，输出

int main(int argc,char *argv[])
{
    signal(SIGCHLD, SIG_DFL);
    int pipe1[2]={};
    int pipe2[2]={};
    pipe(pipe1);
    pipe(pipe2);
    pid_t pid=fork();
    if(pid<0)
    {
        perror("fork");
        return 1;
    }
    else if(pid==0)
    {
        close(STDIN_FILENO);
        dup(pipe2[0]);
        
        close(STDOUT_FILENO);
        dup(pipe1[1]);
        
        close(pipe1[0]);
        close(pipe2[1]);
        
        execl("/Users/huangjian/xcodeproject/CPlusPlusTest/CPlusPlusTest/add",0);
        perror("execl");
    }
    else{
        close(pipe1[1]);
        close(pipe2[0]);
        char line[1024]={};
        while(fgets(line,1024,stdin)!=NULL)
        {
            write(pipe2[1],line,strlen(line));
            memset(line, 0, 1024);
            read(pipe1[0],line,1024);
            fputs(line,stdout);
        }
        int result=0;
        wait(&result);
    }
}
