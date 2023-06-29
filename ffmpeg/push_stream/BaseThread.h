/*
 * BaseThread.h
 *
 *  Created on: Aug 15, 2022
 *      Author: root
 */

#ifndef SRC_THREAD_BASETHREAD_H_
#define SRC_THREAD_BASETHREAD_H_

#include <pthread.h>
#include <unistd.h>
#include <assert.h>
//!线程定义
typedef void* (*ThreadBody)(void *);

class BaseThread {
public:
	//!构造函数
	BaseThread();
	//!构造函数
	BaseThread(ThreadBody vTarget,void *pParam);
	//!析构
	virtual ~BaseThread();

private:
	ThreadBody m_vTarget;
	//!当前线程的线程ID
	pthread_t tid;

	//!线程的状态
	int threadStatus;
	//!线程属性
	pthread_attr_t attr;
	//!线程优先级
	sched_param param;
	void *m_pParam;
	//!获取执行方法的指针
	static void* Run0(void* pVoid);
	//!内部执行方法
	void* Run1();
	//!等待线程直至退出
	void Join();
	//!等待线程退出或者超时
	void Join(unsigned long millisTime);
protected:
	volatile bool  bRun;//!<运行标志
public:
	//!等待线程直至退出
	void Stop();
	//!等待线程退出或者超时
	void Stop(unsigned long millisTime);
	//!停止线程
	void AsyncStop();
	//!等待线程停止
	void WaitAsyn();

	//线程的状态－新建
#define  THREAD_STATUS_NEW  0
	//线程的状态－正在运行
#define  THREAD_STATUS_RUNNING  1
	//线程的状态－运行结束
#define  THREAD_STATUS_EXIT  -1
	//!判断运行状态
	bool IsRun(){return bRun;}
	//!线程的运行实体
	virtual void ThreadProc() = 0;
	//!开始执行线程
	bool Start(int iStackSize = 0);
	//!获取线程状态
	int GetState();

	pthread_t GetThreadID(void);

};

#endif /* SRC_BASECOMMON_BASETHREAD_H_ */
