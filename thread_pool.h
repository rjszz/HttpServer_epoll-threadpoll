#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_
#include <map>
#include <vector>
 
#define MAX_THREAD_NUM 80 // 该值目前需要设定为初始线程数的整数倍
#define ADD_FACTOR 1 // 该值表示一个线程可以处理的最大任务数
#define THREAD_NUM 10 // 初始线程数
 
class ThreadPool
{
public:
    ThreadPool() {};
    static void InitializeThreads();
    void AddWork(int sockfd);
    void ThreadDestroy(int iwait = 2);
private:
    static void* Process(void* arg);
    static void AddThread();
    static void DeleteThread();
    static bool bshutdown_;
    static int icurr_thread_num_;
    static std::map<pthread_t,int> thread_id_map_;
    static std::vector<int> connfd_;
    static pthread_mutex_t command_mutex_;
    static pthread_cond_t command_cond_;
};
 
 
#endif /* THREAD_POOL_H_ */