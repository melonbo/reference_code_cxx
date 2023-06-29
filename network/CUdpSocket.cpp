/*
 * CCommonTimer.cpp
 *
 *  Created on: 2010-9-29
 *      Author: hbs
 */
#include "CCommonThread.h"
#include "CUdpSocket.h"
#include <assert.h>
#include <sys/socket.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
/*!
 *
 */
CUdpSocket::CUdpSocket() :
	m_commonThread(this) {

	//pthread_mutex_init(&m_mutexLock, NULL);
	m_epfd = -1;
	m_iMaxCount = 0;
	m_pSocketInfo = NULL;
	m_pEvents = NULL;
}
/*!
 *
 */
CUdpSocket::~CUdpSocket() {


	//pthread_mutex_destroy(&m_mutexLock);
}
/*!
 *
 * @param iMaxCount
 * @param pSocketInfo
 * @param pEvents
 */
void CUdpSocket::ConfigSocketParam(int iMaxCount,
		CUdpSocket::tagSocketInfo *pSocketInfo, struct epoll_event *pEvents) {
	assert( pSocketInfo != NULL);
	assert( pEvents != NULL);
	m_pSocketInfo = pSocketInfo;
	m_pEvents = pEvents;
	m_iMaxCount = iMaxCount;
	assert(m_iMaxCount > 0 );
	for (int i = 0; i < m_iMaxCount; i++) {
		memset(&m_pEvents[i], 0, sizeof(epoll_event));
		memset(&m_pSocketInfo[i], 0, sizeof(tagSocketInfo));
	}
}
/*!
 *
 * @param iStackSize
 */
void CUdpSocket::StartSocket(int iStackSize) {
	m_epfd = epoll_create(m_iMaxCount);
	assert( m_epfd > 0 );
	m_commonThread.start(iStackSize);
}
/*!
 *
 */
void CUdpSocket::asyncStop() {
	if (m_commonThread.isRun()) {
		m_commonThread.asyncStop();
	}
}
/*!
 *
 */
void CUdpSocket::waitAsyn() {
	m_commonThread.waitAsyn();
	if (m_epfd > 0) {
		close(m_epfd);
	}
}
/*!
 *
 * @param iLocalPort
 * @param szRemoteAddr
 * @param iRemotePort
 * @param iUdpSendBufSizes
 * @param iUdpReciveBufSizes
 * @return
 */
int CUdpSocket::CreateUDPSocket(int iLocalPort, const char *szRemoteAddr,
		int iRemotePort, int iUdpSendBufSizes,int iUdpReciveBufSizes ) {

	assert ( iLocalPort >= 0 );
	int iSockFd = socket(AF_INET, SOCK_DGRAM, 0);

	int iHandle = -1;
	for (int i = 0; i < m_iMaxCount; i++) {
		if (m_pSocketInfo[i]. bUser == true) {
			continue;
		}
		iHandle = i;
		m_pSocketInfo[i].bUser = true;
		break;
	}
	assert( iHandle >= 0 && iHandle < m_iMaxCount );
	bzero(&m_pSocketInfo[iHandle].UDP.remoteAddr, sizeof(struct sockaddr_in));
	bzero(&m_pSocketInfo[iHandle].UDP.localaddr, sizeof(struct sockaddr_in));
	bzero(&m_pSocketInfo[iHandle].UDP.clientaddr, sizeof(struct sockaddr_in));
	assert( iSockFd > 0 );
	m_pSocketInfo[iHandle].UDP.clientaddr.sin_family = AF_INET;
	//m_pSocketInfo[iHandle].UDP.clientaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//m_pSocketInfo[iHandle].UDP.clientaddr.sin_port = htons(10460);


	m_pSocketInfo[iHandle].UDP.localaddr.sin_family = AF_INET;
	m_pSocketInfo[iHandle].UDP.localaddr.sin_addr.s_addr = inet_addr("0.0.0.0");
	m_pSocketInfo[iHandle].UDP.localaddr.sin_port = htons(iLocalPort);

	//	m_pSocketInfo[iHandle].UDP.clientaddr.sin_family = AF_INET;
	//	m_pSocketInfo[iHandle].UDP.clientaddr.sin_addr.s_addr = inet_addr(
	//			szLocalAddr);
	//	m_pSocketInfo[iHandle].UDP.clientaddr.sin_port = htons(50000);
	//printf("bind iLocalPort == %d\n", iLocalPort);

        assert (bind(iSockFd, (sockaddr *) &m_pSocketInfo[iHandle].UDP.localaddr,
                                        sizeof(struct sockaddr_in)) == 0);

	if (szRemoteAddr != NULL) {
		m_pSocketInfo[iHandle].UDP.remoteAddr.sin_family = AF_INET;
		m_pSocketInfo[iHandle].UDP.remoteAddr.sin_addr.s_addr = inet_addr(
				szRemoteAddr);
		m_pSocketInfo[iHandle].UDP.remoteAddr.sin_port = htons(iRemotePort);
	}
	m_pSocketInfo[iHandle].iSocketFd = iSockFd;
	int sndbuf = 0;
	int rcvbuf = 0;
	sndbuf = iUdpSendBufSizes;
	rcvbuf = iUdpReciveBufSizes;
	setsockopt(iSockFd, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf));
	setsockopt(iSockFd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));
	m_pSocketInfo[iHandle].socketStatus = SOCKET_DATA;
	return iHandle;
}
/*!
 *
 * @param iHandle
 * @param pBuffer
 * @param iReadSizes
 * @return
 */
