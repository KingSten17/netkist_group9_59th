#include <string.h>
#include <pthread.h>
#include "path_manager.h"

// 全局字符数组用于存放当前路径
static char current_path[MAX_PATH_LENGTH] = "/home";

// 互斥锁用于保护对全局字符数组的访问
static pthread_mutex_t path_mutex = PTHREAD_MUTEX_INITIALIZER;

// 初始化当前路径
void init_current_path() {
    pthread_mutex_lock(&path_mutex);
    strncpy(current_path, "/home", MAX_PATH_LENGTH);
    pthread_mutex_unlock(&path_mutex);
}

// 设置当前路径
void set_current_path(const char *new_path) {
    pthread_mutex_lock(&path_mutex);
    strncpy(current_path, new_path, MAX_PATH_LENGTH - 1);
    current_path[MAX_PATH_LENGTH - 1] = '\0'; // 确保以'\0'结尾
    pthread_mutex_unlock(&path_mutex);
}

// 获取当前路径（可以用于调试或其他用途）
void get_current_path(char *buffer, size_t buffer_size) {
    pthread_mutex_lock(&path_mutex);
    strncpy(buffer, current_path, buffer_size - 1);
    buffer[buffer_size - 1] = '\0'; // 确保以'\0'结尾
    pthread_mutex_unlock(&path_mutex);
}

// 打印当前路径提示符并刷新输出缓冲区
void print_current_path_prompt() {
    printf("%s$:", current_path);
    fflush(stdout);
}