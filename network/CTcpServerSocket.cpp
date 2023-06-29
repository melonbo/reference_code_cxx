/*
 * CCommonTimer.cpp
 *
 *  Created on: 2010-9-29
 *      Author: hbs
 */
#include "CCommonThread.h"
#include "CTcpServerSocket.h"
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
//#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
/*!
 *
 */
CTcpServerSocket::CTcpServerSocket() :
		m_commonThread(this) {

	pthread_mutex_init(&m_mutexLock, NULL);
	m_epfd = -1;
	memset(&m_serverSocketInfo, 0, sizeof(tagServerSocketInfo));
	m_pSocketInfo = NULL;
	m_pEvents = NULL;
}

CTcpServerSocket::~CTcpServerSocket() {

	m_pSocketInfo = NULL;
	m_pEvents = NULL;
	pthread_mutex_destroy(&m_mutexLock);
}
void CTcpServerSocket::ConfigSocketParam(int iMaxCount,
		CTcpServerSocket::tagClientSocketInfo *pSocketInfo,
		struct epoll_event *pEvents) {
	assert( pSocketInfo != NULL);
	assert( pEvents != NULL);
	m_pSocketInfo = pSocketInfo;
	m_pEvents = pEvents;
	m_serverSocketInfo.iMaxCount = iMaxCount;
	assert(m_serverSocketInfo.iMaxCount > 0);
}
void CTcpServerSocket::SetTCPBufSizes(int iHandle, int iSndSizes,
		int iRevSizes) {
	assert( iHandle >= 0 && iHandle < m_serverSocketInfo.iMaxCount);
	int iSockFd = m_pSocketInfo[iHandle].iSocketFd;
	int sndbuf = 0;
	int rcvbuf = 0;
	sndbuf = iSndSizes;
	rcvbuf = iRevSizes;
	setsockopt(iSockFd, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf));
	setsockopt(iSockFd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));
}
//void CTcpServerSocket::setnonblocking(int sock) {
//	int opts;
//	opts = fcntl(sock, F_GETFL);
//	if (opts < 0) {
//		perror("fcntl(sock,GETFL)");
//
//		assert(0);
//
//		exit(1);
//	}
//	opts = opts | O_NONBLOCK;
//	if (fcntl(sock, F_SETFL, opts) < 0) {
//		perror("fcntl(sock,SETFL,opts)");
//		assert(0);
//		exit(1);
//	}
//
//}
void CTcpServerSocket::StartSocket(int iLocalPort, int iTimerCount) {

	for (int i = 0; i < m_serverSocketInfo.iMaxCount; i++) {
		bzero(&m_pSocketInfo[i], sizeof(tagClientSocketInfo));
		bzero(&m_pEvents[i], sizeof(epoll_event));
	}
	m_epfd = epoll_create(m_serverSocketInfo.iMaxCount);

	assert( m_epfd > 0);
	m_serverSocketInfo.iTimerCount = iTimerCount;
	m_serverSocketInfo.iLocalPort = iLocalPort;
	CreateTCPSocket();
	m_commonThread.start();

}
void CTcpServerSocket::StopSocket() {

	m_commonThread.stop();
	CloseTCPSocket();
	if (m_epfd > 0) {
		close(m_epfd);
		m_epfd = -1;
	}
}
bool CTcpServerSocket::SendSessionData(int iHandle,
		const unsigned char *pBuffer, int iReadSizes) {
	if (m_commonThread.isRun() == false) {
		return false;
	}

	int iCnt = -1;
	pthread_mutex_lock(&m_mutexLock);
	iCnt = WriteTCPSocket(iHandle, pBuffer, iReadSizes);
	pthread_mutex_unlock(&m_mutexLock);
	return (iCnt > 0);
}
bool CTcpServerSocket::SendData(int iHandle, const unsigned char *pBuffer,
		int iReadSizes) {
	if (m_commonThread.isRun() == false) {
		return false;
	}
	int iCnt = -1;
	iCnt = WriteTCPSocket(iHandle, pBuffer, iReadSizes);
	return (iCnt > 0);
}
int CTcpServerSocket::ReciveData(int iHandle, unsigned char *pBuffer,
		size_t __iReadBufSizes) {
	if (m_commonThread.isRun() == false) {
		return -1;
	}
	int iCnt = -1;
	iCnt = ReadTCPSocket(iHandle, pBuffer, __iReadBufSizes);
	return iCnt;
}

