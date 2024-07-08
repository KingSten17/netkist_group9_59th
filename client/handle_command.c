
#include "handle_command.h"

char global_token[MAX_TOKEN_LENGTH];
// 映射命令字符串到枚举类型
command_t get_command_type(const char *command) {
    if (strcmp(command, "register") == 0||strcmp(command, "注册") == 0) return CMD_REGISTER;
    if (strcmp(command, "login")== 0||strcmp(command, "登陆") == 0)  return CMD_LOGIN;
    if (strcmp(command, "ls") == 0) return CMD_LS;
    if (strcmp(command, "cd") == 0) return CMD_CD;
    if (strcmp(command, "pwd") == 0) return CMD_PWD;
    if (strcmp(command, "rm") == 0) return CMD_RM;
    if (strcmp(command, "mkdir") == 0) return CMD_MKDIR;
    if (strcmp(command, "rename") == 0) return CMD_RENAME;
    if (strcmp(command, "puts") == 0 ||strcmp(command, "上传") == 0) return CMD_PUTS;
    if (strcmp(command, "gets") == 0 ||strcmp(command, "下载") == 0)return CMD_GETS;
    if (strcmp(command, "message")== 0 ||strcmp(command, "发消息") == 0) return MESSAGE;
    if (strcmp(command, "quit") == 0||strcmp(command, "退出") == 0) return CMD_QUIT;
    return CMD_UNKNOWN;
}

command_map_t command_map[] = {
    {"register", CMD_REGISTER, 1},
    {"login", CMD_LOGIN, 1},
    {"ls", CMD_LS, 0},
    {"cd", CMD_CD, 0},
    {"pwd", CMD_PWD, 0},
    {"rm", CMD_RM, 0},
    {"mkdir", CMD_MKDIR, 0},
    {"rename", CMD_RENAME, 1},
    {"puts", CMD_PUTS, 1},
    {"gets", CMD_GETS, 1},
    {"message", MESSAGE, 1},
    {"quit", CMD_QUIT, 0},
    {NULL, CMD_UNKNOWN, 0} // 用于未知命令和结束标志
};

void handleServerMessage(int sockfd, header_t *server_msg) {
    // 处理服务器发来的信息
    // ...
}