int CUdpSocket::ReadUDPSocket(int iHandle, void *pBuffer, int iReadSizes) {
	if (iHandle < 0 || iHandle >= m_iMaxCount) {
		return -1;
	}
	//assert( iHandle >= 0 && iHandle < m_iMaxCount);
	socklen_t clilen = sizeof(sockaddr_in);
	if (m_pSocketInfo[iHandle].iSocketFd < 0) {
		return -1;
	}
	//assert( m_pSocketInfo[iHandle].iSocketFd > 0 );
	//bzero(&m_pSocketInfo[iHandle].UDP.clientaddr, sizeof(sockaddr_in));
	int iRet = recvfrom(m_pSocketInfo[iHandle].iSocketFd, pBuffer, iReadSizes,
			0, (struct sockaddr*) &m_pSocketInfo[iHandle].UDP.clientaddr,
			&clilen);
	return iRet;
}


int CUdpSocket::ReadUDPSocket(int iHandle, void *pBuffer, int iReadSizes,
		struct sockaddr_in *clientaddr)
{
	if (iHandle < 0 || iHandle >= m_iMaxCount) {
		return -1;
	}
	//assert( iHandle >= 0 && iHandle < m_iMaxCount);
	socklen_t clilen = sizeof(sockaddr_in);
	if (m_pSocketInfo[iHandle].iSocketFd < 0) {
		return -1;
	}
	//assert( m_pSocketInfo[iHandle].iSocketFd > 0 );
	//bzero(&m_pSocketInfo[iHandle].UDP.clientaddr, sizeof(sockaddr_in));
	int iRet = recvfrom(m_pSocketInfo[iHandle].iSocketFd, pBuffer, iReadSizes,
			0, (struct sockaddr*) &m_pSocketInfo[iHandle].UDP.clientaddr,
			&clilen);
	memcpy(clientaddr,(struct sockaddr*) &m_pSocketInfo[iHandle].UDP.clientaddr,sizeof(sockaddr_in));
	return iRet;
}
/*!
 *
 * @param iHandle
 * @param pBuffer
 * @param iWriteSizes
 * @return
 */
