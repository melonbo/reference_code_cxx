/*
 *  ThreadPool.hpp
 *
 *  ThreadPool for thread running  of TRDS Wrapper by C++11
 *  usage: 
 *  ThreadPool pool;
 *  pool.add_task()
 */
#ifndef __THREAD_POOL_HPP
#define __THREAD_POOL_HPP

#include "SyncQueue.hpp"
#include <atomic>

class ThreadPool
{
public:
    using task_type =  std::function<void()>;

private:
    std::list<std::shared_ptr<std::thread>> m_threadGroup;
    SyncQueue<task_type> m_queue;
    std::atomic_bool m_running;
    std::once_flag m_flag;

public:
    ThreadPool(int numThreads = std::thread::hardware_concurrency()) : m_queue(MAXTASKCOUNT)
    {
        // std::cout << "numThreads = " << numThreads << std::endl;
        start_tp(numThreads);
    }
    ~ThreadPool(void)
    {
        stop_tp();
    }

public:
    void stop_tp()
    {
        std::call_once(m_flag, [this]{stop_thread_group();});
    }

    void add_task(task_type&& task)
    {
        m_queue.put_sq(std::forward<task_type>(task));
    }

    void add_task(const task_type& task)
    {
        m_queue.put_sq(task);
    }

    bool full_task()
    {
        return m_queue.full_sq();
    }

private:
    void start_tp(int numThreads)
    {
        m_running = true;
        for(int i = 0;i< numThreads;++i)
        {
            m_threadGroup.push_back(std::make_shared<std::thread>(&ThreadPool::run_in_thread,this));
        }
    }

    // void run_in_thread()
    // {
    //     while(m_running)
    //     {
    //         std::list<task_type> list;
    //         m_queue.take_sq(list);
    //         for (auto& task : list)
    //         {
    //             if(!m_running)
    //                 return;
    //             task();
    //         }
    //     }
    // }


    void run_in_thread()
    {
        while(m_running)
        {
            task_type task;
            m_queue.take_sq(task);
            task();
        }
    }


    void stop_thread_group()
    {
        m_queue.stop_sq();
        m_running = false;

        for(auto thread : m_threadGroup)
        {
            if(thread)
            {
                thread->join();
            }
        }
        m_threadGroup.clear();
    }

private:
    static constexpr int MAXTASKCOUNT = 4;
};

#endif