void *handleLongCommand(void *arg) {
   // 处理长命令的线程函数
    header_t *msg = (header_t *)arg;
    int sockfd = msg->sockfd;

    // 创建新的 socket 并连接到服务器
    int new_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (new_sockfd < 0) {
        perror("socket");
        pthread_exit(NULL);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(cfg.port));
    server_addr.sin_addr.s_addr = inet_addr(cfg.ip);

    if (connect(new_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(new_sockfd);
        pthread_exit(NULL);
    }
    D_log(INFO,"已经启用与服务器之间的长连接");

    // 执行长命令的处理逻辑
    switch (msg->command) {
        case CMD_PUTS:
            handle_puts(new_sockfd, msg);
            break;
        case CMD_GETS:
            handle_gets(new_sockfd, msg);
            break;
        default:
            break;
    }

    close(new_sockfd);
    free(msg); // 释放传入的消息结构体内存
    pthread_exit(NULL);  // 处理完毕后退出
}

void handleShortCommand(int sockfd, header_t *msg) {
    // 处理短命令
    // 转到各自的处理逻辑
    // 参数对不对，各自检测
    
    
    switch (msg->command) {
        case CMD_REGISTER:
            handle_register(sockfd,msg);
            break;
        case CMD_LOGIN:
            handle_login(sockfd,msg);
            break;
        case CMD_LS:
            handle_ls(sockfd,msg);
            break;
        case CMD_CD:           
            handle_cd(sockfd,msg);
            break;
        case CMD_PWD:
            handle_pwd(sockfd,msg);
            break;
        case CMD_RM:
            handle_rm(sockfd,msg);
            break;
        case CMD_MKDIR:
            handle_mkdir(sockfd,msg);
            break;
        case CMD_RENAME:
            handle_rename(sockfd,msg);
            break;
        case MESSAGE:
            handle_message(sockfd,msg);
            break;
        case CMD_UNKNOWN:
            handle_unknown(sockfd,msg);
            break;
        case CMD_NULL:
            break;
        case CMD_QUIT:
            handleExitSignal();
            return ;
    }

}






void handleExitSignal() {
    D_log(DEBUG, "收到一个退出信号");
    D_log(DEBUG, "客户端即将关闭");
    exit(0);
}



//处理用户输入，从socket读数据到结构体msg中，判断命令类型，分为长短命令，传递sockfd和结构体指针，分别走相应的逻辑
void handleUserInput(int sockfd) 
{
    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) 
    {
        // 判断命令类型
        // 去掉换行符
        buffer[strcspn(buffer, "\n")] = 0;
        // 解析命令和参数
        char *command_str = strtok(buffer, " ");
        char *arg1 = strtok(NULL, " ");
        char *arg2 = strtok(NULL, " ");

        if (command_str == NULL) 
        {
            return;
        }
        
        //D_log(INFO,"命令 = %s", command_str);
        command_t command = get_command_type(command_str);
        if (command == CMD_UNKNOWN || command == CMD_NULL) 
        {
            printf("未知命令或无效命令\n");
            print_current_path_prompt();
            return;  // 可以选择继续循环等待下一个输入
        }
        header_t msg;
        memset(&msg, 0, sizeof(header_t));
        msg.command = command;


        if (arg1 != NULL) {
            //printf("参数1 = %s\n", arg1);
            strncpy(msg.parameter1, arg1, sizeof(msg.parameter1) - 1);
            msg.parameter1[sizeof(msg.parameter1) - 1] = '\0';  // 确保以'\0'结尾
        }

        if (arg2 != NULL) {
            //printf("参数2 = %s\n", arg2);
            strncpy(msg.parameter2, arg2, sizeof(msg.parameter2) - 1);
            msg.parameter2[sizeof(msg.parameter2) - 1] = '\0';  // 确保以'\0'结尾
        }

        if (is_long_command(command)){
            // 长命令创建新线程处理
            pthread_t tid;
            pthread_create(&tid, NULL, handleLongCommand, (void *)&msg);
            pthread_detach(tid);  // 分离线程，无需主线程等待

        }else{
            //短命令，正常处理
            handleShortCommand(sockfd, &msg);
            }

        
    } else {
        D_log(ERROR,"fget 失败");
        return;
    }
}


    


int is_long_command(command_t command) {
    // 判断是否为长命令
    // 假设 CMD_PUTS 和 CMD_GETS 是长命令
    return (command == CMD_PUTS || command == CMD_GETS);
}



//客户端注册逻辑
void handle_register(int sockfd,header_t *msg){
 /*  
typedef struct header_s{
    command_t command;        // CMD_REGISTER
    char filename[500];       // 填写用户名
    char parameter[500];      // 填写密码
    bool flag                 // 当返回true的时候注册成功
} header_t;
*/

    char username[128];
    char password[256];
    char password2[256];
    printf("请输入用户名: ");
    if (fgets(username, sizeof(username), stdin) == NULL) {
        D_log(ERROR, "用户名读入失败");
        return;
    }
    // 去除fgets函数可能会带来的换行符
    username[strcspn(username, "\n")] = 0;


    //待做，对于密码的控制模块
    printf("请输入密码: ");
    if (fgets(password, sizeof(password), stdin) == NULL) {
        D_log(ERROR, "密码读入失败");
        return;
    }
    // 去除fgets函数可能会带来的换行符
    password[strcspn(password, "\n")] = 0;

    printf("请再次输入密码: ");
    if (fgets(password2, sizeof(password2), stdin) == NULL) {
        D_log(ERROR, "密码读入失败");
        return;
    }
    password2[strcspn(password2, "\n")] = 0; // 去除换行符

    if (strcmp(password, password2) == 0) {
        printf("密码设置成功！\n");
        strcpy(msg->parameter1, username);
        strcpy(msg->parameter2, password);

        // 确保字符串以 '\0' 结尾
        msg->parameter1[sizeof(msg->parameter1) - 1] = '\0';
        msg->parameter2[sizeof(msg->parameter2) - 1] = '\0';


        send_to(sockfd,msg);
        D_log(INFO,"客户端发起注册");
        recv_from(sockfd,msg);

        if(msg->flag){
            D_log(INFO,"注册成功");
        }else{
            D_log(WARNING,"注册失败");
            return;
        }

    } else {
        printf("两次输入的密码不一致，请重新输入。\n");
        return;
    }
}



