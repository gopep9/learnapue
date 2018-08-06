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

int my_getline(char* line, int max_size)
{
    int c;
    int len = 0;
    while( (c = getchar()) != EOF && len < max_size ){
        line[len++] = c;
        if('\n' == c)
            break;
    }
    line[len] = '\0';
    return len;
}

int main(int argc,char *argv[])
{
    size_t readwordnum = 0,int1 = 0,int2 = 0;
    char str[1024];
    while(my_getline(str, 1024)>0)
    {
        if(sscanf(str,"%d %d",&int1,&int2)==2)
        {
            sprintf(str,"%d\n",int1+int2);
            readwordnum=strlen(str);
            if(write(STDOUT_FILENO, str, readwordnum)!=readwordnum)
            {
                perror("write");
                return -1;
            }
        }else{
            puts("invalid args");
            return -1;
        }
    }
    return 0;
}