int CUdpSocket::WriteUDPSocket(int iHandle, const void *pBuffer,
		int iWriteSizes) {
	//assert( iHandle >= 0 && iHandle < m_iMaxCount);
	if (iHandle < 0 || iHandle >= m_iMaxCount) {
		return -1;
	}
	if (m_pSocketInfo[iHandle].iSocketFd < 0) {
		return -1;
	}
	//assert( m_pSocketInfo[iHandle].iSocketFd > 0 );
	return sendto(m_pSocketInfo[iHandle].iSocketFd, pBuffer, iWriteSizes, 0,
			(struct sockaddr*) &(m_pSocketInfo[iHandle].UDP.remoteAddr),
			sizeof(struct sockaddr_in));
}
/*!
 *
 * @param iHandle
 * @param RemoteAddr
 * @param pBuffer
 * @param iWriteSizes
 * @return
 */
int  CUdpSocket::WriteUDPSocket(int iHandle, struct sockaddr_in &RemoteAddr,const void *pBuffer, int iWriteSizes)
{
        //assert( iHandle >= 0 && iHandle < m_iMaxCount);
	if (iHandle < 0 || iHandle >= m_iMaxCount) {
		return -1;
	}
	if (m_pSocketInfo[iHandle].iSocketFd < 0) {
		return -1;
	}
	//assert( m_pSocketInfo[iHandle].iSocketFd > 0 );
        int ret = sendto(m_pSocketInfo[iHandle].iSocketFd, pBuffer, iWriteSizes, 0,
			(struct sockaddr*) &RemoteAddr,
			sizeof(struct sockaddr_in));
        printf("udp send %d bytes\n", ret);
        return ret;
}
/*!
 *
 * @param iHandle
 * @param pBuffer
 * @param iWriteSizes
 * @return
 */
int CUdpSocket::WriteUDPSocketBack(int iHandle, const void *pBuffer,
		int iWriteSizes) {
	if (iHandle < 0 || iHandle >= m_iMaxCount) {
		return -1;
	}
	if (m_pSocketInfo[iHandle].iSocketFd < 0) {
		return -1;
	}
	//assert( iHandle >= 0 && iHandle < m_iMaxCount);
	//assert( m_pSocketInfo[iHandle].iSocketFd > 0 );
	return sendto(m_pSocketInfo[iHandle].iSocketFd, pBuffer, iWriteSizes, 0,
			(struct sockaddr*) &m_pSocketInfo[iHandle].UDP.clientaddr,
			sizeof(struct sockaddr_in));
}
/*!
 *
 * @param iHandle
 */
void CUdpSocket::CloseUDPSocket(int iHandle) {
	assert( iHandle >= 0 && iHandle < m_iMaxCount);
	assert( m_pSocketInfo[iHandle].iSocketFd > 0 );

	shutdown(m_pSocketInfo[iHandle].iSocketFd, SHUT_RDWR);
	close(m_pSocketInfo[iHandle].iSocketFd);
	m_pSocketInfo[iHandle].iSocketFd = -1;
	m_pSocketInfo[iHandle].socketStatus = SOCKET_NONE;
	m_pSocketInfo[iHandle].bUser = false;
}
/*!
 *
 *
 * @param iHandle
 * @return
 */
CUdpSocket::enSocketStatus CUdpSocket::GetSocketStatus(int iHandle) {
	//assert( iHandle >= 0 && iHandle < m_iMaxCount);
	if (iHandle < 0 || iHandle >= m_iMaxCount) {
		return SOCKET_NONE;
	}
	return m_pSocketInfo[iHandle].socketStatus;
}
/*!
 *
 * @param iHandle
 */
void CUdpSocket::AddListenEvent(int iHandle) {
	assert( iHandle >= 0 && iHandle < m_iMaxCount);
	assert( m_pSocketInfo[iHandle].iSocketFd > 0 );
	assert( m_epfd > 0 );
	struct epoll_event ev;
	bzero(&ev, sizeof(epoll_event));
	ev.data.fd = m_pSocketInfo[iHandle].iSocketFd;
	//设置要处理的事件类型
	ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
	//注册epoll事件
	epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_pSocketInfo[iHandle].iSocketFd, &ev);
}
/*!
 *
 * @param iHandle
 */
