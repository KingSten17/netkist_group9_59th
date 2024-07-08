#include "logmsg.h"

// 全局变量，用于日志文件和线程锁
static pthread_mutex_t lock; // 静态变量，仅在当前文件内可见
static FILE* log_file = NULL; // 静态变量，仅在当前文件内可见
static config_info* config = NULL; // 配置信息静态变量，仅在当前文件内可见
// 定义北京时区的偏移量（东八区）
#define BEIJING_TIME_OFFSET (8 * 3600)
/**
 * 初始化日志系统
 * @param filename 日志文件名
 * @param cfg 配置信息结构体
 */
void init_log(const char* filename, config_info* cfg) {
    // 初始化线程锁
    pthread_mutex_init(&lock, NULL);
    // 打开日志文件，追加模式
    log_file = fopen(filename, "a");
    if (!log_file) {
        // 如果文件打开失败，输出错误信息并退出程序
        perror("Failed to open log file");
        exit(EXIT_FAILURE);
    }
    // 保存配置信息
    config = cfg;
}

/**
 * 关闭日志系统
 */
void close_log() {
    // 如果日志文件已打开，关闭文件
    if (log_file) {
        fclose(log_file);
    }
    // 销毁线程锁
    pthread_mutex_destroy(&lock);
}

/**
 * 记录日志信息
 * @param log_level 日志级别
 * @param format 格式化字符串
 * @param ... 可变参数
 */
void log_message(const char* file, int line, level log_level, const char* format, ...) {
    // 检查日志级别
    if (log_level < config->log_level) {
        return;
    }

    // 加锁，确保线程安全
    pthread_mutex_lock(&lock);
    time_t raw_time;
    struct tm* local_time;

    // 获取当前时间
    time(&raw_time);
    // 调整为北京时间
    raw_time += BEIJING_TIME_OFFSET;
    // 转换为本地时间
    local_time = localtime(&raw_time);

    const char* log_type;
    switch (log_level) {
        case DEBUG: log_type = "DEBUG"; break;
        case INFO: log_type = "INFO"; break;
        case WARNING: log_type = "WARNING"; break;
        case ERROR: log_type = "ERROR"; break;
        default: log_type = "UNKNOWN"; break;
    }

    // 写入日志时间、级别、文件名和行号到文件
    fprintf(log_file, "[%04d-%02d-%02d %02d:%02d:%02d] [%s] [%s:%d]: ",
            local_time->tm_year + 1900, local_time->tm_mon + 1, local_time->tm_mday,
            local_time->tm_hour, local_time->tm_min, local_time->tm_sec,
            log_type, file, line);

    // 处理可变参数
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args); // 写入日志内容到文件
    fprintf(log_file, "\n");
    va_end(args);

    // 刷新文件缓冲区
    fflush(log_file);

    // 如果配置中 console_log 为 1，则打印到控制台
    if (config->console_log) {
        va_start(args, format);
        vprintf(format, args); // 打印日志内容到控制台
        printf("\n");
        va_end(args);
    }

    // 解锁
    pthread_mutex_unlock(&lock);
}

/*
测试用例
#include <stdio.h>
#include "config.h"
#include "logmsg.h"


int main() {
    config_info cfg;
    init_config(&cfg); // 初始化配置结构体，确保所有字段都被正确初始化

    // 加载配置文件 "config.ini" 中的配置信息
    if (load_config(&cfg) != 0) {
        fprintf(stderr, "Failed to load config file\n");
        return 1;
    }

    // 初始化日志系统，指定日志文件名和配置信息
    init_log("client.log", &cfg);

    // 测试日志输出
    log_message(__FILE__,__LINE__,INFO,"Application started. IP: %s, Port: %s", cfg.ip, cfg.port);
    log_message(__FILE__,__LINE__,DEBUG, "Debug message. User: %s", cfg.mysql_user);
    log_message(__FILE__,__LINE__,WARNING, "Warning message. Password: %s", cfg.mysql_password);
    log_message(__FILE__,__LINE__,ERROR, "Error message. Log type: %s" ,"transFile error");

    // 关闭日志系统
    close_log();

    return 0;
}


*/