//
// Created by xiehao on 2024/11/20.
//

#pragma once

#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <errno.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <signal.h>
#include <iostream>

using namespace std;

/*=================================
 *  自定义打印输出
 ==================================*/
#define INFO_OUTPUT         3
#define WARNING_OUTPUT      2
#define DEBUG_OUTPUT        1
#define ERROR_OUTPUT        0

#define DEBUG
#define DEBUG_LEVEL         INFO_OUTPUT

#define PRINT(info, ...) do{ \
    printf("[Info] (%s:%d->%s):" info"", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
}while(0)

#define INFO_PRINT(info, ...) do{ \
    if(DEBUG_LEVEL>=INFO_OUTPUT){ \
        printf("[Info] (%s:%d->%s):" info"", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
    } \
}while(0)

#define WARNING_PRINT(info, ...) do{ \
    if(DEBUG_LEVEL>=WARNING_OUTPUT){ \
        printf("[Warning] (%s:%d->%s):" info"", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
    } \
}while(0)

#define DEBUG_PRINT(info,...) do{ \
    if(DEBUG_LEVEL>=DEBUG_OUTPUT){ \
        printf("[Debug] (%s:%d->%s):" info"",__FILE__,__FUNCTION__,__LINE__,##__VA_ARGS__); \
    } \
}while(0)

#define ERROR_PRINT(info, ...) do{ if(DEBUG_LEVEL>=ERROR_OUTPUT){ printf("[Error] (%s:%d->%s):" info"", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); } }while(0)
