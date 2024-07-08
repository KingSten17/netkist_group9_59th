

#include "header.h"
#include "logmsg.h"
#include "config.h"
#include "handle_command.h"
#include "path_manager.h"


int exitPipe[2]; // 声明 exitPipe 变量，用于创建管道

int short_sockfd, long_sockfd;
pthread_mutex_t mutex;


char cur_dir[1024] = "/";
//当前路径

pthread_mutex_t path_mutex = PTHREAD_MUTEX_INITIALIZER;
//互斥锁用于保护对全局字符数组的访问
    


// 定义 handler 函数，用于处理 SIGUSR1 信号
void handler(int signum){
    printf("%d号信号来了 \n", signum);
    write(exitPipe[1],"1",1);
}

int epollAdd(int epfd, int fd){
    struct epoll_event events;
    events.events = EPOLLIN;
    events.data.fd = fd;
    epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&events);
    return 0;
}
int epollDel(int epfd, int fd){
    epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL);
    return 0;
}


int main()
{


 
    init_config(&cfg); // 初始化配置结构体，确保所有字段都被正确初始化

    // 加载配置文件 "config.ini" 中的配置信息
    if (load_config(&cfg) != 0) {
        fprintf(stderr, "Failed to load config file\n");
        return 1;
    }

    // 初始化日志系统，指定日志文件名和配置信息
    init_log("client.log", &cfg);
    D_log(INFO,"客户端已启动");



    pipe(exitPipe);//父子进程之间的管道
    if(fork()!= 0){
        close(exitPipe[0]);//父进程关闭都端
        signal(2,handler);
        wait(NULL);
        
        D_log(DEBUG,"线程池已关闭，父进程即将关闭");
        exit(0);
    }

   
    /*
     子进程执行下面的逻辑
     1，根据配置文件建立socket连接
     2，创建线程池，
     3，让线程陷入等待
     4，给他们分配醒来后的任务函数
    
    
    */
    close(exitPipe[1]);//子进程关闭写端
    

    //建立短命令连接
    short_sockfd = socket(AF_INET,SOCK_STREAM,0);
    //错误判断
    
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(cfg.port));
    serverAddr.sin_addr.s_addr = inet_addr(cfg.ip);
    int ret = connect(short_sockfd,(struct sockaddr *)&serverAddr,sizeof(serverAddr));
    ERROR_CHECK(ret,-1,"connect");
    D_log(INFO,"服务器%d已经连接\n欢迎使用C++59th_group9网盘\n",ret);
    
    

    header_t share_msg;
    memset(&share_msg, 0, sizeof(header_t));


    int epfd = epoll_create(1);
    ERROR_CHECK(epfd, -1, "epoll_create");
    epollAdd(epfd,short_sockfd);
    epollAdd(epfd,STDIN_FILENO);
    epollAdd(epfd,exitPipe[0]);

    print_current_path_prompt();

    while(1){
        struct epoll_event readySet[10];
        int readyNum = epoll_wait(epfd,readySet,10,-1);
        //做错误处理

        for(int i = 0; i < readyNum; ++i){
            if(readySet[i].data.fd == short_sockfd){
                // 服务器发信息，通过 socket
                header_t server_msg;
                memset(&server_msg, 0, sizeof(header_t));
                handleServerMessage(short_sockfd, &server_msg);

            }else if(readySet[i].data.fd == exitPipe[0]){
                // 父进程接到信号，通过管道传过来
        
                D_log(DEBUG, "收到一个退出信号");
                handleExitSignal();
                D_log(DEBUG, "客户端即将关闭");
                exit(0);


            }else if(readySet[i].data.fd == STDIN_FILENO){
                // 用户输入,通过输入缓冲区
                // 用户输入，通过输入缓冲区
                handleUserInput(short_sockfd);
                
            }
        }
    }
    close(short_sockfd);
    close(epfd);
    return 0;


    
}