//!创建TCP套接字
int CTcpServerSocket::CreateTCPSocket() {
	m_serverSocketInfo.ilistenfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(m_serverSocketInfo.ilistenfd > 0);
	struct sockaddr_in serveraddr;
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY );
	serveraddr.sin_port = htons(m_serverSocketInfo.iLocalPort);
	//int option = 1;
	//assert(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == 0);
	if (bind(m_serverSocketInfo.ilistenfd, (sockaddr *) &serveraddr,
			sizeof(serveraddr))) {
		printf(
				"CTcpServerSocket::CreateTCPSocket Server Bind Port : %d Failed!\n",
				m_serverSocketInfo.iLocalPort);
		assert(0);
	}
	//setnonblocking(listenfd);
	//SetTCPSocketBlock(listenfd, true);
	assert(m_serverSocketInfo.iMaxCount > 0);
	int option = 1;
	if (setsockopt(m_serverSocketInfo.ilistenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option))
			< 0) {
                printf("setsockopt, errno=%d\n", errno);
	}
	struct epoll_event ev;
	memset(&ev, 0, sizeof(epoll_event));
	ev.data.fd = m_serverSocketInfo.ilistenfd;
	//设置用于注测的读写操作事件
	ev.events = EPOLLIN;	//用这个必须读干净EPOLLET
	//注册ev
	epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_serverSocketInfo.ilistenfd, &ev);
	assert(
			listen(m_serverSocketInfo.ilistenfd,m_serverSocketInfo.iMaxCount) == 0);
	return 0;
}
void CTcpServerSocket::SetTCPSocketBlock(int iHandle, bool bBlock) {

	//todo 这里block写错了
	assert( iHandle >= 0 && iHandle < m_serverSocketInfo.iMaxCount);
	int iSockFd = m_pSocketInfo[iHandle].iSocketFd;
	assert(iSockFd > 0);
	int opts = -1;
	opts = fcntl(iSockFd, F_GETFL);
	assert(opts >= 0);
	if (bBlock == true) {
		opts &= ~O_NONBLOCK;
	} else {
		opts |= O_NONBLOCK;
	}
	assert(fcntl(iSockFd, F_SETFL, opts) == 0);
}
int CTcpServerSocket::ReadTCPSocket(int iHandle, unsigned char *pBuffer,
		int iReadSizes) {
	if (iHandle < 0 || iHandle >= m_serverSocketInfo.iMaxCount) {
		return 0;
	}
	//assert( iHandle >= 0 && iHandle < m_serverSocketInfo.iMaxCount );
	assert( iReadSizes > 0);
	assert( pBuffer != NULL);
	int sockfd = m_pSocketInfo[iHandle].iSocketFd;
	assert(sockfd > 0);
	int iCnt = -1;
	try {
		iCnt = read(sockfd, pBuffer, iReadSizes);
	} catch (...) {
		assert(0);
	}
	return iCnt;
}
int CTcpServerSocket::WriteTCPSocket(int iHandle, const unsigned char *pBuffer,
		int iWriteSizes) {
	if (iHandle < 0 || iHandle >= m_serverSocketInfo.iMaxCount) {
		return 0;
	}
	//assert( iHandle >= 0 && iHandle < m_serverSocketInfo.iMaxCount );
	assert( iWriteSizes > 0);
	assert( pBuffer != NULL);
	int sockfd = m_pSocketInfo[iHandle].iSocketFd;
	if (sockfd <= 0) {
		return 0;
	}
	int iCnt = write(sockfd, pBuffer, iWriteSizes);
	return iCnt;
}
void CTcpServerSocket::CloseTCPSocket() {
	pthread_mutex_lock(&m_mutexLock);
	for (int a = 0; a < m_serverSocketInfo.iMaxCount; a++) {
		if (m_pSocketInfo[a].bClose == true) {
			//也许网络句柄已经关闭么所以需要判断一下
			if (m_pSocketInfo[a].iSocketFd >= 0) {
				epoll_ctl(m_epfd, EPOLL_CTL_DEL, m_pSocketInfo[a].iSocketFd,
						NULL);
				close(m_pSocketInfo[a].iSocketFd);
				m_pSocketInfo[a].iSocketFd = -1;
				m_pSocketInfo[a].bClose = false;
			}
		}
	}
	pthread_mutex_unlock(&m_mutexLock);
	if (m_serverSocketInfo.ilistenfd > 0) {
		close(m_serverSocketInfo.ilistenfd);
		m_serverSocketInfo.ilistenfd = -1;
	}

}
bool CTcpServerSocket::FindSession(int iHandle) {
	if (m_commonThread.isRun() == false) {
		return false;
	}
	pthread_mutex_lock(&m_mutexLock);
	assert(iHandle < m_serverSocketInfo.iMaxCount);
	if (m_pSocketInfo[iHandle].iSocketFd > 0) {
		pthread_mutex_unlock(&m_mutexLock);
		return true;
	}
	pthread_mutex_unlock(&m_mutexLock);
	return false;
}
const char *CTcpServerSocket::GetSessionAddr(int iHandle) {
	assert(iHandle < m_serverSocketInfo.iMaxCount);
	return m_pSocketInfo[iHandle].szCliIpaddr;
}
void CTcpServerSocket::DeleteSession(int iHandle) {
	if (m_commonThread.isRun() == false) {
		return;
	}
	pthread_mutex_lock(&m_mutexLock);
	assert(iHandle < m_serverSocketInfo.iMaxCount);
	assert(m_pSocketInfo[iHandle].iSocketFd > 0);
	if (m_pSocketInfo[iHandle].iSocketFd > 0) {
		m_pSocketInfo[iHandle].bClose = true;
	}
	pthread_mutex_unlock(&m_mutexLock);
}

