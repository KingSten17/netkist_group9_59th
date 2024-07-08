#include "config.h"
#include "logmsg.h"

// 初始化 配置信息结构体
// 将结构体中的所有字段初始化为空字符串或默认值
void init_config(config_info* cfg_info) {
    if (cfg_info == NULL) return;

    // 初始化 IP 地址为空字符串
    strcpy(cfg_info->ip, "");
    // 初始化端口为空字符串
    strcpy(cfg_info->port, "");
    // 初始化 MySQL 用户名为空字符串
    strcpy(cfg_info->mysql_user, "");
    // 初始化 MySQL 密码为空字符串
    strcpy(cfg_info->mysql_password, "");
    // 初始化数据库端口为空字符串
    strcpy(cfg_info->mysql_port, "");
    // 初始化线程数为1
    cfg_info->thread_num = 1;
    // 初始化是否打印到控制台标志为 0 (不打印)
    cfg_info->console_log = 0;
    // 初始化日志等级为 INFO
    cfg_info->log_level = INFO;
}

// 读取配置信息，存储在 配置信息结构体 中
// 从本地文件 "config.ini" 中读取配置信息，并存储到 cfg_info 结构体中
int load_config(config_info* cfg_info) {
    // 检查传入的参数是否为 NULL
    if (cfg_info == NULL) {
        return -1; // 如果参数为 NULL，返回错误码 -1
    }

    // 打开配置文件，文件名为 "config.ini"，模式为 "r"（读取模式）
    FILE *file = fopen("config.ini", "r");
    if (file == NULL) {
        // 如果文件打开失败，输出错误信息并返回错误码 -1
        perror("Failed to open config file");
        return -1;
    }

    char line[256]; // 用于存储读取的每一行内容
    // 循环读取文件中的每一行，直到文件结束
    while (fgets(line, sizeof(line), file)) {
        char key[64], value[64]; // 用于存储键和值
        // 解析每一行，格式为 "key=value"，并将 key 和 value 分别存储到 key 和 value 字符串中
        if (sscanf(line, "%63[^=]=%63s", key, value) == 2) {
            // 根据解析出的键来确定将值存储到结构体的哪个字段中
            if (strcmp(key, "ip") == 0) {
                strncpy(cfg_info->ip, value, sizeof(cfg_info->ip) - 1);
                cfg_info->ip[sizeof(cfg_info->ip) - 1] = '\0'; // 确保字符串以 NULL 结尾
            } else if (strcmp(key, "port") == 0) {
                strncpy(cfg_info->port, value, sizeof(cfg_info->port) - 1);
                cfg_info->port[sizeof(cfg_info->port) - 1] = '\0'; // 确保字符串以 NULL 结尾
            } else if (strcmp(key, "mysql_user") == 0) {
                strncpy(cfg_info->mysql_user, value, sizeof(cfg_info->mysql_user) - 1);
                cfg_info->mysql_user[sizeof(cfg_info->mysql_user) - 1] = '\0'; // 确保字符串以 NULL 结尾
            } else if (strcmp(key, "mysql_password") == 0) {
                strncpy(cfg_info->mysql_password, value, sizeof(cfg_info->mysql_password) - 1);
                cfg_info->mysql_password[sizeof(cfg_info->mysql_password) - 1] = '\0'; // 确保字符串以 NULL 结尾
            } else if (strcmp(key, "mysql_port") == 0) {
                strncpy(cfg_info->mysql_port, value, sizeof(cfg_info->port) - 1);
                cfg_info->port[sizeof(cfg_info->port) - 1] = '\0'; // 确保字符串以 NULL 结尾
            }else if (strcmp(key, "thread_num") == 0) {
                // 将 console_log 字符串转换为整数，并存储到结构体中
                cfg_info->thread_num = atoi(value);
            }else if (strcmp(key, "console_log") == 0) {
                // 将 console_log 字符串转换为整数，并存储到结构体中
                cfg_info->console_log = atoi(value);
            } else if (strcmp(key, "log_level") == 0) {
                // 根据配置文件中的日志等级字符串，将其转换为对应的枚举值
                if (strcmp(value, "DEBUG") == 0) {
                    cfg_info->log_level = DEBUG;
                } else if (strcmp(value, "INFO") == 0) {
                    cfg_info->log_level = INFO;
                } else if (strcmp(value, "WARNING") == 0) {
                    cfg_info->log_level = WARNING;
                } else if (strcmp(value, "ERROR") == 0) {
                    cfg_info->log_level = ERROR;
                }
            }
        }
    }

    // 关闭配置文件
    fclose(file);
    return 0; // 返回 0 表示成功
}
