#include "worker.h"
#include "taskQueue.h"
#include "threadPool.h"
#include "handle_command.h"



int tidArrInit(tidArr_t * ptidArr, int workerNum){
    // 申请内存 存储每个子线程的tid
    ptidArr->arr = (pthread_t *)calloc(workerNum,sizeof(pthread_t));
    ptidArr->workerNum = workerNum;
    return 0;
}
void unlock(void *arg){
    threadPool_t * pthreadPool = (threadPool_t *)arg;
    printf("unlock!\n");
    pthread_mutex_unlock(&pthreadPool->mutex);
}
void *threadFunc(void *arg){
    threadPool_t * pthreadPool = (threadPool_t *)arg;
    while(1){
        pthread_mutex_lock(&pthreadPool->mutex);
        int netfd;
        //pthread_cleanup_push(unlock,pthreadPool);
        while(pthreadPool->exitFlag == 0 && pthreadPool->taskQueue.queueSize <= 0){
            D_log(DEBUG,"线程池陷入等待");
            pthread_cond_wait(&pthreadPool->cond, &pthreadPool->mutex);
        }
        if(pthreadPool->exitFlag == 1){
            
            D_log(DEBUG,"线程正在准备退出");
            pthread_mutex_unlock(&pthreadPool->mutex);
            pthread_exit(NULL);
        }
        netfd = pthreadPool->taskQueue.pFront->netfd;
       
        D_log(DEBUG,"线程准备响应客户端%d", netfd);
        deQueue(&pthreadPool->taskQueue);
        
        D_log(DEBUG,"客户端%d已经被线程认领,将其移出任务队列", netfd);
        pthread_mutex_unlock(&pthreadPool->mutex);
        //pthread_cleanup_pop(1);
        // 执行业务
        //transFile(netfd,&msg); 
        
        handle_command(netfd);


        close(netfd);
    }
}



