#ifndef   SERVER_H
#define   SERVER_H
#include <sys/epoll.h>
#include "unified.h"
#include "thread_pool.h"



class Server
{
    private:
        int listenfd;
        int connfd;
        int epfd;
        int port;
        int maxsize;
        char ip[15];
        struct sockaddr_in  servaddr;
        struct epoll_event ev,events[20];       //声明epoll_event结构体的变量,ev用于注册事件,数组用于回传要处理的事件
        ThreadPool* thread_pool=nullptr;            //线程池


        void Socket();
        void Bind();
        void Listen();
        void Initialize_epoll();
        void epoll_process();

    public:
        Server(char* IP,int Po=8000,int Size=50):port(Po),maxsize(Size){strcpy(ip,IP);}
        Server(char* IP,int Po=8000,ThreadPool* Thread_pool=nullptr,int Size=50):
                                                    port(Po),
                                                    thread_pool(Thread_pool),
                                                    maxsize(Size){strcpy(ip,IP);};

        void Handle();

        void start();
        void stop(){close(listenfd);};

};

#endif