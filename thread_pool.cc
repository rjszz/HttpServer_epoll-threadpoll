#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "thread_pool.h"
#include "unified.h"

 
bool ThreadPool::bshutdown_ = false;
int ThreadPool::icurr_thread_num_ = THREAD_NUM;
std::vector<int> ThreadPool::connfd_;
std::map<pthread_t,int> ThreadPool::thread_id_map_;
pthread_mutex_t ThreadPool::command_mutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ThreadPool::command_cond_ = PTHREAD_COND_INITIALIZER;
 
void ThreadPool::InitializeThreads()
{
    for (int i = 0; i < THREAD_NUM ; ++i)
    {
        pthread_t tempThread;
        pthread_create(&tempThread, NULL, ThreadPool::Process,NULL);
        thread_id_map_[tempThread] = 0;
    }
}
 
void* ThreadPool::Process(void* arg)
{
    
    int connfd;
    while (true)
    {
        pthread_mutex_lock(&command_mutex_);
        // 如果线程需要退出，则此时退出
        if (1 == thread_id_map_[pthread_self()])
        {
            pthread_mutex_unlock(&command_mutex_);
            printf("thread %u will exit\n", pthread_self());
            pthread_exit(NULL);
        }
        // 当线程不需要退出且没有需要处理的任务时，需要缩容的则缩容，不需要的则等待信号
        if (0 == connfd_.size() && !bshutdown_)
        {
            if(icurr_thread_num_ >  THREAD_NUM)
            {
                DeleteThread();
                if (1 == thread_id_map_[pthread_self()])
                {
                    pthread_mutex_unlock(&command_mutex_);
                    printf("thread %u will exit\n", pthread_self());
                    pthread_exit(NULL);
                }
            }
            pthread_cond_wait(&command_cond_,&command_mutex_);
        }
        // 线程池需要关闭，关闭已有的锁，线程退出
        if(bshutdown_)
        {
            pthread_mutex_unlock (&command_mutex_);
            printf ("thread %u will exit\n", pthread_self ());
            pthread_exit (NULL);
        }
        // 如果线程池的最大线程数不等于初始线程数，则表明需要扩容
        if(icurr_thread_num_ < connfd_.size())
        {
            AddThread();
        }
        // 从容器中取出待办任务
        auto iter = connfd_.begin();
        connfd=*iter;
        printf("sockfd3:%d\n", connfd);
        connfd_.erase(iter);
       
        pthread_mutex_unlock(&command_mutex_);
        // 开始业务处理
        Http_server(connfd);
        
    }
    return NULL; // 完全为了消除警告(eclipse编写的代码，警告很烦人)
}
 
void ThreadPool::AddWork(int sockfd)
{
   
    bool bsignal = false;
    pthread_mutex_lock(&command_mutex_);
    if (0 == connfd_.size())
    {
        bsignal = true;
    }
    connfd_.push_back(sockfd);
    printf("sockfd2:%d\n", sockfd);
    pthread_mutex_unlock(&command_mutex_);
    if (bsignal)
    {
        pthread_cond_signal(&command_cond_);
    }
}
 
void ThreadPool::ThreadDestroy(int iwait)
{
    while(0 != connfd_.size())
    {
        sleep(abs(iwait));
    }
    bshutdown_ = true;
    pthread_cond_broadcast(&command_cond_);
    std::map<pthread_t,int>::iterator iter = thread_id_map_.begin();
    for (; iter!=thread_id_map_.end(); ++iter)
    {
        pthread_join(iter->first,NULL);
    }
    pthread_mutex_destroy(&command_mutex_);
    pthread_cond_destroy(&command_cond_);
}
 
void ThreadPool::AddThread()
{
    if(((icurr_thread_num_*ADD_FACTOR) < connfd_.size())
            && (MAX_THREAD_NUM != icurr_thread_num_))
    {
        InitializeThreads();
        icurr_thread_num_ += THREAD_NUM;
    }
}
 
void ThreadPool::DeleteThread()
{
    int size = icurr_thread_num_ - THREAD_NUM;
    std::map<pthread_t,int>::iterator iter = thread_id_map_.begin();
    for(int i=0; i<size; ++i,++iter)
    {
        iter->second = 1;
    }
}