CTcpServerSocket::enWaitEventResult CTcpServerSocket::WaitListenEvent(int imsec,
		int &iHandle, struct epoll_event *pEvent) {

	assert( m_epfd > 0);
	assert( pEvent != NULL);
	int nfds = epoll_wait(m_epfd, pEvent, m_serverSocketInfo.iMaxCount, imsec);
	if (nfds == 0) {
		//这是时间返回
		return WAITEVENT_TIMER;
	}
	iHandle = -1;
	int sockfd = -1;

	for (int e = 0; e < nfds; e++) {
		sockfd = pEvent[e].data.fd;

		if (pEvent[e].events & EPOLLIN) {
			if (sockfd <= 0) {
				continue;
			}
			if (sockfd == m_serverSocketInfo.ilistenfd) {
				return WAITEVENT_ACCEPT;
			}
			for (int i = 0; i < m_serverSocketInfo.iMaxCount; i++) {
				if (sockfd == m_pSocketInfo[i].iSocketFd) {
					iHandle = i;
					return WAITEVENT_READ;
				}
			}
			assert(0);
		} else if (pEvent[e].events & EPOLLERR) {
			//UDP 不去管他
			sockfd = pEvent[e].data.fd;
			for (int i = 0; i < m_serverSocketInfo.iMaxCount; i++) {
				if (sockfd == m_pSocketInfo[i].iSocketFd) {
					iHandle = i;
					return WAITEVENT_ERROR;
				}
			}
			assert(0);
		} else if (pEvent[e].events & EPOLLHUP) {
			//UDP 不去管他
			sockfd = pEvent[e].data.fd;
			for (int i = 0; i < m_serverSocketInfo.iMaxCount; i++) {
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
	assert(0);
	return WAITEVENT_ERROR;
}
//线程的运行实体
void CTcpServerSocket::ThreadProc() {
	int iHandle = -1;
	int connfd = -1;
	struct sockaddr_in clientaddr;
	struct epoll_event ev;
	bzero(&ev, sizeof(epoll_event));
	socklen_t clilen = 0;
	bzero(&clientaddr, sizeof(sockaddr_in));
	clientaddr.sin_family = AF_INET;
	clilen = sizeof(sockaddr_in);
#define SVRSOCKET_BUFFER_LIMTE 1024 * 64
	unsigned char *line = new unsigned char[SVRSOCKET_BUFFER_LIMTE];
	memset(line, 0, sizeof(SVRSOCKET_BUFFER_LIMTE));
	assert( m_serverSocketInfo.ilistenfd > 0);

	while (m_serverSocketInfo.bClose == false && m_commonThread.isRun()) {
		//printf("WaitListenEvent  ilistenfd = %d\n",
		//		m_serverSocketInfo.ilistenfd);
		switch (WaitListenEvent(m_serverSocketInfo.iTimerCount, iHandle,
				m_pEvents)) {

		case WAITEVENT_ACCEPT: {
			connfd = accept(m_serverSocketInfo.ilistenfd,
					(sockaddr *) &clientaddr, &clilen);
			int iLeftBlank = -1;

			if (connfd < 0 || clilen <= 0) {
				perror("accept");
				break;
			}
			pthread_mutex_lock(&m_mutexLock);
			for (int a = 0; a < m_serverSocketInfo.iMaxCount; a++) {
				if (m_pSocketInfo[a].iSocketFd <= 0) {
					//RPU_ASSERT(SockPara.closeflage[a] == false);
					//看看能不能发生，感觉有发生的时候
					m_pSocketInfo[a].bClose = false;
					m_pSocketInfo[a].iSocketFd = connfd;
					iLeftBlank = a;
					break;
				}
			}
			pthread_mutex_unlock(&m_mutexLock);
			if (iLeftBlank < 0) {
				close(connfd);
				continue;
			}
			//设置用于读操作的文件描述符
			ev.data.fd = connfd;
			//设置用于注测的读写操作事件
			ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;		//用这个必须读干净EPOLLET
			//注册ev
			assert(epoll_ctl(m_epfd, EPOLL_CTL_ADD, connfd, &ev) == 0);
			//SetTCPSocketBlock(connfd, false);
			printf("CTcpServerSocket accept a connection from client %s\n",
					inet_ntoa(clientaddr.sin_addr));
			strncpy(m_pSocketInfo[iLeftBlank].szCliIpaddr,
					inet_ntoa(clientaddr.sin_addr),
					sizeof(m_pSocketInfo[iLeftBlank].szCliIpaddr));
			OnTcpEvent(iLeftBlank, ON_ACCEPT);
			break;

		}
		case WAITEVENT_READ: {

			assert(iHandle >= 0 && iHandle < m_serverSocketInfo.iMaxCount);
			int iReadCnt = ReadTCPSocket(iHandle, line,
					SVRSOCKET_BUFFER_LIMTE - 1);
//                        printf("WAITEVENT_READ = %d, handle = %d\n", iReadCnt, iHandle);
			if (iReadCnt <= 0) {
				//网络出现错误
                                perror("WAITEVENT_READ < 0");
                                OnDataReady(m_pSocketInfo[iHandle].iSocketFd, NULL, iReadCnt);
				epoll_ctl(m_epfd, EPOLL_CTL_DEL,
						m_pSocketInfo[iHandle].iSocketFd, NULL);
				close(m_pSocketInfo[iHandle].iSocketFd);
				m_pSocketInfo[iHandle].iSocketFd = -1;
				m_pEvents[iHandle].data.fd = -1;
			} else {
				//正常读取数据
				assert( iReadCnt < SVRSOCKET_BUFFER_LIMTE);
				line[iReadCnt] = '\0';
                                printf("iReadCnt == %d\n%s", iReadCnt, line);
                                if (false == OnDataReady(m_pSocketInfo[iHandle].iSocketFd, line, iReadCnt))

				{
					printf("OnDataReady  == false\n");
					//网络出现错误
					epoll_ctl(m_epfd, EPOLL_CTL_DEL,
							m_pSocketInfo[iHandle].iSocketFd, NULL);
					close(m_pSocketInfo[iHandle].iSocketFd);
					m_pSocketInfo[iHandle].iSocketFd = -1;
					m_pEvents[iHandle].data.fd = -1;
				}
			}
			break;
		}
		case WAITEVENT_ERROR: {
			//网络出现错误
			printf("WAITEVENT_ERROR  == now\n");
			epoll_ctl(m_epfd, EPOLL_CTL_DEL, m_pSocketInfo[iHandle].iSocketFd,
					NULL);
			close(m_pSocketInfo[iHandle].iSocketFd);
			m_pSocketInfo[iHandle].iSocketFd = -1;
			m_pEvents[iHandle].data.fd = -1;
			OnTcpEvent(iHandle, ON_ERROR);
			break;
		}
		case WAITEVENT_TIMER: {
			OnTcpEvent(iHandle, ON_TIMER);
			break;
		}
		case WAITEVENT_UNKOWN:
			break;
		default:
			assert(0);
			break;

		}
		//printf("bClose  == now\n");
		pthread_mutex_lock(&m_mutexLock);
		for (int a = 0; a < m_serverSocketInfo.iMaxCount; a++) {
			if (m_pSocketInfo[a].bClose == true) {
				//也许网络句柄已经关闭么所以需要判断一下
				if (m_pSocketInfo[a].iSocketFd > 0) {
					epoll_ctl(m_epfd, EPOLL_CTL_DEL, m_pSocketInfo[a].iSocketFd,
							NULL);
					close(m_pSocketInfo[a].iSocketFd);
					m_pSocketInfo[a].iSocketFd = -1;
					m_pSocketInfo[a].bClose = false;
				}
			}
		}
		pthread_mutex_unlock(&m_mutexLock);
	}
	delete[] line;

}
void CTcpServerSocket::OnTcpEvent(int iHandle, int iEvent) {
	//printf("CTcpServerSocket::OnAcceptReady:%d\n",iHandle);
}
