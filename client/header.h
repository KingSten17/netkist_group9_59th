#ifndef __HEADER_H__
#define __HEADER_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <time.h>
#include <sys/wait.h>
#include <dirent.h>
#include <syslog.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <sys/sendfile.h>
#include <stdbool.h>
#include "config.h"



#define ARGS_CHECK(argc, num){if(argc != num){fprintf(stderr, "args error !\n"); return -1;} }
#define ERROR_CHECK(ret, num, msg){if(ret==num){perror(msg); return -1;}}
#define THREAD_ERROR_CHECK(ret, msg){if(ret!=0){fprintf(stderr, "%s:%s \n", msg,strerror(ret));}}

#endif
