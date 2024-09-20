#ifndef _LOCKER_H_
#define _LOCKER_H_

#include <iostream>
#include <exception>
//引入 POSIX 线程库，提供线程、互斥锁和条件变量等多线程编程功能
#include <pthread.h>
using namespace std;

/* 线程锁 */
//定义了一个名为 MutexLocker 的类，它用于管理互斥锁
class MutexLocker {
private:
    //m_mutex 是一个 pthread_mutex_t 类型的变量，表示互斥锁
    pthread_mutex_t m_mutex;
public:
    MutexLocker() {  //初始化
        if( pthread_mutex_init( &m_mutex, NULL ) ) {
            cout << "mutex init errror __ 1\n";
            throw exception();//抛出异常
        }
    }
    //析构函数，摧毁互斥锁
    ~MutexLocker() {
        pthread_mutex_destroy( &m_mutex );
    }

    bool mutex_lock() {//上锁
        return pthread_mutex_lock( &m_mutex ) == 0;
    }

    bool mutex_unlock() {//解锁
        return pthread_mutex_unlock( &m_mutex );
    }
};


/* 条件变量 */
class Cond {
private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
public:
    Cond() {
        if( pthread_mutex_init( &m_mutex, NULL ) ) {
            throw exception();
        }
        if( pthread_cond_init( &m_cond, NULL ) ) {
            pthread_cond_destroy( &m_cond );
            throw exception();
        }
    }

    ~Cond() {
        pthread_mutex_destroy( &m_mutex );
        pthread_cond_destroy( &m_cond );
    }

    // 等待条件变量，cond与mutex搭配使用，避免造成共享数据的混乱
    bool wait() {
        pthread_mutex_lock( &m_mutex );
        int ret = pthread_cond_wait( &m_cond, &m_mutex );//等待
        pthread_mutex_unlock( &m_mutex );//解锁
        return ret == 0;//0表示成功
    }

    // 唤醒等待该条件变量的某个线程
    bool signal() {
        return pthread_cond_signal( &m_cond ) == 0;
    }

    // 唤醒所有等待该条件变量的线程
    bool broadcast() {
        return pthread_cond_broadcast( &m_cond ) == 0;
    }
};

#endif