//客户端登陆逻辑
void handle_login(int sockfd,header_t *msg)
{
    char username[128];
    char password[256];
    printf("请输入用户名: ");
    if (fgets(username, sizeof(username), stdin) == NULL) {
        D_log(ERROR, "用户名读入失败");
        return;
    }
    // 去除fgets函数可能会带来的换行符
    username[strcspn(username, "\n")] = 0;


    //待做，对于密码的控制模块
    printf("请输入密码: ");
   if (fgets(password, sizeof(password), stdin) == NULL) {
        D_log(ERROR, "密码读入失败");
        return;
    }
    // 去除fgets函数可能会带来的换行符
    password[strcspn(password, "\n")] = 0;

    // 将用户名和密码放入消息结构体中
    strncpy(msg->parameter1, username, sizeof(msg->parameter1) - 1);
    strncpy(msg->parameter2, password, sizeof(msg->parameter2) - 1);

    // 确保字符串以 '\0' 结尾
    msg->parameter1[sizeof(msg->parameter1) - 1] = '\0';
    msg->parameter2[sizeof(msg->parameter2) - 1] = '\0';

    send_to(sockfd,msg);


    recv_from(sockfd,msg);

    if (msg->command == CMD_LOGIN && msg->flag == true) {
        D_log(INFO, "登录成功！");
        // 处理登录成功后的操作，例如存储 token
        // 这里假设服务器返回的 token 存储在 msg->parameter1 中
        strncpy(global_token, msg->parameter1, sizeof(global_token) - 1);
        global_token[sizeof(global_token) - 1] = '\0';  // 确保字符串以 '\0' 结尾
    } else {
        D_log(ERROR, "登录失败\n");
    }

}
void handle_ls(int sockfd,header_t *msg){

    strcpy(msg->user,global_token);

    char current_path[MAX_PATH_LENGTH];
    get_current_path(current_path, sizeof(current_path));

    strcpy(msg->parameter1,current_path);
    send_to(sockfd,msg);

    recv_from(sockfd,msg);
    if(msg->flag){
        D_log(INFO,"ls命令成功");
        printf("%s",msg->parameter1);
        

    }else{
        D_log(ERROR,"ls命令失败");
    }
    print_current_path_prompt();
    
    return;


}


void handle_cd(int sockfd,header_t *msg)
{
    // 获取目标路径
    
    const char *new_dir = msg->parameter1;

    char current_path[MAX_PATH_LENGTH];
    get_current_path(current_path, sizeof(current_path));
    
    char new_path[MAX_PATH_LENGTH];
    
     if (new_dir[0] == '/') {
        // 绝对路径
        strncpy(new_path, new_dir, sizeof(new_path) - 1);
        new_path[sizeof(new_path) - 1] = '\0';  // 确保以'\0'结尾
    } else {
        // 相对路径
        if (strcmp(new_dir, ".") == 0) {
            // 当前目录，路径不变
            strncpy(new_path, current_path, sizeof(new_path) - 1);
            new_path[sizeof(new_path) - 1] = '\0';  // 确保以'\0'结尾
        } else if (strcmp(new_dir, "..") == 0) {
            // 上级目录，路径回退
            char *last_slash = strrchr(current_path, '/');
            if (last_slash != NULL) {
                *last_slash = '\0';  // 截断路径，去掉最后一个目录名及其前面的 '/'
            } else {
                // 如果没有 '/'，说明当前路径已经是根目录，不能再回退
                D_log(ERROR, "Cannot go up from root directory\n");
                return;
            }
            strncpy(new_path, current_path, sizeof(new_path) - 1);
            new_path[sizeof(new_path) - 1] = '\0';  // 确保以'\0'结尾
        } else {
            // 其他相对路径
            int path_length = snprintf(new_path, sizeof(new_path), "%s/%s", current_path, new_dir);
            if (path_length >= sizeof(new_path)) {
                // 如果路径长度超过缓冲区大小，返回错误
                D_log(ERROR, "Path too long\n");
                
                
                return;
            }
        }
    }


    // 设置新的当前路径
    set_current_path(new_path);

    // 发送成功消息给客户端
    print_current_path_prompt();
    // 检查目录是否存在
    /*if (access(real_path, F_OK) != 0) {
        D_log(ERROR, "Directory does not exist: %s\n", strerror(errno));
        const char *error_msg = "Directory does not exist\n";
        send(sockfd, error_msg, strlen(error_msg), 0);
        return;
    }*/

    // 设置新的当前路径
    //set_current_path(new_path);

    // 发送成功消息给客户端
    //D_log(INFO, "更改路径成功");
    //print_current_path_prompt();
}

