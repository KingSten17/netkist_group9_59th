#ifndef PATH_MANAGER_H
#define PATH_MANAGER_H

#include <stddef.h>
#include <stdio.h>

// 定义最大路径长度
#define MAX_PATH_LENGTH 512

// 函数声明
void init_current_path();
void set_current_path(const char *new_path);
void get_current_path(char *buffer, size_t buffer_size);
void print_current_path_prompt();

#endif // PATH_MANAGER_H