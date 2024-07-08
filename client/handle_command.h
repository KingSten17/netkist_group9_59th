#ifndef __HANDLE_COMMAND__
#define __HANDLE_COMMAND__

#include "config.h"
#include "header.h"
#include "logmsg.h"
#include "path_manager.h"

#define MAX_TOKEN_LENGTH 128

extern char global_token[MAX_TOKEN_LENGTH];

// 定义命令类型的枚举
typedef enum {
    CMD_REGISTER,  // 注册命令
    CMD_LOGIN,     // 登录命令
    CMD_LS,        // 列出文件命令
    CMD_CD,        // 切换目录命令
    CMD_PWD,       // 显示当前目录命令
    CMD_RM,        // 删除文件命令
    CMD_MKDIR,     // 创建目录命令
    CMD_RENAME,    // 重命名  ，需要传文件名
    CMD_PUTS,      // 上传文件命令
    CMD_GETS,      // 下载文件命令
    MESSAGE,       // 消息
    CMD_UNKNOWN,   // 未知命令
    CMD_NULL,       // 空命令
    CMD_QUIT        // 退出命令
} command_t;

typedef struct {
    const char *cmd_str;
    command_t cmd_type;
    int is_long_cmd;
} command_map_t;

// 在头文件中声明command_map数组，但不定义
extern command_map_t command_map[];


// 定义消息头结构体
typedef struct header_s {
    uint64_t task_id;
    command_t command; 
    char user[128];                 
    char parameter1[512];      
    char parameter2[512];
    off_t fileSize;
    bool flag;
    int sockfd;
} header_t;



int init_header(header_t *msg);
//初始化结构体


// 发送函数
int send_to(int sockfd, header_t *msg);

// 接收函数
int recv_from(int sockfd,header_t *msg);

//发送 结构体+文件
int transFile(int netfd,header_t *msg);

// 接收文件
int recv_file(int sockfd,header_t *msg);


int recvn(int sockfd, void *buf, long total);




int is_long_command(command_t command);

void handleServerMessage(int sockfd, header_t *server_msg);

void handleUserInput(int sockfd);

void *handleLongCommand(void *arg);

void handleShortCommand(int sockfd, header_t *msg);

void handleUserInput(int sockfd);

void handleExitSignal();









void handle_register(int sockfd,header_t *msg);
void handle_login(int sockfd,header_t *msg);
void handle_ls(int sockfd,header_t *msg);
void handle_cd(int sockfd,header_t *msg);
void handle_pwd(int sockfd,header_t *msg);
void handle_rm(int sockfd,header_t *msg) ;
void handle_mkdir(int sockfd,header_t *msg);
void handle_rename(int sockfd,header_t *msg) ;
void handle_puts(int sockfd,header_t *msg);
void handle_gets(int sockfd,header_t *msg);
void handle_message(int sockfd,header_t *msg);
void handle_quit(int sockfd,header_t *msg);
void handle_unknown(int sockfd,header_t *msg);


command_t get_command_type(const char *command);



#endif