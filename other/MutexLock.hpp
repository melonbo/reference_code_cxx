/*
 * MutexLock.hpp
 *
 * linux Mutex Wrapper by C++03/98
 *
 *  Created on: 2020-08-12
 *  Author: Songwei Yu
 */
#ifndef _MUTEX_LOCK_HPP
#define _MUTEX_LOCK_HPP

#include <pthread.h>

class MutexLock //final
{
public:
    MutexLock()
    {
        pthread_mutex_init(&m_lock,NULL);
    }
    ~MutexLock()
    {
        pthread_mutex_destroy(&m_lock);
    }

    //MutexLock(const MutexLock&) = delete;
    //MutexLock& operator=(const MutexLock&) = delete;

public:
    int lock()
    {
        return pthread_mutex_lock(&m_lock);
    }

    int try_lock()
    {
        return pthread_mutex_trylock(&m_lock);
    }

    int unlock()
    {
        return pthread_mutex_unlock(&m_lock);
    }

private:
    pthread_mutex_t  m_lock;
};

class MutexLockGuard
{
public:
    MutexLockGuard(MutexLock& lock) : m_lock(lock)
    {
        m_lock.lock();
    }
    ~MutexLockGuard()
    {
        m_lock.unlock();
    }

    //MutexLockGuard() = delete;
    //MutexLockGuard(const MutexLockGuard&) = delete;
    //MutexLockGuard& operator=(const MutexLockGuard&) = delete;

private:
    MutexLock&   m_lock;
};

#endif