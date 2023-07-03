/*
 *  SyncQueue.hpp
 *
 *  SyncQueue for thread running  of TRDS Wrapper by C++11

 */
#ifndef __SYNC_QUEUE_HPP
#define __SYNC_QUEUE_HPP


#include <list>

template <typename T>
class SyncQueue
{
private:
    std::list<T> m_queue; //缓冲队列
    std::mutex m_mutex;
    std::condition_variable m_notEmpty;
    std::condition_variable m_notFull;
    std::size_t m_maxSize;//最大队列大小
    bool m_needStop; //停止标志

public:
    SyncQueue(int maxSize) 
    : m_queue{},
      m_mutex{},
      m_notEmpty{},
      m_notFull{},
      m_maxSize(maxSize),
      m_needStop(false) 
      {}

    ~SyncQueue() = default;

public:
    void put_sq(const T& x)
    {
        add_sq(x);
    }

    void put_sq(T&& x)
    {
        add_sq(std::forward<T>(x));
        // std::cout << "now size: " << size_sq() << std::endl;
    }

    void take_sq(std::list<T>& list)
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        m_notEmpty.wait(locker,[this]{return m_needStop || NotEmpty();});

        if(m_needStop)
        {
            return;
        }

        list = std::move(m_queue);
        m_notFull.notify_one();
    }

    void take_sq(T& t)
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        m_notEmpty.wait(locker, [this]{return m_needStop || NotEmpty();});
        if(m_needStop)
        {
            return;
        }
        t = m_queue.front();
        m_queue.pop_front();
        m_notFull.notify_one();
    }

    void stop_sq()
    {
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_needStop  = true;
        }
        m_notFull.notify_all();
        m_notEmpty.notify_all();
    }

    bool empty_sq()
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        return m_queue.empty();
    }

    bool full_sq()
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        return  m_queue.size() == m_maxSize;
    }

    std::size_t size_sq()
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        return m_queue.size();
    }

    // int count_sq()
    // {
    //     return m_queue.size();
    // }

private:
    bool NotFull() const
    {
        bool full  = m_queue.size() >= m_maxSize;
        // std::cout << "size: " << m_queue.size() <<  " and maxsize: " << m_maxSize << std::endl;
        if(full)
        {
            // std::cout << "queue has full." << std::endl;
        }
        return !full;
    }

    bool NotEmpty() const
    {
        bool empty = m_queue.empty();
        if(empty)
        {
            // std::cout << "queue has empty,threadId: " << std::this_thread::get_id() << std::endl;
        }
        return !empty;
    }

    template<typename F>
    void add_sq(F&& x)
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        m_notFull.wait(locker,[this]{return m_needStop || NotFull();});
        if(m_needStop)
        {
            return;
        }
        m_queue.push_back(std::forward<F>(x));
        m_notEmpty.notify_one();
    }
};

#endif