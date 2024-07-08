#include "handle_command.h"
#include <unistd.h>// 使用access函数


int handle_command(int netfd){
while (1) {
        header_t msg;
        memset(&msg, 0, sizeof(header_t));
        recv_from(netfd,&msg);
         // 解析命令类型
        
        switch (msg.command) {
            case CMD_REGISTER:
                
                D_log(INFO,"客户端请求注册");
                // 处理注册命令
                break;
            case CMD_LOGIN:
               
                D_log(INFO,"客户端请求登陆");
                // 处理登录命令
                break;
            case CMD_LS:
                
                D_log(INFO,"客户端请求ls"); 
                // 处理登录命令
                command_ls(netfd,&msg);
                break;
            case CMD_CD:
                
                D_log(INFO,"客户端请求cd"); 
                // 处理登录命令
                break;
            case CMD_PWD:
                
                D_log(INFO,"客户端请求pwd"); 
                // 处理登录命令
                break;
            case CMD_RM:
                
                D_log(INFO,"客户端请求rm"); 
                // 处理登录命令
                break;
            case CMD_MKDIR:
                
                D_log(INFO,"客户端请求mkdir");
                // 处理登录命令
                break;

            // 其他命令处理...
            case CMD_QUIT:
                // 客户端请求退出
                
                D_log(INFO,"客户端请求退出");
                close(netfd);
                return 0;; // 结束处理该客户端连接
            default:
                printf("未知命令\n");
                break;
        }


        char current_dir[1024];
        char file_dir[128];
        char user_dir[128];

        // 获取当前工作目录
        if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
            perror("getcwd");
            return EXIT_FAILURE;
        }

        // 接收客户端发送的命令
        if (recv_from(netfd, &msg) == -1) {
            //perror("Error receiving command from client");
            D_log(ERROR,"接受客户端命令失败");
            break; // 如果接收失败，可以考虑断开连接或其他处理
        }

       
    }
}



/* Useage:  */
int command_ls(int netfd,header_t *msg)
{
    char buf[1024] = {0};
    strcpy(buf,msg->parameter1);
    if(strcmp(buf,"ls")==0)
    {
        //处理ls逻辑
        char *str = getcwd(NULL,0);
        char path[1024] = {0};
        FILE *fp = popen(str,"r");
        if(fp==NULL)
        {
            //服务器读取错误
            D_log(ERROR,"command:ls failed!");
            return -1;
        }
        while(fgets(path,sizeof(path),fp)!=NULL)
        {
            //send(netfd,path,strlen(path),0);
            //head->command==CMD_LS;
            strcpy(msg->parameter1,path);
            msg->flag = true;
            send_to(netfd,msg);
        }
    }else{
        char error[50] = "Wrong ls command!";
        strcpy(msg->parameter1,error);
        msg->fileSize = strlen(error);
        send_to(netfd,msg);
        D_log(ERROR,"command:format wrong!");
    }
    close(netfd);

    return 0;
}

int directory_exists(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        // 检查stat函数是否成功
        return 0;
    }
    // 检查是否为目录
    return S_ISDIR(statbuf.st_mode);
}


int command_register(int netfd,header_t *msg)
{
    /*
    1， 查看文件夹存在不存在
    2， 存在的话返回用户已经注册
        不存在的话
    
    
     */
}









//发送一切
int send_to(int sockfd, header_t *msg) {
    
    if(send(sockfd,msg,sizeof(*msg),MSG_NOSIGNAL)==-1){
        //perror("send failed in transFile\n");
        D_log(ERROR,"发送失败");
        return -1;}
    
    D_log(DEBUG,"发送成功");
    return 0;
}


//发送文件的实现
int transFile(int netfd,header_t *msg){


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
        D_log(ERROR,"接受信息失败");

        return -1;

    }
    return 0;

}

int recv_file(int sockfd,header_t *msg){
    char filename[500] = {0};
    // 先收消息头结构体
    
    if(recvn(sockfd,msg,sizeof(*msg))!= 0){
        //perror("recvn failed in recv_from\n");
        D_log(ERROR,"接受信息失败");

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
        D_log(ERROR,"创建本地文件失败");
        close(fd);
        return -1;
    }
    char *p = (char *)mmap(NULL,filesize,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    ERROR_CHECK(p,MAP_FAILED,"mmap");
    if(recvn(sockfd,p,filesize)!=0){
        //perror("recvn failed in recv_file when recv file\n");
        D_log(ERROR,"接受%s文件时失败",filename);
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
