/*
 * CCommonTimer.h
 *
 *  Created on: 2010-9-29
 *      Author: hbs
 */

#ifndef CBSCOMMONSERVERSOCKET_H_
#define CBSCOMMONSERVERSOCKET_H_
#include "CCommonThread.h"
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
//定时器子项数组定义
#define MAX_TIME_ITEMCNT 10
//!TCP服务端辅助类
class CTcpServerSocket: public ICommonThread {
public:
	CTcpServerSocket();
	virtual ~CTcpServerSocket();
	//!网络状态信息
	typedef struct {
		int iSocketFd; //!< 文件ID
		bool bClose; //!< 关闭标志
		char szCliIpaddr[32]; //!< 客户端IP
		pthread_mutex_t mutexLock; //!< 线程锁
	} tagClientSocketInfo;
	//!套接字状态
	typedef enum {
		SOCKET_NONE = 0, //!< 无数据
		SOCKET_DATA //!< 有数据
	} enSocketStatus;
	//!事件状态
	typedef enum {
		WAITEVENT_ERROR = -1,
		WAITEVENT_READ = 0,
		WAITEVENT_TIMER = 1,
		WAITEVENT_ACCEPT = 2,
		WAITEVENT_UNKOWN = 3
	} enWaitEventResult;
	//!TCP事件
	typedef enum {
		ON_ERROR = -1, ON_NONE = 0, ON_TIMER = 1, ON_ACCEPT = 2, ON_UNKOWN = 3
	} enTcpEvent;
protected:
	//!配置套接字参数
	void ConfigSocketParam(int iMaxCount,
			CTcpServerSocket::tagClientSocketInfo *pSocketInfo,
			struct epoll_event *pEvents);
	//!开始服务
	void StartSocket(int iLocalPort, int iTimerCount);
	//!停止服务
	void StopSocket();
	//!发现会话
	bool FindSession(int iHandle);
	//!获得会话地址
	const char *GetSessionAddr(int iHandle);
	//!删除会话
	void DeleteSession(int iHandle);
	//!具有锁定的发送数据
	bool SendSessionData(int iHandle, const unsigned char *pBuffer,
			int iReadSizes);
	//!不再锁定的发送数据
	bool SendData(int iHandle, const unsigned char *pBuffer, int iReadSizes);
	//!接收数据
	int ReciveData(int iHandle, unsigned char *pBuffer, size_t __iReadBufSizes);
	//!数据准备好，虚函数，需要实现
	virtual bool OnDataReady(int iHandle, const unsigned char *pBuffer,
			int iReadSizes) = 0;
	//!TCP 事件
	virtual void OnTcpEvent(int iHandle, int iEvent);
	//!设置TCP传输模式
	void SetTCPSocketBlock(int iHandle, bool bBlock);
	//!设置TCP传输Buf
	void SetTCPBufSizes(int iHandle, int iSndSizes, int iRevSizes);
private:
	//!创建TCP套接字
	int CreateTCPSocket();
	//!关掉套接字
	void CloseTCPSocket();
	//!读TCP数据
	int ReadTCPSocket(int iHandle, unsigned char *pBuffer, int iReadSizes);
	//!写TCP数据
	int WriteTCPSocket(int iHandle, const unsigned char *pBuffer,
			int iWriteSizes);
	//!等待事件
	CTcpServerSocket::enWaitEventResult WaitListenEvent(int imsec, int &iHandle,
			struct epoll_event *pEvent);
	//void setnonblocking(int sock);
	pthread_mutex_t m_mutexLock; //线程锁

	//TCP套接字信息
	typedef struct {
		volatile int ilistenfd;
		int iLocalPort;
		int iMaxCount;
		int iTimerCount; //没有任何连接的滴答
		bool bClose;
	} tagServerSocketInfo;
	tagServerSocketInfo m_serverSocketInfo;

	int m_epfd;
	CCommonThread m_commonThread;
	tagClientSocketInfo *m_pSocketInfo;
	struct epoll_event *m_pEvents;
protected:
	//线程的运行实体
	void ThreadProc();
};

#endif /* CCOMMONTIMER_H_ */