void handle_pwd(int sockfd,header_t *msg)
{
    char current_path[MAX_PATH_LENGTH];
    get_current_path(current_path, sizeof(current_path));

    printf("%s\n",current_path);

    // 发送当前路径给客户端
    print_current_path_prompt();

}

//删除文件和文件夹的逻辑
void handle_rm(int sockfd,header_t *msg) 
{
    //rm 1.txt            rm  fold1
    char current_path[MAX_PATH_LENGTH];
    get_current_path(current_path, sizeof(current_path));
        
    // 将当前目录放入消息结构体中
    
    strncpy(msg->parameter2, current_path, sizeof(msg->parameter2) - 1);

    // 确保字符串以 '\0' 结尾
    msg->parameter2[sizeof(msg->parameter2) - 1] = '\0';

    send_to(sockfd,msg);

    recv_from(sockfd,msg);

    if (msg->flag) {
        // 如果服务器确认文件信息，提示用户确认删除
        printf("文件信息如下:\n%s\n确定要删除吗?(Y/N)\n", msg->parameter1);
        print_current_path_prompt();

        char confirm[3];
        fgets(confirm, sizeof(confirm), stdin);
        // 去除fgets函数可能会带来的换行符
        confirm[strcspn(confirm, "\n")] = 0;

        if (strcmp(confirm, "Y") == 0 || strcmp(confirm, "y") == 0) {
            // 用户确认删除，发送删除确认消息到服务器
            msg->flag = true;  // 表示用户确认删除
            send_to(sockfd, msg);
            
            // 接收最终的删除结果
            recv_from(sockfd, msg);
            if (msg->flag) {
                printf("文件删除成功。\n");
            } else {
                printf("文件删除失败。\n");
            }
        } else {
            // 用户取消删除
            printf("取消删除。\n");
        }
    } else {
        // 如果服务器没有确认文件信息
        printf("文件或文件夹不存在。\n");
    }

    print_current_path_prompt();
}



//创建文件夹逻辑
void handle_mkdir(int sockfd,header_t *msg)
{
    printf("待开发");
}


//文件或者文件夹重命名逻辑
void handle_rename(int sockfd,header_t *msg) 
{
    printf("待开发");
}

//上传文件逻辑
void handle_puts(int sockfd,header_t *msg)
{
    printf("待开发");
}

//下载文件逻辑
void handle_gets(int sockfd,header_t *msg)
{
    printf("待开发");
}

//发送信息逻辑
void handle_message(int sockfd,header_t *msg)
{
    printf("待开发");
}

//用户退出逻辑
void handle_quit(int sockfd,header_t *msg)
{
    handleExitSignal();
}

//未知命令逻辑
void handle_unknown(int sockfd,header_t *msg)
{
    printf("未知命令，请检查输入\n");
    print_current_path_prompt();
}




