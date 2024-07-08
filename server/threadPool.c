#include "threadPool.h"


int threadPoolInit(threadPool_t *pthreadPool, int workerNum){
    tidArrInit(&pthreadPool->tidArr,workerNum);
    taskQueueInit(&pthreadPool->taskQueue);
    pthread_mutex_init(&pthreadPool->mutex,NULL);
    pthread_cond_init(&pthreadPool->cond,NULL);
    pthreadPool->exitFlag = 0;
    return 0;
}
int makeWorker(threadPool_t *pthreadPool){
    for(int i = 0; i < pthreadPool->tidArr.workerNum; ++i){
        pthread_create(&pthreadPool->tidArr.arr[i], NULL,threadFunc, pthreadPool);
    }
    return 0;
}

int tcpInit(const char *ip,const char* port, int *psockfd){
    // socket setsockopt bind listen
    *psockfd = socket(AF_INET,SOCK_STREAM,0);
    int reuse = 1;
    int ret = setsockopt(*psockfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
    ERROR_CHECK(ret,-1,"setsockopt");
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(port));
    addr.sin_addr.s_addr = inet_addr(ip);
    ret = bind(*psockfd,(struct sockaddr *)&addr,sizeof(addr));
    ERROR_CHECK(ret,-1,"bind");
    listen(*psockfd,50);
    return 0;
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