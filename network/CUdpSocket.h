/*
 * CCommonTimer.h
 *
 *  Created on: 2010-9-29
 *      Author: hbs
 */

#ifndef CCOMMONUDPSOCKET_H_
#define CCOMMONUDPSOCKET_H_
#include "NvrCommon.h"
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
//定时器子项数组定义
#define MAX_TIME_ITEMCNT 10

class CUdpSocket: private ICommonThread {
public:
	typedef enum {
		SOCKET_NONE = 0, SOCKET_CREATE, SOCKET_DATA
	} enSocketStatus;
	typedef struct {
		int iSocketFd;
		enSocketStatus socketStatus;
		bool bUser;
		union {
			struct {
				struct sockaddr_in localaddr;
				struct sockaddr_in remoteAddr;
				struct sockaddr_in clientaddr;
			} UDP;
		};
	} tagSocketInfo;
protected:
	CUdpSocket();
	virtual ~CUdpSocket();

	typedef enum {
		WAITEVENT_ERROR = -1,
		WAITEVENT_READ = 0,
		WAITEVENT_TIMER = 1,
		WAITEVENT_UNKOWN = 2
	} enWaitEventResult;
	void ConfigSocketParam(int iMaxCount,
			CUdpSocket::tagSocketInfo *pSocketInfo,
			struct epoll_event *pEvents);
protected:
	void StartSocket(int iStackSize = 0);
	void asyncStop();
	void waitAsyn();

	int  CreateUDPSocket(int iLocalPort,
			const char *szRemoteAddr, int iRemotePort, int iUdpSendBufSizes,int iUdpReciveBufSizes );

	int  ReadUDPSocket(int iHandle, void *pBuffer, int iReadSizes);
	int  ReadUDPSocket(int iHandle, void *pBuffer, int iReadSizes,
			struct sockaddr_in *clientaddr);
	int  WriteUDPSocket(int iHandle, const void *pBuffer, int iWriteSizes);
	int  WriteUDPSocket(int iHandle, struct sockaddr_in &RemoteAddr,const void *pBuffer, int iWriteSizes);

	int  WriteUDPSocketBack(int iHandle, const void *pBuffer, int iWriteSizes);
	CUdpSocket::enSocketStatus GetSocketStatus(int iHandle);
	void CloseUDPSocket(int iHandle);

	void AddListenEvent(int iHandle);
	void DelListenEvent(int iHandle);

	CUdpSocket::enWaitEventResult WaitListenEvent(int imsec, int &iHandle);

	virtual void ThreadRun() = 0;
protected:
	bool IsRun();
        int  getSocket(int handle);
        bool JoinGroup(int socketHandle, char* groupIP, unsigned int port);
        bool LeaveGroup(int socketHandle);
private:
	//pthread_mutex_t m_mutexLock; //线程锁
	int m_epfd;
	CCommonThread m_commonThread;

	int m_iMaxCount;

	struct epoll_event *m_pEvents;
	//线程的运行实体
	void ThreadProc();

        tagSocketInfo *m_pSocketInfo;
        struct ip_mreq mreq;
};

#endif /* CCOMMONTIMER_H_ */
