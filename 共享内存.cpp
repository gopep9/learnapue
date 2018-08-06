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

int main(int argc, const char * argv[]) {
    key_t ipcKey = ftok("/Users/huangjian/xcodeproject/CPlusPlusTest/CPlusPlusTest/main.cpp",0);
    int shmHandle=shmget(ipcKey, sizeof(int), IPC_CREAT|0777);
    //    perror("shmget");
    void *addr=shmat(shmHandle,NULL,NULL);
    //    perror("shmat");
    memset(addr,0,sizeof(int));
    sem_t *sem=sem_open("mysem1", O_CREAT,0777,1);
    pid_t pid=fork();
    if(pid<0)
    {
        perror("fork error");
    }
    else if(pid==0)
    {
        while(true){
            sem_wait(sem);
            int i=*(int*)addr;
            i++;
            *(int*)addr=i;
            printf("%d child\n",i);
            if(i>1000)
            {
                printf("done\n");
                shmdt(addr);
                sem_unlink("mysem1");
                return 0;
            }
            sem_post(sem);
        }
    }
    else{
        while(true){
            sem_wait(sem);
            int i=*(int*)addr;
            i++;
            *(int*)addr=i;
            printf("%d parent\n",i);
            if(i>1000)
            {
                printf("done\n");
                shmdt(addr);
                sem_unlink("mysem1");
                return 0;
            }
            sem_post(sem);
        }
    }
    return 0;
}