void CUdpSocket::DelListenEvent(int iHandle) {
	assert( iHandle >= 0 && iHandle < m_iMaxCount);
	assert( m_pSocketInfo[iHandle].iSocketFd > 0 );
	assert( m_epfd > 0 );
	epoll_ctl(m_epfd, EPOLL_CTL_DEL, m_pSocketInfo[iHandle].iSocketFd, NULL);
}
/*!
 *
 * @param imsec
 * @param iHandle
 * @return
 */
CUdpSocket::enWaitEventResult CUdpSocket::WaitListenEvent(int imsec,
		int &iHandle) {
	assert( m_epfd > 0 );
	int nfds = epoll_wait(m_epfd, m_pEvents, m_iMaxCount, imsec);//100ms
	if (nfds == 0) {
		//这是时间返回
		return WAITEVENT_TIMER;
	}
	iHandle = -1;
	int sockfd = -1;
	for (int e = 0; e < nfds; e++) {
		if (m_pEvents[e].events & EPOLLIN) {
			if ((sockfd = m_pEvents[e].data.fd) <= 0) {
				assert(0);
				continue;
			}
			for (int i = 0; i < m_iMaxCount; i++) {
				if (m_pSocketInfo[i].bUser == false) {
					continue;
				}
				if (sockfd == m_pSocketInfo[i].iSocketFd) {
					iHandle = i;
					return WAITEVENT_READ;
				}
			}
			//todo 不知道为什么会执行这个事件,不知道下面断言宏为什么会起作用!
			printf("WAITEVENT_ERROR sozkfd:%d\n",sockfd);
			//assert(0);
			return WAITEVENT_ERROR;
		} else if (m_pEvents[e].events & EPOLLERR) {
			//UDP 不去管他
			sockfd = m_pEvents[e].data.fd;
			for (int i = 0; i < m_iMaxCount; i++) {
				if (m_pSocketInfo[i].bUser == false) {
					continue;
				}
				if (sockfd == m_pSocketInfo[i].iSocketFd) {
					iHandle = i;
					return WAITEVENT_ERROR;
				}
			}
			assert(0);
		} else if (m_pEvents[e].events & EPOLLHUP) {
			//UDP 不去管他
			sockfd = m_pEvents[e].data.fd;
			for (int i = 0; i < m_iMaxCount; i++) {
				if (m_pSocketInfo[i].bUser == false) {
					continue;
				}
				if (sockfd == m_pSocketInfo[i].iSocketFd) {
					iHandle = i;
					return WAITEVENT_ERROR;
				}
			}

			assert(0);
		} else {
			assert(0);
		}
	}
	printf("nfds == %d\n", nfds);
	if (errno == EINTR) {
		return WAITEVENT_UNKOWN;
	}

	//assert(0);
	return WAITEVENT_ERROR;
}
//线程的运行实体
/*!
 *
 */
void CUdpSocket::ThreadProc() {
	ThreadRun();
}
/*!
 *
 * @return
 */
bool CUdpSocket::IsRun() {
	return m_commonThread.isRun();
}

bool CUdpSocket::JoinGroup(int socketHandle, char* groupIP, unsigned int port)
{
    int ret;

    mreq.imr_multiaddr.s_addr = inet_addr(groupIP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    ret = setsockopt(m_pSocketInfo[socketHandle].iSocketFd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
    if(ret == 0)
        return true;
    else
    {
        printf("setsockopt failed, errno=%d\n", errno);
        return false;
    }
}

bool CUdpSocket::LeaveGroup(int socketHandle)
{
    if(setsockopt (m_pSocketInfo[socketHandle].iSocketFd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char FAR *)&mreq, sizeof(mreq)) < 0)
        return FALSE;

//        m_SendSocket.Close();		// Close sending socket
    CloseUDPSocket(socketHandle);
    return true;
}

int CUdpSocket::getSocket(int iHandle)
{
    assert( iHandle >= 0 && iHandle < m_iMaxCount);
    assert( m_pSocketInfo[iHandle].iSocketFd > 0 );

    return m_pSocketInfo[iHandle].iSocketFd;
}