//发送结构体到socket
int send_to(int sockfd, header_t *msg) {
   
    if(send(sockfd,msg,sizeof(*msg),MSG_NOSIGNAL)==-1){
        //perror("send failed in transFile\n");
        D_log(ERROR,"发送失败");
        return -1;}
    
    D_log(DEBUG,"发送成功");
        
    return 0;
}


//发送文件,先发一个结构体告诉对方文件大小，在用sendfile发送文件
int transFile(int netfd,header_t *msg){

    //parameter1 为文件名


    struct stat statbuf;//定义一个 stat 结构体变量 statbuf，用于存储文件的状态信息
    

    int fd = open(msg->parameter1,O_RDWR);//以读写模式（O_RDWR）打开文件 filename，返回文件描述符 fd
    if(fd == -1){
        //perror("open failed in transFile\n");
        D_log(ERROR,"在发送%s时未能找到或打开该文件",msg->parameter1);
        return -1;
    }

    int fs = fstat(fd,&statbuf);//获取文件描述符 fd 指向文件的状态信息，并将其存储在 statbuf 中。
                                // statbuf 结构体包含文件的大小、权限、最后修改时间等信息。
    if(fs == -1){
        //perror("fstat failed in transFile\n");
        D_log(ERROR,"获取%s的信息时出现错误",msg->parameter1);
        close(fd);
        return -1;
    }
    msg->fileSize = statbuf.st_size;

    if(send(netfd,msg,sizeof(*msg),MSG_NOSIGNAL)==-1){
        //发送消息头
        //perror("send failed in transFile\n");
        D_log(ERROR,"发送%s的信息时出现错误",msg->parameter1);
        close(fd);
        return -1;

    }
    if (sendfile(netfd, fd, NULL, statbuf.st_size) == -1) {//发送文件
        //perror("sendfile failed in transFile\n");
        D_log(ERROR,"发送%s时出错",msg->parameter1);
        close(fd);
        return -1;
    }
    close(fd);
    return 0;    
}




int recv_from(int sockfd,header_t *msg){

    if(recvn(sockfd,msg,sizeof(*msg))!= 0){
        //perror("recvn failed in recv_from\n");
        D_log(ERROR,"recvn接受信息失败");

        return -1;

    }
    D_log(DEBUG,"recvn接受信息成功");
    return 0;

}

int recv_file(int sockfd,header_t *msg){
    char filename[500] = {0};
    // 先收消息头结构体
    
    if(recvn(sockfd,msg,sizeof(*msg))!= 0){
        //perror("recvn failed in recv_from\n");
        D_log(ERROR,"recv_file接受文件大小失败");

        return -1;

    }
    
    
    int name_length =strlen(msg->parameter1);
    memcpy(filename,msg->parameter1,name_length);
    filename[name_length] = '\0';
    off_t filesize = msg->fileSize;    
    printf("filesize = %ld\n", filesize);
    int fd = open(filename,O_CREAT|O_RDWR|O_TRUNC,0666);
    if (ftruncate(fd, filesize) == -1) { // 先修改文件大小
        //perror("ftruncate failed in recv_file\n");
        D_log(ERROR,"recv_file创建本地文件失败");
        close(fd);
        return -1;
    }
    char *p = (char *)mmap(NULL,filesize,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    ERROR_CHECK(p,MAP_FAILED,"mmap");
    if(recvn(sockfd,p,filesize)!=0){
        //perror("recvn failed in recv_file when recv file\n");
        D_log(ERROR,"recv_file接受%s文件时失败",filename);
        close(fd);
        return -1;
    }

    munmap(p,filesize);
    close(fd);
    return 0;
}



int recvn(int sockfd, void *buf, long total){
    char *p = (char *)buf;
    long cursize = 0;
    while(cursize < total){
        ssize_t sret = recv(sockfd,p+cursize,total-cursize,0);
        if(sret == 0){
            return 1;
        }
        cursize += sret;
    }
    return 0;
}


int init_header(header_t* msg){
    //初始化消息头
    memset(msg, 0, sizeof(header_t));
    return 0;

}
