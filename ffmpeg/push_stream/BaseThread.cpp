/*
 * BaseThread.cpp
 *
 *  Created on: Aug 15, 2022
 *      Author: root
 */

#include "BaseThread.h"

BaseThread::BaseThread() {
	// TODO Auto-generated constructor stub

	tid = 0;
	bRun = false;
	m_vTarget = NULL;
	m_pParam = NULL;
	//设置线程栈
	threadStatus = THREAD_STATUS_NEW;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 1024 * 1024);//PTHREAD_STACK_MIN);
}

BaseThread::BaseThread(ThreadBody vTarget,void *pParam) {
	tid = 0;
	bRun = false;
	m_vTarget = vTarget;
	m_pParam = pParam;
	//设置线程栈
	threadStatus = THREAD_STATUS_NEW;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 1024 * 1024);//PTHREAD_STACK_MIN);
}

BaseThread::~BaseThread() {
	// TODO Auto-generated destructor stub
	pthread_attr_destroy(&attr);
}

void* BaseThread::Run0(void* pVoid) {
	BaseThread* p = (BaseThread*) pVoid;
	p->Run1();

	return p;
}

void* BaseThread::Run1(void)
{
	tid = pthread_self();
	ThreadProc();
	threadStatus = THREAD_STATUS_EXIT;
	tid = 0;
	pthread_exit(NULL);
}

bool BaseThread::Start(int iStackSize) {
	assert(bRun == false);
	bRun = true;
	if (iStackSize > 0) {
		//printf(" CCommonThread::start iStackSize:%d\n", iStackSize);
		assert(pthread_attr_setstacksize(&attr, iStackSize) == 0);//PTHREAD_STACK_MIN);
	}
	//创建线程
	if ( m_vTarget != NULL ){
		return ((pthread_create(&tid, &attr, m_vTarget, m_pParam) == 0)?true:false);
	}
	else
	{
		return ((pthread_create(&tid, &attr, Run0, this) == 0)? true :false);
	}
	return false;
}

pthread_t BaseThread::GetThreadID(void)
{
	return tid;
}


void BaseThread::AsyncStop()
{
	//异步停止
	assert(bRun == true);
	bRun = false;
}

//!等待线程停止
void BaseThread::WaitAsyn()
{
	//等待停止
	Join();
}

//等待线程直至退出
void BaseThread::Stop()
{
	//停止线程
	assert(bRun == true);
	bRun = false;
	Join();
}

//等待线程退出或者超时
void BaseThread::Stop(unsigned long millisTime)
{
	//等待时间停止
	assert(bRun == true);
	bRun = false;
	Join(millisTime);
}

int BaseThread::GetState() {
	//获得线程状态
	return threadStatus;
}
/*!
 *
 */
void BaseThread::Join() {
	//等待停止
	if (tid > 0) {
		pthread_join(tid, NULL);
	}
}
/*!
 *
 * @param millisTime
 */
void BaseThread::Join(unsigned long millisTime) {
	//等待时长停止
	if (tid == 0) {
		return;
	}
	if (millisTime == 0) {
		Join();
	} else {
		unsigned long k = 0;
		while (threadStatus != THREAD_STATUS_EXIT && k <= millisTime) {
			usleep(1000);
			k++;
		}
	}
}
