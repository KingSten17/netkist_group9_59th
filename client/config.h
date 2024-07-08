#ifndef _CONFIG_
#define _CONFIG_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 配置信息结构体
typedef struct configInfo {
    char ip[16];
    char port[6];
    int thread_num;  //线程池中的线程数量
    int console_log; // 是否打印到控制台（0: 不打印, 1: 打印）
    int log_level;   // 日志等级（DEBUG, INFO, WARNING, ERROR）
} config_info;


extern config_info cfg;

// 读取配置信息，存储在 配置信息结构体 中
int load_config(config_info* cfg_info);

// 初始化 配置信息结构体
void init_config(config_info* cfg_info);

#endif // _CONFIG_
