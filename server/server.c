
#include "threadPool.h"
#include "handle_command.h"

int exitPipe[2]; // 声明 exitPipe 变量，用于创建管道


// 定义 handler 函数，用于处理 SIGUSR1 信号
void handler(int signum){
    printf("%d号信号来了 \n", signum);
    write(exitPipe[1],"1",1);
}


int main()
{


    config_info cfg;
    init_config(&cfg); // 初始化配置结构体，确保所有字段都被正确初始化

    // 加载配置文件 "config.ini" 中的配置信息
    if (load_config(&cfg) != 0) {
        fprintf(stderr, "加载配置文件失败\n");
        return 1;
    }

    // 初始化日志系统，指定日志文件名和配置信息
    init_log("server.log", &cfg);
    D_log(INFO,"服务器已启动");



    pipe(exitPipe);//父子进程之间的管道
    if(fork()!= 0){
        close(exitPipe[0]);//父进程关闭都端
        signal(2,handler);
        wait(NULL);
        
        D_log(DEBUG,"线程池已关闭，父进程即将关闭");
        exit(0);
    }
    // 只有子进程 才能创建线程池
    close(exitPipe[1]);//子进程关闭写端
    threadPool_t threadPool;
    threadPoolInit(&threadPool, cfg.thread_num);
    makeWorker(&threadPool);

    int sockfd;
    tcpInit(cfg.ip,cfg.port,&sockfd);

    int epfd = epoll_create(1);
    epollAdd(epfd,sockfd);
    epollAdd(epfd,exitPipe[0]);
    while(1){
        struct epoll_event readySet[1024];
        int readyNum = epoll_wait(epfd,readySet,1024,-1);
        for(int i = 0; i < readyNum; ++i){
            if(readySet[i].data.fd == sockfd){
                int netfd = accept(sockfd,NULL,NULL);
                
                D_log(INFO,"客户端=%d已经连接",netfd);
                // 分配任务
                pthread_mutex_lock(&threadPool.mutex);
                enQueue(&threadPool.taskQueue,netfd);
                
                D_log(DEBUG,"父进程把客户端%d交给任务队列", netfd);
                pthread_mutex_unlock(&threadPool.mutex);
                pthread_cond_broadcast(&threadPool.cond);
                D_log(DEBUG,"父进程做了广播，以便让子线程醒来");
               
            }
            else if(readySet[i].data.fd == exitPipe[0]){
                // 线程池知道信号来了
                
                D_log(DEBUG,"收到一个退出信号");
                //for(int j = 0; j < threadPool.tidArr.workerNum; ++j){
                //    pthread_cancel(threadPool.tidArr.arr[j]);
                //    
                //}
                pthread_mutex_lock(&threadPool.mutex);
                threadPool.exitFlag = 1;
                pthread_cond_broadcast(&threadPool.cond);
                
                D_log(DEBUG,"广播退出信号");
                pthread_mutex_unlock(&threadPool.mutex);
                for(int j = 0; j < threadPool.tidArr.workerNum; ++j){
                    pthread_join(threadPool.tidArr.arr[j],NULL);
                }
                D_log(DEBUG,"线程池已关闭");
                exit(0);
            }
        }
    }
    return 0;
}



