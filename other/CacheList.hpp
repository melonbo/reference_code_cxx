/*
 *  CacheList.hpp
 *
 *  gnerate data cache for net receiving Wrapper by C++11
 *
 * 
 */
#ifndef __CACHE_LIST_HPP
#define __CACHE_LIST_HPP

#include "MutexLock.hpp"
#include <list>

class CacheList
{
private:
    using  list_type  =  std::list<void *>;

public:
    CacheList()  = default;
    ~CacheList() =  default;

    bool AddtoList(void *ptr)
    {
        MutexLockGuard guard(m_lock);
        if(m_list.size() < m_limitNum)
        {
            m_list.push_back(ptr);//尾添加一元素
            return true;
        }
        return false;
    }

    bool GetfromList(void **ptr)
    {
        MutexLockGuard guard(m_lock);
        if (!m_list.empty())
        {
            *ptr = *m_list.begin();
            m_list.pop_front();//删除第一个元素
            return true;
        }
        return false;
    }

    size_t GetListSize()
    {
        return m_list.size();
    }

private:
    list_type  m_list{};
    MutexLock  m_lock{};
    size_t     m_limitNum = 1;
};

#endif