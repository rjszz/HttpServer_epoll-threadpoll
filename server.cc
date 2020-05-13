#include "server.h"

void Server::Socket()
{
        if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
        Error("create socket error: %s(errno: %d)\n");

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_aton(ip, &servaddr.sin_addr);
    // servaddr.sin_addr.s_addr = htonl(ip);
    servaddr.sin_port = htons(port);

    return ;
}

void Server::Bind()
{
     if( bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)
            Error("bind socket error: %s(errno: %d)\n");
    return ;
}

void Server::Listen()
{
    if( listen(listenfd, maxsize) == -1)
         Error("listen socket error: %s(errno: %d)\n");
    return ;
}

/*
EPOLLIN ：表示对应的文件描述符可以读（包括对端SOCKET正常关闭）；

EPOLLOUT：表示对应的文件描述符可以写；

EPOLLPRI：表示对应的文件描述符有紧急的数据可读（这里应该表示有带外数据到来）；

EPOLLERR：表示对应的文件描述符发生错误；

EPOLLHUP：表示对应的文件描述符被挂断；

EPOLLET： 将EPOLL设为边缘触发(Edge Triggered)模式，这是相对于水平触发(Level Triggered)来说的。

EPOLLONESHOT：只监听一次事件，当监听完这次事件之后，
如果还需要继续监听这个socket的话，需要再次把这个socket加入到EPOLL队列里


EPOLL_CTL_ADD：注册新的fd到epfd中；

EPOLL_CTL_MOD：修改已经注册的fd的监听事件；

EPOLL_CTL_DEL：从epfd中删除一个fd；
*/
void Server::Initialize_epoll()
{
    
    epfd=epoll_create(256);

    //设置与要处理的事件相关的文件描述符
    ev.data.fd=listenfd;

     //设置要处理的事件类型
    ev.events=EPOLLIN|EPOLLET;

    //注册epoll事件
    epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);

    return;
}

void Server::epoll_process()
{
    int nfds;
    int sockfd;
    struct sockaddr_in  clientaddr;
    socklen_t addrlen=sizeof(clientaddr);
    for(; ; )
    {
        //等待epoll事件的发生
        nfds=epoll_wait(epfd,events,20,500);

        //处理所发生的所有事件
        
        int i;
        for(i=0;i<nfds;++i)
        {
            //如果新监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接。
            if(events[i].data.fd==listenfd)
            {
                connfd = accept(listenfd,(sockaddr *)&clientaddr, &addrlen);
                if(connfd<0)
                    Error("用户连接失败");
                //连接成功
                printf("Connect client:<%s,%d>\n",
                        inet_ntoa(clientaddr.sin_addr),
                        ntohs(clientaddr.sin_port)
                        );
                fd_addr_map_[connfd]=&clientaddr;
                ev.data.fd=connfd;
                ev.events=EPOLLIN|EPOLLET;
                epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev);
            }
            //如果是已经连接的用户，并且收到数据，那么进行读入。
            else if(events[i].events&EPOLLIN)
            {
                if ( (sockfd = events[i].data.fd) < 0)
                        continue;
               printf("sockfd1:%d\n", sockfd);
                thread_pool->AddWork(sockfd);
            }

        }
    }

    return;
}

void Server::Handle()
{
     printf("======Httpserver is working======\n");

    epoll_process();
}

void Server::start()
{
    Socket();
    Initialize_epoll();
    Bind();
    Listen();
    thread_pool->InitializeThreads();
}