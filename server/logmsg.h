
#ifndef _LOGMSG_
#define _LOGMSG_

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdarg.h>
#include "config.h"


// 枚举日志的级别
typedef enum {
    DEBUG = 1,  // 调试信息
    INFO,    // 普通信息
    WARNING, // 警告            
    ERROR    // 错误
} level;

// 初始化日志系统
void init_log(const char* filename, config_info* cfg);

// 关闭日志系统
void close_log();

// 打印日志
void log_message(const char* file, int line, level log_level, const char* format, ...);

// 定义宏函数
#define D_log(log_level, format, ...) log_message(__FILE__, __LINE__, log_level, format, ##__VA_ARGS__)

#endif // _LOGMSG_