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
pthread_key_t threadKey;
void * _Nullable func(void * _Nullable)
{
    sleep(5);
    char test[10]={};
    pthread_setspecific(threadKey, test);
    printf("child thread set %p\n",(void*)test);
    void *test1=pthread_getspecific(threadKey);
    printf("child thread get %p\n",(void*)test1);
    return NULL;
}

int main(int argc,char *argv[])
{
    pthread_t tid=0;
    pthread_key_create(&threadKey, NULL);
    pthread_create(&tid, NULL, func, NULL);
    char test[10]={};
    pthread_setspecific(threadKey, test);
    printf("main thread set %p\n",(void*)test);
    
    
    pthread_join(tid, NULL);
    void *test2=pthread_getspecific(threadKey);
    printf("main thread get %p\n",(void*)test2);
    
    pthread_key_delete(threadKey);
}
